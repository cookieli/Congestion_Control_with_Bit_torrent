#include "bt_server.h"
void receive_GET_packet(GET_packet_t *packet, bt_config_t *config, struct sockaddr_in from){
    if(get_peer_state() == I_HAVE_RESOURCE){
        if(check_hash_peer_own(&packet->hash)){
            load_one_chunk_in_pool_server_side_by_hash(&packet->hash, config);
        } else{
            
        }
    }
}