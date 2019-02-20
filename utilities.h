#ifndef _UTILITIES_H_
#define _UTILITIES_H_
#include "try_find_peer.h"
uint8_t* to_binary_hash(char *ascii);
chunk_hash **read_hash_from_chunk_files(char *chunkfile, int *length);
void set_hash_value(chunk_hash *hash, char *ascii);
void free_hashes(chunk_hash **hashes, int len);
int find_hash_id_in_master_chunk_file(char *hexhash, char *master_chunk_filename);
int cmp_two_sock(struct sockaddr_in *a, struct sockaddr_in *b);
int find_hash_id_in_chunk_file(char *hexhash, char *chunk_file);
#define CHUNK_FILE_LINE_LEN 43
#endif