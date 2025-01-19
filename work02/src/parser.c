#include "parser.h"

#include "parse_error.h"
#include "scan.h"
#include "token.h"

extern int token;
int iter_flag = 0;

int parse_program() {
  if (token != TPROGRAM) return error(ERRMSG_PROGRAM_NOT_FOUND);
  token = scan();
  if (token != TNAME) return error("Program name is not found");
  token = scan();
  if (token != TSEMI) return error("Semicolon(;) is not found");
  token = scan();
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
  if (parse_variable_name() == ERROR) return ERROR;
  while (token == TCOMMA) {
    token = scan();
    if (parse_variable_name() == ERROR) return ERROR;
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
  if (token != TNUMBER) return error("Unsigned integer is not found");
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
  token = scan();
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
  if (parse_statement() == ERROR) return ERROR;
  while (token == TSEMI) {
    token = scan();
    if (parse_statement() == ERROR) return ERROR;
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
    token = scan();
    if (parse_statement() == ERROR) return ERROR;
  }
  return NORMAL;
}

int parse_iteration_statement() {
  if (token != TWHILE) return error(ERRMSG_WHILE_NOT_FOUND);
  iter_flag = 1;
  token = scan();
  if (parse_expression() == ERROR) return ERROR;
  if (token != TDO) return error(ERRMSG_DO_NOT_FOUND);
  token = scan();
  if (parse_expression() == ERROR) return ERROR;
  return NORMAL;
}

int parse_exit_statement() {
  if (token != TBREAK) return error(ERRMSG_BREAK_NOT_FOUND);
  if (!iter_flag) return error("Keyword 'break' should be found in iteration statement");
  iter_flag = 0;
  token = scan();
  return NORMAL;
}

int parse_call_statement() {
  if (token != TCALL) return error(ERRMSG_CALL_NOT_FOUND);
  token = scan();
  if (parse_procedure_name() == ERROR) return ERROR;
  if (token == TLPAREN) {
    token = scan();
    if (parse_expressions() == ERROR) return ERROR;
    if (token != TRPAREN) return error("Right parenthesis(')') is not found");
    token = scan();
  }
  return NORMAL;
}

int parse_expressions() {
  if (parse_expression() == ERROR) return ERROR;
  while (token == TCOMMA) {
    token = scan();
    if (parse_expression() == ERROR) return ERROR;
  }
  return NORMAL;
}

int parse_return_statement() {
  if (token != TRETURN) error(ERRMSG_RETURN_NOT_FOUND);
  token = scan();
  return NORMAL;
}

int parse_assignment_statement() {
  if (parse_left_part() == ERROR) return ERROR;
  if (token != TASSIGN) return error("Assignment(':=') is not found");
  token = scan();
  if (parse_expression() == ERROR) return ERROR;
  return NORMAL;
}

int parse_left_part() {
  if (parse_variable() == ERROR) return ERROR;
  return NORMAL;
}

int parse_variable() {
  if (parse_variable_name() == ERROR) return ERROR;
  if (token == TLSQPAREN) {
    token = scan();
    if (parse_expression() == ERROR) return ERROR;
    if (token != TRSQPAREN) return error("Right bracket(']') is not found");
    token = scan();
  }
  return NORMAL;
}

int parse_expression() {
  if (parse_simple_expression() == ERROR) return ERROR;
  while (token == TEQUAL || token == TNOTEQ || token == TLE || token == TLEEQ || token == TGR || token == TGREQ) {
    if (parse_relational_operator() == ERROR) return ERROR;
    if (parse_simple_expression() == ERROR) return ERROR;
  }
  return NORMAL;
}

int parse_simple_expression() {
  if (token == TPLUS || token == TMINUS) token = scan();
  if (parse_term() == ERROR) return ERROR;
  while (token == TPLUS || token == TMINUS || token == TOR) {
    if (parse_additive_operator() == ERROR) return ERROR;
    if (parse_term() == ERROR) return ERROR;
  }
  return NORMAL;
}

int parse_term() {
  if (parse_factor() == ERROR) return ERROR;
  while (token == TSTAR || token == TDIV || token == TAND) {
    if (parse_multiplicative_operator() == ERROR) return ERROR;
    if (parse_factor() == ERROR) return ERROR;
  }
  return NORMAL;
}

int parse_factor() {
  if (token == TNAME && parse_variable() == ERROR)
    return ERROR;
  else if ((token == TNUMBER || token == TFALSE || token == TTRUE || token == TSTRING) && parse_constant() == ERROR)
    return ERROR;
  else if (token == TLPAREN) {
    token = scan();
    if (parse_expression() == ERROR) return ERROR;
    if (token != TRPAREN) return error("Right parenthesis(')') is not found");
    token = scan();
  } else if (token == TNOT) {
    token = scan();
    if (parse_factor() == ERROR) return ERROR;
  } else if (token == TINTEGER || token == TBOOLEAN || token == TCHAR) {
    if (parse_standard_type() == ERROR) return ERROR;
    if (token != TLPAREN) return error("Left parenthesis('(') is not found");
    token = scan();
    if (parse_expression() == ERROR) return ERROR;
    if (token != TRPAREN) return error("Right parenthesis(')') is not found");
    token = scan();
  }
  return NORMAL;
}

int parse_constant() {
  if (token != TNUMBER && token != TFALSE && token != TTRUE && token != TSTRING) return error("Constant is not found");
  token = scan();
  return NORMAL;
}

int parse_multiplicative_operator() {
  if (token != TSTAR && token != TDIV && token != TAND) return error("Multiplicative operator is not found");
  token = scan();
  return NORMAL;
}

int parse_additive_operator() {
  if (token != TPLUS && token != TMINUS && token != TOR) return error("Additive operator is not found");
  token = scan();
  return NORMAL;
}

int parse_relational_operator() {
  if (token != TEQUAL && token != TNOTEQ && token != TLE && token != TLEEQ && token != TGR && token != TGREQ) return error("Relational operator is not found");
  token = scan();
  return NORMAL;
}

int parse_input_statement() {
  if (token != TREAD && token != TREADLN) return error("The input statement must begin with either 'read' or 'readln'");
  token = scan();
  if (token == TLPAREN) {
    token = scan();
    if (parse_variable() == ERROR) return ERROR;
    while (token == TCOMMA) {
      token = scan();
      if (parse_variable() == ERROR) return ERROR;
    }
    if (token != TRPAREN) return error("Right parenthesis(')') is not found");
    token = scan();
  }
  return NORMAL;
}

int parse_output_statement() {
  if (token != TWRITE && token != TWRITELN) return error("The output statement must begin with either 'write' or 'writeln'");
  token = scan();
  if (token == TLPAREN) {
    token = scan();
    if (parse_output_format() == ERROR) return ERROR;
    while (token == TCOMMA) {
      token = scan();
      if (parse_output_format() == ERROR) return ERROR;
    }
    if (token != TRPAREN) return error("Right parenthesis(')') is not found");
    token = scan();
  }
  return NORMAL;
}

int parse_output_format() {
  if (token == TSTRING && get_string_attr_len() != 1) {
    token = scan();
    if (token == TCOLON) return error("The string before the colon (:) must be a single character");
  } else {
    if (parse_expression() == ERROR) return ERROR;
    if (token == TCOLON) {
      token = scan();
      if (token != TNUMBER) return error("Integer is not found");
      token = scan();
    }
  }
  return NORMAL;
}