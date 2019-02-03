#include <stdio.h>
#include <netinet/in.h>
#include "data_transfer.h"
#include "peer_storage.h"
#include "bt_server.h"
#include "spiffy.h"
void receive_GET_packet(int sockfd, GET_packet_t *packet, bt_config_t *config, struct sockaddr_in from){
    //DATA_packet_t *d;
    transfer_t *t;
    if(get_peer_state() == I_HAVE_RESOURCE){
        if(check_hash_peer_own(&packet->hash)){
            t = create_new_transfer_in_server_pool(&packet->hash, config, from);
            send_DATA_packet_in_window(sockfd, t, from);
        } else{
            fprintf(stderr, "this GET packet send wrong place\n");
        }
    }
}

void receive_ACK_packet(int sockfd, ACK_packet_t *packet, struct sockaddr_in from){
    fprintf(stderr, "the ack num: %d\n", packet->header.ack_num);
    peer_server_info_t *ps = p->peer_server_info;
    transfer_t *the_transfer = ps->transfers + ps->cursor;
    if(packet->header.ack_num == MAX_SEQ_NUM){
        fprintf(stderr, "the client has receive all the data about one hash\n");
        DATA_packet_t *d = construct_DATA_packet(NULL, 0, MAX_SEQ_NUM+1);
        spiffy_sendto(sockfd, d, d->header.packet_len, 0, (struct sockaddr *)(&from), sizeof(struct sockaddr_in));
        return;
    }
    else if(packet->header.ack_num > MAX_SEQ_NUM){
        return;
    }
    int next_to_send = packet->header.ack_num ;
    
    adjust_sender_window(&the_transfer->sender_window, next_to_send);
    send_DATA_packet_in_window(sockfd, the_transfer, from);
}

