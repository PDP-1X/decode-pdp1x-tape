CFLAGS=-g

all: decode-pdp1x-tape convert-fiodec binary-paper-tape

decode-pdp1x-tape: decode-pdp1x-tape.o
	$(CC) $(CFLAGS) $< -o $@

convert-fiodec: convert-fiodec.o
	$(CC) $(CFLAGS) $< -o $@

binary-paper-tape: binary-paper-tape.o
	$(CC) $(CFLAGS) $< -o $@
