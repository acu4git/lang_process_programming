﻿#include "cross_referencer.h"
#include "parser.h"
#include "scan.h"

/* keyword list */
struct KEY key[KEYWORDSIZE] = {
    {"and", TAND},
    {"array", TARRAY},
    {"begin", TBEGIN},
    {"boolean", TBOOLEAN},
    {"break", TBREAK},
    {"call", TCALL},
    {"char", TCHAR},
    {"div", TDIV},
    {"do", TDO},
    {"else", TELSE},
    {"end", TEND},
    {"false", TFALSE},
    {"if", TIF},
    {"integer", TINTEGER},
    {"not", TNOT},
    {"of", TOF},
    {"or", TOR},
    {"procedure", TPROCEDURE},
    {"program", TPROGRAM},
    {"read", TREAD},
    {"readln", TREADLN},
    {"return", TRETURN},
    {"then", TTHEN},
    {"true", TTRUE},
    {"var", TVAR},
    {"while", TWHILE},
    {"write", TWRITE},
    {"writeln", TWRITELN},
};

/* Token counter */
int numtoken[NUMOFTOKEN + 1];

/* string of each token */
char *tokenstr[NUMOFTOKEN + 1] = {
    "", "NAME", "program", "var", "array", "of", "begin",
    "end", "if", "then", "else", "procedure", "return", "call",
    "while", "do", "not", "or", "div", "and", "char",
    "integer", "boolean", "readln", "writeln", "true", "false", "NUMBER",
    "STRING", "+", "-", "*", "=", "<>", "<",
    "<=", ">", ">=", "(", ")", "[", "]",
    ":=", ".", ",", ":", ";", "read", "write",
    "break"};

int main(int nc, char *np[]) {
  if (nc < 2) {
    error("File name is not given.");
    return 0;
  }
  if (init_scan(np[1]) < 0) {
    error("Cannot open input file.");
    end_scan();
    return 0;
  }

  init_tab();

  // if (parse() == ERROR) {
  //   return 1;
  // }
  parse();
  end_scan();

  add_cr_table(IS_GLOBAL);
  print_tab();

  clear_table(IS_GLOBAL);
  clear_table(IS_CROSS);

  return 0;
}

int error(char *mes) {
  fprintf(stderr, "Line: %4d ERROR: %s\n", get_linenum(), mes);
  return S_ERROR;
}
