#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "bt_parse.h"
#define FILENAME_LEN 13
FILE *log_file;

void setup_logging(bt_config_t *config){
    char filename[FILENAME_LEN];

    memset(filename, 0, FILENAME_LEN);
    sprintf(filename, "peer%d.log", config->identity);

    log_file = fopen(filename, "w+");

    if(log_file == NULL){
        fprintf(stderr, "can't open log file\n");
    } else{
        fprintf(stderr, "it is ok to start logging\n");
    }
}


void log_printf(const char *func, const int line, const char *format, ...){
    va_list args;
    FILE *stream;
    va_start(args, format);
    if(log_file == NULL){
        stream = stderr;
    }else{
        stream = log_file;
    }
    if(func != NULL){
        fprintf(stream, "[%s (%d)]\t", func, line);
    }

    vfprintf(stream, format, args);

    fflush(stream);

    va_end(args);
}