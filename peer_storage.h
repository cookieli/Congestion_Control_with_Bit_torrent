#ifndef _PEER_STORAGE_H_
#define _PEER_STORAGE_H_


#include "utilities.h"
#include "node_list.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
typedef enum peer_state_s{
    INITIAL_STATE,
    ASK_RESOURCE_LOCATION,
    I_HAVE_RESOURCE
} peer_state_t;

typedef struct hash_addr_map_s{
    chunk_hash hash;
    node_t *node_list;
} hash_addr_map_t;

typedef struct peer_temp_state_for_GET_s{
    contact_packet_t **WHOHAS_cache;
    int WHOHAS_num;
    chunk_hash *want_hashes;
    size_t want_num;
    int *hashes_found;
} peer_temp_state_for_GET_t;

typedef struct peer_storage_pool{

    peer_temp_state_for_GET_t *peer_temp_state_for_GET;
    
    chunk_hash *my_hashes;
    int chunk_hash_num;

    peer_state_t peer_state;

    hash_addr_map_t *hash_maps;
    int maps_num;
} peer_storage_pool;

typedef struct host_and_port{
    char host[INET_ADDRSTRLEN];
    int port;
} host_and_port;



//extern node_hash_t *node_hash;
extern peer_storage_pool *p;

void init_peer_storage_pool(bt_config_t *config);
void set_WHOHAS_cache(contact_packet_t **packets, int length, peer_temp_state_for_GET_t *pt);
void send_WHOHAS_packet(int sockfd, bt_config_t *config);
void set_peer_pool_hashes(bt_config_t *config);
void set_want_hashes(char *chunkfile, peer_temp_state_for_GET_t *pt);

void set_temp_state_for_peer_storage_pool(char *chunkfile);
void set_peer_pool_hash_addr_map();

int *peer_hashes_own_from(contact_packet_t *packet);
chunk_hash *get_IHAVE_hashes_from_pool(int *suffices);
void print_peer_storage_pool();
void handle_WHOHAS_packet(int sock, contact_packet_t *packet, struct sockaddr_in to, socklen_t tolen);

//void set_want_hashes(char *chunkfile);
void check_IHAVE_packet(contact_packet_t *packet, struct sockaddr_in from);
int found_all_resource_locations();

//about host and port

host_and_port *convert_from_sockaddr(struct sockaddr_in from);
void print_host_and_port(host_and_port *hap);
struct sockaddr_in convert_from_hap(host_and_port *hap);


//about node hashes
hash_addr_map_t *init_hash_addr_map(int want_num, chunk_hash *want_hashes);

void add_node_to_addr_map(chunk_hash hash, struct sockaddr_in s, hash_addr_map_t *maps, int maps_length);

hash_addr_map_t *get_map_by_hash(chunk_hash hash, hash_addr_map_t *maps, int want_num);

void print_hash_addr_map(hash_addr_map_t *maps, int want_num);
void print_peer_hash_addr_map();

void free_peer_temp_state_for_GET(peer_temp_state_for_GET_t *pt);
void free_peer_temp_state_for_GET_in_pool();

#endif