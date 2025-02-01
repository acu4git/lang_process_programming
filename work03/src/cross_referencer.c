#include "cross_referencer.h"

struct ID *globalidroot = NULL; /* Pointer to root of global symbol tables */
struct ID *localidroot = NULL;  /* Pointer to root of local symbol tables */
struct ID *idtab = NULL;        /* result table */

void init_idtab() {
  globalidroot = NULL;
  localidroot = NULL;
  idtab = NULL;
}