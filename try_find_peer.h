#include "bt_parse.h"
#include <stdint.h>
#include "sha.h"
typedef struct chunk_hash{
    uint8_t binary_hash[SHA1_HASH_SIZE];
} chunk_hash;

typedef struct header_s {
    uint16_t magicnum;
    char version;
    char packet_type;
    uint16_t header_len;
    uint16_t packet_len;
    uint32_t seq_num;
    uint32_t ack_num;
} header_t;

typedef struct contact_packet {
    header_t header;
    char hashes_num;
    chunk_hash hashes[];
} contact_packet_t; //contact to ask who has and responsd i have

#define MAX_HASH_NUM 74
#define MAX_UDP_PACKET_SIZE 1500


void send_WHOHAS_to_peers(int sockfd, bt_config_t *config, contact_packet_t *who_has);
void construct_packet_header(header_t *header, char packet_type, uint16_t packet_len, uint32_t seq_num, uint32_t ack_num);

int get_packet_type(header_t *header);

void print_packet_header(header_t *header);
contact_packet_t **construct_WHOHAS_packet(char *chunkfile, int *packets_length);
void print_WHOHAS_packet(contact_packet_t *packet);
void print_chunk_hash(chunk_hash h);
void set_packet_hashes(chunk_hash *from, chunk_hash *to);
contact_packet_t *set_WHOHAS_packet(chunk_hash **hashes, int length);
int map_this_hash_to_packet(chunk_hash h1, contact_packet_t *packet);
int two_hash_equal(chunk_hash h1, chunk_hash h2);
int hashes_num_of_WHOHAS_packet(contact_packet_t *packet);
int len_of_WHOHAS_packet(contact_packet_t *packet);
contact_packet_t *construct_IHAVE_packet(chunk_hash *hashes, int length);