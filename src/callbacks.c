#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include <conversation.h>
#include <connection.h>
#include <account.h>
#include <xmlnode.h>
#include <plugin.h>
#include <blist.h>
#include <util.h>

#include "pending-list.h"
#include "privacy.h"
#include "string.h"
#include "config.h"
#include "plugin.h"
#include "misc.h"
#include "qa.h"

#include "callbacks.h"


#define AUTH_PROMT 0
#define AUTH_GRANT 1
#define AUTH_DENY -1

#define PERM_UNDEF 0
#define PERM_GRANT 1
#define PERM_DENY -1


#define stanza_is_message(node) \
    (strcmp((node)->name, "message") == 0)

#define plugin_is_enabled(acc) \
    purple_account_get_bool((acc), PLUGIN_ID "-enabled", TRUE)

#define answer_is_correct(msg, ctx) \
    (purple_utf8_strcasecmp((ctx)->qa_pending->answer, \
        purple_markup_strip_html(msg)) == 0)


struct sender_ctx {
    struct qa_record *qa_pending;
    int attempts_left;
};


static int
check_permissions(PurpleAccount *account, const char *sender)
{
    if (purple_find_buddy(account, sender))
        return PERM_GRANT;

    int status = privacy_check(account, sender);
    if (status == PRIVACY_ALLOW)
        return PERM_GRANT;

    if (status == PRIVACY_DENY)
        return PERM_DENY;

    return PERM_UNDEF;
}


int
account_authorization_requested(PurpleAccount *account, const char *sender)
{
    if (!plugin_is_enabled(account))
        return AUTH_PROMT;

    if (check_permissions(account, sender) == PERM_GRANT)
        return AUTH_PROMT;

    return AUTH_DENY;
}


static bool
send_message(PurpleAccount *account, const char *rcpt, const char *msg)
{
    PurplePluginProtocolInfo *info;
    PurpleConnection *gc = purple_account_get_connection(account);

    if (gc && gc->prpl) {
        info = PURPLE_PLUGIN_PROTOCOL_INFO(gc->prpl);
        if (info && info->send_im) {
            info->send_im(gc, rcpt, msg, PURPLE_MESSAGE_AUTO_RESP);
            return true;
        }
    }
    return false;
}


static bool
send_composite_message(PurpleAccount *account, const char *rcpt,
    const char *msg_prefix, const char *msg_suffix)
{
    char *msg = string_concat(msg_prefix, msg_suffix);
    bool sent = send_message(account, rcpt, msg);
    free(msg);
    return sent;
}


static bool
add_buddy(PurpleAccount *account, const char *sender)
{
    PurpleBuddy *buddy = purple_buddy_new(account, sender, NULL);
    PurpleGroup *group = purple_group_new(PLUGIN_ID);
    if (buddy && group) {
        char *alias = misc_normalize_username(account, sender);
        purple_blist_add_buddy(buddy, NULL, group, NULL);
        purple_blist_alias_buddy(buddy, alias);
        free(alias);
        return true;
    }
    return false;
}


static void
start_challenge(PurpleAccount *account, const char *sender)
{
    struct qa_record *qa = qa_next();
    struct sender_ctx *ctx = malloc(sizeof(*ctx));
    char *msg_prefix = config_get_challenge_explanation();
    send_composite_message(account, sender, msg_prefix, qa->question);
    pending_list_insert(account, sender, ctx);
    ctx->attempts_left = config_get_num_attempts();
    ctx->qa_pending = qa;
    free(msg_prefix);
}


static void
process_answer_success(PurpleAccount *account, const char *sender,
    struct sender_ctx *ctx)
{
    char *msg = config_get_response_success();
    send_message(account, sender, msg);
    if (config_get_auto_add_buddy())
        add_buddy(account, sender);
    privacy_allow(account, sender);
    qa_remove(ctx->qa_pending);
    free(ctx);
    free(msg);
}


