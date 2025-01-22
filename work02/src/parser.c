#include "parser.h"

#include "parse_error.h"
#include "scan.h"
#include "token.h"

extern char* tokenstr[];
int token;
int indent_level = 0;
int br_flag = 0;          // break(new line) flag
int iter_flag = 0;        // iteration flag
int formal_arg_flag = 0;  // formal argument part flag
int empty_stmt_flag = 0;  // empty statement flag

static void newline() {
  putchar('\n');
  br_flag = 1;
}

static void pretty_print() {
  // preprocessing
  if (br_flag == 1) {
    for (int i = 0; i < indent_level; i++) {
      printf("    ");
    }
    br_flag = 0;
  } else {
    if (token != TSEMI && token != TDOT && token != TPROGRAM) {
      printf(" ");
    }
  }

  if (token == TSTRING) {
    printf("'%s'", string_attr);
  } else if (token == TNAME) {
    printf("%s", string_attr);
  } else if (token == TNUMBER) {
    printf("%s", string_attr);
    // num_attrにすると00のような表記が0に直されてしまう
    // printf("%d", num_attr);
  } else {
    printf("%s", tokenstr[token]);
  }

  if ((token == TSEMI && formal_arg_flag == 0)) {
    newline();
  }
}

int parse() {
  token = scan();
  return parse_program();
}

int parse_program() {
  if (token != TPROGRAM) return error(ERRMSG_PROGRAM_NOT_FOUND);
  pretty_print();
  token = scan();
  if (token != TNAME) return error("Program name is not found");
  pretty_print();
  token = scan();
  if (token != TSEMI) return error("Semicolon(;) is not found");
  pretty_print();
  token = scan();
  if (parse_block() == ERROR) return ERROR;
  if (token != TDOT) return error("Period is not found at the end of program");
  pretty_print();
  newline();
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
  indent_level++;
  pretty_print();
  newline();
  token = scan();
  do {
    indent_level++;
    if (parse_variable_names() == ERROR) return ERROR;
    if (token != TCOLON) return error("Colon(':') is not found");
    pretty_print();
    token = scan();
    if (parse_type() == ERROR) return ERROR;
    if (token != TSEMI) return error("Semicolon(;) is not found");
    pretty_print();
    token = scan();
    indent_level--;
  } while (token == TNAME);
  indent_level--;
  return NORMAL;
}

int parse_variable_names() {
  if (parse_variable_name() == ERROR) return ERROR;
  while (token == TCOMMA) {
    pretty_print();
    token = scan();
    if (parse_variable_name() == ERROR) return ERROR;
  }
  return NORMAL;
}

int parse_variable_name() {
  if (token != TNAME) return error("Variable name is invalid");
  pretty_print();
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
    pretty_print();
    token = scan();
    return NORMAL;
  }
  return error("Standard type is not found");
}

int parse_array_type() {
  if (token != TARRAY) return error(ERRMSG_ARRAY_NOT_FOUND);
  pretty_print();
  token = scan();
  if (token != TLSQPAREN) return error("Left square bracket('[') is not found");
  pretty_print();
  token = scan();
  if (token != TNUMBER) return error("Unsigned integer is not found");
  pretty_print();
  token = scan();
  if (token != TRSQPAREN) return error("Right square bracket(']') is not found");
  pretty_print();
  token = scan();
  if (token != TOF) return error(ERRMSG_OF_NOT_FOUND);
  pretty_print();
  token = scan();
  if (parse_standard_type() == ERROR) return ERROR;
  return NORMAL;
}

int parse_subprogram_declaration() {
  if (token != TPROCEDURE) return error(ERRMSG_PROCEDURE_NOT_FOUND);
  indent_level++;
  pretty_print();
  token = scan();
  if (parse_procedure_name() == ERROR) return ERROR;
  if (token == TLPAREN && parse_formal_parameters() == ERROR) return ERROR;
  if (token != TSEMI) return error("Semicolon(';') is not found");
  pretty_print();
  token = scan();
  if (token == TVAR && parse_variable_declaration() == ERROR) return ERROR;
  if (parse_compound_statement() == ERROR) return ERROR;
  if (token != TSEMI) return error("Semicolon(';') is not found");
  pretty_print();
  token = scan();
  indent_level--;
  return NORMAL;
}

