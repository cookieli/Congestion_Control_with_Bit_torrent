#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "peer_storage.h"
#include "bt_parse.h"
#include "spiffy.h"
#include "node_list.h"
//#include "try_find_peer.h"
peer_storage_pool *p;

peer_state_t get_peer_state(){
    return p->peer_state;
}
void set_peer_state(peer_state_t state){
    p->peer_state = state;
}
peer_temp_state_for_GET_t *init_peer_temp_state_for_GET(){
    peer_temp_state_for_GET_t *ret = (peer_temp_state_for_GET_t *)malloc(sizeof(peer_temp_state_for_GET_t));
    ret->WHOHAS_cache = NULL;
    ret->WHOHAS_num = 0;
    ret->want_hashes = NULL;
    ret->want_num = 0;
    ret->hashes_found = NULL;
    return ret;
}

void init_peer_client_info_in_pool(){
    peer_client_info_t *pc = (peer_client_info_t *)malloc(sizeof(peer_client_info_t));
    pc->peer_temp_state_for_GET = NULL;
    pc->hash_maps = NULL;
    pc->maps_num = 0;
    pc->GET_packet_sender = NULL;
    pc->sender_list = NULL;
    p->peer_client_info = pc;
}
void init_peer_server_info_in_pool(){
    peer_server_info_t *ps = (peer_server_info_t *)malloc(sizeof(peer_server_info_t));
    ps->transfer_head = NULL;
    p->peer_server_info = ps;
}

void insert_new_transfer_into_server_pool(chunk_hash *hash, bt_config_t *config, struct sockaddr_in to){
    peer_server_info_t *ps = p->peer_server_info;
    transfer_t *the_transfer;
    if(ps == NULL){
        fprintf(stderr, "peer don't have server info ,you need to malloc\n");
        init_peer_server_info_in_pool();
        ps = p->peer_server_info;
    }
    the_transfer = (transfer_t *)malloc(sizeof(transfer_t));
    init_transfer(the_transfer, hash, config, to);
    if(ps->transfer_head == NULL){
        ps->transfer_head = (Node *)malloc(sizeof(Node));
        ps->transfer_head->data = (void *)the_transfer;
        ps->transfer_head->next = NULL;
    } else{
        insert_node((&(ps->transfer_head)), (void *)the_transfer);
    }
}


void free_peer_temp_state_for_GET_in_pool(){
    peer_client_info_t *pc = p->peer_client_info;
    free_peer_temp_state_for_GET(pc->peer_temp_state_for_GET);
    pc->peer_temp_state_for_GET = NULL;
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
    p->my_hashes = NULL;
    p->chunk_hash_num = 0;
    p->peer_state = INITIAL_STATE;
    p->peer_client_info = NULL;
    p->peer_server_info = NULL;
    set_peer_pool_hashes(config);
}

void set_temp_state_for_peer_storage_pool(char *chunkfile){
    int packets_length = 0;
    contact_packet_t **packets = construct_WHOHAS_packet(chunkfile, &packets_length);
    peer_temp_state_for_GET_t *pt = init_peer_temp_state_for_GET();
    set_WHOHAS_cache(packets, packets_length, pt);
    set_want_hashes(chunkfile, pt);
    p->peer_client_info->peer_temp_state_for_GET = pt;
    p->peer_state = ASK_RESOURCE_LOCATION;
}

void set_peer_pool_hash_addr_map(){
    peer_client_info_t *pc = p->peer_client_info;
    peer_temp_state_for_GET_t *pt = pc->peer_temp_state_for_GET;
    pc->hash_maps = init_hash_addr_map(pt->want_num, pt->want_hashes);
    pc->maps_num = pt->want_num;
}

void set_peer_pool_GET_packet_sender(){
    peer_client_info_t *pc = p->peer_client_info;
    pc->GET_packet_sender = init_GET_packet_sender(pc->hash_maps, pc->maps_num);
}
void set_WHOHAS_cache(contact_packet_t **packets, int length, peer_temp_state_for_GET_t *pt){
    pt->WHOHAS_cache = packets;
    pt->WHOHAS_num = length;
    //p->peer_state = ASK_RESOURCE_LOCATION;
}



void send_WHOHAS_packet(int sockfd, bt_config_t *config){
    peer_client_info_t *pc = p->peer_client_info;
    peer_temp_state_for_GET_t *pt = pc->peer_temp_state_for_GET;
    if (pt->WHOHAS_cache == NULL){
        fprintf(stderr, "don't have WHOHAS packets to send\n");
        return;
    }

    fprintf(stderr, "have WHOHAS packets to send\n");
    for(int i = 0; i < pt->WHOHAS_num; i++){
        send_WHOHAS_to_peers(sockfd, config, *(pt->WHOHAS_cache + i));
    }
}
void send_GET_packet_in_peer_pool(int sock){
    peer_client_info_t *pc = p->peer_client_info;
    GET_packet_sender_t *sender = pc->GET_packet_sender;
    if(sender->cursor == sender->tunnel_num){
        fprintf(stderr, "have already send all tunnel(GET packet) and receive all the data\n");
        set_peer_state(FOUND_ALL_DATA);
        return;
    }
    send_GET_tunnel(sock, sender->tunnels + sender->cursor);
    //sender->cursor += 1;
}

