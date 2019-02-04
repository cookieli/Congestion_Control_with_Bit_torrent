#include "flow_control.h"

void init_sender_window(flow_window_t *s){
    s->begin = 0;
    s->window_size = 8;
    s->seq_index = 0;
}

void adjust_sender_window(flow_window_t *s, int begin_index){
    s->begin = begin_index;
}