#include <stdio.h>
#include "peer_storage.h"
#include "bt_client.h"
#include "bt_parse.h"
#include <time.h>
#include <string.h>
#include "peer.h"
#include "data_transfer.h"
#include "try_find_peer.h"
#include "spiffy.h"
void handle_IHAVE_packet(contact_packet_t *packet, struct sockaddr_in from){
    peer_client_info_t *pc = p->peer_client_info;
    if(pc == NULL){
        fprintf(stderr, "don't have peer client info\n");
        return;
    }
    peer_temp_state_for_GET_t *pt = pc->peer_temp_state_for_GET;
     if(pt == NULL){
        if(get_peer_state() == FOUND_ALL_RESOURCE){
            fprintf(stderr, "Sorry we have enough source\n");
        } else{
            fprintf(stderr, "Sorry we don't need this IHAVE\n");
        }
        return;
    }
    check_IHAVE_packet(packet, from);
    if(found_all_resource_locations()){
        set_peer_state(FOUND_ALL_RESOURCE);
        fprintf(stderr, "i have found all resource locations\n");
        free_peer_temp_state_for_GET_in_pool();
    }
}
void send_ACK_packet(int sock, int ack_num, struct sockaddr_in from){
    ACK_packet_t *packet = (ACK_packet_t *)malloc(sizeof(ACK_packet_t));
    construct_packet_header(&packet->header, 4, 16, 0, ack_num);
    spiffy_sendto(sock, packet, packet->header.packet_len, 0, (struct sockaddr *)(&from), sizeof(struct sockaddr_in));
    free(packet);
}