void send_GET_packet_in_pool_sender_list(int sock){
    Node *n;
    GET_packet_sender_t *sender;
    peer_client_info_t *pc = p->peer_client_info;
    if(pc == NULL){
        fprintf(stderr, "err for peer_client_info\n");
    }
    Node *sender_list = pc->sender_list;
    if(sender_list == NULL){
        fprintf(stderr, "error  sender list for GET\n");
    }
    for(n = sender_list; n != NULL; n = n->next){
        sender = (GET_packet_sender_t *)n->data;
        if(sender->cursor == sender->tunnel_num){
            fprintf(stderr, "have already send all tunnel(GET packet) and receive all the data\n");
            continue;
        }
        send_GET_tunnel(sock, sender->tunnels + sender->cursor);
    }
}

void send_GET_packet_in_sender(GET_packet_sender_t *sender, int sock){
    if(sender->cursor == sender->tunnel_num){
        fprintf(stderr, "have already send all tunnel(GET packet) and receive all the data\n");
        return;
    }
    send_GET_tunnel(sock, sender->tunnels + sender->cursor);
}

void increase_to_another_GET_packet_tunnel(GET_packet_sender_t *sender){
    // peer_client_info_t *pc = p->peer_client_info;
    //GET_packet_sender_t *sender = pc->GET_packet_sender;
    sender->cursor += 1;
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
    peer_client_info_t *pc = p->peer_client_info;
    peer_temp_state_for_GET_t *pt = pc->peer_temp_state_for_GET;
    for(i = 0; i < pt->want_num; i++){
        if(map_this_hash_to_packet(pt->want_hashes[i], packet) && pt->hashes_found[i] == 0){
            pt->hashes_found[i] = 1;
            add_tunnel_to_sender_lst(pt->want_hashes+i, &from);
            //add_node_to_addr_map(pt->want_hashes[i], from, pc->hash_maps, pc->maps_num);
        }
    }
}



