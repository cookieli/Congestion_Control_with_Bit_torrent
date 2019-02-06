#ifndef _FLOW_CONTROL_H_
#define _FLOW_CONTROL_H_
#include <stdint.h>
/*****************************
in sender_window:
the begin points the first packet to send
and we send window size packets once a time
we move the seq_index, found if not send, send it,
not acked ,check time and retransmit or wait it;
begin start value is 0 means first packet to send
******************************/

typedef struct flow_window_s{
    uint32_t begin;
    uint32_t window_size;
    uint32_t seq_index;
} flow_window_t;


void init_flow_window(flow_window_t *s);
void adjust_flow_window(flow_window_t *s, int begin_index);
int num_in_flow_window(int num, flow_window_t win);
#endif