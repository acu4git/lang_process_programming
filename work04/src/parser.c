#include "parser.h"

#include "compiler.h"
#include "cross_referencer.h"
#include "parse_error.h"
#include "scan.h"

// task2
extern char* tokenstr[];
int token;
int indent_level = 0;
int br_flag = 0;          // break(new line) flag
int iter_flag = 0;        // iteration flag
int formal_arg_flag = 0;  // formal argument part flag
int empty_stmt_flag = 0;  // empty statement flag

// task3
char* name = NULL;
char* procname = NULL;
int deflinenum = 0;
int ttype = 0;
int arrsize = 0;
int num_flag = 0;            // whether it's unsigned integer, not variable
int array_flag = 0;          // whether it's an array.
struct ID* id = NULL;        // temporary id, which store elements when call parse_variable_names().
struct ID* procid = NULL;    // temporary procedure id, which store an element when call parse_procedure_name().
struct TYPE* tp = NULL;      // updated when call parse_type()
struct TYPE* proctp = NULL;  // for procedure id
struct TYPE* etp = NULL;
struct ID* var = NULL;

// task4
int break_label = 0;
int var_flag = 0;
int left_flag = 0;
int addr_flag = 0;

static void newline() {
  // putchar('\n');
  br_flag = 1;
}

static void pretty_print() {
  // preprocessing
  if (br_flag == 1) {
    // for (int i = 0; i < indent_level; i++) {
    //   printf("    ");
    // }
    br_flag = 0;
  } else {
    // if (token != TSEMI && token != TDOT && token != TPROGRAM) {
    //   printf(" ");
    // }
  }

  // if (token == TSTRING) {
  //   printf("'%s'", string_attr);
  // } else if (token == TNAME) {
  //   printf("%s", string_attr);
  // } else if (token == TNUMBER) {
  //   printf("%s", string_attr);
  //   // num_attrにすると00のような表記が0に直されてしまう
  //   // printf("%d", num_attr);
  // } else {
  //   printf("%s", tokenstr[token]);
  // }

  if ((token == TSEMI && formal_arg_flag == 0)) {
    newline();
  }
}

void reset_ptr() {
  id = NULL;
  etp = NULL;
  procid = NULL;
  proctp = NULL;
  tp = NULL;
}

int parse() {
  token = scan();
  return parse_program();
}

int parse_program() {
  int first_label;

  if (token != TPROGRAM) return error(ERRMSG_PROGRAM_NOT_FOUND);
  pretty_print();
  token = scan();

  if (token != TNAME) return error("Program name is not found");
  char* progname = get_string();
  pretty_print();
  token = scan();

  if (token != TSEMI) return error("Semicolon(;) is not found");
  pretty_print();
  token = scan();

  /**
   * %%(PROGRAM_NAME)  START  L0001
   */
  first_label = get_label_num();
  gen_start(progname, first_label);

  if (parse_block(first_label) == ERROR) return ERROR;
  if (token != TDOT) return error("Period is not found at the end of program");

  gen_code("CALL", "FLUSH");

  gen_ret();
  outlib();
  gen_end();

  pretty_print();
  newline();
  token = scan();
  return NORMAL;
}

int parse_block(int first_label) {
  while (token == TVAR || token == TPROCEDURE) {
    if (token == TVAR) {
      if (parse_variable_declaration() == ERROR) return ERROR;

    } else if (token == TPROCEDURE) {
      if (parse_subprogram_declaration() == ERROR) return ERROR;
      procname = NULL;
      add_cr_table(IS_LOCAL);
      clear_table(IS_LOCAL);
    }
    reset_ptr();
  }

  /**
   * L0001
   *   LAD  GR0,0
   */
  gen_label(first_label);
  gen_lad_num("GR0", 0);  // init GR0

  if (parse_compound_statement() == ERROR) return ERROR;
  return NORMAL;
}

int parse_variable_declaration() {
  id = NULL;

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

    assign_type(id, tp);  // generate variable define code simultaneously
    if (register_bulk(id) == ERROR) return ERROR;
    id = NULL;

    if (token != TSEMI) return error("Semicolon(;) is not found");

    pretty_print();
    token = scan();
    indent_level--;
  } while (token == TNAME);
  indent_level--;
  return NORMAL;
}

