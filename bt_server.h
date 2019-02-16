#ifndef _BT_SERVER_H_
#define _BT_SERVER_H_
#include "data_transfer.h"
#include "bt_parse.h"
#include <netinet/in.h>
void receive_GET_packet(int sockfd, GET_packet_t *packet, bt_config_t *config, struct sockaddr_in from);
void receive_ACK_packet(int sockfd, ACK_packet_t *packet, struct sockaddr_in from);
void handle_server_timeout(int sockfd);
#endif