int found_all_resource_locations(){
    int i;
    peer_client_info_t *pc = p->peer_client_info;
    peer_temp_state_for_GET_t *pt = pc->peer_temp_state_for_GET;
    
    for(i = 0; i < pt->want_num; i++){
        if(pt->hashes_found[i] == 0)     return 0;
    }
    // p->peer_state = FOUND_ALL_RESOURCE;
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

int *peer_hashes_own_from(contact_packet_t *packet, int *len){
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
    *len = total_hash_num;
    int *suffices = (int *)malloc(sizeof(int)* total_hash_num);
    for(i = 0; i < total_hash_num; i++){
        suffices[i] = suffix_buffer[i];
    }
    return suffices;
}

chunk_hash *get_IHAVE_hashes_from_pool(int *suffices, int length){
    
    //int length = sizeof(suffices)/sizeof(int);
    chunk_hash *hashes = (chunk_hash *)malloc(length*sizeof(chunk_hash));
    for(int i = 0; i < length; i++){
        hashes[i] = p->my_hashes[suffices[i]];
    }
    // fprintf(stderr, "the hash length is %d\n", sizeof(hashes)/sizeof(chunk_hash));
    return hashes;
}

void handle_WHOHAS_packet(int sock, contact_packet_t *packet, struct sockaddr_in to, socklen_t tolen){
    int len;
    int *suffices = peer_hashes_own_from(packet, &len);
    if(suffices == NULL){
        fprintf(stderr, "i dont' have request files\n");
        return ;
    }
    p->peer_state = I_HAVE_RESOURCE;
    chunk_hash *hashes = get_IHAVE_hashes_from_pool(suffices, len);
    //int length = sizeof(suffices)/sizeof(int);
    contact_packet_t *IHAVE_packet = construct_IHAVE_packet(hashes, len);
    free(hashes);
    //print_WHOHAS_packet(IHAVE_packet);
    spiffy_sendto(sock, IHAVE_packet, IHAVE_packet->header.packet_len, 0, (struct sockaddr *)&to, tolen);
}

void print_peer_storage_pool(){
    fprintf(stderr, "WHOHAS packet:\n");
    peer_client_info_t *pc = p->peer_client_info;
    peer_temp_state_for_GET_t *pt = pc->peer_temp_state_for_GET;
    if(pt == NULL){
        fprintf(stderr, "don't have peer state for pool");
        return;
    }
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

int check_hash_peer_own(chunk_hash *hash){
    for(int i = 0; i < p->chunk_hash_num; i++){
        if(two_hash_equal(*hash, p->my_hashes[i])){
            return 1;
        }
    }
    return 0;
}

void free_hash_addr_map(hash_addr_map_t *maps, int maps_size){
    int i;
    for(i = 0; i < maps_size; i++){
        free_node_list((maps + i)->node_list);
    }
    free(maps);
}

void add_node_to_addr_map(chunk_hash hash, struct sockaddr_in s, hash_addr_map_t *maps, int maps_length){
    hash_addr_map_t *corresponding_map = get_map_by_hash(hash, maps, maps_length);
    if(corresponding_map != NULL){
        insert_addr_to_list(s, corresponding_map->node_list);
    }
}

void add_hash_to_peer_temp_state_for_GET_in_pool(chunk_hash *hash){
    peer_client_info_t *pc = p->peer_client_info;
    if(pc->peer_temp_state_for_GET == NULL){
        pc->peer_temp_state_for_GET = init_peer_temp_state_for_GET();
    }
    peer_temp_state_for_GET_t *pt = pc->peer_temp_state_for_GET;
    for(int i = 0; i < pt->want_num; i++){
        if(two_hash_equal(*hash, pt->want_hashes[i]))   return;
    }
    if(pt->want_num == 0){
        pt->want_hashes = (chunk_hash *)malloc(sizeof(chunk_hash));
        pt->WHOHAS_cache = (contact_packet_t **)malloc(sizeof(contact_packet_t *));
        pt->hashes_found = (int *)malloc(sizeof(int));
    } else{
        pt->want_hashes = (chunk_hash *)realloc(pt->want_hashes,sizeof(chunk_hash) * (pt->want_num + 1));
        pt->hashes_found = (int *)realloc(pt->hashes_found, sizeof(int *) * (pt->want_num + 1));
    }
    pt->want_hashes[pt->want_num] = *hash;
    pt->hashes_found[pt->want_num] = 0;
    pt->want_num += 1;
}

void set_WHOHAS_cache_in_pool(){
    peer_client_info_t *pc = p->peer_client_info;
    peer_temp_state_for_GET_t *pt = pc->peer_temp_state_for_GET;
    if(pt->want_num <= MAX_HASH_NUM){
        pt->WHOHAS_cache[pt->WHOHAS_num] = set_WHOHAS_packet_from_hash(pt->want_hashes, pt->want_num);
    }else {
        int num = pt->want_num;
        chunk_hash *cursor = pt->want_hashes;
        while(num > MAX_HASH_NUM){
            pt->WHOHAS_cache[pt->WHOHAS_num] = set_WHOHAS_packet_from_hash(cursor, MAX_HASH_NUM);
            num -= MAX_HASH_NUM;
            cursor = cursor +MAX_HASH_NUM;
            pt->WHOHAS_num += 1;
            pt->WHOHAS_cache = (contact_packet_t **)realloc(pt->WHOHAS_cache, sizeof(contact_packet_t *) * (pt->WHOHAS_num+1));
        }
        pt->WHOHAS_cache[pt->WHOHAS_num] = set_WHOHAS_packet_from_hash(cursor, num);
    }
    pt->WHOHAS_num += 1;
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
        print_node_list((maps+i)->node_list);
    }
}

void print_peer_hash_addr_map(){
    //peer_temp_state_for_GET_t *pt = p->peer_temp_state_for_GET;
    peer_client_info_t *pc = p->peer_client_info;
    print_hash_addr_map(pc->hash_maps, pc->maps_num);
}

//to issue sender_list in peer

void add_tunnel_to_sender_lst(chunk_hash *hash, struct sockaddr_in *addr){
    GET_packet_sender_t *sender;
    Node *node;
    GET_packet_tunnel_t *tunnel;
    peer_client_info_t *pc = p->peer_client_info;
    if(pc->sender_list == NULL){
        fprintf(stderr, "we need to create sender list\n");
        pc->sender_list = (Node *)malloc(sizeof(Node));
        pc->sender_list->data = (void *)build_GET_packet_sender(addr, hash);
        pc->sender_list->next = NULL;
        return;
    }
    node = node_find(pc->sender_list, cmp_sender_by_sockaddr, addr);
    if(node == NULL){
        fprintf(stderr, "we need to add sender to list\n");
        sender = build_GET_packet_sender(addr, hash);
        insert_node(&pc->sender_list, (void *)sender);
    } else{
        sender = (GET_packet_sender_t *)node->data;
        tunnel = find_corresponding_tunnel(sender, hash);
        if(tunnel == NULL){
            fprintf(stderr, "we need to add tunnel to sender\n");
            add_tunnel_to_sender(hash, addr, sender);
        }
    }
}


int check_sender_lst_all_received(Node *sender_list){
    Node *n;
    GET_packet_sender_t *s;
    for(n = sender_list ; n!= NULL; n = n->next){
        s = (GET_packet_sender_t *)n->data;
        if(s->cursor != s->tunnel_num)  return 0;
    }
    return 1;
}
