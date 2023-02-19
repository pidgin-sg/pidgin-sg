#ifndef SPAMGUARD_QA
#define SPAMGUARD_QA

#include <stdbool.h>


struct qa_record {
    const char *question;
    const char *answer;
    int id;
};


bool qa_open(void);
void qa_close(void);

struct qa_record *qa_next(void);
void qa_remove(struct qa_record *rec);

#endif
