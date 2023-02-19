#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <glib.h>

#include <account.h>
#include <util.h>

#include "misc.h"
#include "string.h"
#include "privacy.h"


#define calculate_key_length(account, sender) \
    (strlen((account)->protocol_id) + \
     strlen((account)->username) + \
     strlen(sender) + 3)


GHashTable *storage;


static char *
normalize_username(PurpleAccount *account, const char *name)
{
    char *res = misc_normalize_username(account, name);
    string_lower(res, res);
    return res;
}


static char *
create_key(PurpleAccount *account, const char *sender)
{
    char *name = normalize_username(account, sender);
    char *key = malloc(calculate_key_length(account, name));
    sprintf(key, "%s|%s|%s", name, account->username, account->protocol_id);
    free(name);
    return key;
}


static int *
create_value(int num)
{
    int *val = malloc(sizeof(*val));
    *val = num;
    return val;
}


static void *
storage_lookup(PurpleAccount *account, const char *sender)
{
    char *key = create_key(account, sender);
    void *res = g_hash_table_lookup(storage, key);
    free(key);
    return res;
}


static void
storage_insert(PurpleAccount *account, const char *sender, int status)
{
    char *key = create_key(account, sender);
    int *val = create_value(status);
    g_hash_table_insert(storage, key, val);
}


static void
storage_remove(PurpleAccount *account, const char *sender)
{
    char *key = create_key(account, sender);
    g_hash_table_remove(storage, key);
    free(key);
}


bool
privacy_allow(PurpleAccount *account, const char *sender)
{
    storage_insert(account, sender, PRIVACY_ALLOW);
    return true;
}


bool
privacy_deny(PurpleAccount *account, const char *sender)
{
    storage_insert(account, sender, PRIVACY_DENY);
    return true;
}


bool
privacy_reset(PurpleAccount *account, const char *sender)
{
    storage_remove(account, sender);
    return true;
}


int
privacy_check(PurpleAccount *account, const char *sender)
{
    int *status = storage_lookup(account, sender);
    return (status) ? *status : PRIVACY_UNDEF;
}


void
privacy_open(void)
{
    storage = g_hash_table_new_full(g_str_hash, g_str_equal, free, free);
}


void
privacy_close(void)
{
    g_hash_table_destroy(storage);
    storage = NULL;
}
