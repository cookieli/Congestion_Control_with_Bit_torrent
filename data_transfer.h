#ifndef _DATA_TRANSFER_H_
#define _DATA_TRANSFER_H_
#include "types.h"
#include "try_find_peer.h"
#include "my_time.h"
#include "chunk.h"
#include "flow_control.h"
//#include "peer_storage.h"
#define TIMEOUT_THRESHOLD 3000

typedef struct GET_packet_s{
    header_t header;
    chunk_hash hash;
} GET_packet_t;

typedef struct DATA_packet_s{
    header_t header;
    uint8_t data[];
} DATA_packet_t;

typedef struct ACK_packet_s{
    header_t header;
} ACK_packet_t;

//tunnel's func: send GET packet and receive data
typedef struct GET_packet_tunnel_s{
    GET_packet_t *packet;
    struct sockaddr_in addr;
    mytime_t begin_sent;
    int retransmit_time;
    int have_been_acked;


    chunk_t *chunk;
    flow_window_t receive_window;
} GET_packet_tunnel_t;

typedef struct GET_packet_sender_s{
    GET_packet_tunnel_t *tunnels;
    chunk_t *chunks;
    int tunnel_num;
    int cursor;
} GET_packet_sender_t;

typedef struct transfer_s{
    chunk_t *chunk;
    uint32_t next_to_send;
    uint32_t seq_num;
    struct sockaddr_in to;

    mytime_t time_stamp;
    mytime_t rtt;
    int retransmit_time;
    
    flow_window_t sender_window;
} transfer_t;



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
int check_transfer_with_bin_hash(transfer_t *t, chunk_hash *h);

int transfer_has_timeout(transfer_t *t);


DATA_packet_t *construct_DATA_packet(uint8_t *data, int data_num, int seq_num);
void send_DATA_packet_from_transfer(int sockfd, transfer_t *t, struct sockaddr_in from);
void send_DATA_packet_in_window(int sockfd, transfer_t *t, struct sockaddr_in from);
void set_data_been_acked(int ack_num, transfer_t *t);
void send_DATA_packet_from_transfer_by_seq(transfer_t *t, uint32_t seq_num, int sockfd, struct sockaddr_in from);
void create_output_file(char *output_file, GET_packet_sender_t *sender);
int transfer_has_timeout(transfer_t *t);

void init_transfer(transfer_t *the_transfer,chunk_hash *hash, bt_config_t *config, struct sockaddr_in to);
int cmp_transfer_by_sockaddr(void *a, void *b);

void remove_transfer(void *data);
#endif

