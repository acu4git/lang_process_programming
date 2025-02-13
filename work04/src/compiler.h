#ifndef COMPILER_H
#define COMPILER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int get_label_num(void);
void gen_label(int label);
void gen_code(char *opc, char *opr);
void gen_code_label(char *opc, char *opr, int label);
void gen_start(char *progname, int label);
void gen_def_var(char *label, int n);
void gen_proc(char *label);
void gen_st(char *opr1, char *opr2, char *opr3);
void gen_param(void);
void gen_ld(char *opr1, char *opr2, char *opr3);
void gen_lad(char *opr1, char *opr2, char *opr3);
void gen_lad_num(char *opr1, int n);
void gen_suba(char *opr1, char *opr2, char *opr3);
void gen_and(char *opr1, char *opr2, char *opr3);
void gen_ret(void);
void gen_end(void);
void gen_comment(char *);
void outlib(void);

#endif