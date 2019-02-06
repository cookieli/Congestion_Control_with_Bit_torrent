#include "flow_control.h"

void init_flow_window(flow_window_t *s){
    s->begin = 0;
    s->window_size = 8;
    s->seq_index = 0;
}

void adjust_flow_window(flow_window_t *s, int begin_index){
    s->begin = begin_index;
}

int num_in_flow_window(int num, flow_window_t win){
    return ((num >= win.begin) && (num < win.begin + win.window_size));
}