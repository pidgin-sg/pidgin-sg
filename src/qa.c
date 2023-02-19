#include <stdbool.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stddef.h>
#include <stdio.h>

#include <notify.h>

#include "config.h"
#include "plugin.h"
#include "string.h"
#include "crc32.h"
#include "qa.h"


#define LINE_MAXLEN 4096
#define QA_LIST_INITLEN 64

#define memdouble(ptr, len) \
    double_memory_space(ptr, (len) * sizeof(*(ptr)))


struct qa_list {
    struct qa_record **list;
    size_t len;
    size_t n;
    size_t i;
    int *map;
};


static struct qa_list qa_list;
static char *used_records_log;


static void *
double_memory_space(void *ptr, size_t size)
{
    void *res = realloc(ptr, size * 2);
    memset(res + size, 0, size);
    return res;
}


static struct qa_record *
allocate_record(void)
{
    if (qa_list.n == qa_list.len) {
        qa_list.map = memdouble(qa_list.map, qa_list.len);
        qa_list.list = memdouble(qa_list.list, qa_list.len);
        qa_list.len *= 2;
    }
    struct qa_record *rec = malloc(sizeof(*rec));
    qa_list.map[qa_list.n] = rec->id = qa_list.n;
    qa_list.list[qa_list.n++] = rec;
    return rec;
}


static void
swap_records(size_t idx1, size_t idx2)
{
    struct qa_record *rec1 = qa_list.list[idx1];
    struct qa_record *rec2 = qa_list.list[idx2];
    qa_list.map[rec1->id] = idx2;
    qa_list.map[rec2->id] = idx1;
    qa_list.list[idx1] = rec2;
    qa_list.list[idx2] = rec1;
}


static void
shuffle_records(void)
{
    for (size_t i = qa_list.n; i-- > 1;)
        swap_records(i, rand() % i);
}


static void
remove_record(size_t index)
{
     if (index < qa_list.n)
         swap_records(index, --qa_list.n);
}


static void
reset_records(void)
{
    size_t index = 0;
    for (struct qa_record **rec = qa_list.list; *rec; rec++) {
        while ((*rec)->id != index)
            swap_records((*rec)->id, index);
        index++;
    }
    qa_list.n = index;
}


void
qa_remove(struct qa_record *rec)
{
    FILE *file;
    if (qa_list.n <= config_get_min_questions() + 1) {
        unlink(used_records_log);
        reset_records();
        return;
    }
    remove_record(qa_list.map[rec->id]);
    if (file = fopen(used_records_log, "ab")) {
        fwrite(&rec->id, sizeof(rec->id), 1, file);
        fclose(file);
    }
}


struct qa_record *
qa_next(void)
{
    qa_list.i = (qa_list.i + 1) % qa_list.n;
    return qa_list.list[qa_list.i];
}


static bool
parse_question(char *line, const char **question, const char **answer)
{
    char *delim, *newline;
    if ((delim = strchr(line, '|')) && (newline = strchr(delim, '\n'))) {
        size_t answer_len = newline - delim - (newline[-1] == '\r') - 1;
        size_t question_len = delim - line;
        *answer = string_normalize(delim + 1, answer_len);
        *question = string_normalize(line, question_len);
        return true;
    }
    return false;
}


static bool
load_questions(const char *path, long *hashsum)
{
    FILE *file = fopen(path, "rb");
    if (file) {
        char line[LINE_MAXLEN];
        while (fgets(line, sizeof(line), file)) {
            const char *question, *answer;
            if (parse_question(line, &question, &answer)) {
                struct qa_record *rec = allocate_record();
                rec->question = question;
                rec->answer = answer;
            }
            crc32_calculate(line, hashsum);
        }
        int done = feof(file);
        fclose(file);
        return done;
    }
    return false;
}


static int
discard_used_records(FILE *file)
{
    int id, n;
    while ((n = fread(&id, sizeof(id), 1, file)) == 1)
        remove_record(qa_list.map[id]);
    return n == 0 && feof(file);
}


static int
verify_hashsum(FILE *file, long hashsum)
{
    long hashsum_current;
    if (fread(&hashsum_current, sizeof(hashsum_current), 1, file) == 1)
        return hashsum_current == hashsum;
    return (feof(file)) ? 0 : -1;
}


static int
write_hashsum(const char *path, long hashsum)
{
    FILE *file = fopen(path, "wb");
    if (file) {
        if (fwrite(&hashsum, sizeof(hashsum), 1, file) == 1) {
            fclose(file);
            return 1;
        };
        fclose(file);
    }
    return 0;
}


static int
process_used_records(const char *path, long hashsum)
{
    FILE *file = fopen(path, "rb");
    if (file) {
        int rc = verify_hashsum(file, hashsum);
        if (rc == 1) {
            rc = discard_used_records(file);
            fclose(file);
            return rc;
        }
        fclose(file);
        if (rc == -1)
            return 0;
    }
    return write_hashsum(path, hashsum);
}


static bool
process_used_records_notify(long hashsum)
{
    char *path = config_get_used_questions_path();
    bool done = process_used_records(path, hashsum);
    if (!done)
        purple_notify_info(NULL, "Pidgin " PLUGIN_NAME,
            "Unable to process the file of used Q&A", path);
    free(path);
    return done;
}


static bool
load_questions_notify(long *hashsum)
{
    char *path = config_get_question_list_path();
    bool done = load_questions(path, hashsum);
    if (!done)
        purple_notify_info(NULL, "Pidgin " PLUGIN_NAME,
            "Unable to load the Q&A list", path);
    free(path);
    return done;
}


static bool
build_qa_list(void)
{
    long hashsum = 0;
    if (load_questions_notify(&hashsum)) {
        if (process_used_records_notify(hashsum)) {
            shuffle_records();
            return true;
        }
    }
    return false;
}


bool
qa_open(void)
{
    qa_list.n = 0;
    qa_list.len = QA_LIST_INITLEN;
    qa_list.list = calloc(qa_list.len, sizeof(*qa_list.list));
    qa_list.map = calloc(qa_list.len, sizeof(*qa_list.map));
    used_records_log = config_get_used_questions_path();
    return build_qa_list();
}


void
qa_close(void)
{
    for (struct qa_record **rec = qa_list.list; *rec; rec++) {
        free((char *)(*rec)->question);
        free((char *)(*rec)->answer);
        free(*rec);
    }
    free(used_records_log);
    free(qa_list.list);
    free(qa_list.map);
}
