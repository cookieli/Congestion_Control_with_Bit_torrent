#ifndef _DATA_TRANSFER_H_
#define _DATA_TRANSFER_H_
#include "types.h"
#include "try_find_peer.h"
#include "my_time.h"
//#include "peer_storage.h"

typedef struct GET_packet_s{
    header_t header;
    chunk_hash hash;
} GET_packet_t;

typedef struct GET_packet_tunnel_s{
    GET_packet_t *packet;
    struct sockaddr_in addr;
    mytime_t begin_sent;
    int retransmit_time;
} GET_packet_tunnel_t;

typedef struct GET_packet_sender_s{
    GET_packet_tunnel_t *tunnels;
    int tunnel_num;
    int cursor;
} GET_packet_sender_t;

GET_packet_t *construct_GET_packet(chunk_hash *hash);
GET_packet_sender_t *init_GET_packet_sender(hash_addr_map_t *maps, int map_num);
void init_GET_packet_tunnel(GET_packet_tunnel_t *tunnel, GET_packet_t *packet, struct sockaddr_in addr);
GET_packet_tunnel_t *construct_GET_tunnel(hash_addr_map_t *maps, int map_num);
void free_GET_tunnel(GET_packet_tunnel_t *tunnel);
void send_GET_tunnel(int sockfd, GET_packet_tunnel_t *tunnel);

void print_sockaddr(struct sockaddr_in addr);
void print_GET_packet(GET_packet_t *packet);
void print_GET_packet_tunnel(GET_packet_tunnel_t *t);
void print_GET_packet_sender();

int check_time_out_in_GET_tunnnel_after_last_sent(GET_packet_tunnel_t *t);
int check_GET_tunnel_retransmit_time(GET_packet_tunnel_t *t);

#endif
