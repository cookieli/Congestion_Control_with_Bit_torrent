#include "flow_control.h"
#include "data_transfer.h"
#include <stdio.h>
#include "log.h"
#define MAX(a, b) (a < b ? b : a)

static void slow_start(transfer_t *t);
static void congestion_avoid(transfer_t *t);
void init_flow_window(flow_window_t *s){
    s->begin = 0;
    s->window_size =1; //DEFAULT_WIN_SIZE;//default is 8;
    s->seq_index = 0;
    s->window_threshold = DEFAULT_WIN_THRESHOLD;
    s->congestion_state = SS;
}

void adjust_flow_window(flow_window_t *s, int begin_index){
    s->begin = begin_index;
}

int num_in_flow_window(int num, flow_window_t win){
    return ((num >= win.begin) && (num < win.begin + win.window_size));
}

void detect_first_loss(void *v){
    transfer_t *t = (transfer_t *)v;
    flow_window_t *win = &t->sender_window;
    win->window_threshold = MAX(win->window_size/2, 2);
    if(win->congestion_state == SS){
        fprintf(stderr, "switch to CA mode\n");
        win->congestion_state = CA;
        win->start_win_size_forCA = win->window_size;
        //win->window_size /= 2;
        t->time_stamp = millitime(NULL);
        //exit(-1);
    } else {
        fprintf(stderr, "switch to SS mode\n");
        win->congestion_state = SS;
        win->window_size = 1;
        gragh_wind_size(t);
    }
}
void congestion_control(void *v){
    transfer_t *t = (transfer_t *)v;
    switch(t->sender_window.congestion_state){
    case SS:
        slow_start(t);
        break;
    case CA:
        congestion_avoid(t);
        break;
    default:
        fprintf(stderr, "err about congestion state\n");
        break;
    }
}
static void slow_start(transfer_t *t){
    if(t->rtt == 0){
        t->rtt = millitime(NULL) - t->time_stamp;
    }
    flow_window_t *win = &t->sender_window;
    if(win->window_size < win->window_threshold){
        win->window_size += 1;
        gragh_wind_size(t);
    } else {
        win->congestion_state = CA;
        win->start_win_size_forCA = win->window_size;
        t->time_stamp = millitime(NULL);
    }
}
//now assume transfer rtt is constant
static void congestion_avoid(transfer_t *t){
    flow_window_t *win = &t->sender_window;
    mytime_t now_rtt = millitime(NULL) - t->time_stamp;
    uint32_t new_win_size = win->start_win_size_forCA + now_rtt / t->rtt;
    if(new_win_size > win->window_size){
        win->window_size = new_win_size;
        gragh_wind_size(t);
    }
}

void gragh_wind_size(void *v){
    transfer_t *t = (transfer_t *)v;
    GRAPH("T%d\t%lu\t%u\n", t->chunk->id, millitime(NULL) - t->start_time, t->sender_window.window_size);
}