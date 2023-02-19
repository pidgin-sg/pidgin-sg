#ifndef SPAMGUARD_FRAME
#define SPAMGUARD_FRAME

#include <plugin.h>
#include <pluginpref.h>

PurplePluginPrefFrame *frame_create(PurplePlugin *plugin);
void frame_init(void);

#endif
