#ifndef _LINKEDLIST_H_
#define _LINKEDLIST_H_

#include <stdlib.h>

typedef struct node {
    struct node *next;
    void *data;
} Node;

int count_node(Node **head);

Node *node_insert_node(Node **head, Node *node);

Node *insert_node(Node **head, void *data);

Node *node_find(Node *head, int(* compare)(const void *, const void *), void *key);

int node_delete(Node **head, Node *node, int(*remove_data)(void *data));

size_t node_iterate_delete(Node **head, int(*test)(void *data), int(*remove_data)(void *data));

size_t node_length(Node *head);

size_t node_delete_all(Node **head, int(remove_data)(void *));
#endif