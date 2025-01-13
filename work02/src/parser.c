#include "parser.h"

#include "parse_error.h"
#include "scan.h"

extern int token;

int parse_program() {
  if (token != TPROGRAM) return error(ERRMSG_PROGRAM_NOT_FOUND);
  token = scan();
  if (token != TNAME) return error("Program name is not found");
  token = scan();
  if (token != TSEMI) return error("Semicolon(;) is not found");
  if (parse_block() == ERROR) return ERROR;
  if (token != TDOT) return error("Period is not found at the end of program");
  token = scan();
  return NORMAL;
}

int parse_block() {
  while (token == TVAR || token == TPROCEDURE) {
    if (token == TVAR && parse_variable_declaration() == ERROR)
      return ERROR;
    else if (token == TPROCEDURE && parse_subprogram_declaration() == ERROR)
      return ERROR;
  }
  if (parse_compound_statement() == ERROR) return ERROR;
  return NORMAL;
}

int parse_variable_declaration() {
  if (token != TVAR) return error(ERRMSG_VAR_NOT_FOUND);
  token = scan();
  do {
    if (parse_variable_names() == ERROR) return ERROR;
    if (token != TCOLON) return error("Colon(':') is not found");
    token = scan();
    if (parse_type() == ERROR) return ERROR;
    if (token != TSEMI) return error("Semicolon(;) is not found");
    token = scan();
  } while (token == TNAME);
  return NORMAL;
}

int parse_variable_names() {
  while (1) {
    if (parse_variable_name() == ERROR) return ERROR;
    if (token != TCOMMA) break;
    token = scan();
  }
  return NORMAL;
}

int parse_variable_name() {
  if (token != TNAME) return error("Variable name is invalid");
  token = scan();
  return NORMAL;
}

int parse_type() {
  if ((token == TINTEGER || token == TBOOLEAN || token == TCHAR) && parse_standard_type() == ERROR)
    return ERROR;
  else if (token == TARRAY && parse_array_type() == ERROR)
    return ERROR;
  return NORMAL;
}

int parse_standard_type() {
  if (token == TINTEGER || token == TBOOLEAN || token == TCHAR) {
    token = scan();
    return NORMAL;
  }
  return error("Standard type is not found");
}

int parse_array_type() {
  if (token != TARRAY) return error(ERRMSG_ARRAY_NOT_FOUND);
  token = scan();
  if (token != TLSQPAREN) return error("Left square bracket('[') is not found");
  token = scan();
  if (token != TINTEGER) return error("Integer is not found");
  token = scan();
  if (token != TRSQPAREN) return error("Right square bracket(']') is not found");
  token = scan();
  if (token != TOF) return error(ERRMSG_OF_NOT_FOUND);
  token = scan();
  if (parse_standard_type() == ERROR) return ERROR;
  token = scan();
  return NORMAL;
}

int parse_subprogram_declaration() {
  if (token != TPROCEDURE) return error(ERRMSG_PROCEDURE_NOT_FOUND);
  token = scan();
  if (parse_procedure_name() == ERROR) return ERROR;
  if (token == TLPAREN && parse_formal_parameters() == ERROR) return ERROR;
  if (token != TSEMI) return error("Semicolon(';') is not found");
  token = scan();
  if (token == TVAR && parse_variable_declaration() == ERROR) return ERROR;
  if (parse_compound_statement() == ERROR) return ERROR;
  if (token != TSEMI) return error("Semicolon(';') is not found");
  return NORMAL;
}

int parse_procedure_name() {
  if (token != TNAME) return error("Procedure name is not found");
  token = scan();
  return NORMAL;
}

int parse_formal_parameters() {
  if (token != TLPAREN) return error("Left parenthesis('(') is not found");
  token = scan();
  while (token == TNAME) {
    if (parse_variable_names() == ERROR) return ERROR;
    if (token != TCOLON) return error("Colon(':') is not found");
    token = scan();
    if (parse_type() == ERROR) return ERROR;
    if (token != TSEMI) break;
    token = scan();
  }
  if (token != TRPAREN) return error("Right parenthesis(')') is not found");
  token = scan();
  return NORMAL;
}

int parse_compound_statement() {
  if (token != TBEGIN) return error(ERRMSG_BEGIN_NOT_FOUND);
  token = scan();
  while (1) {
    if (parse_statement() == ERROR) return ERROR;
    if (token != TSEMI) break;
    token = scan();
  }
  if (token != TEND) return error(ERRMSG_END_NOT_FOUND);
  token = scan();
  return NORMAL;
}

int parse_statement() {
  if (token == TNAME && parse_assignment_statement() == ERROR)
    return ERROR;
  else if (token == TIF && parse_condition_statement() == ERROR)
    return ERROR;
  else if (token == TWHILE && parse_iteration_statement() == ERROR)
    return ERROR;
  else if (token == TBREAK && parse_exit_statement() == ERROR)
    return ERROR;
  else if (token == TCALL && parse_call_statement() == ERROR)
    return ERROR;
  else if (token == TRETURN && parse_return_statement() == ERROR)
    return ERROR;
  else if ((token == TREAD || token == TREADLN) && parse_input_statement() == ERROR)
    return ERROR;
  else if ((token == TWRITE || token == TWRITELN) && parse_output_statement() == ERROR)
    return ERROR;
  else if (token == TBEGIN && parse_compound_statement() == ERROR)
    return ERROR;
  return NORMAL;
}

int parse_condition_statement() {
  if (token != TIF) return error(ERRMSG_IF_NOT_FOUND);
  token = scan();
  if (parse_expression() == ERROR) return ERROR;
  if (token != TTHEN) return error(ERRMSG_THEN_NOT_FOUND);
  token = scan();
  if (parse_statement() == ERROR) return ERROR;
  while (token == TELSE) {
    if (parse_statement() == ERROR) return ERROR;
  }
  return NORMAL;
}

int parse_iteration_statement() {
  if (token != TWHILE) return error(ERRMSG_WHILE_NOT_FOUND);
  if (parse_expression() == ERROR) return ERROR;
  if (token != TDO) return error(ERRMSG_DO_NOT_FOUND);
}