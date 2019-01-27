#ifndef _NODE_LIST_H_
#define _NODE_LIST_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

typedef struct node_s {
    struct sockaddr_in addr;
    struct node_s *next;
} node_t;

//extern node_t *node_list;

node_t *init_node_list();
void free_node_list(node_t *node_list);
node_t *init_node(struct sockaddr_in s);
void insert_addr_to_list(struct sockaddr_in s, node_t *node_list);
void insert_node_to_list(node_t *n, node_t *node_list);
int compare_two_addr(struct sockaddr_in s1, struct sockaddr_in s2);
int is_addr_in_list(struct sockaddr_in s1, node_t *node_list);
void print_node_list(node_t *node_list);
struct sockaddr_in get_addr_from_node_list(node_t *lst);

#endif /*_NODE_LIST_H_*/