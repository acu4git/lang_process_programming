#ifndef CROSS_REFERENCER_H
#define CROSS_REFERENCER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NORMAL 0
#define ERROR -1

#define TPINT 0
#define TPCHAR 1
#define TPBOOL 2
#define TPARRAYINT 3
#define TPARRAYCHAR 4
#define TPARRAYBOOL 5
#define TPPROC 6

#define IS_LOCAL 0
#define IS_GLOBAL 1
#define IS_CROSS 2

#define MAX_NAMELIST_SIZE 16

#define TAB_SIZE_NAME 20
#define TAB_SIZE_TYPE 30
#define TAB_SIZE_DEFINE 6
#define TAB_SIZE_REFERENCES 10

extern int error(char *mes);

struct TYPE {
  int ttype;           /* TPINT TPCHAR TPBOOL TPARRAYINT TPARRAYCHAR TPARRAYBOOL TPPROC*/
  int arraysize;       /* 配列型の場合の配列サイズ */
  struct TYPE *etp;    /* 配列型の場合の要素の型 */
  struct TYPE *paratp; /* 手続き型の場合の，仮引数の型リスト */
};

struct LINE {
  int reflinenum;
  struct LINE *nextlinep;
};

struct ID {
  char *name;
  char *procname;     /* 手続き宣言内で定義されている時の手続き名，それ以外はNULL */
  struct TYPE *itp;   /* 型 */
  int ispara;         /* 1:仮引数, 0:通常の変数 */
  int deflinenum;     /* 定義行 */
  struct LINE *irefp; /* 参照行のリスト */
  struct ID *nextp;
  char *label;  // label for casl
}; /* Pointers to root of global & local symbol tables */

void init_tab();
struct ID *get_tab(int n);
struct ID *new_id(char *name, char *procname, int ispara, int deflinenum);
struct TYPE *new_type(int ttype, int arraysize, struct TYPE *etp, struct TYPE *paratp);
void push(struct ID **head, struct ID *id);
int register_bulk(struct ID *);
int is_valid_param();
void assign_type(struct ID *idlist, struct TYPE *);
struct TYPE *assign_parameter(struct TYPE *);
void add_refline(struct ID *, int);
struct ID *get_declared_id(char *);
void add_cr_table(int);
void clear_table(int);
void print_tab();

#endif