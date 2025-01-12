#include "pretty_printer.h"

#include "scan.h"

extern int numtoken[];
int token;

void pretty_print() {
  while ((token = scan()) >= 0) {
  }
}