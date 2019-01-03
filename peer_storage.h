#include "utilities.h"

typedef enum peer_state_s{
    INITIAL_STATE,
    ASK_RESOURCE_LOCATION,
    I_HAVE_RESOURCE
} peer_state_t;

typedef struct peer_storage_pool{
    contact_packet_t **WHOHAS_cache;
    int WHOHAS_num;

    
    chunk_hash *my_hashes;
    int chunk_hash_num;

    peer_state_t peer_state;

    
} peer_storage_pool;

extern peer_storage_pool *p;

void init_peer_storage_pool(bt_config_t *config);
void set_WHOHAS_cache(contact_packet_t **packets, int length);
void send_WHOHAS_packet(int sockfd, bt_config_t *config);
void set_peer_pool_hashes(bt_config_t *config);
int *peer_hashes_own_from(contact_packet_t *packet);
chunk_hash *get_IHAVE_hashes_from_pool(int *suffices);
void print_peer_storage_pool();
void handle_WHOHAS_packet(int sock, contact_packet_t *packet, struct sockaddr_in to, socklen_t tolen);