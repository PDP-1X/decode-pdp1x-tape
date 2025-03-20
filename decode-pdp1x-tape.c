#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

static uint8_t buffer[100000];
static uint32_t image[100 * 2048];
static uint32_t *next;
static uint32_t dir[3*256];
static uint32_t ptb[3*256];
static int depth;
static int files;
static int uc_state = 0;
static int file_no = -1;
static int block_max = 01102-1;

char uctbl[64]={
   ' ', '"', '\'','~', '.', '.', '.', '<',    // 000
   '>', '^',  0,   0,   0,   0,   0,   0,     // 010
   ' ', '?', 'S', 'T', 'U', 'V', 'W', 'X',     // 020
   'Y', 'Z', ' ', '=',  0,   0, '\t', ' ',    // 030
   '-', 'J', 'K', 'L', 'M', 'N', 'O', 'P',     // 040
   'Q', 'R', '!', '$', '+', ']', '|', '[',     // 050
    0,  'A', 'B', 'C', 'D', 'E', 'F', 'G',     // 060
   'H', 'I',  0, 0xd7,  0,   8,   0, '\n' };  // 070

char lctbl[64]={
   ' ', '1', '2', '3', '4', '5', '6', '7',     // 000
   '8', '9',  0,   0,   0,   0,   0,   0,     // 010
   '0', '/', 's', 't', 'u', 'v', 'w', 'x',     // 020
   'y', 'z', ' ', ',',  0,   0, '\t', ' ',    // 030
   '_', 'j', 'k', 'l', 'm', 'n', 'o', 'p',     // 040
   'q', 'r', '!', '$', '-', ')',  0,  '(',     // 050
    0,  'a', 'b', 'c', 'd', 'e', 'f', 'g',     // 060
   'h', 'i',  0,  '.',  0,   8,   0, '\n' };  // 070

#define LC 072
#define UC 074
#define BS 075
#define BLACK 034
#define RED 035

static const char *insn[] =
{
  "*",   "*",     "and", "and i", "ior", "ior i", "xor",     "xor i",
  "xct", "xct i", "lxr", "lxr i", "jdp", "jdp i", "cal",     "jda",
  "lac", "lac i", "lio", "lio i", "dac", "dac i", "dap",     "dap i",
  "dip", "dip i", "dio", "dio i", "dzm", "dzm i", "adm",     "adm i",
  "add", "add i", "sub", "sub i", "idx", "idx i", "isp",     "isp i",
  "sad", "sad i", "sas", "sas i", "mul", "mul i", "div",     "div i",
  "jmp", "jmp i", "jsp", "jsp i", "skp", "skp i", "(shift)", "(shift)",
  "law", "law i", "iot", "iot",   "ivk", "*",     "opr",     "opr"
};

static void fatal(const char *message)
{
  fprintf(stderr, "%s\n", message);
  exit(1);
}

static void fiodec_char(uint8_t x, int (*output)(int))
{
  x &= 077;
  switch (x) {
#if 0
  case 077:
    return;
#endif
  case UC:
    uc_state = 1;
    return;
  case LC:
    uc_state = 0;
    return;
  case RED: case BLACK:
    return;
  default:          
    if (uc_state)
      output(uctbl[x]);
    else
      output(lctbl[x]);
  }
}

static void fiodec_word(uint32_t word, int (*output)(int))
{
  fiodec_char(word >> 12, output);
  fiodec_char(word >>  6, output);
  fiodec_char(word >>  0, output);
}

static void dasm(uint32_t word)
{
  printf("%s %04o", insn[word >> 12], word & 07777);
}

static uint32_t reclen(FILE *f)
{
  uint32_t x1 = fgetc(f);
  uint32_t x2 = fgetc(f);
  uint32_t x3 = fgetc(f);
  uint32_t x4 = fgetc(f);
  if (feof(f))
    fatal("Physical end of medium.");
  return x1 | (x2 << 8) | (x3 << 16) | (x4 << 24);
}

static uint32_t record(FILE *f)
{
  uint32_t n1, n2, n3;
  size_t n4;

  n1 = reclen(f);
  if (n1 == 0)
    return 0;
  if (n1 & 0x80000000)
    fprintf(stderr, "Error in tape record.\n");
  n2 = n1 & 0xFFFFFF;
  if (n2 != 6*1024)
    fprintf(stderr, "Record %u %x\n", n2, n1);
  if (n2 > sizeof buffer)
    fatal("Record too large.");
  n4 = fread(buffer, 1, n2, f);
  if (n4 != n2)
    fatal("Incomplete read.");

  int i;
  for (i = 0; i < n2; i++)
    if (buffer[i] & 0xC0)
      fatal("Bletcherous magtape frame.");

  if (n2 & 1)
    fgetc (f);
  n3 = reclen(f);
  if (n1 != n3)
    fatal("Record size mismatch.");
  return n3;
}

static uint32_t logical(uint32_t n)
{
  if (n & 1)
    return n >> 1;
  else
    return block_max - (n >> 1);
}

static uint32_t physical(uint32_t n)
{
  if (n < (block_max+1)/2)
    return 1 + 2*n;
  else
    return block_max - 1 - 2 * (n - (block_max+1)/2);
}

static int filenum(uint32_t x)
{
  if (x == 0 || x == 0777777)
    return 0;
  return ((x & 0777) - 2) / 5;
}

static int blocks(uint32_t i)
{
  int n = 0;
  while ((i & 01777) != 0) {
    //printf("[%06o]", i);
    n++;
    i &= 01777;
    i = ptb[i];
    if (n > 01110)
      break;
  }
  return n;
}

static void indent(int n)
{
  int i;
  for (i = 0; i < n; i++)
    putchar(' ');
}

