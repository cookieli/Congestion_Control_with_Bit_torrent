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
    if(get_peer_state() == ASK_RESOURCE_LOCATION){
        check_IHAVE_packet(packet, from);
        if(found_all_resource_locations()){
            set_peer_state(FOUND_ALL_RESOURCE);
            fprintf(stderr, "i have found all resource locations\n");
            free_peer_temp_state_for_GET_in_pool();
        }
    } else if(get_peer_state() == GET_ERROR_FOR_RESOURCE_LOCATION){
        //we need to resend GET tunnel
        peer_client_info_t *pc = p->peer_client_info;
        GET_packet_sender_t *sender = pc->GET_packet_sender;
        if((sender->tunnels + sender->cursor)->packet == NULL){
            fprintf(stderr, "we need to reconstruct sender tunnel\n");
            GET_packet_tunnel_t *tunnel = sender->tunnels + sender->cursor;
            if(packet->hashes_num > 1){
                fprintf(stderr, "the IHAVE packet not for GET ERROR: too many hashes\n");
                return;
            }
            init_GET_packet_tunnel(tunnel, construct_GET_packet(&packet->hashes[0]), from);
            print_GET_packet_tunnel(sender->tunnels + sender->cursor);
        } else{
            fprintf(stderr, "something wrong about GET retransmititon in handle_IHAVE_packet");
        }
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
    peer_client_info_t *pc = p->peer_client_info;
    GET_packet_sender_t *sender = pc->GET_packet_sender;
    GET_packet_tunnel_t *tunnel = sender->tunnels + sender->cursor;
    flow_window_t win;
    uint32_t data_position = packet->header.seq_num - 1;
    chunk_t c;
    int ack_num;
    int data_len = packet->header.packet_len - packet->header.header_len;
    if(data_len == 0 && packet->header.seq_num == MAX_SEQ_NUM + 1){
        fprintf(stderr, "i have receive all the data packet\n");
        print_GET_packet_sender();
        increase_to_another_GET_packet_tunnel();
        send_GET_packet_in_peer_pool(sockfd);
        if(get_peer_state() == FOUND_ALL_DATA){
            create_output_file(config->output_file, sender);
        }
        return;
    } else if(packet->header.seq_num > MAX_SEQ_NUM + 1){
        return;
    }
    if(tunnel->have_been_acked == 0){
        tunnel->have_been_acked = 1;
        init_flow_window(&tunnel->receive_window);
        tunnel->chunk = (chunk_t *)malloc(sizeof(chunk_t));
        binhash_copy(tunnel->packet->hash.binary_hash, c.binhash);
        binary2hex(c.binhash, BIN_HASH_SIZE, c.hexhash);
        c.id = find_hash_id_in_master_chunk_file(c.hexhash, config->chunk_file);
        memset(c.seq_bits, 0, MAX_SEQ_NUM);
        c.cursor = 0;
    } else{
        c = *tunnel->chunk;
    }
    win = tunnel->receive_window;
    if(!num_in_flow_window(data_position, win)){
        return;
    }
    //memcpy(c.data + data_position*PACKET_DATA_SIZE, packet->data, data_len);
    if(tunnel->chunk->seq_bits[data_position] == 0){
        for(int i = 0; i < data_len; i++){
            c.data[data_position*PACKET_DATA_SIZE + i] = packet->data[i];
        }
    }
    *tunnel->chunk = c;
    tunnel->chunk->seq_bits[data_position] = 1;
    ack_num = find_biggest_ack_num_in_window(win, tunnel->chunk);
    fprintf(stderr, "the ack num is: %d\n", ack_num);
    adjust_flow_window(&tunnel->receive_window, ack_num);
    //send ack packet to the server
    send_ACK_packet(sockfd, ack_num ,from);
}
uint32_t find_biggest_ack_num_in_window(flow_window_t win, chunk_t *chunk){
    uint32_t i;
    for(i = win.begin; (i < win.begin + win.window_size) && (i < MAX_SEQ_NUM); i++){
        // fprintf(stderr, "%d: %d\n", i, chunk->seq_bits[i]);
        if(chunk->seq_bits[i] == 0)  break;
    }
    if(chunk->seq_bits[i] == 1){
        i = i+1;
        //return MAX_SEQ_NUM;
    }
    if(i >= MAX_SEQ_NUM){
        return MAX_SEQ_NUM;
    }
    return i-1+1;
}
void handle_client_timeout(int sockfd, bt_config_t *config){
    //fprintf(stderr, "handle_client_timeout\n");
    if(get_peer_state() == ASK_RESOURCE_LOCATION){
        send_WHOHAS_packet(sockfd, config);
    }
    else if(get_peer_state() == FOUND_ALL_RESOURCE){
        // print_GET_packet_sender();
        peer_client_info_t *pc = p->peer_client_info;
        GET_packet_sender_t *sender = pc->GET_packet_sender;
        GET_packet_tunnel_t *tunnel = sender->tunnels + sender->cursor;
        if(tunnel->have_been_acked == 0){
            if(check_time_out_in_GET_tunnnel_after_last_sent(tunnel) && check_GET_tunnel_retransmit_time(tunnel) < 3){
                fprintf(stderr, "You need to retransmit GET packet for hash: ");
                print_chunk_hash(tunnel->packet->hash);
                send_GET_tunnel(sockfd, tunnel);
                return;
            } else if(check_GET_tunnel_retransmit_time(tunnel) >= 3){
                fprintf(stderr, "you need to find another source in peer for hash: ");
                print_chunk_hash(tunnel->packet->hash);
                add_hash_to_peer_temp_state_for_GET_in_pool(&tunnel->packet->hash);
                set_WHOHAS_cache_in_pool();
                set_peer_state(GET_ERROR_FOR_RESOURCE_LOCATION);
                free_GET_tunnel(tunnel);
                tunnel = (GET_packet_tunnel_t *)malloc(sizeof(GET_packet_tunnel_t));
                tunnel->packet = NULL;
                send_WHOHAS_packet(sockfd, config);
                return;
            }
        } else {
            fprintf(stderr, "this get data has been received\n");
        }
    }
}