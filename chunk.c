#include "sha.h"
#include "chunk.h"
#include <ctype.h>
#include <assert.h>
#include <stdlib.h> // for malloc
#include <string.h> // for memset
#include "try_find_peer.h"
#include "data_transfer.h"
#include "utilities.h"
#include "bt_parse.h"
/**
 * fp -- the file pointer you want to chunkify.
 * chunk_hashes -- The chunks are stored at this locations.
 */
/* Returns the number of chunks created */
int make_chunks(FILE *fp, uint8_t *chunk_hashes[]) {
	//allocate a big buffer for the chunks from the file.
	uint8_t *buffer = (uint8_t *) malloc(BT_CHUNK_SIZE);
	int numchunks = 0;
	int numbytes = 0;

	// read the bytes from the file and fill in the chunk_hashes
	while((numbytes = fread(buffer, sizeof(uint8_t), BT_CHUNK_SIZE, fp)) > 0 ) {
		shahash(buffer, numbytes, chunk_hashes[numchunks++]);
	}

	return numchunks;
}

int check_chunk_with_bin_hash(chunk_t c, uint8_t *bin){
    for(int i = 0; i < BIN_HASH_SIZE; i++){
        if(c.binhash[i] != bin[i])  return -1;
    }
    return 0;
}
chunk_t load_chunk_from_tar(chunk_hash *h, bt_config_t *config){
    chunk_t chunk;
    char buf[CHUNK_FILE_LINE_LEN];
    char master_chunk_filename[PATH_LEN];
    char line[FILE_LEN];
    FILE *hc;
    FILE *master_chunk_f;
    chunk.cursor = 0;
    binhash_copy(h->binary_hash, chunk.binhash);
    binary2hex(chunk.binhash, BIN_HASH_SIZE, chunk.hexhash);
    hc = fopen(config->has_chunk_file, "rb");
    ssize_t len; //= fread(buf, 1, CHUNK_FILE_LINE_LEN, hc);
    //buf[len] = '\0';
    while((len = fread(buf, 1, CHUNK_FILE_LINE_LEN, hc)) == CHUNK_FILE_LINE_LEN){
        buf[len] = '\0';
        fprintf(stderr, "%s\n", &buf[2]);
        fprintf(stderr, "%s\n", chunk.hexhash);
        fprintf(stderr, "print the buf: \n");
        //for(int i = 0; i < CHUNK_FILE_LINE_LEN; )
        if(!compare_two_hex_hashes(&buf[2], chunk.hexhash)){
            fprintf(stderr, "the chunk id: %c\n", buf[0]);
            chunk.id = buf[0] - '0';
            break;
        }
    }
    fclose(hc);
    master_chunk_f = fopen(config->chunk_file, "r");
    if(fgets(line, FILE_LEN, master_chunk_f) != NULL){
        sscanf(line, "File: %s\n", master_chunk_filename);
    }
    fclose(master_chunk_f);
    read_chunk_data_by_id(master_chunk_filename, chunk.id, chunk.data);
    chunk.cursor = DATA_CHUNK_SIZE - 1;
    memset(chunk.seq_bits, 0, MAX_SEQ_NUM);
    return chunk;
}


void read_chunk_data_by_id(char *filename, int id, uint8_t *data){
    FILE *fp = fopen(filename, "r");
    fseek(fp, DATA_CHUNK_SIZE*id, SEEK_SET);
    fread((char *)data, 1, DATA_CHUNK_SIZE, fp);
    fclose(fp);
}
int compare_two_hex_hashes(char *a, char *b){
    for(int i = 0; i < HEX_HASH_SIZE; i++){
        if(a[i] != b[i]){
            return -1;
        }
    }
    return 0;
}
void binhash_copy(uint8_t *from, uint8_t *to){
    int i;
    for(i =0 ; i < BIN_HASH_SIZE; i++){
        to[i] = from[i];
    }
}

void print_chunk(chunk_t *t){
    fprintf(stderr, "id: %d\n", t->id);
    for(int i = 0; i < BIN_HASH_SIZE; i++){
        fprintf(stderr, "%d ", t->binhash[i]);
    }
    fprintf(stderr, "\n");
    for(int i = 0; i < HEX_HASH_SIZE; i++){
        fprintf(stderr, "%c ", t->hexhash[i]);
    }
    fprintf(stderr, "\n");
    fprintf(stderr, "data: ");
    // for(int i = 0; i < DATA_CHUNK_SIZE; i++){
    //    fprintf(stderr, " %d", t->data[i]);
    //}
    //fprintf(stderr, "the chunk current position: %d\n", t->cursor);
}
/**
 * str -- is the data you want to hash
 * len -- the length of the data you want to hash.
 * hash -- the target where the function stores the hash. The length of the
 *         array should be SHA1_HASH_SIZE.
 */
void shahash(uint8_t *str, int len, uint8_t *hash) {
	SHA1Context sha;

	// init the context.
	SHA1Init(&sha);

	// update the hashes.
	// this can be used multiple times to add more
	// data to hash.
	SHA1Update(&sha, str, len);

	// finalize the hash and store in the target.
	SHA1Final(&sha, hash);

	// A little bit of paranoia.
	memset(&sha, 0, sizeof(sha));
}

/**
 * converts the binary char string str to ascii format. the length of 
 * ascii should be 2 times that of str
 */
void binary2hex(uint8_t *buf, int len, char *hex) {
	int i=0;
	for(i=0;i<len;i++) {
		sprintf(hex+(i*2), "%.2x", buf[i]);
	}
	hex[len*2] = 0;
}
  
/**
 *Ascii to hex conversion routine
 */
static uint8_t _hex2binary(char hex)
{
     hex = toupper(hex);
     uint8_t c = ((hex <= '9') ? (hex - '0') : (hex - ('A' - 0x0A)));
     return c;
}

/**
 * converts the ascii character string in "ascii" to binary string in "buf"
 * the length of buf should be atleast len / 2
 */
void hex2binary(char *hex, int len, uint8_t*buf) {
	int i = 0;
	for(i=0;i<len;i+=2) {
		buf[i/2] = 	_hex2binary(hex[i]) << 4 
				| _hex2binary(hex[i+1]);
	}
}

#ifdef _TEST_CHUNK_C_
int main(int argc, char *argv[]) {
	uint8_t *test = "dash";
	uint8_t hash[SHA1_HASH_SIZE], hash1[SHA1_HASH_SIZE];
	char ascii[SHA1_HASH_SIZE*2+1];

	shahash(test,4,hash);
	
	binary2hex(hash,SHA1_HASH_SIZE,ascii);

	printf("%s\n",ascii);

	assert(strlen(ascii) == 40);

	hex2binary(ascii, strlen(ascii), hash1);

	binary2hex(hash1,SHA1_HASH_SIZE,ascii);

	printf("%s\n",ascii);

    //to test load chunk file
    //bt_config_t config;
    //config.chunk_file="example/all.masterchunks";
    //config.has_chunk_file = "example/b.haschunks";
    //chunk_t c = load_chunk_from_tar()
}
#endif //_TEST_CHUNK_C_
