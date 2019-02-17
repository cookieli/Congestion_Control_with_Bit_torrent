#include "linkedlist.h"
#include <stdio.h>
int count_node(Node **head){
    Node *n;
    int count;

    if(head == NULL) return 0;

    count = 0;
    for(n = *head; n != NULL; n = n->next){
        count ++;
    }

    return count;
}

Node *node_insert_node(Node **head, Node *node){
    if(head == NULL){
        return NULL;
    }
    node->next = *head;
    *head = node;
    return node;
}

Node *insert_node(Node **head, void *data){
    Node *node;
    if((node = calloc(1, sizeof(Node))) == NULL){
        return NULL;
    }

    node->data = data;
    node->next = *head;
    *head = node;
    return node;
}

Node *node_find(Node *head, int(* compare)(const void *,const void *), void *key){
    
    while(head != NULL){
        if(compare(key, head->data)){
            return head;
        }
        head = head->next;
    }
    return NULL;
}

int node_delete(Node **head, Node *node, int(*remove_data)(void *data)){
    Node *temp;

    if(*head == NULL){
        return -1;
    }

    if(*head == node) {
        *head = node->next;
        if (remove_data != NULL){
            remove_data(node->data);
        }
        free(node);
        return 0;
    }

    temp = *head;
    while(temp != NULL && temp->next != node) {
        temp = temp->next;
    }

    if(temp != NULL){
        temp->next = node->next;

        if(remove_data != NULL){
            remove_data(node->data);
        }

        free(node);
        return 0;
    } else {
        return -1;
    }
}

size_t node_iterate_delete(Node **head,
                           int (*test)(void *data),
                           int(*remove_data)(void *data)){
    Node *temp, *next;
    size_t rm_count = 0;

    if(head == NULL || test == NULL){
        return -1;
    }

    for(temp = *head; temp != NULL; temp = next){
        next = temp->next;

        if(test(temp->data)){
            node_delete(head, temp, remove_data);
            rm_count++;
        }
    }

    return rm_count;
}

size_t node_length(Node *head){
    size_t length = 0;

    while(head != NULL){
        length++;
        head = head->next;
    }

    return length;
}

size_t node_delete_all(Node **head, int(remove_data)(void *)){
    Node *temp;
    size_t remove_count = 0;

    while(*head != NULL){
        temp = (*head)->next;
        if (remove_data != NULL){
            remove_data((*head)->data);
        }
        free(*head);
        *head = temp;
        remove_count++;
    }

    return remove_count;
}