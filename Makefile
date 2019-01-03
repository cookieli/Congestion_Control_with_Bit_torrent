# Some variables
CC 		= gcc
CFLAGS		= -g -Wall -DDEBUG
LDFLAGS		= -lm
TESTDEFS	= -DTESTING			# comment this out to disable debugging code
OBJS		= peer.o bt_parse.o spiffy.o debug.o input_buffer.o chunk.o sha.o try_find_peer.o utilities.o peer_storage.o
MK_CHUNK_OBJS   = make_chunks.o chunk.o sha.o

BINS            = peer make-chunks
TESTBINS        = test_debug test_input_buffer

# Implicit .o target
.c.o:
	$(CC) $(TESTDEFS) -c $(CFLAGS) $<

# Explit build and testing targets

all: ${BINS} ${TESTBINS}

run: peer_run
	./peer_run

test: peer_test
	./peer_test

peer: $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $@ $(LDFLAGS)

make-chunks: $(MK_CHUNK_OBJS)
	$(CC) $(CFLAGS) $(MK_CHUNK_OBJS) -o $@ $(LDFLAGS)

utilities: utilities.o chunk.o sha.o
try_find_peer: try_find_peer.o utilities.o chunk.o sha.o spiffy.o

clean:
	rm -f *.o $(BINS) $(TESTBINS) utilities try_find_peer

bt_parse.c: bt_parse.h

try_find_peer.c: try_find_peer.h

peer_storage.c: peer_storage.h

# The debugging utility code
debug-text.h: debug.h
	./debugparse.pl < debug.h > debug-text.h

test_debug.o: debug.c debug-text.h
	${CC} debug.c ${INCLUDES} ${CFLAGS} -c -D_TEST_DEBUG_ -o $@

#test_debug: test_debug.o

test_input_buffer:  test_input_buffer.o input_buffer.o
