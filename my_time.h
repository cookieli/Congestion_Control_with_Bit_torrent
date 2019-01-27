#ifndef _MY_TIME_H_
#define _MY_TIME_H_

#include <sys/time.h>
#include <stdlib.h>

typedef unsigned long mytime_t;

mytime_t millitime(mytime_t *time);
#endif