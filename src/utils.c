#include "utils.h"

void TraceLog(const char *logType, const char *text, ...) {
    va_list args;
    va_start(args, text);

    char buffer[MAX_TRACELOG_BUFFER_SIZE] = { 0 };
    strcat(buffer, logType);

    strcat(buffer, text);
    strcat(buffer, "\n");

    vprintf(buffer, args);
    va_end(args);
}
