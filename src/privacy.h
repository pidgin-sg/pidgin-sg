#ifndef SPAMGUARD_PRIVACY
#define SPAMGUARD_PRIVACY

#include <stdbool.h>
#include <account.h>

#define PRIVACY_UNDEF 0
#define PRIVACY_ALLOW 1
#define PRIVACY_DENY -1


void privacy_open(void);
void privacy_close(void);

bool privacy_reset(PurpleAccount *account, const char *sender);
bool privacy_allow(PurpleAccount *account, const char *sender);
bool privacy_deny(PurpleAccount *account, const char *sender);
int privacy_check(PurpleAccount *account, const char *sender);

#endif
