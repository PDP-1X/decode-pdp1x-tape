CFLAGS=-g

all: decode-pdp1x-tape

decode-pdp1x-tape: decode-pdp1x-tape.o
	$(CC) $(CFLAGS) $< -o $@
