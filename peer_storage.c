#include <stdlib.h>
#include <stdio.h>
#include "peer_storage.h"
#include "bt_parse.h"
#include "spiffy.h"
#include "node_list.h"
//#include "try_find_peer.h"
peer_storage_pool *p;
void init_peer_storage_pool(bt_config_t *config){
    p = (peer_storage_pool *)malloc(sizeof(peer_storage_pool));
    p->WHOHAS_cache = NULL;
    p->WHOHAS_num = 0;
    p->my_hashes = NULL;
    p->chunk_hash_num = 0;
    p->peer_state = INITIAL_STATE;

    set_peer_pool_hashes(config);
}

void set_WHOHAS_cache(contact_packet_t **packets, int length){
    p->WHOHAS_cache = packets;
    p->WHOHAS_num = length;
    p->peer_state = ASK_RESOURCE_LOCATION;
}

void send_WHOHAS_packet(int sockfd, bt_config_t *config){
    if (p->WHOHAS_cache == NULL){
        fprintf(stderr, "don't have WHOHAS packets to send\n");
        return;
    }

    for(int i = 0; i < p->WHOHAS_num; i++){
        send_WHOHAS_to_peers(sockfd, config, *(p->WHOHAS_cache));
    }
}

void set_want_hashes(char *chunkfile){
    int length = 0;
    chunk_hash **hashes = read_hash_from_chunk_files(chunkfile, &length);
    p->want_hashes = (chunk_hash *)malloc(sizeof(chunk_hash) * length);
    memset(p->want_hashes, 0, sizeof(chunk_hash) * length);
    p->want_num = length;
    p->hashes_found = (int *)malloc(sizeof(int) * length);
    memset(p->hashes_found, 0, sizeof(int) * length);

    for(int i = 0; i < length; i++){
        set_packet_hashes(hashes[i], &p->want_hashes[i]);
    }
    free_hashes(hashes, length);
}

void check_IHAVE_packet(contact_packet_t *packet, struct sockaddr_in from){
    int i;
    for(i = 0; i < p->want_num; i++){
        if(map_this_hash_to_packet(p->want_hashes[i], packet)){
            p->hashes_found[i] = 1;
            add_node_to_addr_map(p->want_hashes[i], from, p->hash_maps, p->want_num);
        }
    }
}

int found_all_resource_locations(){
    int i;
    for(i = 0; i < p->want_num; i++){
        if(p->hashes_found[i] == 0)     return 0;
    }
    return 1;
}



void set_peer_pool_hashes(bt_config_t *config){
    int length = 0;
    chunk_hash **hashes = read_hash_from_chunk_files(config->has_chunk_file,&length);
    p->my_hashes = (chunk_hash *)malloc(sizeof(chunk_hash) * length);
    p->chunk_hash_num = length;
    for(int i = 0; i < length; i++){
        set_packet_hashes(hashes[i], &p->my_hashes[i]);
    }
    free_hashes(hashes, length);
}

int *peer_hashes_own_from(contact_packet_t *packet){
    int total_hash_num = 0;
    int suffix_buffer[MAX_UDP_PACKET_SIZE];
    int i, j;
    j = 0;
    for(i = 0; i < p->chunk_hash_num; i++){
        if(map_this_hash_to_packet(p->my_hashes[i], packet)){
            total_hash_num += 1;
            suffix_buffer[j++] = i;
        }
    }
    if(total_hash_num == 0){
        return NULL;
    }
    int *suffices = (int *)malloc(sizeof(int)* total_hash_num);
    for(i = 0; i < total_hash_num; i++){
        suffices[i] = suffix_buffer[i];
    }
    return suffices;
}

chunk_hash *get_IHAVE_hashes_from_pool(int *suffices){
    
    int length = sizeof(suffices)/sizeof(int);
    chunk_hash *hashes = (chunk_hash *)malloc(length*sizeof(chunk_hash));
    for(int i = 0; i < length; i++){
        hashes[i] = p->my_hashes[suffices[i]];
    }
    fprintf(stderr, "the hash length is %d\n", sizeof(hashes)/sizeof(chunk_hash));
    return hashes;
}

void handle_WHOHAS_packet(int sock, contact_packet_t *packet, struct sockaddr_in to, socklen_t tolen){
    int *suffices = peer_hashes_own_from(packet);
    if(suffices == NULL){
        fprintf(stderr, "i dont' have request files\n");
        return ;
    }
    chunk_hash *hashes = get_IHAVE_hashes_from_pool(suffices);
    int length = sizeof(suffices)/sizeof(int);
    contact_packet_t *IHAVE_packet = construct_IHAVE_packet(hashes, length);
    free(hashes);
    //print_WHOHAS_packet(IHAVE_packet);
    spiffy_sendto(sock, IHAVE_packet, IHAVE_packet->header.packet_len, 0, (struct sockaddr *)&to, tolen);
}

void print_peer_storage_pool(){
    fprintf(stderr, "WHOHAS packet:\n");
    for(int i = 0; i < p->WHOHAS_num; i++){
        print_WHOHAS_packet(p->WHOHAS_cache[i]);
    }
    fprintf(stderr, "i have hashes:\n");
    for(int i = 0; i < p->chunk_hash_num; i++){
        print_chunk_hash(p->my_hashes[i]);
    }
}


host_and_port *convert_from_sockaddr(struct sockaddr_in from){
    host_and_port *hap = (host_and_port *)malloc(sizeof(host_and_port));
    inet_ntop(AF_INET, &(from.sin_addr), hap->host, INET_ADDRSTRLEN);
    hap->port = ntohs(from.sin_port);

    return hap;
}

void print_host_and_port(host_and_port *hap){
    fprintf(stderr, "host_and_port addr: %s:%d\n", hap->host, hap->port);
}

struct sockaddr_in convert_from_hap(host_and_port *hap){
    struct sockaddr_in addr;
    inet_pton(AF_INET, hap->host, &(addr.sin_addr));
    addr.sin_port = htons(hap->port);
    addr.sin_family = AF_INET;
    return addr;
}

hash_addr_map_t *init_hash_addr_map(int want_num, chunk_hash *want_hashes){
    hash_addr_map_t *maps = (hash_addr_map_t *)malloc(sizeof(hash_addr_map_t) * want_num);
    int i;
    for(i = 0; i < want_num; i++){
        (maps+i)->hash = want_hashes[i];
        (maps+i)->node_list = init_node_list();
    }
    return maps;
}

void add_node_to_addr_map(chunk_hash hash, struct sockaddr_in s, hash_addr_map_t *maps, int maps_length){
    hash_addr_map_t *corresponding_map = get_map_by_hash(hash, maps, maps_length);
    if(corresponding_map != NULL){
        insert_addr_to_list(s, corresponding_map->node_list);
    }
}

hash_addr_map_t *get_map_by_hash(chunk_hash hash, hash_addr_map_t *maps, int want_num){
    int i;
    for(i = 0; i < want_num; i++){
        if(two_hash_equal(hash, (maps+i)->hash)){
            return maps + i;
        }
    }
    return NULL;
}


void print_hash_addr_map(hash_addr_map_t *maps, int want_num){
    int i;
    fprintf(stderr, "hash_add_maps:\n");
    for(i = 0; i < want_num; i++){
        fprintf(stderr, "chunk hash is: ");
        print_chunk_hash((maps + i)->hash);
        print_node_list(maps->node_list);
    }
}

void print_peer_hash_addr_map(){
    print_hash_addr_map(p->hash_maps, p->want_num);
}