void receive_DATA_packet(int sockfd, DATA_packet_t *packet, bt_config_t *config, struct sockaddr_in from){
    fprintf(stderr, "seq_num:%d, packet_len: %d\n", packet->header.seq_num, packet->header.packet_len);
    Node *node;
    peer_client_info_t *pc = p->peer_client_info;
    node = node_find(pc->sender_list, cmp_sender_by_sockaddr, &from);
    GET_packet_sender_t *sender = (GET_packet_sender_t *)node->data;
    GET_packet_tunnel_t *tunnel = sender->tunnels + sender->cursor;
    uint32_t data_position = packet->header.seq_num - 1;
    chunk_t c;
    int ack_num;
    int data_len = packet->header.packet_len - packet->header.header_len;
    tunnel->retransmit_time = 0;
    if(data_len == 0 && packet->header.seq_num == MAX_SEQ_NUM + 1){
        if(!validate_chunk(tunnel->chunk)){
            memset(tunnel->chunk->data, 0, DATA_CHUNK_SIZE);
            memset(tunnel->chunk->seq_bits, 0, MAX_SEQ_NUM);
            //tunnel->retransmit_time = 0;
            //init_flow_window(&tunnel->receive_window);
            send_GET_packet_in_sender(sender, sockfd);
            return;
        }
        increase_to_another_GET_packet_tunnel(sender);
        fprintf(stderr, "i have receive all the data packet\n");
        if(check_sender_lst_all_received(pc->sender_list)){
            fprintf(stderr, "Found all data\n");
            set_peer_state(FOUND_ALL_DATA);
            if(get_peer_state() == FOUND_ALL_DATA){
                fprintf(stderr, "now to create output file\n");
                create_output_file_from_client_side(pc);
                add_new_hash_to_peer();
                fprintf(stderr, "the GET tar have been finished\n");
                clear_peer_client_side();
                set_peer_state(INITIAL_STATE);
            }
            return;
        }
        fprintf(stderr, "to send packet in this position\n");
        send_GET_packet_in_sender(sender, sockfd);
        return;
    } else if(packet->header.seq_num > MAX_SEQ_NUM + 1){
        return;
    }
    if(tunnel->have_been_acked == 0){
        tunnel->have_been_acked = 1;
        //tunnel->retransmit_time = 0;
        tunnel->begin_sent = millitime(NULL);
        //init_flow_window(&tunnel->receive_window);
        tunnel->chunk = (chunk_t *)malloc(sizeof(chunk_t));
        binhash_copy(tunnel->packet->hash.binary_hash, c.binhash);
        binary2hex(c.binhash, BIN_HASH_SIZE, c.hexhash);
        //c.id = find_hash_id_in_master_chunk_file(c.hexhash, pc->chunk_file);
        c.id = find_hash_id_in_chunk_file(c.hexhash, pc->chunk_file);
        //fprintf(stderr, "chunk id: %d\n", c.id);
        //exit(-1);
        memset(c.seq_bits, 0, MAX_SEQ_NUM);
        c.cursor = 0;
    } else{
        c = *tunnel->chunk;
    }
    //win = tunnel->receive_window;
    //if(!num_in_flow_window(data_position, win)){
    //  return;
    //}
    //memcpy(c.data + data_position*PACKET_DATA_SIZE, packet->data, data_len);
    if(tunnel->chunk->seq_bits[data_position] == 0){
        for(int i = 0; i < data_len; i++){
            c.data[data_position*PACKET_DATA_SIZE + i] = packet->data[i];
        }
    }
    *tunnel->chunk = c;
    tunnel->chunk->seq_bits[data_position] = 1;
    ack_num = find_biggest_ack_num_in_window(tunnel->chunk);
    fprintf(stderr, "the ack num is: %d\n", ack_num);
    //adjust_flow_window(&tunnel->receive_window, ack_num);
    //send ack packet to the server
    send_ACK_packet(sockfd, ack_num ,from);
}
uint32_t find_biggest_ack_num_in_window(chunk_t *chunk){
    uint32_t i;
    for(i = 0; i < MAX_SEQ_NUM; i++){
        if(chunk->seq_bits[i] == 0)  break;
    }
    return i-1+1;
}
void handle_peer_crashed(Node *node, int sockfd, bt_config_t *config){
    peer_client_info_t *pc = p->peer_client_info;
    GET_packet_sender_t *sender = (GET_packet_sender_t *)node->data;
    GET_packet_tunnel_t *tunnel;
    int i;
    fprintf(stderr, "the corresponding peer is crashed,");
    fprintf(stderr, "you need to find another source in peer for hash: \n");
    for(i = sender->cursor; i < sender->tunnel_num; i++){
        tunnel = sender->tunnels + i;
        add_hash_to_peer_temp_state_for_GET_in_pool(&tunnel->packet->hash);
//free_GET_tunnel(tunnel);
        free(tunnel->packet);
        free(tunnel->chunk);
    }
    sender->tunnel_num = sender->cursor;
    set_WHOHAS_cache_in_pool();
    if(sender->tunnel_num == 0){
        node_delete(&pc->sender_list, node, remove_sender);
    }else {
        GET_packet_tunnel_t *temp = sender->tunnels;
        sender->tunnels = (GET_packet_tunnel_t *)realloc(sender->tunnels, sender->tunnel_num * (sizeof(GET_packet_tunnel_t)));
        if(sender->tunnels == NULL){
            fprintf(stderr, "can't free storage\n");
            sender->tunnels = temp;
        }
    }
    set_peer_state(GET_ERROR_FOR_RESOURCE_LOCATION);
    send_WHOHAS_packet(sockfd, config);
    return;
}
void handle_client_timeout(int sockfd, bt_config_t *config){
    //fprintf(stderr, "handle_client_timeout\n");
    peer_client_info_t *pc = p->peer_client_info;
    peer_temp_state_for_GET_t *pt = pc->peer_temp_state_for_GET;
    if(get_peer_state() == ASK_RESOURCE_LOCATION || (pt != NULL && pt->WHOHAS_cache != NULL)){
        send_WHOHAS_packet(sockfd, config);
    }
    else if(get_peer_state() == FOUND_ALL_RESOURCE){
        // print_GET_packet_sender();
        peer_client_info_t *pc = p->peer_client_info;
        Node *node = NULL;
        Node *next = NULL;
        GET_packet_sender_t *sender;// = pc->GET_packet_sender;
        GET_packet_tunnel_t *tunnel;// = sender->tunnels + sender->cursor;
        for(node = pc->sender_list; node != NULL; node = next){
            next = node->next;
            sender = (GET_packet_sender_t *)node->data;
            if(sender->cursor == sender->tunnel_num){
                continue;
            }
            tunnel = sender->tunnels + sender->cursor;
            if(tunnel->have_been_acked == 0){
                if(check_time_out_in_GET_tunnnel_after_last_sent(tunnel) && check_GET_tunnel_retransmit_time(tunnel) < 3){
                    fprintf(stderr, "You need to retransmit GET packet for hash: \n");
                    print_chunk_hash(tunnel->packet->hash);
                    send_GET_tunnel(sockfd, tunnel);
                    return;
                } else if(check_GET_tunnel_retransmit_time(tunnel) >= 3){
                    handle_peer_crashed(node, sockfd, config);
                }
            } else {//the GET is sent and data is received ,now we need to resend ack packet
                //fprintf(stderr, "this get data has been received\n");
                if(check_time_out_in_GET_tunnnel_after_last_sent(tunnel) && check_GET_tunnel_retransmit_time(tunnel) < 5){
                    fprintf(stderr, "You need to retransmit ack packet \n");
                    int ack_num = find_biggest_ack_num_in_window(tunnel->chunk);
                    send_ACK_packet(sockfd, ack_num, tunnel->addr);
                    tunnel->retransmit_time += 1;
                } else if(check_GET_tunnel_retransmit_time(tunnel) >= 5){
                    handle_peer_crashed(node,sockfd, config);
                }
            }
        }
    }
}

int validate_chunk(chunk_t *c){
    uint8_t hash[SHA1_HASH_SIZE];
    shahash(c->data, DATA_CHUNK_SIZE, hash);
    if(!check_chunk_with_bin_hash(*c, hash)){
        return 1;// the chunk is right;
    }
    return 0;// the chunk is wrong
}