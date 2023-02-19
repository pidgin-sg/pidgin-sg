#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <plugin.h>
#include <util.h>

#include "prefs.h"
#include "string.h"
#include "plugin.h"
#include "config.h"


#ifdef _WIN32
#define PATH_DELIMITER "\\"
#else
#define PATH_DELIMITER "/"
#endif


#define getpath(name) \
    "/plugins/core/" PLUGIN_ID "/" name

#define get_string_pref(name) \
    prefs_get_string(getpath(name))

#define get_bool_pref(name) \
    purple_prefs_get_bool(getpath(name))

#define get_int_pref(name) \
    purple_prefs_get_int(getpath(name))


char *
prefs_get_string(const char *path)
{
    const char *str = purple_prefs_get_string(path);
    return string_normalize(str, strlen(str));
}


char *
config_get_challenge_explanation(void)
{
    return get_string_pref(PREFS_CHALLENGE_EXPLANATION);
}


char *
config_get_question_list_path(void)
{
    return get_string_pref(PREFS_QUESTION_LIST_PATH);
}


char *
config_get_response_success(void)
{
    return get_string_pref(PREFS_RESPONSE_SUCCESS);
}


char *
config_get_response_mistake(void)
{
    return get_string_pref(PREFS_RESPONSE_MISTAKE);
}


char *
config_get_response_failure(void)
{
    return get_string_pref(PREFS_RESPONSE_FAILURE);
}


bool
config_get_auto_add_buddy(void)
{
    return get_bool_pref(PREFS_AUTO_ADD_BUDDY);
}


int
config_get_min_questions(void)
{
    return get_int_pref(PREFS_MIN_QUESTIONS);
}


int
config_get_num_attempts(void)
{
    return get_int_pref(PREFS_NUM_ATTEMPTS);
}


char *
config_get_used_questions_path(void)
{
    const char *pfx = purple_user_dir();
    const char sfx[] = PLUGIN_NAME PATH_DELIMITER "used_qa";
    char *path = malloc(strlen(pfx) + sizeof(sfx) + 2);
    sprintf(path, "%s" PATH_DELIMITER "%s", pfx, sfx);
    return path;
}
