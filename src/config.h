#ifndef SPAMGUARD_CONFIG
#define STOPSMAP_CONFIG

#include <stdbool.h>


char *config_get_challenge_explanation(void);
char *config_get_used_questions_path(void);
char *config_get_question_list_path(void);
char *config_get_response_success(void);
char *config_get_response_mistake(void);
char *config_get_response_failure(void);

bool config_get_auto_add_buddy(void);

int config_get_min_questions(void);
int config_get_num_attempts(void);

#endif
