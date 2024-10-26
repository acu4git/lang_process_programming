#include "scan.h"

static FILE* fp;

int num_attribute;
static int linenum = 0;

char string_attr[MAXSTRSIZE];

int init_scan(char* filename) {
  if ((fp = fopen(filename, "r")) == NULL) return -1;
  return 0;
}

int scan() {
  char c;
  while (1) {
    switch (c = fgetc(fp)) {
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9':
    }
  }
}

int get_linenum() { return linenum; }

void end_scan() { fclose(fp); }
