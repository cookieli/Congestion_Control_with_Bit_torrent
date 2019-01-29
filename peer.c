/*
 * peer.c
 *
 * Authors: Ed Bardsley <ebardsle+441@andrew.cmu.edu>,
 *          Dave Andersen
 * Class: 15-441 (Spring 2005)
 *
 * Skeleton for 15-441 Project 2.
 *
 */

#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "spiffy.h"
#include "bt_parse.h"
#include "input_buffer.h"
#include "try_find_peer.h"
#include "utilities.h"
#include "peer_storage.h"
#include "bt_client.h"
#include <time.h>
#include "data_transfer.h"
#include "chunk.h"
void peer_run(bt_config_t *config);

int main(int argc, char **argv) {
    debug = DEBUG_INIT;
    bt_config_t config;
    bt_init(&config, argc, argv);
    DPRINTF(DEBUG_INIT, "peer.c main beginning\n");
#ifdef TESTING
    config.identity = 1; // your group number here
    strcpy(config.chunk_file, "chunkfile");
    strcpy(config.has_chunk_file, "haschunks");
#endif
    bt_parse_command_line(&config);

#ifdef DEBUG
    if (debug & DEBUG_INIT) {
        bt_dump_config(&config);
    }
#endif
    print_peer_list(&config);
    init_peer_storage_pool(&config);
    //set_peer_pool_hashes(&config);
    peer_run(&config);
    return 0;
}


void process_inbound_udp(int sock) {
  #define BUFLEN 1500
  struct sockaddr_in from;
  socklen_t fromlen;
  char buf[BUFLEN];

  fromlen = sizeof(from);
  spiffy_recvfrom(sock, buf, BUFLEN, 0, (struct sockaddr *) &from, &fromlen);
  fprintf(stderr, "buf length: %d\n", strlen(buf));
  printf("PROCESS_INBOUND_UDP SKELETON -- replace!\n"
         "Incoming message from %s:%d\n%s\n\n",
	 inet_ntoa(from.sin_addr),
	 ntohs(from.sin_port),
	 buf);
#ifdef _TEST_PEER
  host_and_port *hap = convert_from_sockaddr(from);
  print_host_and_port(hap);
  struct sockaddr_in temp = convert_from_hap(hap);
  fprintf(stderr, "sock addr: %s:%d\n", inet_ntoa(temp.sin_addr), ntohs(temp.sin_port));
  struct sockaddr_in a = temp;
  fprintf(stderr, "sock addr: %s: %d\n", inet_ntoa(a.sin_addr), ntohs(temp.sin_port));
#endif
  header_t *header = (header_t *)buf;
  int type = get_packet_type(header);
  switch(type){
  case(0):// it is WHOHAS packet
      fprintf(stderr, "I have receive WHOHAS packet\n");
      //fprintf(stderr, "%d\n", ((contact_packet_t *)buf)->header.packet_type);
      print_WHOHAS_packet((contact_packet_t *)buf);
      handle_WHOHAS_packet(sock, (contact_packet_t *)buf, from, fromlen);
      break;
  case(1)://it is IHAVE packet
      fprintf(stderr, "I have receive IHAVE packet\n");
      print_WHOHAS_packet((contact_packet_t *)buf);
      handle_IHAVE_packet((contact_packet_t *)buf, from);
      if(get_peer_state() == FOUND_ALL_RESOURCE){
          fprintf(stderr, "------------peer_hashe_addr_map--------------\n");
          print_peer_hash_addr_map();
          fprintf(stderr, "------------peer_hashe_addr_map--------------\n");
          //NOW we need to construct GET packet from node-list
          set_peer_pool_GET_packet_sender();
          send_GET_packet_in_peer_pool(sock);
          fprintf(stderr, "------------peer_GET_packet_sender--------------\n");
          print_GET_packet_sender();
          fprintf(stderr, "------------peer_GET_packet_sender--------------\n");
      } else if(get_peer_state() == GET_ERROR_FOR_RESOURCE_LOCATION){
          send_GET_packet_in_peer_pool(sock);
          set_peer_state(FOUND_ALL_RESOURCE);
      }
      break;
  case(2)://it is GET packet
      fprintf(stderr, "I have receive GET packet\n");
      print_GET_packet((GET_packet_t *)buf);
      break;
  case(3)://it is DATA packet
      break;
  case(4)://it is ACK packet
      break;
  case(5)://it is DENIED packet
      break;
  }
  //fflush(stdout);
}


void process_get(char *chunkfile, char *outputfile) {
    printf("PROCESS GET SKELETON CODE CALLED.  Fill me in!  (%s, %s)\n",chunkfile, outputfile);
    init_peer_client_info_in_pool();
    set_temp_state_for_peer_storage_pool(chunkfile);
    set_peer_pool_hash_addr_map();
}

void handle_user_input(char *line, void *cbdata) {
  char chunkf[128], outf[128];

  bzero(chunkf, sizeof(chunkf));
  bzero(outf, sizeof(outf));

  if (sscanf(line, "GET %120s %120s", chunkf, outf)) {
    if (strlen(outf) > 0) {
      process_get(chunkf, outf);
    }
  } else if (strcmp(line, "quit") == 0){
      fprintf(stderr, "the peer need to quit\n");
      exit(0);
  }
}


void peer_run(bt_config_t *config) {
    int sock;
    struct sockaddr_in myaddr;
    fd_set readfds;
    struct user_iobuf *userbuf;
    struct timeval tv;
    if ((userbuf = create_userbuf()) == NULL) {
        perror("peer_run could not allocate userbuf");
        exit(-1);
    }
    if ((sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_IP)) == -1) {
        perror("peer_run could not create socket");
        exit(-1);
    }
    bzero(&myaddr, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    myaddr.sin_port = htons(config->myport);
    if (bind(sock, (struct sockaddr *) &myaddr, sizeof(myaddr)) == -1) {
        perror("peer_run could not bind socket");
        exit(-1);
    }
    spiffy_init(config->identity, (struct sockaddr *)&myaddr, sizeof(myaddr));
  //send_WHOHAS_to_peers(sock, config);
    while (1) {
        int nfds;
        FD_ZERO(&readfds);
        FD_SET(STDIN_FILENO, &readfds);
        FD_SET(sock, &readfds);

        tv.tv_sec = 5;
        tv.tv_usec = 0;
        nfds = select(sock+1, &readfds, NULL, NULL, &tv);
        if (nfds > 0) {//set this part  as state transfer
            if (FD_ISSET(sock, &readfds)) {
                process_inbound_udp(sock);
            }
            else if (FD_ISSET(STDIN_FILENO, &readfds)) {
                process_user_input(STDIN_FILENO, userbuf, handle_user_input,"Currently unused");
                send_WHOHAS_packet(sock, config);
            }
        }else if(nfds == 0){
            handle_client_timeout(sock, config);
        }
    }
}
