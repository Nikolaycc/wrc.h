#ifndef WRC_H
#define WRC_H

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#define ARRAY_LEN(x) sizeof((x))/sizeof((x)[0])

#define MAX_TEXTFORMAT_BUFFERS 2048
#define MAX_TEXT_BUFFER_LENGTH 2048

void todo(FILE *, const char *, int);
char *wrc_format(const char *, ...);

#endif // WRC_H

#ifndef WRC_IMPL

void todo(FILE *f, const char *text, int code) {
    fprintf((f != NULL) ? f : stderr, text, 0);
    exit(code);
}

char *wrc_format(const char *text, ...) {
    static char buffers[MAX_TEXTFORMAT_BUFFERS][MAX_TEXT_BUFFER_LENGTH] = {0};
    static int index = 0;

    char *currentBuffer = buffers[index];
    memset(currentBuffer, 0, MAX_TEXT_BUFFER_LENGTH);

    va_list args;
    va_start(args, text);
    vsnprintf(currentBuffer, MAX_TEXT_BUFFER_LENGTH, text, args);
    va_end(args);

    index += 1;
    if (index >= MAX_TEXTFORMAT_BUFFERS)
        index = 0;

    return currentBuffer;
}

#endif // WRC_IMPL