int parse_variable_names() {
  struct ID* id_tmp;
  if (parse_variable_name() == ERROR) return ERROR;
  if ((id_tmp = new_id(name, procname, formal_arg_flag, deflinenum)) == NULL) return ERROR;
  push(&id, id_tmp);

  while (token == TCOMMA) {
    pretty_print();
    token = scan();
    if (parse_variable_name() == ERROR) return ERROR;
    if ((id_tmp = new_id(name, procname, formal_arg_flag, deflinenum)) == NULL) return ERROR;
    push(&id, id_tmp);
  }
  return NORMAL;
}

int parse_variable_name() {
  if (token != TNAME) return error("Variable name is invalid");

  name = get_string();
  deflinenum = get_linenum();

  pretty_print();
  token = scan();
  return NORMAL;
}

int parse_type() {
  tp = NULL;
  if (token == TINTEGER || token == TBOOLEAN || token == TCHAR) {
    if ((ttype = parse_standard_type()) == ERROR) return ERROR;
    arrsize = 0;
    etp = NULL;
  } else if (token == TARRAY) {
    if ((ttype = parse_array_type()) == ERROR) return ERROR;
    arrsize = get_num();
    if (arrsize <= 0) return error("Array size must be unsigned integer");
    etp = new_type(ttype - 3, 0, NULL, NULL);
    array_flag = 0;
  }

  tp = new_type(ttype, arrsize, etp, NULL);

  return NORMAL;
}

int parse_standard_type() {
  int type = 0;
  if (token != TINTEGER && token != TBOOLEAN && token != TCHAR) return error("Standard type is not found");

  if (array_flag) {
    if (token == TINTEGER)
      type = TPARRAYINT;
    else if (token == TBOOLEAN)
      type = TPARRAYBOOL;
    else if (token == TCHAR)
      type = TPARRAYCHAR;
  } else {
    if (token == TINTEGER)
      type = TPINT;
    else if (token == TBOOLEAN)
      type = TPBOOL;
    else if (token == TCHAR)
      type = TPCHAR;
  }

  pretty_print();
  token = scan();
  return type;
}

int parse_array_type() {
  int ttype;
  array_flag = 1;

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
  if ((ttype = parse_standard_type()) == ERROR) return ERROR;
  return ttype;
}

int parse_subprogram_declaration() {
  int has_parameter = 0;

  if (token != TPROCEDURE) return error(ERRMSG_PROCEDURE_NOT_FOUND);

  indent_level++;
  pretty_print();
  token = scan();

  if (parse_procedure_name() == ERROR) return ERROR;

  // create id for procedure name
  ttype = TPPROC;

  struct ID* procid_tmp;
  if ((procid_tmp = new_id(name, procname, formal_arg_flag, deflinenum)) == NULL) return ERROR;
  push(&procid, procid_tmp);

  proctp = new_type(ttype, 0, NULL, NULL);

  procname = get_string();

  // modified - task4
  if (token == TLPAREN) {
    has_parameter = 1;
    if (parse_formal_parameters() == ERROR) return ERROR;
  }
  if (token != TSEMI) return error("Semicolon(';') is not found");

  if (is_valid_param() == ERROR) return ERROR;
  proctp = assign_parameter(proctp);
  procid->itp = proctp;
  if (register_bulk(procid) == ERROR) return ERROR;

  pretty_print();
  token = scan();
  if (token == TVAR) {
    if (parse_variable_declaration() == ERROR) return ERROR;
  }

  gen_proc(procid->label);
  if (has_parameter) {
    gen_code("POP", "GR2");  // temporary pop
    gen_param();             // get parameter
    gen_code("PUSH", "0,GR2");
  }

  if (parse_compound_statement() == ERROR) return ERROR;
  if (token != TSEMI) return error("Semicolon(';') is not found");
  pretty_print();
  token = scan();
  indent_level--;

  gen_ret();

  return NORMAL;
}

int parse_procedure_name() {
  if (token != TNAME) return error("Procedure name is not found");
  name = get_string();
  deflinenum = get_linenum();
  pretty_print();
  token = scan();
  return NORMAL;
}

