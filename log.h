#ifndef _LOG_H_
#define _LOG_H_

#include "bt_parse.h"

#ifdef ENABLE_LOG
#define LOG(...) \
    do { log_printf(__func__, __LINE__, __VA_ARGS__); } while(0)
#else
#define LOG(...)
#endif

#define GRAPH(...) \
    do { log_printf(NULL, 0, __VA_ARGS__); } while(0)

void setup_logging(bt_config_t *config);
void log_printf(const char *func, const int line, const char *format, ...);




#endif