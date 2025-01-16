#include "pretty_printer.h"

#include "parser.h"
#include "scan.h"
#include "token.h"

int token;

void pretty_print() {
  token = scan();
  parse_program();
  printf("token = %d\n", token);
}