#include <stdio.h>
#include "peer_storage.h"
#include "bt_client.h"
#include "bt_parse.h"
void handle_IHAVE_packet(contact_packet_t *packet, struct sockaddr_in from){
    if(get_peer_state() == ASK_RESOURCE_LOCATION){
        check_IHAVE_packet(packet, from);
        if(found_all_resource_locations()){
            set_peer_state(FOUND_ALL_RESOURCE);
            fprintf(stderr, "i have found all resource locations\n");
            free_peer_temp_state_for_GET_in_pool();
        }
    }
}

void handle_client_timeout(int sockfd, bt_config_t *config){
    if(get_peer_state() == ASK_RESOURCE_LOCATION){
        send_WHOHAS_packet(sockfd, config);
    }
}