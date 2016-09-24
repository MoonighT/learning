#include "log.h"

FILE *log_file;

void init_log(char *file) {
    log_file = fopen(file, "w+");
}


void loginfo(char *format, ...) {
    char *prefix = "[Info] ";
    va_list args;
    char buf[256];
    strcpy(buf, prefix);
    va_start(args, format);
    vsprintf(buf+strlen(prefix), format, args);
    fputs(buf,log_file);
    va_end(args);
    fflush(log_file);
}

void logerr(char *format, ...) {
    char *prefix = "[Error] ";
    va_list args;
    char buf[256];
    strcpy(buf, prefix);
    va_start(args, format);
    vsprintf(buf+strlen(prefix), format, args);
    fputs(buf,log_file);
    va_end(args);
    fflush(log_file);
}



void log_close(void) {
    fflush(log_file);
    fclose(log_file);
}