int parse_formal_parameters() {
  id = NULL;

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

    assign_type(id, tp);
    if (register_bulk(id) == ERROR) return ERROR;
    id = NULL;

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
  int type;
  int label1, label2;

  if (token != TIF) return error(ERRMSG_IF_NOT_FOUND);
  pretty_print();
  token = scan();

  if ((type = parse_expression()) == ERROR) return ERROR;
  if (type != TPBOOL) return error("IF requires boolean");

  label1 = get_label_num();
  gen_code("CPA", "GR1,GR0");
  gen_code_label("JZE", NULL, label1);

  if (token != TTHEN) return error(ERRMSG_THEN_NOT_FOUND);
  pretty_print();
  newline();
  indent_level++;
  token = scan();

  if (parse_statement() == ERROR) return ERROR;
  if (token == TELSE) {
    label2 = get_label_num();
    gen_code_label("JUMP", NULL, label2);
    gen_label(label1);

    newline();
    indent_level--;
    pretty_print();
    newline();
    indent_level++;
    token = scan();
    if (parse_statement() == ERROR) return ERROR;

    gen_label(label2);
  } else {
    gen_label(label1);
  }
  indent_level--;
  return NORMAL;
}

int parse_iteration_statement() {
  int type;
  int label1, label2;

  label1 = get_label_num();
  label2 = get_label_num();
  break_label = label2;

  gen_label(label1);

  if (token != TWHILE) return error(ERRMSG_WHILE_NOT_FOUND);
  iter_flag++;
  pretty_print();
  token = scan();

  if ((type = parse_expression()) == ERROR) return ERROR;
  if (type != TPBOOL) return error("WHILE requires boolean");

  // debug
  // gen_code("<while statement occurred>", NULL);
  gen_code("CPA", "GR1,GR0");
  gen_code_label("JZE", NULL, label2);

  if (token != TDO) return error(ERRMSG_DO_NOT_FOUND);

  pretty_print();
  newline();
  indent_level++;
  token = scan();
  if (parse_statement() == ERROR) return ERROR;

  gen_code_label("JUMP", NULL, label1);

  gen_label(label2);

  indent_level--;
  iter_flag--;
  return NORMAL;
}

int parse_exit_statement() {
  if (token != TBREAK) return error(ERRMSG_BREAK_NOT_FOUND);
  if (!iter_flag) return error("Keyword 'break' should be found in iteration statement");
  pretty_print();
  token = scan();

  gen_code_label("JUMP", NULL, break_label);

  return NORMAL;
}

int parse_call_statement() {
  char* proclabel;

  if (token != TCALL) return error(ERRMSG_CALL_NOT_FOUND);
  pretty_print();
  token = scan();

  // if (parse_procedure_name() == ERROR) return ERROR;
  if (token != TNAME) return error("Procedure name is not found");

  char* procname_tmp = get_string();
  if ((var = get_declared_id(procname_tmp)) == NULL) return error("Procedure name is not defined");
  free(procname_tmp);

  add_refline(var, get_linenum());

  pretty_print();
  token = scan();

  if (procname != NULL && !strcmp(var->name, procname)) return error("Recursive call occurred");

  proclabel = var->label;

  if (token == TLPAREN) {
    pretty_print();
    token = scan();
    if (parse_expressions() == ERROR) return ERROR;
    if (token != TRPAREN) return error("Right parenthesis(')') is not found");
    pretty_print();
    token = scan();
  }

  gen_code("CALL", proclabel);

  return NORMAL;
}

int parse_expressions() {
  int type;
  struct TYPE* ptp = var->itp->paratp;
  if (ptp == NULL) return error("Procedure needs parameters");

  left_flag = 1;
  if ((type = parse_expression()) == ERROR) return ERROR;
  left_flag = 0;

  if (type != ptp->ttype) return error("Expression type is different to paratp");

  if (var_flag) {
    gen_code("PUSH", "0,GR1");
    var_flag = 0;
  } else {
    gen_lad("GR2", "=0", NULL);
    gen_st("GR1", "0", "GR2");
    gen_code("PUSH", "0,GR2");
  }

  ptp = ptp->paratp;

  while (token == TCOMMA) {
    pretty_print();
    token = scan();

    left_flag = 1;
    if ((type = parse_expression()) == ERROR) return ERROR;
    left_flag = 0;

    if (ptp == NULL)
      return error("Not correct number of dummy arguments");
    if (type != ptp->ttype)  // types don't match
      return error("Expression type is different to paratp");

    if (var_flag) {
      gen_code("PUSH", "0,GR1");
      var_flag = 0;
    } else {
      gen_lad("GR2", "=0", NULL);
      gen_st("GR1", "0", "GR2");
      gen_code("PUSH", "0,GR2");
    }

    ptp = ptp->paratp;
  }
  if (ptp != NULL) return error("Procedure requires arguments");
  return NORMAL;
}

