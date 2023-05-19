#ifndef _stringutil_h
#define _stringutil_h

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char* concat(const char* s1, const char* s2)
{
    const size_t len1 = strlen(s1);
    const size_t len2 = strlen(s2);
    char* result = (char*)malloc(len1 + len2 + 1);
    if (!result) return NULL;
    memcpy(result, s1, len1);
    memcpy(result + len1, s2, len2 + 1);
    return result;
}

char* concatThree(const char* s1, const char* s2, const char* s3)
{
    char* tmp = concat(s1, s2);
    char* res = concat(tmp, s3);
    free(tmp);
    return res;
}

char* intToString(int in) {
    char buffer[10];
    snprintf(buffer, 10, "%d", in);
    return _strdup(buffer);
}

#endif