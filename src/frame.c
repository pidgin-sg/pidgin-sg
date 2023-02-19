#include <stdbool.h>

#include <plugin.h>
#include <pluginpref.h>

#include "prefs.h"
#include "plugin.h"


#define getpath(name) \
    "/plugins/core/" PLUGIN_ID "/" name


#define prefs_add_bool(name, b) \
    purple_prefs_add_bool(getpath(name), (b))

#define prefs_add_string(name, s) \
    purple_prefs_add_string(getpath(name), (s))

#define prefs_add_int(name, n) \
    purple_prefs_add_int(getpath(name), (n))


#define create_label(frame, text) \
    do { \
        PurplePluginPref *pref; \
        pref = purple_plugin_pref_new_with_label(text); \
        purple_plugin_pref_frame_add(frame, pref); \
    } while (false)


#define create_field(frame, name) \
    do { \
        PurplePluginPref *pref; \
        pref = purple_plugin_pref_new_with_name(getpath(name)); \
        purple_plugin_pref_frame_add(frame, pref); \
    } while (false)


#define create_entry(frame, name, text) \
    do { \
        PurplePluginPref *pref; \
        pref = purple_plugin_pref_new_with_name_and_label(getpath(name), text); \
        purple_plugin_pref_frame_add(frame, pref); \
    } while (false)


#define create_numeric_entry(frame, name, text, min, max) \
    do { \
        PurplePluginPref *pref; \
        pref = purple_plugin_pref_new_with_name_and_label(getpath(name), text); \
        purple_plugin_pref_set_bounds(pref, min, max); \
        purple_plugin_pref_frame_add(frame, pref); \
    } while (false)


PurplePluginPrefFrame *
frame_create(PurplePlugin *plugin)
{
    PurplePluginPrefFrame *frame = purple_plugin_pref_frame_new();

    create_entry(frame, PREFS_QUESTION_LIST_PATH,
        "The file path of a Q&A list:");

    create_entry(frame, PREFS_CHALLENGE_EXPLANATION,
        "The challenge explanation:");

    create_label(frame, "Define the responses");
    create_entry(frame, PREFS_RESPONSE_SUCCESS, "in case of success:");
    create_entry(frame, PREFS_RESPONSE_MISTAKE, "in case of mistake:");
    create_entry(frame, PREFS_RESPONSE_FAILURE, "in case of failure:");

    create_entry(frame, PREFS_AUTO_ADD_BUDDY,
        "In case of success add a sender to the buddy list");

    create_numeric_entry(frame, PREFS_NUM_ATTEMPTS,
        "Attempts to pass a challenge", 1, 100);

    create_numeric_entry(frame, PREFS_MIN_QUESTIONS,
        "Reset Q&A when records left", 1, 1000);

    create_label(frame,
        "Pidgin must be restarted to update the question list");

    return frame;
}


void
frame_init(void)
{
    purple_prefs_add_none("/plugins");
    purple_prefs_add_none("/plugins/core");
    purple_prefs_add_none("/plugins/core/" PLUGIN_ID);

    prefs_add_string(PREFS_CHALLENGE_EXPLANATION,
        "To start a conversation with me, please, answer the following question "
        "correctly:\\n\\n");

    prefs_add_string(PREFS_QUESTION_LIST_PATH, "qa.txt");

    prefs_add_string(PREFS_RESPONSE_SUCCESS,
        "Your answer has been accepted! From now I can receive your messages "
        "and authorization requests.");

    prefs_add_string(PREFS_RESPONSE_MISTAKE,
        "You made a mistake. Please, try again.\\n\\n");

    prefs_add_string(PREFS_RESPONSE_FAILURE,
        "You have been blocked for a while due to the wrong answers. However, "
        "you could ask me through other channels to add you to my contact list "
        "so that I can receive your messages.");

    prefs_add_int(PREFS_NUM_ATTEMPTS, 5);
    prefs_add_int(PREFS_MIN_QUESTIONS, 10);
    prefs_add_bool(PREFS_AUTO_ADD_BUDDY, FALSE);
}