int parse_return_statement() {
  if (token != TRETURN) error(ERRMSG_RETURN_NOT_FOUND);

  pretty_print();
  token = scan();
  return NORMAL;
}

int parse_assignment_statement() {
  int l_type, r_type;

  left_flag = 1;
  if ((l_type = parse_left_part()) == ERROR) return ERROR;
  left_flag = 0;

  if (token != TASSIGN) return error("Assignment(':=') is not found");

  gen_code("PUSH", "0,GR1");

  pretty_print();
  token = scan();
  if ((r_type = parse_expression()) == ERROR) return ERROR;

  // if (l_type != r_type) return error("Cannot assign different type");
  if (l_type != r_type) {
    if (!(l_type == TPINT && (r_type == TBOOLEAN || r_type == TPCHAR))) {
      return error("These type are not same.");
    }
  }

  if (addr_flag) addr_flag = 0;

  gen_code("POP", "GR2");
  gen_st("GR1", "0", "GR2");

  return NORMAL;
}

int parse_left_part() {
  int type;
  if ((type = parse_variable()) == ERROR) return ERROR;
  return type;
}

int parse_variable() {
  int type;
  num_flag = 0;
  var_flag = 1;
  addr_flag = 0;

  struct ID* vp;

  // task2
  // if (parse_variable_name() == ERROR) return ERROR;

  // task3
  if (token != TNAME) return ERROR;
  char* np = get_string();
  if ((var = get_declared_id(np)) == NULL) {
    free(np);
    return error("Variable is not defined");
  }
  free(np);

  // 変数の参照行番号を追加
  add_refline(var, get_linenum());
  type = var->itp->ttype;
  vp = var;

  // instead of calling parse_variable_name()
  pretty_print();
  token = scan();

  if (token == TLSQPAREN) {
    int lf_tmp = left_flag;
    left_flag = 0;
    num_flag = 0;

    int idx_type;
    if (type != TPARRAYINT && type != TPARRAYBOOL && type != TPARRAYCHAR) return error("Expected standard type, but there is [] after variable name");

    pretty_print();
    token = scan();
    if ((idx_type = parse_expression()) == ERROR) return ERROR;
    if (idx_type != TPINT) return error("Index of array must be integer");

    if (var_flag && addr_flag) {  // 12-22追加
      gen_ld("GR1", "0", "GR1");
      var_flag = 0;
    }

    // overflow check
    gen_code("CPA", "GR1,GR0");
    gen_code("JMI", "EROV");
    gen_lad_num("GR2", vp->itp->arraysize - 1);
    gen_code("CPA", "GR1,GR2");
    gen_code("JPL", "EROV");

    if (lf_tmp) {
      gen_lad("GR1", vp->label, "GR1");
    } else {
      gen_ld("GR1", vp->label, "GR1");
    }

    if (num_flag) {
      int idx = get_num();
      if (idx < 0 || idx >= var->itp->arraysize) return error("Index of array is out of range");
    }

    if (token != TRSQPAREN) return error("Right bracket(']') is not found");
    pretty_print();
    token = scan();

    // convert TPARRAY** to TP**
    type -= 3;
  } else {
    if (vp->ispara) {
      gen_ld("GR1", vp->label, NULL);
      addr_flag = 1;
      if (!left_flag) {
        gen_ld("GR1", "0", "GR1");
        var_flag = 0;
      }
    } else {
      if (left_flag) {
        gen_lad("GR1", vp->label, NULL);
        addr_flag = 1;
      } else {
        gen_ld("GR1", vp->label, NULL);
      }
    }
  }
  return type;
}