static void list_file(uint32_t offset)
{
  //printf("File %02d: ", filenum(offset));
  indent(depth);
  uc_state = 0;
  fiodec_word(dir[offset + 1], putchar);
  fiodec_word(dir[offset + 2], putchar);
  fiodec_word(dir[offset + 3], putchar);
  indent(10 - depth);
  switch (dir[offset + 4] & 0770000) {
  case 0400000:
  case 0500000:
  case 0600000:
    printf("%3o", blocks(dir[offset + 4]));
    break;
  }
  putchar('\n');
}

static void silently(uint32_t offset)
{
}

static void traverse(uint32_t offset,
                     void (*process_file) (uint32_t))
{
  offset &= 0777;
  if (offset == 0)
    return;
  files++;
  if (files > 102)
    return;
  process_file(offset);
  depth++;
  if (depth > 20) {
    depth--;
    return;
  }
  if ((dir[offset + 4] & 0770000) == 0)
    traverse(dir[offset + 4], process_file);
  depth--;
  if (dir[offset] != 0)
    traverse(dir[offset], process_file);
}

static void newfs()
{
  printf("New file system\n");
  block_max = 01102 - 1;

  memcpy(dir,               image + 256 * physical(0) + 040, (256 - 040) * sizeof(uint32_t));
  memcpy(dir + (256 - 040), image + 256 * physical(1), 256 * sizeof(uint32_t));
  memcpy(dir + (512 - 040), image + 256 * physical(2), 040 * sizeof(uint32_t));

  memcpy(ptb,               image + 256 * physical(2) + 040, (256 - 040) * sizeof(uint32_t));
  memcpy(ptb + (256 - 040), image + 256 * physical(3), 256 * sizeof(uint32_t));
  memcpy(ptb + (512 - 040), image + 256 * physical(4), 256 * sizeof(uint32_t));
}

static void oldfs()
{
  printf("Old file system\n");
  block_max = 0777;

  memcpy(dir,       image + 256 * physical(6), 256 * sizeof(uint32_t));
  memcpy(dir + 256, image + 256 * physical(7), 256 * sizeof(uint32_t));

  memcpy(ptb,       image + 256 * physical(8), 256 * sizeof(uint32_t));
  memcpy(ptb + 256, image + 256 * physical(9), 256 * sizeof(uint32_t));
}

static void process(void)
{
  int i;

#if 0
  for (i = 0; i < 256*512; i += 256) {
    printf("BLOCK: file %d, physical %u, logical %u\n",
           file_no, i/256, logical(i/256));

    for (j = i; j < i+256; j++) {
      printf("%03o/%03o: %06o ", i/256, j-i, image[j]);
      dasm(image[j]);
      printf("   ");
      fiodec_word(image[j], putchar);
      putchar('\n');
    }
    
    uc_state = 0;
    for (j = i; j < i+256; j++)
      fiodec_word(image[j], putchar);
    putchar('\n');
  }
#endif

  if (file_no > 0)
    putchar('\n');
  if (file_no >= 0)
    printf("File %d on magtape; ", file_no);

  i = 256 * physical(0);
  if (((image[i+040] | image[i+041]) & 0777000) == 0)
    newfs();
  else
    oldfs();

  printf("Tape name: ");
  fiodec_word(ptb[0], putchar);
  fiodec_word(ptb[1], putchar);
  fiodec_word(ptb[2], putchar);
  putchar('\n');

  if (ptb[4] > 0 && ptb[4] < 01102)
    printf("Usable blocks: %o\n", ptb[4]);

  files = depth = 0;
  traverse(dir[0], list_file);
  i = files;

  files = depth = 0;
  traverse(dir[1], silently);
  printf("%o free directory entries\n", files);

  if (files + i != 102)
    printf("Used and free files don't match up.\n");
}

static uint32_t file(FILE *f)
{
  uint32_t n1 = 0, n2 = 0, n3, n4;

  next = image;

  while ((n3 = record(f)) != 0) {
    n4 = n3 & 0xFFFFFF;
    n1 += n4;
    n2++;

    if ((n4 % 6) != 0)
      fprintf(stderr, "Not an even number of words.\n");

    int i;
    for (i = 0; i < n4; i += 3) {
      *next  = buffer[i+0] << 12;
      *next |= buffer[i+1] <<  6;
      *next |= buffer[i+2];
      next++;
      if (next - image > sizeof image / sizeof image[0])
        fatal("Microtape image too large.");
    }
  }

  fprintf(stderr, "File %u records, %u octets\n", n2, n1);

  process();

  return n1;
}


static void mtape(FILE *f)
{
  file_no = 0;
  while (file(f))
    file_no++;
  fprintf(stderr, "Number of files: %d\n", file_no);
}

static void utape(FILE *f)
{
  uint32_t n1 = 0;
  uint32_t *next = image;

  for (;;) {
    uint32_t x1 = fgetc(f);
    uint32_t x2 = fgetc(f);
    uint32_t x3 = fgetc(f);
    uint32_t x4 = fgetc(f);
    if (feof(f))
      break;
    if ((x3 & 0xC0) != 0 || x4 != 0)
      fatal("Bletcherous microtape word.");
    n1++;
    *next++ = x1 | (x2 << 8) | (x3 << 16);
    if (next - image > sizeof image / sizeof image[0])
      fatal("Microtape image too large.");
  }

  fprintf(stderr, "Tape %u octets\n", n1);
  process();
}

int main(int argc, char **argv)
{
  if (argc != 2)
    exit(1);
  if (strcmp(argv[1], "-u") == 0)
    utape(stdin);
  else if (strcmp(argv[1], "-m") == 0)
    mtape(stdin);
  else
    exit(1);
  return 0;
}
