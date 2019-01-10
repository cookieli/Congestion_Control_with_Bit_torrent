#include "node_list.h"

#include <stdio.h>
#include <stdlib.h>
//#define _TEST_NODE_LIST
node_t *init_node_list(){
    node_t *node_list = (node_t *)malloc(sizeof(node_t));
    node_list->next = NULL;
    return node_list;
}

node_t *init_node(struct sockaddr_in s){
    node_t *node = (node_t *)malloc(sizeof(node_t));
    node->addr = s;
    node->next = NULL;
    return node;
}

void insert_addr_to_list(struct sockaddr_in s, node_t *node_list){
    node_t *n = init_node(s);
    insert_node_to_list(n, node_list);
}

void insert_node_to_list(node_t *n, node_t *node_list){
    if(is_addr_in_list(n->addr, node_list))   return;
    n->next = node_list->next;
    node_list->next = n;
}

int compare_two_addr(struct sockaddr_in s1, struct sockaddr_in s2){
    return ((s1.sin_addr.s_addr == s2.sin_addr.s_addr) &&(s1.sin_port == s2.sin_port) && (s1.sin_family  == s2.sin_family));
}

int is_addr_in_list(struct sockaddr_in s1, node_t *node_list){
    node_t *cursor = node_list->next;
    while(cursor != NULL) {
        if(compare_two_addr(cursor->addr, s1))  return 1;
        cursor = cursor->next;
    }
    return 0;
}

void print_node_list(node_t *node_list){
    node_t *cursor = node_list->next;
    fprintf(stderr, "node addr list:\n");
    while(cursor != NULL){
        struct sockaddr_in addr = cursor->addr;
        fprintf(stderr, "node addr:%s: %d\n", inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));
        cursor = cursor->next;
    }
}

#ifdef _TEST_NODE_LIST
int main(int argc, char **argv){
    struct sockaddr_in sa1;
    inet_pton(AF_INET, "127.0.0.1", &(sa1.sin_addr));
    sa1.sin_family = AF_INET;
    sa1.sin_port = htons(1111);
    node_t *node_list = init_node_list();
    insert_addr_to_list(sa1, node_list);
    struct sockaddr_in sa2;
    inet_pton(AF_INET, "127.0.0.1", &(sa2.sin_addr));
    sa2.sin_family = AF_INET;
    sa2.sin_port = htons(2222);
    insert_addr_to_list(sa2, node_list);
    struct sockaddr_in sa3;
    inet_pton(AF_INET, "127.0.0.1", &(sa3.sin_addr));
    sa3.sin_port = htons(3333);
    sa3.sin_family = AF_INET;
    insert_addr_to_list(sa3, node_list);
    print_node_list(node_list);
}
#endif