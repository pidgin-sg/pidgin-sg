#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <ctype.h>

#include "string.h"


inline static char
get_escape_char(const char *p)
{
    if (*p == '\\') {
        switch (*(p + 1)) {
            case 'a': return '\a';
            case 'b': return '\b';
            case 'f': return '\f';
            case 'n': return '\n';
            case 'r': return '\r';
            case 't': return '\t';
            case 'v': return '\v';
            case '\\': return '\\';
        }
    }
    return '\0';
}


char *
string_normalize(const char *str, size_t len)
{
    const char *out = str + len, *ps = str;
    char *buf = malloc(len + 1), *pb = buf;
    for (; ps < out; ps++, pb++) {
        char esc = get_escape_char(ps);
        *pb = (esc) ? ps++, esc : *ps;
    }
    *pb = '\0';
    return buf;
}


char *
string_concat(const char *s1, const char *s2)
{
    size_t len1 = strlen(s1);
    size_t len2 = strlen(s2);
    char *str = malloc(len1 + len2 + 1);
    memcpy(str, s1, len1);
    memcpy(str + len1, s2, len2);
    str[len1 + len2] = '\0';
    return str;
}


void
string_lower(const char *str, char *result)
{
    const unsigned char *ps = str;
    unsigned char *pr = result;
    while (*ps)
        *pr++ = tolower(*ps++);
    *pr = '\0';
}
