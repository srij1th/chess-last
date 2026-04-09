#include "utils.h"

#include <ctype.h>
#include <stdio.h>
#include <string.h>

void trim_newline(char *s) {
    size_t len;
    if (!s) return;
    len = strlen(s);
    if (len > 0 && s[len - 1] == '\n') {
        s[len - 1] = '\0';
    }
}

void read_line(char *buffer, int size) {
    if (!fgets(buffer, size, stdin)) {
        if (size > 0) buffer[0] = '\0';
        return;
    }
    trim_newline(buffer);
}

int equals_ignore_case(const char *a, const char *b) {
    while (*a && *b) {
        if (tolower((unsigned char)*a) != tolower((unsigned char)*b)) return 0;
        a++;
        b++;
    }
    return *a == '\0' && *b == '\0';
}
