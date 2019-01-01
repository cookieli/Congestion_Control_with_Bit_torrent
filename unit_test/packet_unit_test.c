#include <stdio.h>
#include "../try_find_peer.h"
#include <assert.h>

int main(int argc, char *argv[]){
    assert(sizeof(header_t) == 16);
    printf("%ld\n",sizeof(contact_packet_t));
}