int parse_procedure_name() {
  if (token != TNAME) return error("Procedure name is not found");
  pretty_print();
  token = scan();
  return NORMAL;
}

int parse_formal_parameters() {
  if (token != TLPAREN) return error("Left parenthesis('(') is not found");
  formal_arg_flag = 1;
  pretty_print();
  token = scan();
  while (token == TNAME) {
    if (parse_variable_names() == ERROR) return ERROR;
    if (token != TCOLON) return error("Colon(':') is not found");
    pretty_print();
    token = scan();
    if (parse_type() == ERROR) return ERROR;
    if (token != TSEMI) break;
    pretty_print();
    token = scan();
  }
  if (token != TRPAREN) return error("Right parenthesis(')') is not found");
  pretty_print();
  formal_arg_flag = 0;
  token = scan();
  return NORMAL;
}

int parse_compound_statement() {
  if (token != TBEGIN) return error(ERRMSG_BEGIN_NOT_FOUND);
  pretty_print();
  newline();
  indent_level++;
  token = scan();
  if (parse_statement() == ERROR) return ERROR;
  while (token == TSEMI) {
    pretty_print();
    token = scan();
    if (parse_statement() == ERROR) return ERROR;
  }
  indent_level--;
  if (token != TEND) return error(ERRMSG_END_NOT_FOUND);
  if (!empty_stmt_flag) newline();
  empty_stmt_flag = 0;
  pretty_print();
  token = scan();
  return NORMAL;
}

int parse_statement() {
  if (token == TNAME)
    return parse_assignment_statement();
  else if (token == TIF)
    return parse_condition_statement();
  else if (token == TWHILE)
    return parse_iteration_statement();
  else if (token == TBREAK)
    return parse_exit_statement();
  else if (token == TCALL)
    return parse_call_statement();
  else if (token == TRETURN)
    return parse_return_statement();
  else if (token == TREAD || token == TREADLN)
    return parse_input_statement();
  else if ((token == TWRITE || token == TWRITELN))
    return parse_output_statement();
  else if (token == TBEGIN)
    return parse_compound_statement();
  else
    parse_empty_statement();
  return NORMAL;
}

int parse_condition_statement() {
  if (token != TIF) return error(ERRMSG_IF_NOT_FOUND);
  pretty_print();
  token = scan();
  if (parse_expression() == ERROR) return ERROR;
  if (token != TTHEN) return error(ERRMSG_THEN_NOT_FOUND);
  pretty_print();
  newline();
  indent_level++;
  token = scan();
  if (parse_statement() == ERROR) return ERROR;
  if (token == TELSE) {
    newline();
    indent_level--;
    pretty_print();
    newline();
    indent_level++;
    token = scan();
    if (parse_statement() == ERROR) return ERROR;
  }
  indent_level--;
  return NORMAL;
}

int parse_iteration_statement() {
  if (token != TWHILE) return error(ERRMSG_WHILE_NOT_FOUND);
  iter_flag++;
  pretty_print();
  token = scan();
  if (parse_expression() == ERROR) return ERROR;
  if (token != TDO) return error(ERRMSG_DO_NOT_FOUND);
  pretty_print();
  newline();
  indent_level++;
  token = scan();
  if (parse_statement() == ERROR) return ERROR;
  indent_level--;
  return NORMAL;
}

int parse_exit_statement() {
  if (token != TBREAK) return error(ERRMSG_BREAK_NOT_FOUND);
  if (!iter_flag) return error("Keyword 'break' should be found in iteration statement");
  pretty_print();
  iter_flag--;
  token = scan();
  return NORMAL;
}

int parse_call_statement() {
  if (token != TCALL) return error(ERRMSG_CALL_NOT_FOUND);
  pretty_print();
  token = scan();
  if (parse_procedure_name() == ERROR) return ERROR;
  if (token == TLPAREN) {
    pretty_print();
    token = scan();
    if (parse_expressions() == ERROR) return ERROR;
    if (token != TRPAREN) return error("Right parenthesis(')') is not found");
    pretty_print();
    token = scan();
  }
  return NORMAL;
}

int parse_expressions() {
  if (parse_expression() == ERROR) return ERROR;
  while (token == TCOMMA) {
    pretty_print();
    token = scan();
    if (parse_expression() == ERROR) return ERROR;
  }
  return NORMAL;
}

