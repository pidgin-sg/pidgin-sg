#include <string.h>
#include <account.h>

#include "misc.h"


#ifdef _WIN32

static char *
strndup(const char *str, size_t maxlen)
{
    size_t len = strlen(str);
    size_t total_len = (len > maxlen) ? maxlen : len;
    char *dup = malloc(total_len + 1);
    memcpy(dup, str, total_len);
    dup[total_len] = '\0';
    return dup;
}

#endif


char *
misc_normalize_username(PurpleAccount *account, const char *name)
{
    char *delim;
    if (strcmp(account->protocol_id, "prpl-jabber") == 0) {
        if (delim = strchr(name, '/'))
            return strndup(name, delim - name);
    }
    return strdup(name);
}
