CFLAGS=-g

all: decode-pdp1x-tape convert-fiodec

decode-pdp1x-tape: decode-pdp1x-tape.o
	$(CC) $(CFLAGS) $< -o $@

convert-fiodec: convert-fiodec.o
	$(CC) $(CFLAGS) $< -o $@
