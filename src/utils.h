#ifndef CGAME_ENGINE_UTILS_H
#define CGAME_ENGINE_UTILS_H

#include <stdio.h>
#include <stdarg.h>
#include <string.h>

#define MAX_TRACELOG_BUFFER_SIZE 128
#define LOG_WARNING "WARNING: "
#define LOG_DEBUG "DEBUG: "
#define LOG_INFO "INFO: "

void TraceLog(const char *logType, const char *text, ...);

#endif // CGAME_ENGINE_UTILS_H
