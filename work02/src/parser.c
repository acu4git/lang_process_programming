#include "parser.h"

#include "parse_error.h"
#include "scan.h"

static int parse_program() {
  if (token != TPROGRAM) return error(ERRMSG_PROGRAM_NOT_FOUND);
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
  if (token != TVAR) return error(ERRMSG_VAR_NOT_FOUND);
  token = scan();
  if (parse_variable_names() == ERROR) return ERROR;
  if (token != TCOLON) return error("Colon is not found");
  token = scan();
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
  if (token != TNAME) return error("Invalid variable name");
  token = scan();
  return NORMAL;
}

static int parse_type() {
}

static int parse_standard_type() {
  if (token == TINTEGER || token == TBOOLEAN || token == TCHAR) {
    token = scan();
    return NORMAL;
  }
  return error("Standard type is not found");
}

static int parse_array_type() {
  if (token != TARRAY) return error(ERRMSG_ARRAY_NOT_FOUND);
}