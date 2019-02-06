#ifndef _BT_CLIENT_H_
#define _BT_CLIENT_H_
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
void handle_IHAVE_packet(contact_packet_t *packet, struct sockaddr_in from);
void handle_client_timeout(int sockfd, bt_config_t *config);
void receive_DATA_packet(int sockfd, DATA_packet_t *packet, bt_config_t *config, struct sockaddr_in from);
void send_ACK_packet(int sock, int ack_num, struct sockaddr_in from);
uint32_t find_biggest_ack_num_in_window(flow_window_t win, chunk_t *chunk);
#endif