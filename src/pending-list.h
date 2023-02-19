#ifndef SPAMGUARD_PENDING_LIST
#define SPAMGUARD_PENDING_LIST

#include <stdbool.h>
#include <account.h>

void pending_list_open(void);
void pending_list_close(void);
void pending_list_expire(void);
bool pending_list_find(PurpleAccount *account, const char *sender);
bool pending_list_insert(PurpleAccount *account, const char *sender, void *data);
bool pending_list_remove(PurpleAccount *account, const char *sender,
    void **data);

#endif
