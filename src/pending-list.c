#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include <account.h>


#define WAITING_TIME_MAX_SEC 600
#define PENDING_LIST_INITLEN 64


struct pending_record {
    int index;
    time_t wait_until;
    char *protocol_id;
    char *username;
    char *sender;
    void *data;
};


struct pending_list {
    struct pending_record **list;
    size_t len;
    size_t n;
};


static struct pending_list pending_list;


static void
free_record(struct pending_record *rec)
{
    free(rec->protocol_id);
    free(rec->username);
    free(rec->sender);
    free(rec);
}


static void
remove_record(struct pending_record *rec)
{
    pending_list.list[rec->index] = pending_list.list[--pending_list.n];
    pending_list.list[rec->index]->index = rec->index;
    free_record(rec);
}


static struct pending_record *
allocate_record(void)
{
    if (pending_list.n == pending_list.len)
        pending_list.list = realloc(pending_list.list,
            (pending_list.len *= 2) * sizeof(*pending_list.list));
    struct pending_record *rec = malloc(sizeof(*rec));
    pending_list.list[pending_list.n] = rec;
    rec->index = pending_list.n++;
    return rec;
}


static struct pending_record *
find_record(PurpleAccount *account, const char *sender)
{
    struct pending_record **rec = pending_list.list;
    struct pending_record **out = pending_list.n + rec;
    for (; rec < out; rec++) {
        if (strcmp((*rec)->sender, sender) == 0 &&
            strcmp((*rec)->username, account->username) == 0 &&
            strcmp((*rec)->protocol_id, account->protocol_id) == 0)
            return *rec;
    }
    return NULL;
}


bool
pending_list_find(PurpleAccount *account, const char *sender)
{
    return find_record(account, sender) && true;
}


bool
pending_list_insert(PurpleAccount *account, const char *sender, void *data)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    struct pending_record *rec = allocate_record();
    rec->wait_until = ts.tv_sec + WAITING_TIME_MAX_SEC;
    rec->protocol_id = strdup(account->protocol_id);
    rec->username = strdup(account->username);
    rec->sender = strdup(sender);
    rec->data = data;

    return true;
}


bool
pending_list_remove(PurpleAccount *account, const char *sender, void **data)
{
    struct pending_record *rec = find_record(account, sender);
    if (rec) {
        *data = rec->data;
        remove_record(rec);
        return true;
    }
    return false;
}


void
pending_list_expire(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);

    struct pending_record **rec = pending_list.list;
    struct pending_record **out = pending_list.n + rec;

    for (; rec < out; rec++)
        if ((*rec)->wait_until <= ts.tv_sec)
            remove_record(*rec);
}


void
pending_list_open(void)
{
    pending_list.len = PENDING_LIST_INITLEN;
    pending_list.list = malloc(sizeof(*pending_list.list) * pending_list.len);
}


void
pending_list_close(void)
{
    for (size_t i = 0; i < pending_list.n; i++) {
        struct pending_record *rec = pending_list.list[i];
        free(rec->data);
        free_record(rec);
    }
    pending_list.n = 0;
    free(pending_list.list);
}
