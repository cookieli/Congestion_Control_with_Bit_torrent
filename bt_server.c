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
            send_DATA_packet_from_transfer(sockfd, t, from);
        } else{
            fprintf(stderr, "this GET packet send wrong place\n");
        }
    }
}

void receive_ACK_packet(int sockfd, DATA_packet_t *packet, struct sockaddr_in from){
    peer_server_info_t *ps = p->peer_server_info;
    transfer_t *the_transfer = ps->transfers + ps->cursor;
    if(packet->header.ack_num == MAX_SEQ_NUM){
        fprintf(stderr, "the client has receive all the data about one hash\n");
        DATA_packet_t *d = construct_DATA_packet(NULL, 0, MAX_SEQ_NUM+1);
        spiffy_sendto(sockfd, d, d->header.packet_len, 0, (struct sockaddr *)(&from), sizeof(struct sockaddr_in));
        return;
    }
    the_transfer->seq_num += 1;
    the_transfer->next_to_send += 1;
    send_DATA_packet_from_transfer(sockfd, the_transfer, from);
}

