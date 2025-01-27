#include "logger.h"
#include "screen.h"

#include <stdio.h>
#include <string.h>
#include <stdarg.h>

static uint8_t _filled_log;
static char _log[MAX_LOG][CHAR_LIMIT];

inline static void strcpy_log(char* dst, const char* src)
{
    uint8_t i = 0;
    for (i = 0; (src[i] != '\0') && (i < (CHAR_LIMIT-1)); i++)
        dst[i] = src[i];
    for(uint8_t j = i; j < (CHAR_LIMIT-1); j++)
        dst[j] = ' '; 
    dst[CHAR_LIMIT] = '\0'; 
}

inline static void copy_log(const char *src, char log[][CHAR_LIMIT], uint8_t pos)
{
    // Adding a new log when empty buffer
    if (pos < MAX_LOG)
        strcpy_log(log[pos],src);
    // Adding a new log when buffer is full
    else
    {
        for(uint8_t i = 1; i<MAX_LOG; i++)
            strcpy(log[i-1], log[i]);
        strcpy_log(log[MAX_LOG-1],src);
    }
}

void screen_log(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    char str[16] = {0}; 
    vsnprintf(str, 16, format, args);
    va_end(args);

    // update the log if filled > MAX_LOG
    copy_log(str, _log, _filled_log);
    // _filled_log (0-6) : filling screen, (7): removing last log, updating
    _filled_log = (_filled_log > MAX_LOG) ? MAX_LOG+1 : _filled_log+1;

    screen_clear();
    // Loop between the filled log or the max log that could be printed
    for (uint8_t i = 0; (i < MAX_LOG) && (i < _filled_log); i++)
        screen_draw(i, _log[i], CHAR_LIMIT);
    screen_update();
}
