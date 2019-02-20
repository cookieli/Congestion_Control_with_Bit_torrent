#include <stdio.h>
#include <netinet/in.h>
#include "data_transfer.h"
#include "peer_storage.h"
#include "bt_server.h"
#include "spiffy.h"
#include "linkedlist.h"
void handle_server_timeout(int sockfd){
    if(get_peer_state() == I_HAVE_RESOURCE){
        //if()
        peer_server_info_t *ps = p->peer_server_info;
        if(ps == NULL){
            fprintf(stderr, "don't receive GET packet\n");
            return;
        }
        if(!node_length(ps->transfer_head)){
            fprintf(stderr, "has no transfers\n");
            return;
        }
        
    }
}

void receive_GET_packet(int sockfd, GET_packet_t *packet, bt_config_t *config, struct sockaddr_in from){
    //DATA_packet_t *d;
    transfer_t *t;
    if(get_peer_state() == I_HAVE_RESOURCE){
        if(check_hash_peer_own(&packet->hash)){
            //t = create_new_transfer_in_server_pool(&packet->hash, config, from);
            insert_new_transfer_into_server_pool(&packet->hash, config, from);
            t =(transfer_t *)p->peer_server_info->transfer_head->data;
            t->time_stamp = millitime(NULL);
            send_DATA_packet_in_window(sockfd, t, from);
        } else{
            fprintf(stderr, "this GET packet send wrong place\n");
        }
    }
}

void receive_ACK_packet(int sockfd, ACK_packet_t *packet, struct sockaddr_in from){
    fprintf(stderr, "the ack num: %d\n", packet->header.ack_num);
    peer_server_info_t *ps = p->peer_server_info;
    transfer_t *the_transfer;// =(transfer_t *) ps->transfer_head->data;
    Node *transfer_node = node_find(ps->transfer_head, cmp_transfer_by_sockaddr, &from);
    if(transfer_node == NULL){
        fprintf(stderr, "can't find corresponding node\n");
    }
    the_transfer = (transfer_t *)transfer_node->data;
    //transfer_node = node_find(ps->transfer_head, cmp_two_sock(), &from);
    the_transfer->time_stamp = millitime(NULL);
    flow_window_t win = the_transfer->sender_window;
    if(packet->header.ack_num == MAX_SEQ_NUM){
        set_data_been_acked(packet->header.ack_num, the_transfer);
        fprintf(stderr, "the client has receive all the data about one hash\n");
        DATA_packet_t *d = construct_DATA_packet(NULL, 0, MAX_SEQ_NUM+1);
        spiffy_sendto(sockfd, d, d->header.packet_len, 0, (struct sockaddr *)(&from), sizeof(struct sockaddr_in));
        node_delete(&ps->transfer_head, transfer_node, remove_transfer);
        return;
    }
    else if(packet->header.ack_num > MAX_SEQ_NUM){
        return;
    } else if(packet->header.ack_num == win.begin){
        if(the_transfer->retransmit_time >=3){
            fprintf(stderr, "we have receive 3 same ack ");
            fprintf(stderr, "now we need to retransmit the data: %d\n", packet->header.ack_num);
            send_DATA_packet_from_transfer_by_seq(the_transfer, packet->header.ack_num+1, sockfd, from);
            the_transfer->retransmit_time = 0;
            //exit(-1);
            return;
        }
        fprintf(stderr, "the data: %d receiver dont' received\n", packet->header.ack_num);
        the_transfer->retransmit_time += 1;
        return;
    } else{
        set_data_been_acked(packet->header.ack_num, the_transfer);
    }
    int next_to_send = packet->header.ack_num;
    adjust_flow_window(&the_transfer->sender_window, next_to_send);
    send_DATA_packet_in_window(sockfd, the_transfer, from);
    the_transfer->retransmit_time = 0;
}

