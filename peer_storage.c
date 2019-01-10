#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "peer_storage.h"
#include "bt_parse.h"
#include "spiffy.h"
#include "node_list.h"
//#include "try_find_peer.h"
peer_storage_pool *p;

peer_temp_state_for_GET_t *init_peer_temp_state_for_GET(){
    peer_temp_state_for_GET_t *ret = (peer_temp_state_for_GET_t *)malloc(sizeof(peer_temp_state_for_GET_t));
    ret->WHOHAS_cache = NULL;
    ret->WHOHAS_num = 0;
    ret->want_hashes = NULL;
    ret->want_num = 0;
    ret->hashes_found = NULL;
    return ret;
}

void free_peer_temp_state_for_GET_in_pool(){
    free_peer_temp_state_for_GET(p->peer_temp_state_for_GET);
    p->peer_temp_state_for_GET = NULL;
}

void free_peer_temp_state_for_GET(peer_temp_state_for_GET_t *pt){
    free_contact_packets(pt->WHOHAS_cache, pt->WHOHAS_num);
    free(pt->want_hashes);
    free(pt->hashes_found);
    pt->WHOHAS_cache = 0;
    pt->want_num = 0;
    free(pt);
}
void init_peer_storage_pool(bt_config_t *config){
    p = (peer_storage_pool *)malloc(sizeof(peer_storage_pool));
    p->peer_temp_state_for_GET = NULL;
    p->my_hashes = NULL;
    p->chunk_hash_num = 0;
    p->peer_state = INITIAL_STATE;

    set_peer_pool_hashes(config);
}

void set_temp_state_for_peer_storage_pool(char *chunkfile){
    int packets_length = 0;
    contact_packet_t **packets = construct_WHOHAS_packet(chunkfile, &packets_length);
    peer_temp_state_for_GET_t *pt = init_peer_temp_state_for_GET();
    set_WHOHAS_cache(packets, packets_length, pt);
    set_want_hashes(chunkfile, pt);
    p->peer_temp_state_for_GET = pt;
}

void set_peer_pool_hash_addr_map(){
    peer_temp_state_for_GET_t *pt = p->peer_temp_state_for_GET;
    p->hash_maps = init_hash_addr_map(pt->want_num, pt->want_hashes);
    p->maps_num = pt->want_num;
}


void set_WHOHAS_cache(contact_packet_t **packets, int length, peer_temp_state_for_GET_t *pt){
    pt->WHOHAS_cache = packets;
    pt->WHOHAS_num = length;
    //p->peer_state = ASK_RESOURCE_LOCATION;
}

void send_WHOHAS_packet(int sockfd, bt_config_t *config){
    peer_temp_state_for_GET_t *pt = p->peer_temp_state_for_GET;
    if (pt->WHOHAS_cache == NULL){
        fprintf(stderr, "don't have WHOHAS packets to send\n");
        return;
    }

    for(int i = 0; i < pt->WHOHAS_num; i++){
        send_WHOHAS_to_peers(sockfd, config, *(pt->WHOHAS_cache));
    }
}

void set_want_hashes(char *chunkfile, peer_temp_state_for_GET_t *pt){
    int length = 0;
    chunk_hash **hashes = read_hash_from_chunk_files(chunkfile, &length);
    pt->want_hashes = (chunk_hash *)malloc(sizeof(chunk_hash) * length);
    memset(pt->want_hashes, 0, sizeof(chunk_hash) * length);
    pt->want_num = length;
    pt->hashes_found = (int *)malloc(sizeof(int) * length);
    memset(pt->hashes_found, 0, sizeof(int) * length);

    for(int i = 0; i < length; i++){
        set_packet_hashes(hashes[i], &pt->want_hashes[i]);
    }
    free_hashes(hashes, length);
}

void check_IHAVE_packet(contact_packet_t *packet, struct sockaddr_in from){
    int i;
    peer_temp_state_for_GET_t *pt = p->peer_temp_state_for_GET;
    for(i = 0; i < pt->want_num; i++){
        if(map_this_hash_to_packet(pt->want_hashes[i], packet)){
            pt->hashes_found[i] = 1;
            add_node_to_addr_map(pt->want_hashes[i], from, p->hash_maps, p->maps_num);
        }
    }
}



int found_all_resource_locations(){
    int i;
    peer_temp_state_for_GET_t *pt = p->peer_temp_state_for_GET;
    
    for(i = 0; i < pt->want_num; i++){
        if(pt->hashes_found[i] == 0)     return 0;
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
    // fprintf(stderr, "the hash length is %d\n", sizeof(hashes)/sizeof(chunk_hash));
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
    peer_temp_state_for_GET_t *pt = p->peer_temp_state_for_GET;
    for(int i = 0; i < pt->WHOHAS_num; i++){
        print_WHOHAS_packet(pt->WHOHAS_cache[i]);
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
    //peer_temp_state_for_GET_t *pt = p->peer_temp_state_for_GET;
    print_hash_addr_map(p->hash_maps, p->maps_num);
}