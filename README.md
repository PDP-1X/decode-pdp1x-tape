Decodes tape images from the PDP-1X.  This works with both micotapes
and magtapes found here:

http://bitsavers.org/bits/MIT/rle_pdp1x/

Usage: pass "-u" for microtape, or "-m" for magtape.  Pass "-t" to
produce a file listing, or "-w" to extract tape file contents.  Files
are written as three sixbit bytes from each 18-bit word; i.e. much
like a paper tape.  Pipe the image to stdin.

There's another program to convert files to Unicode text.
