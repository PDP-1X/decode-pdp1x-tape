#include <stdio.h>

static const char *translation[256] = {
  " ", "1", "2", "3", "4", "5", "6", "7",
  "8", "9", 0, 0, 0, 0, 0, 0,
  "0", "/", "s", "t", "u", "v", "w", "x",
  "y", "z", 0, ",", 0, 0, "\t", 0,
  "\xC2\xB7", "j", "k", "l", "m", "n", "o", "p",
  "q", "r", 0, 0, "-", ")", "\xE2\x80\x94", "(",
  0, "a", "b", "c", "d", "e", "f", "g",
  "h", "i", 0, ".", 0, "\b", 0, "\n",

  " ", "\"", "'", "~", "\xE2\x8A\x83", "\xE2\x88\xA8", "\xE2\x88\xA7", "<",
  ">", "\xE2\x86\x91", 0, 0, 0, 0, 0, 0,
  "\xE2\x86\x92", "?", "S", "T", "U", "V", "W", "X",
  "Y", "Z", 0, "=", 0, 0, "\t", 0,
  "_", "J", "K", "L", "M", "N", "O", "P",
  "Q", "R", 0, 0, "+", "]", "|", "[",
  0, "A", "B", "C", "D", "E", "F", "G",
  "H", "I", 0, "\xC3\x97", 0, "\b", 0, "\n",


  "\xC2\xB7", "1", "2", "3", "4", "5", "6", "7",
  "8", "9", 0, 0, 0, 0, 0, 0,
  "0", "\xC2\xB7""\xCC\xB7", "s", "t", "u", "v", "w", "x",
  "y", "z", 0, ";", 0, 0, "\xC2\xB7""\t", 0,
  "\xC2\xB7", "j", "k", "\xC5\x80", "m", "n", "o", "p",
  "q", "r", 0, 0, "-", ")", "\xE2\x80\x94", "(",
  0, "a", "b", "c", "d", "e", "f", "g",
  "h", "i", 0, ":", 0, "\b", 0, "\xC2\xB7""\n",

  "\xC2\xB7", "\"", "'", "~", "\xE2\x8A\x83", "\xE2\x88\xA8", "\xE2\x88\xA7", "<",
  ">", "\xE2\x86\x91", 0, 0, 0, 0, 0, 0,
  "\xE2\x86\x92", "?", "S", "T", "U", "V", "W", "X",
  "Y", "Z", 0, "=", 0, 0, "\xC2\xB7""\t", 0,
  "\xC2\xB7""\xCC\xB2", "J", "K", "\xC4\xBF", "M", "N", "O", "P",
  "Q", "R", 0, 0, "+", "]", "\xC2\xB7""\xE2\x83\x92", "[",
  0, "A", "B", "C", "D", "E", "F", "G",
  "H", "I", 0, "\xC3\x97", 0, "\b", 0, "\xC2\xB7""\n"
};

#define LC 072
#define UC 074
#define BS 075
#define BLACK 034
#define RED 035
#define CENTERDOT 040

#define UPPER  0100
#define DOT    0200
#define STRIKE 0400

static int state;

static void output(int c)
{
  const char *s;

  switch (c & 0177) {
  case CENTERDOT:
    state |= DOT;
    break;
  default:
    s = translation[c];
    if (s)
      fputs(s, stdout);
    state &= ~DOT;
  }
}

static void input(int c)
{
  switch (c) {
  case UC:
    state |= UPPER;
    break;
  case LC:
    state &= ~UPPER;
    break;
  case BLACK:
  case RED:
    break;
  default:          
    output(c | state);
  }
}

int main(void)
{
  int c;

  state = 0;

  while ((c = getchar()) != EOF)
    input(c);
}
