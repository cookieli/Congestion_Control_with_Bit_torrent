#ifndef _UTILITIES_H_
#define _UTILITIES_H_
#include "try_find_peer.h"
uint8_t* to_binary_hash(char *ascii);
chunk_hash **read_hash_from_chunk_files(char *chunkfile, int *length);
void set_hash_value(chunk_hash *hash, char *ascii);
void free_hashes(chunk_hash **hashes, int len);
#define CHUNK_FILE_LINE_LEN 43
#endif