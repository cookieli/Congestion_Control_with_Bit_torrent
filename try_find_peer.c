#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "spiffy.h"
#include "bt_parse.h"
#include "utilities.h"
//#include "try_find_peer.h"
//#define _TEST_PACKET
void send_WHOHAS_to_peers(int sockfd, bt_config_t *config, contact_packet_t *who_has){
    bt_peer_t *peers = config->peers;
    short my_id = config->identity;
    while(peers != NULL){
        if(peers->id != my_id){
            //toaddr = &peers->addr;
            spiffy_sendto(sockfd, who_has, who_has->header.packet_len, 0, (struct sockaddr *)(&(peers->addr)), sizeof(peers->addr));
            fprintf(stderr, "i have sent WHOHAS message to peers\n");
        }
        peers = peers->next;
    }
}

int get_packet_type(header_t *header){
    return header->packet_type;
}

void construct_packet_header(header_t *header, char packet_type, uint16_t packet_len, uint32_t seq_num, uint32_t ack_num){
    header->magicnum = 15441;
    header->version = 1;
    header->packet_type = packet_type;
    header->header_len = 16;
    header->packet_len = packet_len;
    header->seq_num = seq_num;
    header->ack_num = ack_num;
}

void print_packet_header(header_t *header){
    //fprintf(stderr, " header length: %d\n", sizeof(header));
    fprintf(stderr, "magicnum: %d\n", header->magicnum);
    fprintf(stderr, "version : %d\n", header->version);
    fprintf(stderr, "packet_type: %d\n", header->packet_type);
    fprintf(stderr, "header_len: %d\n", header->header_len);
    fprintf(stderr, "packet_len: %d\n", header->packet_len);
    fprintf(stderr, "seq_num: %d\n", header->seq_num);
    fprintf(stderr, "ack_num: %d\n", header->ack_num);
}

contact_packet_t **construct_WHOHAS_packet(char *chunkfile, int *packets_length){
    int length = 0;
    chunk_hash **hashes = read_hash_from_chunk_files(chunkfile, &length);
    //fprintf(stderr, "size: %d\n", sizeof(contact_packet_t) + length * sizeof(chunk_hash));
    int packet_num = 1;
    if(length > MAX_HASH_NUM){
        if(length % MAX_HASH_NUM ==0)  packet_num = length / MAX_HASH_NUM;
        else                           packet_num = length / MAX_HASH_NUM + 1;
    }
    *packets_length = packet_num;
    contact_packet_t **packets = (contact_packet_t **)malloc(sizeof(contact_packet_t *) * packet_num);
    chunk_hash **hash_cursor = hashes;
    for(int i = 0; i < packet_num; i++){
        int hash_num = (length > MAX_HASH_NUM) ? MAX_HASH_NUM : length;
        packets[i] = set_WHOHAS_packet(hash_cursor, hash_num);
        hash_cursor += hash_num;
        length -= hash_num;
    }
    free_hashes(hashes, length);
    return packets;
}
void free_contact_packets(contact_packet_t **packets, int len){
    for(int i = 0; i < len; i++){
        free(packets[i]);
    }
    free(packets);
}
contact_packet_t *set_WHOHAS_packet(chunk_hash **hashes, int length){
    if(length > MAX_HASH_NUM){
        fprintf(stderr, "too many hashes\n");
        exit(-1);
    }
    contact_packet_t *WHOHAS_packet = (contact_packet_t *)malloc(sizeof(contact_packet_t) + length * sizeof(chunk_hash));
    uint16_t packet_len = 20 + 20*length;
    construct_packet_header(&WHOHAS_packet->header, 0, packet_len, 0, 0);
    for(int i = 0; i < length; i++){
        set_packet_hashes(hashes[i], &WHOHAS_packet->hashes[i]);
    }
    WHOHAS_packet->hashes_num = (char)length;
    return WHOHAS_packet;
}

contact_packet_t *construct_IHAVE_packet(chunk_hash *hashes, int length){
    contact_packet_t *IHAVE_packet = (contact_packet_t *)malloc(sizeof(contact_packet_t) + length * sizeof(chunk_hash));
    uint16_t packet_len = 20 + 20*length;
    construct_packet_header(&IHAVE_packet->header, 1, packet_len, 0, 0);
    for(int i = 0; i < length; i++){
        set_packet_hashes(&hashes[i], &IHAVE_packet->hashes[i]);
    }
    IHAVE_packet->hashes_num = (char)length;
    return IHAVE_packet;
}


void print_WHOHAS_packet(contact_packet_t *packet){
    //fprintf(stderr, "WHOHAS length: %d", sizeof(*packet));
    fprintf(stderr, "the packet type is: %d", packet->header.packet_type);
    print_packet_header(&packet->header);
    fprintf(stderr, "hashes num %d\n", packet->hashes_num);
    for(int i = 0; i < packet->hashes_num; i++){
        print_chunk_hash(packet->hashes[i]);
    }
}




void print_chunk_hash(chunk_hash h){
    for(int i = 0; i < SHA1_HASH_SIZE; i++){
        fprintf(stderr, "%d ", h.binary_hash[i]);
    }
    fprintf(stderr, "\n");
}
void set_packet_hashes(chunk_hash *from, chunk_hash *to){
    for(int i = 0; i < SHA1_HASH_SIZE; i++){
        to->binary_hash[i] = from->binary_hash[i];
    }
}

//we need some tool to parse WHOHAS packet

int len_of_WHOHAS_packet(contact_packet_t *packet){
    return packet->header.packet_len;
}

int hashes_num_of_WHOHAS_packet(contact_packet_t *packet){
    return packet->hashes_num;
}

int map_this_hash_to_packet(chunk_hash h1, contact_packet_t *packet){
    //int total_num = 0;
    int total_hash_num = packet->hashes_num;
    for(int i = 0; i < total_hash_num; i++){
        if(two_hash_equal(h1, packet->hashes[i]))  return 1;
    }
    return 0;
}

int two_hash_equal(chunk_hash h1, chunk_hash h2){
    for(int i = 0; i < SHA1_HASH_SIZE; i++){
        if(h1.binary_hash[i] != h2.binary_hash[i]){
            return 0;
        }
    }
    return 1;
}

#ifdef _TEST_PACKET
int main(int argc, char *argv[]){
    int length = 0;
    contact_packet_t **WHOHAS_packets = construct_WHOHAS_packet("/tmp/B.chunks", &length);
    fprintf(stderr, "%d\n", length);
    for(int i = 0; i < length; i++){
        print_WHOHAS_packet(WHOHAS_packets[i]);
        fprintf(stderr, "next\n");
    }
    //print_WHOHAS_packet(WHOHAS_packet);
    //fprintf(stderr, "%d ", 22/4);
    //contact_packet_t *packets = (contact_packet_t **)malloc(sizeof(contact_packet_t *) * 2);
    //fprintf(stderr, "num: %d\n", sizeof(packets) / sizeof(contact_packet_t *));
}
#endif
