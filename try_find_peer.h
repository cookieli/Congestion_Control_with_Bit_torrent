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

void send_WHOHAS_to_peers(int sockfd, bt_config_t *config);