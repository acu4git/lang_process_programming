#include "parser.h"

#include "scan.h"

static int parse_program() {
  if (token != TPROGRAM) return error("Keyword 'program' is not found");
  token = scan();
  if (token != TNAME) return error("Program name is not found");
  token = scan();
  if (token != TSEMI) return error("Semicolon is not found");
  if (parse_block() == ERROR) return ERROR;
  if (token != TDOT) return error("Period is not found at the end of program");
  token = scan();
  return NORMAL;
}

static int parse_block() {}

static int parse_variable_declaration() {
  if (token != TVAR) return error("Keyword 'var' is not found");
  token = scan();
  if (parse_variable_names() == ERROR) return ERROR;
}

static int parse_variable_names() {
  if (parse_variable_name() == ERROR) return ERROR;
  while (token == TCOMMA) {
    token = scan();
    if (parse_variable_name() == ERROR) return ERROR;
  }
  return NORMAL;
}

static int parse_variable_name() {
  if (token != TNAME) return error("Variable name is not found");
  token = scan();
  return NORMAL;
}