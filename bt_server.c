#include <stdio.h>
#include <netinet/in.h>
#include "data_transfer.h"
#include "peer_storage.h"
#include "bt_server.h"
void receive_GET_packet(int sockfd, GET_packet_t *packet, bt_config_t *config, struct sockaddr_in from){
    //DATA_packet_t *d;
    transfer_t *t;
    if(get_peer_state() == I_HAVE_RESOURCE){
        if(check_hash_peer_own(&packet->hash)){
            t = create_new_transfer_in_server_pool(&packet->hash, config, from);
            send_DATA_packet_from_transfer(sockfd, t, from);
        } else{
            fprintf(stderr, "this GET packet send wrong place\n");
        }
    }
}