int parse_return_statement() {
  if (token != TRETURN) error(ERRMSG_RETURN_NOT_FOUND);
  pretty_print();
  token = scan();
  return NORMAL;
}

int parse_assignment_statement() {
  if (parse_left_part() == ERROR) return ERROR;
  if (token != TASSIGN) return error("Assignment(':=') is not found");
  pretty_print();
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
    pretty_print();
    token = scan();
    if (parse_expression() == ERROR) return ERROR;
    if (token != TRSQPAREN) return error("Right bracket(']') is not found");
    pretty_print();
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
  if (token == TPLUS || token == TMINUS) {
    pretty_print();
    token = scan();
  }
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
  if (token == TNAME)
    return parse_variable();
  else if (token == TNUMBER || token == TFALSE || token == TTRUE || token == TSTRING)
    return parse_constant();
  else if (token == TLPAREN) {
    pretty_print();
    token = scan();
    if (parse_expression() == ERROR) return ERROR;
    if (token != TRPAREN) return error("Right parenthesis(')') is not found");
    pretty_print();
    token = scan();
  } else if (token == TNOT) {
    pretty_print();
    token = scan();
    if (parse_factor() == ERROR) return ERROR;
  } else if (token == TINTEGER || token == TBOOLEAN || token == TCHAR) {
    if (parse_standard_type() == ERROR) return ERROR;
    if (token != TLPAREN) return error("Left parenthesis('(') is not found");
    pretty_print();
    token = scan();
    if (parse_expression() == ERROR) return ERROR;
    if (token != TRPAREN) return error("Right parenthesis(')') is not found");
    pretty_print();
    token = scan();
  }
  return NORMAL;
}

int parse_constant() {
  if (token != TNUMBER && token != TFALSE && token != TTRUE && token != TSTRING) return error("Constant is not found");
  pretty_print();
  token = scan();
  return NORMAL;
}

int parse_multiplicative_operator() {
  if (token != TSTAR && token != TDIV && token != TAND) return error("Multiplicative operator is not found");
  pretty_print();
  token = scan();
  return NORMAL;
}

int parse_additive_operator() {
  if (token != TPLUS && token != TMINUS && token != TOR) return error("Additive operator is not found");
  pretty_print();
  token = scan();
  return NORMAL;
}

int parse_relational_operator() {
  if (token != TEQUAL && token != TNOTEQ && token != TLE && token != TLEEQ && token != TGR && token != TGREQ) return error("Relational operator is not found");
  pretty_print();
  token = scan();
  return NORMAL;
}

int parse_input_statement() {
  if (token != TREAD && token != TREADLN) return error("The input statement must begin with either 'read' or 'readln'");
  pretty_print();
  token = scan();
  if (token == TLPAREN) {
    pretty_print();
    token = scan();
    if (parse_variable() == ERROR) return ERROR;
    while (token == TCOMMA) {
      pretty_print();
      token = scan();
      if (parse_variable() == ERROR) return ERROR;
    }
    if (token != TRPAREN) return error("Right parenthesis(')') is not found");
    pretty_print();
    token = scan();
  }
  return NORMAL;
}

int parse_output_statement() {
  if (token != TWRITE && token != TWRITELN) return error("The output statement must begin with either 'write' or 'writeln'");
  pretty_print();
  token = scan();
  if (token == TLPAREN) {
    pretty_print();
    token = scan();
    if (parse_output_format() == ERROR) return ERROR;
    while (token == TCOMMA) {
      pretty_print();
      token = scan();
      if (parse_output_format() == ERROR) return ERROR;
    }
    if (token != TRPAREN) return error("Right parenthesis(')') is not found");
    pretty_print();
    token = scan();
  }
  return NORMAL;
}

int parse_output_format() {
  if (token == TSTRING && get_string_attr_len() != 1) {
    pretty_print();
    token = scan();
    if (token == TCOLON) return error("The string before the colon (:) must be a single character");
  } else {
    if (parse_expression() == ERROR) return ERROR;
    if (token == TCOLON) {
      pretty_print();
      token = scan();
      if (token != TNUMBER) return error("Integer is not found");
      pretty_print();
      token = scan();
    }
  }
  return NORMAL;
}

int parse_empty_statement() {
  empty_stmt_flag = 1;
  return NORMAL;
}