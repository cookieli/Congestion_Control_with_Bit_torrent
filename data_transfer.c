#include "data_transfer.h"
#include "try_find_peer.h"
#include "node_list.h"
#include <stdlib.h>
#include <stdio.h>
#include "spiffy.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include "my_time.h"
#include "peer_storage.h"

//packet_type = 2
GET_packet_t *construct_GET_packet(chunk_hash  *hash){
    GET_packet_t *packet = (GET_packet_t *)malloc(sizeof(GET_packet_t) );
    uint16_t packet_len = 16 + 20;
    construct_packet_header(&packet->header, 2, packet_len, 0, 0);
    set_packet_hashes(hash, &packet->hash);
    return packet;
}
DATA_packet_t *construct_DATA_packet(uint8_t *data, int data_num, int seq_num){
    DATA_packet_t *packet = (DATA_packet_t *)malloc(sizeof(DATA_packet_t) + data_num * sizeof(uint8_t));
    uint16_t packet_len = 16 + data_num;
    construct_packet_header(&packet->header, 3, packet_len, seq_num, 0);
    for(int i = 0; i < data_num; i++){
        packet->data[i] = data[i];
    }
    return packet;
}
void init_GET_packet_tunnel(GET_packet_tunnel_t *tunnel, GET_packet_t *packet, struct sockaddr_in addr){
    tunnel->packet = packet;
    tunnel->addr = addr;
    tunnel->begin_sent = 0;
    tunnel->retransmit_time = 0;
    tunnel->have_been_acked = 0;
}

GET_packet_sender_t *init_GET_packet_sender(hash_addr_map_t *maps, int map_num){
    GET_packet_sender_t *sender = (GET_packet_sender_t *)malloc(sizeof(GET_packet_sender_t));
    sender->tunnels = construct_GET_tunnel(maps, map_num);
    sender->tunnel_num = map_num;
    sender->chunks = (chunk_t *)malloc(sizeof(chunk_t) * map_num);
    sender->cursor = 0;
    return sender;
}

GET_packet_tunnel_t *construct_GET_tunnel(hash_addr_map_t *maps, int map_num){
    GET_packet_tunnel_t *tunnels = (GET_packet_tunnel_t *)malloc(sizeof(GET_packet_tunnel_t) * map_num);
    int i;
    hash_addr_map_t *the_map;
    GET_packet_tunnel_t *the_tunnel;
    struct sockaddr_in addr;
    for(i = 0; i < map_num; i++){
        the_tunnel = (tunnels + i);
        the_map = (maps + i);
        GET_packet_t *packet = construct_GET_packet(&the_map->hash);
        addr = get_addr_from_node_list(the_map->node_list);
        init_GET_packet_tunnel(the_tunnel, packet, addr);
    }
    return tunnels;
}
void send_GET_tunnel(int sockfd, GET_packet_tunnel_t *tunnel){
    tunnel->begin_sent = millitime(NULL);
    tunnel->retransmit_time += 1;
    spiffy_sendto(sockfd, tunnel->packet, tunnel->packet->header.packet_len, 0, (struct sockaddr *)(&(tunnel->addr)), sizeof(tunnel->addr));
}

void free_GET_tunnel(GET_packet_tunnel_t *tunnel){
    free(tunnel->packet);
    free(tunnel);
}
void print_sockaddr(struct sockaddr_in addr){
    char str[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(addr.sin_addr), str, INET_ADDRSTRLEN);
    fprintf(stderr, "%s: %d\n", str, ntohs(addr.sin_port));
}

void print_GET_packet(GET_packet_t *packet){
    fprintf(stderr, "the packet type is: %d", packet->header.packet_type);
    print_packet_header(&packet->header);
    print_chunk_hash(packet->hash);
}

void print_GET_packet_tunnel(GET_packet_tunnel_t *t){
    print_GET_packet(t->packet);
    fprintf(stderr, "the tunnel to addr: ");
    print_sockaddr(t->addr);
    fprintf(stderr, "begin_time is: %d\n", t->begin_sent);
    mytime_t now = millitime(NULL);
    fprintf(stderr, "time spent: %ld\n", now - t->begin_sent);
    fprintf(stderr, "limit times: %d\n", t->retransmit_time);
}

void print_GET_packet_sender(){
    fprintf(stderr, "the GET packet sender: \n");
    peer_client_info_t *pc = p->peer_client_info;
    GET_packet_sender_t *s = pc->GET_packet_sender;
    int i;
    for(i = 0; i < s->tunnel_num; i++){
        print_GET_packet_tunnel(&s->tunnels[i]);
    }
    print_chunk(s->chunks + s->cursor);
}

int check_time_out_in_GET_tunnnel_after_last_sent(GET_packet_tunnel_t *t){
    mytime_t now = millitime(NULL);
    mytime_t begin = t->begin_sent;
    if(now - begin > 5000)   return 1;
    return 0;
}

int check_GET_tunnel_retransmit_time(GET_packet_tunnel_t *t){
    return t->retransmit_time;
}

int check_transfer_with_bin_hash(transfer_t *t, chunk_hash *h){
    return check_chunk_with_bin_hash(*t->chunk, h->binary_hash);
}

//construct data packet from transfer
void send_DATA_packet_from_transfer(int sockfd, transfer_t *t, struct sockaddr_in from){
    uint8_t data[PACKET_DATA_SIZE];
    DATA_packet_t *d;
    int size;
    if(t->next_to_send < MAX_SEQ_NUM - 1){
        for(int i = 0; i < PACKET_DATA_SIZE; i++){
            data[i] = t->chunk->data[t->next_to_send * PACKET_DATA_SIZE+i];
        }
        size = PACKET_DATA_SIZE;
    } else if(t->next_to_send == MAX_SEQ_NUM - 1){
        for(int i = t->next_to_send * PACKET_DATA_SIZE; i < DATA_CHUNK_SIZE ; i++){
            int j = i - t->next_to_send * PACKET_DATA_SIZE;
            data[j] = t->chunk->data[i];
        }
        size = DATA_CHUNK_SIZE - t->next_to_send * PACKET_DATA_SIZE;
    }
    d = construct_DATA_packet(data, size, t->next_to_send + 1);
    spiffy_sendto(sockfd, d, d->header.packet_len, 0, (struct sockaddr *)(&from), sizeof(struct sockaddr_in));
    t->next_to_send += 1;
}