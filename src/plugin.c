#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <glib.h>

#include <plugin.h>
#include <version.h>
#include <accountopt.h>
#include <util.h>

#include "pending-list.h"
#include "callbacks.h"
#include "privacy.h"
#include "config.h"
#include "frame.h"
#include "qa.h"

#include "plugin.h"


#ifdef _WIN32

#include <direct.h>

#define PATH_DELIMITER "\\"
#define make_directory(dir) _mkdir(dir)

#else

#include <sys/stat.h>
#include <sys/types.h>

#define PATH_DELIMITER "/"
#define make_directory(dir) mkdir((dir), 0755)

#endif


#define setting_has_prefix(data, pfx) \
    g_str_has_prefix(purple_account_option_get_setting(data), (pfx))


static gboolean plugin_load(PurplePlugin *plugin);
static gboolean plugin_unload(PurplePlugin *plugin);


static PurplePluginUiInfo prefs_info = {
    frame_create,
};


static PurplePluginInfo info = {
    PURPLE_PLUGIN_MAGIC,
    PURPLE_MAJOR_VERSION,
    0,
    PURPLE_PLUGIN_STANDARD,                               // type
    NULL,                                                 // ui_requirement
    0,                                                    // flags
    NULL,                                                 // dependencies
    PURPLE_PRIORITY_DEFAULT,                              // priority
    PLUGIN_ID,                                            // id
    PLUGIN_NAME,                                          // name
    PLUGIN_VERSION,                                       // version
    NULL,                                                 // summary
    NULL,                                                 // description
    NULL,                                                 // author
    NULL,                                                 // homepage
    plugin_load,                                          // load
    plugin_unload,                                        // unload
    NULL,                                                 // destroy
    NULL,                                                 // ui_info
    NULL,                                                 // extra_info
    &prefs_info,                                          // prefs_info
    NULL
};


static void
plugin_enable(void)
{
    PurplePluginProtocolInfo *info;
    GList *proto = purple_plugins_get_protocols();
    for (; proto; proto = proto->next) {
        PurplePlugin *plugin = proto->data;
        if (plugin && (info = PURPLE_PLUGIN_PROTOCOL_INFO(plugin))) {
            PurpleAccountOption *option = purple_account_option_bool_new(
                "Enable " PLUGIN_NAME " for this account",
                PLUGIN_ID "-enabled", TRUE);
            info->protocol_options = g_list_append(
                info->protocol_options, option);
        }
    }
}


static void
plugin_connect(PurplePlugin *plugin)
{
    PurplePlugin *jabber = purple_find_prpl("prpl-jabber");
    if (jabber)
        purple_signal_connect(
            jabber, "jabber-receiving-xmlnode",
            plugin, PURPLE_CALLBACK(receiving_xmlnode), NULL);

    purple_signal_connect(
        purple_conversations_get_handle(), "receiving-im-msg",
        plugin, PURPLE_CALLBACK(receiving_im_msg), NULL);

    purple_signal_connect(
        purple_conversations_get_handle(), "sending-im-msg",
        plugin, PURPLE_CALLBACK(sending_im_msg), NULL);

    purple_signal_connect(
        purple_accounts_get_handle(), "account-authorization-requested",
        plugin, PURPLE_CALLBACK(account_authorization_requested), NULL);

    purple_signal_connect(
        purple_blist_get_handle(), "blist-node-added",
        plugin, PURPLE_CALLBACK(buddy_node_added), NULL);
}


static void
destroy_protocol_options(PurplePluginProtocolInfo *info, const char *prefix)
{
    GList *option_next;
    GList *option = info->protocol_options;
    for (; option; option = option_next) {
        option_next = g_list_next(option);
        if (setting_has_prefix(option->data, prefix)) {
            purple_account_option_destroy(option->data);
            info->protocol_options = g_list_remove(
                info->protocol_options, option->data);
        }
    }
}


static void
destroy_plugin_options(void)
{
    PurplePluginProtocolInfo *info;
    GList *proto = purple_plugins_get_protocols();
    for (; proto; proto = proto->next) {
        PurplePlugin *plugin = proto->data;
        if (plugin && (info = PURPLE_PLUGIN_PROTOCOL_INFO(plugin)))
            destroy_protocol_options(info, PLUGIN_ID "-");
    }
}


static gboolean
plugin_unload(PurplePlugin *plugin)
{
    purple_signals_disconnect_by_handle(plugin);
    destroy_plugin_options();
    pending_list_close();
    privacy_close();
    qa_close();
    return TRUE;
}


static gboolean
plugin_load(PurplePlugin *plugin)
{
    if (qa_open()) {
        privacy_open();
        pending_list_open();
        plugin_connect(plugin);
    }
    plugin_enable();
    return TRUE;
}


static void
create_local_directory(const char *name)
{
    const char *dir = purple_user_dir();
    char *path = malloc(strlen(dir) + sizeof(name) + 2);
    sprintf(path, "%s" PATH_DELIMITER "%s", dir, name);
    make_directory(path);
    free(path);
}


static void
plugin_init(PurplePlugin *plugin)
{
    info.summary = "A simple anti-spam plugin for Pidgin";
    info.homepage = "https://github.com/pidgin-sg/pidgin-sg";
    info.author = "<pidgin-sg@proton.me>";

    srand(time(NULL));
    create_local_directory(PLUGIN_NAME);
    frame_init();
}


PURPLE_INIT_PLUGIN(PLUGIN_ID, plugin_init, info)
