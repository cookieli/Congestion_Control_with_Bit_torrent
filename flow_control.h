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
#define DEFAULT_WIN_THRESHOLD 64
#define DEFAULT_WIN_SIZE 1
typedef enum congestion_state_s{
    SS,
    CA
} congestion_state_t;

typedef struct flow_window_s{
    uint32_t begin;
    uint32_t window_size;
    uint32_t start_win_size_forCA;
    uint32_t seq_index;
    uint32_t window_threshold;
    congestion_state_t congestion_state;
    
} flow_window_t;


void init_flow_window(flow_window_t *s);
void adjust_flow_window(flow_window_t *s, int begin_index);
int num_in_flow_window(int num, flow_window_t win);

void congestion_control(void *v);
void detect_first_loss(void *v);
void gragh_wind_size(void *t);
#endif