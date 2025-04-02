#include <stdio.h>

#define NUL 0

static void punch_blank(FILE *f, int n, int c)
{
  int i;
  for (i = 0; i < n; i++)
    fputc(c, f);
}

static int character(int c)
{
  if (c == EOF)
    c = 0;
  return 0200 | (c & 077);
}

static void punch_word(FILE *f, int c1, int c2, int c3)
{
  fputc(character(c1), f);
  fputc(character(c2), f);
  fputc(character(c3), f);
}

static void convert_to_binary(FILE *input, FILE *output)
{
  int c1, c2, c3;
  do {
    c1 = fgetc(input);
    if (c1 == EOF)
      return;
    c2 = fgetc(input);
    c3 = fgetc(input);
    punch_word(output, c1, c2, c3);
  } while (c3 != EOF);
}

int main(void)
{
  int leader = 100, trailer = 100;
  punch_blank(stdout, leader, NUL);
  convert_to_binary(stdin, stdout);
  punch_blank(stdout, trailer, NUL);
}
