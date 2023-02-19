#ifndef SPAMGUARD_STRING
#define STOPSMAP_STRING

#include <stddef.h>

char *string_normalize(const char *str, size_t len);
char *string_concat(const char *s1, const char *s2);
void string_lower(const char *str, char *result);

#endif
