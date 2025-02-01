#ifndef CROSS_REFERENCER_H
#define CROSS_REFERENCER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define TPINT 0
#define TPCHAR 1
#define TPBOOL 2
#define TPARRAYINT 3
#define TPARRAYCHAR 4
#define TPARRAYBOOL 5
#define TPPROC 6

#define IS_GLOBAL 0
#define IS_LOCAL 1

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
}; /* Pointers to root of global & local symbol tables */

void init_idtab();
int register_id(struct ID *);
void print_idtab();

#endif