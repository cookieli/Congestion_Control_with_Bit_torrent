#ifndef _TYPES_H_
#define _TYPES_H_

#include "try_find_peer.h"
#include "node_list.h"

typedef struct hash_addr_map_s{
    chunk_hash hash;
    node_t *node_list;
} hash_addr_map_t;

#endif