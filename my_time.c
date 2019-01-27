#include "my_time.h"
#include <stdio.h>
//#define TEST_TIME
mytime_t millitime(mytime_t *time){
    struct timeval tv;
    mytime_t nCount;

    gettimeofday(&tv, NULL);
    nCount = (mytime_t)(tv.tv_usec/1000 + (tv.tv_sec & 0xfffff) * 1000);

    if(time != NULL){
        *time = nCount;
    }

    return nCount;
}

#ifdef TEST_TIME
int main(int argc, char *argv[]){
    mytime_t begin = millitime(NULL);
    mytime_t time_spent;
    unsigned int i;
    for(i = 0; 1; i++){
        fprintf(stderr, "hello\n");
        time_spent = millitime(NULL) - begin;
        if (time_spent > 5000)  break;
    }
    printf("Number of iterations completed in 5 CPU(?) seconds = %d. begin time is %d\n",i, begin);
  
}
#endif