int parse_expression() {
  // gen_comment("parse_expression()");
  int op;
  int type, type1, type2;
  int label1, label2;

  if ((type = parse_simple_expression()) == ERROR) return ERROR;

  while (token == TEQUAL || token == TNOTEQ || token == TLE || token == TLEEQ || token == TGR || token == TGREQ) {
    if (var_flag && addr_flag) {
      gen_ld("GR1", "0", "GR1");
      var_flag = 0;
      addr_flag = 0;
    }

    gen_code("PUSH", "0,GR1");
    type1 = type;
    if ((op = parse_relational_operator()) == ERROR) return ERROR;
    if ((type2 = parse_simple_expression()) == ERROR) return ERROR;
    if (type1 != type2) return error("Cannot be compared due to type mismatch.");
    type = TPBOOL;

    gen_code("POP", "GR2");
    label1 = get_label_num();
    label2 = get_label_num();

    if (op == TEQUAL) {
      gen_code("CPA", "GR2,GR1");
      gen_code_label("JZE", NULL, label1);
      // not equal
      gen_lad_num("GR1", 0);
      gen_code_label("JUMP", NULL, label2);
      // equal
      gen_label(label1);
      gen_lad_num("GR1", 1);
      gen_label(label2);
    } else if (op == TNOTEQ) {
      gen_code("CPA", "GR2,GR1");
      gen_code_label("JZE", NULL, label1);
      // not equal
      gen_lad_num("GR1", 1);
      gen_code_label("JUMP", NULL, label2);
      // equal
      gen_label(label1);
      gen_lad_num("GR1", 0);
      gen_label(label2);
    } else if (op == TLE) {
      gen_code("CPA", "GR2,GR1");
      gen_code_label("JMI", NULL, label1);
      // not less
      gen_lad_num("GR1", 0);
      gen_code_label("JUMP", NULL, label2);
      // less
      gen_label(label1);
      gen_lad_num("GR1", 1);
      gen_label(label2);
    } else if (op == TLEEQ) {
      gen_code("CPA", "GR2,GR1");
      gen_code_label("JZE", NULL, label1);
      gen_code_label("JMI", NULL, label1);
      // not less or equal
      gen_lad_num("GR1", 0);
      gen_code_label("JUMP", NULL, label2);
      // less or equal
      gen_label(label1);
      gen_lad_num("GR1", 1);
      gen_label(label2);
    } else if (op == TGR) {
      gen_code("CPA", "GR2,GR1");
      gen_code_label("JPL", NULL, label1);
      // not greater
      gen_lad_num("GR1", 0);
      gen_code_label("JUMP", NULL, label2);
      // greater
      gen_label(label1);
      gen_lad_num("GR1", 1);
      gen_label(label2);
    } else if (op == TGREQ) {
      gen_code("CPA", "GR2,GR1");
      gen_code_label("JMI", NULL, label1);
      // greater or equal
      gen_lad_num("GR1", 1);
      gen_code_label("JUMP", NULL, label2);
      // not greater or equal
      gen_label(label1);
      gen_lad_num("GR1", 0);
      gen_label(label2);
    }
  }
  return type;
}

int parse_simple_expression() {
  int flag = 0;  // whether prefix '+' or '-' is found
  int type, type1, type2, op;

  if (token == TPLUS || token == TMINUS) {
    pretty_print();
    token = scan();
    flag = 1;
  }

  int token_tmp = token;

  if ((type = parse_term()) == ERROR) return ERROR;
  if (flag && type != TPINT) return error("Expected + or - followed by integer, but followed by not integer");

  if (flag && token_tmp == TMINUS) {
    gen_code("XOR", "GR1,#FFFF");
    gen_lad_num("GR2", 1);
    gen_code("ADDA", "GR1,GR2");
  }

  while (token == TPLUS || token == TMINUS || token == TOR) {
    if (var_flag && addr_flag) {
      gen_ld("GR1", "0", "GR1");
      var_flag = 0;
      addr_flag = 0;
    }

    gen_code("PUSH", "0,GR1");

    type1 = type;
    if ((op = parse_additive_operator()) == ERROR) return ERROR;
    if ((type2 = parse_term()) == ERROR) return ERROR;

    gen_code("POP", "GR2");

    if (op == TOR) {
      if (type1 != TPBOOL && type2 != TPBOOL) return error("OR operation require two boolean");
      type = TPBOOL;
      gen_code("OR", "GR1,GR2");
    } else {
      if (type1 != TPINT || type2 != TPINT) return error("ADD or SUB require two integer");
      type = TPINT;

      if (op == TPLUS) {
        gen_code("ADDA", "GR1,GR2");
        gen_code("JOV", "EOVF");
      } else if (op == TMINUS) {
        gen_code("SUBA", "GR2,GR1");
        gen_code("JOV", "EOVF");
        gen_code("LD", "GR1,GR2");
      }
    }
  }
  return type;
}

