#ifndef LOG_H
#define LOG_H

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

void init_log(char *file);
void loginfo(char *format, ...);
void logerr(char *format, ...);
void log_close(void);

#endif//LOG_H
