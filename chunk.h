/*
 * bt_parse.h
 *
 * Initial Author: Debabrata Dash
 * Class: 15-441 (Spring 2005)
 *
 * Skeleton for 15-441 Project 2 chunk processing
 *
 */

#ifndef _CHUNK_H_
#define _CHUNK_H_
#include <stdio.h>
#include <inttypes.h>
#include "try_find_peer.h"
//#include "try_find_peer.h"
//#include "data_transfer.h"

#define BT_CHUNK_SIZE (512 * 1024)

#define ascii2hex(ascii,len,buf) hex2binary((ascii),(len),(buf))
#define hex2ascii(buf,len,ascii) binary2hex((buf),(len),(ascii))

#define BIN_HASH_SIZE 20
#define HEX_HASH_SIZE 40
#define DATA_CHUNK_SIZE (512*1024)
#define PACKET_DATA_SIZE 1000

#define PATH_LEN 255
#define FILE_LEN 255

#define MAX_SEQ_NUM  525//524+1:it means data chunk must send by 525 times

#ifdef __cplusplus
extern "C" {
#endif
    typedef struct chunk_s{
        int id;//the chunk id
        char hexhash[HEX_HASH_SIZE+1];
        uint8_t binhash[BIN_HASH_SIZE];
        uint8_t data[DATA_CHUNK_SIZE];
        long cursor;

        uint8_t seq_bits[MAX_SEQ_NUM];
    } chunk_t;

    //void create_output_file(char *output_file, GET_packet_sender_t *sender);
    void binhash_copy(uint8_t *from, uint8_t *to);
    void read_chunk_data_by_id(char *filename, int id, uint8_t *data);
    chunk_t load_chunk_from_tar(chunk_hash *h, bt_config_t *config);
    void print_chunk(chunk_t *t);
    int compare_two_hex_hashes(char *a, char *b);

    int check_chunk_with_bin_hash(chunk_t c, uint8_t *bin);
    /* Returns the number of chunks created, return -1 on error */
    int make_chunks(FILE *fp, uint8_t **chunk_hashes);  

    /* returns the sha hash of the string */
    void shahash(uint8_t *chr, int len, uint8_t *target);

    /* converts a hex string to ascii */
    void binary2hex(uint8_t *buf, int len, char *ascii);

    /* converts an ascii to hex */
    void hex2binary(char *hex, int len, uint8_t*buf);
#ifdef __cplusplus
}
#endif

#endif /* _CHUNK_H_ */
