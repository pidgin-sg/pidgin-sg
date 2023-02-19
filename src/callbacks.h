#ifndef SPAMGUARD_CALLBACKS
#define SPAMGUARD_CALLBACKS


#include <conversation.h>
#include <connection.h>
#include <account.h>
#include <xmlnode.h>
#include <blist.h>


int account_authorization_requested(PurpleAccount *account, const char *user);

void buddy_node_added(PurpleBlistNode *node);

void receiving_xmlnode(PurpleConnection *gc, xmlnode **stanza);

gboolean receiving_im_msg(PurpleAccount *account, char **sender,
    char **message, PurpleConversation *conv, PurpleMessageFlags *flags);

void sending_im_msg(PurpleAccount *account, const char *receiver,
    char **message);

#endif