static void
process_answer_mistake(PurpleAccount *account, const char *sender,
    struct sender_ctx *ctx)
{
    struct qa_record *qa = qa_next();
    char *msg_prefix = config_get_response_mistake();
    send_composite_message(account, sender, msg_prefix, qa->question);
    pending_list_insert(account, sender, ctx);
    ctx->qa_pending = qa;
    free(msg_prefix);
}


static void
process_answer_failure(PurpleAccount *account, const char *sender,
    struct sender_ctx *ctx)
{
    char *msg = config_get_response_failure();
    send_message(account, sender, msg);
    privacy_deny(account, sender);
    free(ctx);
    free(msg);
}


gboolean
receiving_im_msg(PurpleAccount *account, char **sender, char **message,
    PurpleConversation *conv, PurpleMessageFlags *flags)
{
    pending_list_expire();

    if (!plugin_is_enabled(account))
        return FALSE;

    if (!sender || !purple_account_get_connection(account))
        return FALSE;

    int perm = check_permissions(account, *sender);
    if (perm != PERM_UNDEF)
        return perm == PERM_DENY;

    struct sender_ctx *ctx;
    if (!pending_list_remove(account, *sender, (void **)&ctx)) {
        start_challenge(account, *sender);
        return TRUE;
    }

    if (answer_is_correct(*message, ctx))
        process_answer_success(account, *sender, ctx);
    else if (--ctx->attempts_left > 0)
        process_answer_mistake(account, *sender, ctx);
    else
        process_answer_failure(account, *sender, ctx);

    return TRUE;
}


static void
discard_stanza(xmlnode **stanza)
{
    xmlnode_free(*stanza);
    *stanza = NULL;
}


static void
process_error_message(PurpleAccount *account, xmlnode **stanza)
{
    const char *sender = xmlnode_get_attrib(*stanza, "from");
    if (!sender)
        return;

    int perm = check_permissions(account, sender);
    if (perm == PERM_GRANT)
        return;

    discard_stanza(stanza);
}


static void
process_otr_message(PurpleAccount *account, xmlnode **stanza)
{
    const char *sender = xmlnode_get_attrib(*stanza, "from");
    if (!sender)
        return;

    int perm = check_permissions(account, sender);
    if (perm == PERM_GRANT)
        return;

    if (perm == PERM_UNDEF)
        if (!pending_list_find(account, sender))
            start_challenge(account, sender);

    discard_stanza(stanza);
}


static bool
stanza_is_error_message(xmlnode *stanza)
{
    const char *type = xmlnode_get_attrib(stanza, "type");
    return type && strcmp(type, "error") == 0;
}


static bool
stanza_is_otr_message(xmlnode *stanza)
{
    const char *msg;
    for (xmlnode *child = stanza->child; child; child = child->next) {
        if (strcmp(child->name, "body") == 0 && (msg = xmlnode_get_data(child)))
            return strncasecmp(msg, "?OTR", 4) == 0;
    }
    return false;
}


void
receiving_xmlnode(PurpleConnection *gc, xmlnode **stanza)
{
    PurpleAccount *account = gc->account;

    if (!plugin_is_enabled(account))
        return;

    if (!stanza_is_message(*stanza))
        return;

    if (stanza_is_otr_message(*stanza)) {
        process_otr_message(account, stanza);
        return;
    }

    if (stanza_is_error_message(*stanza))
        process_error_message(account, stanza);
}


void
buddy_node_added(PurpleBlistNode *node)
{
    if (PURPLE_BLIST_NODE_IS_BUDDY(node)) {
        PurpleBuddy *buddy = PURPLE_BUDDY(node);
        if (buddy->account && buddy->name)
            privacy_reset(buddy->account, buddy->name);
    }
}


void
sending_im_msg(PurpleAccount *account, const char *receiver, char **message)
{
    if (!purple_find_buddy(account, receiver))
        privacy_allow(account, receiver);
}
