#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "spiffy.h"
#include "bt_parse.h"

#include "try_find_peer.h"

void send_WHOHAS_to_peers(int sockfd, bt_config_t *config){
    //struct sockaddr_in *toaddr;
    char *who_has = "Hello\n";
    bt_peer_t *peers = config->peers;
    short my_id = config->identity;
    //bt_peer_t *my_peer = bt_peer_info(config, my_id);
    //bzero(&myaddr, sizeof(myaddr));
    
    while(peers != NULL){
        if(peers->id != my_id){
            //toaddr = &peers->addr;
            spiffy_sendto(sockfd, who_has, strlen(who_has), 0, (struct sockaddr *)(&(peers->addr)), sizeof(peers->addr));
            fprintf(stderr, "i have sent WHOHAS message to peers\n");
        }
        peers = peers->next;
    }
}

