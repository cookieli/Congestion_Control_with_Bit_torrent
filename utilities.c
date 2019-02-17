#include <stdio.h>
#include <stdlib.h>
#include "chunk.h"
#include "sha.h"
#include <unistd.h>
#include <string.h>
#include "utilities.h"

//#include "try_find_peer.h"
//#define _TEST_UTILITIES
//#define CHUNK_FILE_LINE_LEN 43
chunk_hash **read_hash_from_chunk_files(char *chunkfile, int *length){
    FILE *fd;
    char buf[CHUNK_FILE_LINE_LEN + 1];
    fd = fopen(chunkfile, "rb");
    if(fd == NULL){
        fprintf(stderr, "can't open file %s\n", chunkfile);
        perror("fopen");
        exit(-1);
    }
    //to find how many chunks we have
    fseek(fd, -CHUNK_FILE_LINE_LEN, SEEK_END);
    ssize_t len = fread(buf, 1,  CHUNK_FILE_LINE_LEN,fd);
    buf[len] = '\0';
#ifdef _TEST_UTILITIES
    fprintf(stderr, "buf: %s\n", buf);
    fprintf(stderr, "hashes: %s\n", &buf[2]);
    to_binary_hash(&buf[2]);
#endif
    int max_id = buf[0] - '0';
    fseek(fd, 0, SEEK_SET);
    memset(buf, 0 ,CHUNK_FILE_LINE_LEN + 1);
    len = fread(buf, 1, CHUNK_FILE_LINE_LEN, fd);
    buf[len] = '\0';
    int min_id = buf[0] - '0';
;    max_id = max_id - min_id;
#ifdef _TEST_UTILITIES
    fprintf(stderr, "%d\n", max_id);
#endif
    //chunk_hash *hashes[max_id+1];
    chunk_hash **hashes;
    hashes = (chunk_hash **)malloc(sizeof(chunk_hash *) * (max_id + 1));
    *length = max_id + 1;
    for(int i = 0; i < max_id + 1; i++){
        hashes[i] = (chunk_hash *)malloc(sizeof(chunk_hash));
    }
    //set_hash_value(hashes[0], &buf[2]);
    fseek(fd, 0, SEEK_SET);
    for(int i = 0; i <= max_id; i++){
        memset(buf, 0, CHUNK_FILE_LINE_LEN + 1);
        ssize_t len = fread(buf, 1, CHUNK_FILE_LINE_LEN, fd);
        buf[len] = '\0';
        set_hash_value(hashes[i], &buf[2]);
    }
#ifdef _TEST_UTILITIES
    fprintf(stderr, "test for hash\n");
    //fprintf(stderr, "len: %d\n", sizeof(hashes) / sizeof(chunk_hash *));
    for(int m = 0; m <= max_id; m++){
        for(int n = 0; n < SHA1_HASH_SIZE; n++){
            fprintf(stderr, "%d ", hashes[m]->binary_hash[n]);
        }
        fprintf(stderr, "\n");
    }
#endif
    return hashes;
}
void free_hashes(chunk_hash **hashes, int len){
    for(int i = 0; i < len; i++){
        free(hashes[i]);
    }
    free(hashes);
}
void set_hash_value(chunk_hash *hash, char *ascii){
    uint8_t *temp = to_binary_hash(ascii);
    for(int i = 0; i < SHA1_HASH_SIZE; i++){
        hash->binary_hash[i] = temp[i];
    }
    free(temp);
}
uint8_t* to_binary_hash(char *ascii){
    uint8_t *hash = (uint8_t *)malloc((SHA1_HASH_SIZE+1) * sizeof(uint8_t));
    int len = SHA1_HASH_SIZE*2 + 1;
    hex2binary(ascii, strlen(ascii), hash);
#ifdef _TEST_UTILITIES
    char ascii2[len];
    for(int i = 0; i < SHA1_HASH_SIZE; i++){
        fprintf(stderr, "%d ", hash[i]);
    }
    fprintf(stderr, "\n");
    binary2hex(hash, SHA1_HASH_SIZE, ascii2);
    fprintf(stderr, "%s\n", ascii2);
#endif
    return hash;
}

int find_hash_id_in_master_chunk_file(char *hexhash, char *master_chunk_filename){
    FILE *fd;
    int id;
    char buf[FILE_LEN];
    fd = fopen(master_chunk_filename, "rb");
    if(fd == NULL){
        fprintf(stderr, "can't open file %s\n", master_chunk_filename);
        perror("fopen");
        exit(-1);
    }
    fgets(buf, FILE_LEN, fd);
    fgets(buf, FILE_LEN, fd);
    memset(buf, 0, FILE_LEN);
    while(fgets(buf, FILE_LEN, fd) != NULL){
        buf[CHUNK_FILE_LINE_LEN]  = '\0';
        if(!compare_two_hex_hashes(hexhash, &buf[2])){
            id = buf[0] - '0';
            return id;
        }
    }
    return -1;
}
int cmp_two_sock(struct sockaddr_in *a, struct sockaddr_in *b){
    if((a->sin_family == b->sin_family) && (a->sin_port == b->sin_port) && (a->sin_addr.s_addr == b->sin_addr.s_addr) )    return 1;
    return 0;
}
#ifdef _TEST_UTILITIES
int main(int argc, char *argv[]){
    int len = 0;
    chunk_hash **hashes = read_hash_from_chunk_files("/tmp/B.haschunks", &len);
    fprintf(stderr, "main: %d\n", len);
    free_hashes(hashes, len);
}
#endif