int parse_term() {
  int type, type1, type2, op;
  if ((type = parse_factor()) == ERROR) return ERROR;

  while (token == TSTAR || token == TDIV || token == TAND) {
    if (var_flag && addr_flag) {
      gen_ld("GR1", "0", "GR1");
      var_flag = 0;
      addr_flag = 0;
    }

    gen_code("PUSH", "0,GR1");

    type1 = type;
    if ((op = parse_multiplicative_operator()) == ERROR) return ERROR;
    if ((type2 = parse_factor()) == ERROR) return ERROR;

    if (var_flag && addr_flag) {
      gen_ld("GR1", "0", "GR1");
      var_flag = 0;
      addr_flag = 0;
    }

    gen_code("POP", "GR2");

    if (op == TAND) {
      if (type1 != TPBOOL && type2 != TPBOOL) return error("AND operation requires two boolean");
      type = TPBOOL;
    } else {
      if (type1 != TPINT && type2 != TPINT) return error("STAR or DIV operation requires two integer");
      type = TPINT;
    }

    if (op == TSTAR) {
      gen_code("MULA", "GR1,GR2");
      gen_code("JOV", "EOVF");
    } else if (op == TDIV) {
      gen_code("DIVA", "GR2,GR1");
      gen_code("JOV", "EOVF");
      gen_code("LD", "GR1,GR2");
    } else if (op == TAND) {
      gen_code("AND", "GR1,GR2");
    }
  }
  return type;
}

int parse_factor() {
  int type;
  int label1, label2;

  if (token == TNAME) {
    if ((type = parse_variable()) == ERROR) return ERROR;
  } else if (token == TNUMBER || token == TFALSE || token == TTRUE || token == TSTRING) {
    if ((type = parse_constant()) == ERROR) return ERROR;
  } else if (token == TLPAREN) {
    pretty_print();
    token = scan();
    if ((type = parse_expression()) == ERROR) return ERROR;
    if (token != TRPAREN) return error("Right parenthesis(')') is not found");
    pretty_print();
    token = scan();
  } else if (token == TNOT) {
    pretty_print();
    token = scan();
    if ((type = parse_factor()) == ERROR) return ERROR;
    if (type != TPBOOL) return error("Expected NOT followed by boolean, but not found boolean");

    label1 = get_label_num();
    label2 = get_label_num();

    gen_code("CPL", "GR1,GR0");
    gen_code_label("JZE", NULL, label1);
    // true
    gen_lad_num("GR1", 0);
    gen_code_label("JUMP", NULL, label2);
    // false
    gen_label(label1);
    gen_lad_num("GR1", 1);
    gen_label(label2);

  } else if (token == TINTEGER || token == TBOOLEAN || token == TCHAR) {
    if ((type = parse_standard_type()) == ERROR) return ERROR;
    if (token != TLPAREN) return error("Left parenthesis('(') is not found");
    pretty_print();
    token = scan();

    int exp_type;
    if ((exp_type = parse_expression()) == ERROR) return ERROR;

    // typeとexp_typeの互換性も確認する...??
    if (exp_type != TPINT && exp_type != TPCHAR && exp_type != TPBOOL) return error("Array type cannot be cast");

    if (token != TRPAREN) return error("Right parenthesis(')') is not found");
    pretty_print();
    token = scan();

    if (exp_type == TPINT) {
      if (type == TPCHAR) {
        gen_lad("GR2", "#007F", NULL);
        gen_and("GR1", "GR2", NULL);
      } else if (type == TPBOOL) {
      }
    } else if (exp_type == TPCHAR) {
      if (type == TPINT) {
      } else if (type == TPBOOL) {
      }
    } else if (exp_type == TPBOOL) {
      if (type == TPINT) {
      } else if (type == TPCHAR) {
      }
    }
  }

  return type;
}

int parse_constant() {
  int type, num;
  if (token != TNUMBER && token != TFALSE && token != TTRUE && token != TSTRING) return error("Constant is not found");

  char literal[MAXSTRSIZE] = "";

  if (token == TNUMBER) {
    type = TPINT;
    num_flag = 1;
    num = get_num();
    gen_lad_num("GR1", num);
    // snprintf(literal, sizeof(literal) + 1, "=%d", get_num());
  } else if (token == TFALSE || token == TTRUE) {
    type = TPBOOL;
    num = (token == TTRUE) ? 1 : 0;
    gen_lad_num("GR1", num);
    // snprintf(literal, sizeof(literal) + 1, "=%d", (token == TTRUE) ? 1 : 0);
  } else if (token == TSTRING) {
    if (get_string_attr_len() != 1) return error("string length must be 1 in constant");
    type = TPCHAR;
    snprintf(literal, sizeof(literal) + 3, "='%s'", string_attr);
    gen_ld("GR1", literal, NULL);
  }

  // gen_lad("GR1", literal, NULL);
  // gen_ld("GR1", "0", "GR1");

  pretty_print();
  token = scan();
  return type;
}

int parse_multiplicative_operator() {
  int op;
  if (token != TSTAR && token != TDIV && token != TAND) return error("Multiplicative operator is not found");
  op = token;
  pretty_print();
  token = scan();
  return op;
}

int parse_additive_operator() {
  int op;
  if (token != TPLUS && token != TMINUS && token != TOR) return error("Additive operator is not found");
  op = token;
  pretty_print();
  token = scan();
  return op;
}

int parse_relational_operator() {
  int op;
  if (token != TEQUAL && token != TNOTEQ && token != TLE && token != TLEEQ && token != TGR && token != TGREQ) return error("Relational operator is not found");
  op = token;
  pretty_print();
  token = scan();
  return op;
}

int parse_input_statement() {
  int type;
  int is_readln = 0;

  if (token != TREAD && token != TREADLN) return error("The input statement must begin with either 'read' or 'readln'");

  if (token == TREADLN) is_readln = 1;

  pretty_print();
  token = scan();
  if (token == TLPAREN) {
    pretty_print();
    token = scan();

    left_flag = 1;
    if ((type = parse_variable()) == ERROR) return ERROR;
    left_flag = 0;

    if (type != TPINT && type != TPCHAR) return error("Input statements require integer or char args");

    if (type == TPINT) {
      gen_code("CALL", "READINT");
    } else {
      gen_code("CALL", "READCHAR");
    }

    while (token == TCOMMA) {
      pretty_print();
      token = scan();

      left_flag = 1;
      if ((type = parse_variable()) == ERROR) return ERROR;
      left_flag = 0;

      if (type == TPINT) {
        gen_code("CALL", "READINT");
      } else {
        gen_code("CALL", "READCHAR");
      }
    }

    if (is_readln) gen_code("CALL", "READLINE");

    if (token != TRPAREN) return error("Right parenthesis(')') is not found");
    pretty_print();
    token = scan();
  }
  return NORMAL;
}

int parse_output_statement() {
  int is_writeln = 0;
  if (token != TWRITE && token != TWRITELN) return error("The output statement must begin with either 'write' or 'writeln'");

  if (token == TWRITELN) is_writeln = 1;

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

  if (is_writeln) gen_code("CALL", "WRITELINE");

  return NORMAL;
}

int parse_output_format() {
  int type;

  char literal[MAXSTRSIZE] = "";

  if (token == TSTRING && get_string_attr_len() != 1) {
    type = TPCHAR;

    pretty_print();
    token = scan();
    if (token == TCOLON) return error("The string before the colon (:) must be a single character");

    snprintf(literal, sizeof(literal) + 3, "='%s'", string_attr);
    gen_lad("GR1", literal, NULL);
    gen_lad_num("GR2", 0);
    gen_code("CALL", "WRITESTR");
  } else {
    if ((type = parse_expression()) == ERROR) return ERROR;
    if (type != TPINT && type != TPBOOL && type != TPCHAR) return error("Expression must be standard type at output format");

    if (token == TCOLON) {
      pretty_print();
      token = scan();
      if (token != TNUMBER) return error("Integer is not found");
      pretty_print();
      token = scan();
      gen_lad_num("GR2", get_num());
    } else {
      gen_lad_num("GR2", 0);
    }

    switch (type) {
      case TPINT:
        gen_code("CALL", "WRITEINT");
        break;
      case TPCHAR:
        gen_code("CALL", "WRITECHAR");
        break;
      case TPBOOL:
        gen_code("CALL", "WRITEBOOL");
        break;
    }
  }
  return type;
}

int parse_empty_statement() {
  empty_stmt_flag = 1;
  return NORMAL;
}