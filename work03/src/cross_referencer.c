#include "cross_referencer.h"

struct ID *global_tab = NULL; /* Pointer to root of global symbol tables */
struct ID *local_tab = NULL;  /* Pointer to root of local symbol tables */
struct ID *cr_tab = NULL;     /* result table */

void init_tab() {
  global_tab = NULL;
  local_tab = NULL;
  cr_tab = NULL;
}

struct ID *new_id(char *name, char *procname, int ispara, int deflinenum) {
  struct ID *p;
  if ((p = (struct ID *)malloc(sizeof(struct ID))) == NULL) {
    error("Failed to allocate memory");
    return NULL;
  }
  p->name = name;
  p->procname = procname;
  p->itp = NULL;
  p->ispara = ispara;
  p->deflinenum = deflinenum;
  p->irefp = NULL;
  p->nextp = NULL;
  return p;
}

struct TYPE *new_type(int ttype, int arraysize, struct TYPE *etp, struct TYPE *paratp) {
  struct TYPE *p;
  if ((p = (struct TYPE *)malloc(sizeof(struct TYPE))) == NULL) {
    error("Failed to allocate memory");
    return NULL;
  }
  p->ttype = ttype;
  p->arraysize = arraysize;
  p->etp = etp;
  p->paratp = paratp;
  return p;
}

struct ID *search_global(struct ID *id) {
  struct ID *p, *q;
  for (p = global_tab; p != NULL; p = p->nextp) {
    for (q = id; q != NULL; q = q->nextp) {
      if (!strcmp(p->name, q->name)) return p;
    }
  }
  return NULL;
}

struct ID *search_local(struct ID *id) {
  struct ID *p, *q;
  for (p = local_tab; p != NULL; p = p->nextp) {
    for (q = id; q != NULL; q = q->nextp) {
      if (!strcmp(p->name, q->name)) return p;
    }
  }
  return NULL;
}

int register_bulk(struct ID *id) {
  struct ID *p = NULL;
  if (id->procname != NULL)
    p = search_local(id);
  else
    p = search_global(id);

  if (id->procname != NULL) {
    if (p != NULL) return error("Duplicate registration for local symbol id");

    p = local_tab;
    if (p == NULL) {
      local_tab = id;
    } else {
      for (p = local_tab; p->nextp != NULL; p = p->nextp);
      p->nextp = id;
    }
  } else {
    if (p != NULL) return error("Duplicate registration for global symbol id");

    p = global_tab;
    if (p == NULL) {
      global_tab = id;
    } else {
      for (p = global_tab; p->nextp != NULL; p = p->nextp);
      p->nextp = id;
    }
  }

  return NORMAL;
}

// 仮引数内の型が標準型かどうか
int is_valid_param() {
  struct ID *p;
  for (p = local_tab; p != NULL; p = p->nextp) {
    if (p->itp == NULL) {
      return error("Type information is missing at is_valid_param");
    }
    int type = p->itp->ttype;
    if (type != TPINT && type != TPCHAR && type != TPBOOL) return error("Parameter must be standard type");
  }
  return NORMAL;
}

void push(struct ID **head, struct ID *id) {
  if (*head == NULL) {
    *head = id;
  } else {
    struct ID *p;
    for (p = *head; p->nextp != NULL; p = p->nextp);
    p->nextp = id;
  }
}

void assign_type(struct ID *idlist, struct TYPE *type) {
  for (struct ID *p = idlist; p != NULL; p = p->nextp) {
    p->itp = type;
  }
}

struct TYPE *assign_parameter(struct TYPE *tp) {
  struct ID *p;
  struct TYPE *t = tp;
  struct TYPE *newtp;

  for (p = local_tab; p != NULL; p = p->nextp) {
    if ((newtp = (struct TYPE *)malloc(sizeof(struct TYPE))) == NULL) {
      error("Failed to allocate memory at assign_parameter");
      return NULL;
    }

    newtp->ttype = p->itp->ttype;
    newtp->arraysize = 0;
    newtp->etp = NULL;
    newtp->paratp = NULL;

    t->paratp = newtp;
    t = t->paratp;
  }

  return tp;
}

void add_refline(struct ID *id, int linenum) {
  struct LINE *line;
  if ((line = (struct LINE *)malloc(sizeof(struct LINE))) == NULL) {
    error("Failed to allocate memory at assign_line");
    return;
  }
  line->reflinenum = linenum;
  line->nextlinep = NULL;

  if (id->irefp == NULL) {
    id->irefp = line;
  } else {
    struct LINE *lp;
    for (lp = id->irefp; lp->nextlinep != NULL; lp = lp->nextlinep);
    lp->nextlinep = line;
  }
}

struct ID *get_declared_id(char *varname) {
  struct ID *p;

  for (p = local_tab; p != NULL; p = p->nextp) {
    if (!strcmp(p->name, varname)) return p;
  }

  for (p = global_tab; p != NULL; p = p->nextp) {
    if (!strcmp(p->name, varname)) return p;
  }

  return NULL;
}

void add_cr_table(int n) {
  struct ID *id, *p;
  char *name = NULL, *procname = NULL;
  int len;

  switch (n) {
    case IS_GLOBAL:
      p = global_tab;
      break;
    case IS_LOCAL:
      p = local_tab;
      break;
    default:
      error("Invalid case is found at add_cr_table()");
      return;
  }

  for (; p != NULL; p = p->nextp) {
    if ((id = (struct ID *)malloc(sizeof(struct ID))) == NULL) {
      error("Failed to allocate memory at add_cr_table()");
      return;
    }

    len = strlen(p->name);
    if ((name = (char *)malloc(sizeof(char) * (len + 1))) == NULL) {
      error("Failed to allocate memory at add_cr_table()");
      return;
    }
    strcpy(name, p->name);

    if (p->procname != NULL) {
      len = strlen(p->procname);
      if ((procname = (char *)malloc(sizeof(char) * (len + 1))) == NULL) {
        error("Failed to allocate memory at add_cr_table()");
        return;
      }
      strcpy(procname, p->procname);
    }

    id->name = name;
    id->procname = procname;
    id->itp = p->itp;
    id->ispara = p->ispara;
    id->deflinenum = p->deflinenum;
    id->irefp = p->irefp;
    id->nextp = cr_tab;
    cr_tab = id;
  }
}

void clear_table(int n) {
  struct ID *tab, *p, *q;
  // struct TYPE *tp;
  // struct LINE *l1, *l2;

  switch (n) {
    case IS_GLOBAL:
      tab = global_tab;
      break;
    case IS_LOCAL:
      tab = local_tab;
      break;
    case IS_CROSS:
      tab = cr_tab;
      break;
    default:
      error("Invalid case is found at clear_table");
      return;
  }

  if (tab == NULL) return;

  for (p = tab; p != NULL; p = q) {
    // clear name
    if (p->name != NULL) {
      free(p->name);
      p->name = NULL;
    }

    // clear procname
    if (p->procname != NULL && p->procname != p->name) {
      struct ID *check;
      int is_unique = 1;
      for (check = tab; check != NULL; check = check->nextp) {
        if (check != p && check->procname == p->procname) {
          is_unique = 0;  // 他の struct ID* も参照している
          break;
        }
      }
      if (is_unique) {  // 参照が 1 つしかない場合のみ free()
        free(p->procname);
      }
      p->procname = NULL;
    }

    if (n == IS_CROSS) {
      // clear type
      if (p->itp != NULL) {
        struct ID *check;
        int is_unique = 1;

        // 他の struct ID* も同じ itp を参照しているかチェック
        for (check = cr_tab; check != NULL; check = check->nextp) {
          if (check != p && check->itp == p->itp) {
            is_unique = 0;  // 他にも参照しているものがある
            break;
          }
        }

        if (is_unique) {                                                                       // **唯一の所有者の場合のみ free()**
          if (p->itp->ttype == TPINT || p->itp->ttype == TPCHAR || p->itp->ttype == TPBOOL) {  // standard type
            free(p->itp);
          } else if (p->itp->ttype == TPARRAYINT || p->itp->ttype == TPARRAYCHAR || p->itp->ttype == TPARRAYBOOL) {  // array type
            if (p->itp->etp != NULL) {
              free(p->itp->etp);
              p->itp->etp = NULL;
            }
            free(p->itp);
          } else if (p->itp->ttype == TPPROC) {  // procedure
            struct TYPE *tp = p->itp->paratp;
            while (tp != NULL) {
              struct TYPE *tp_tmp = tp;
              tp = tp->paratp;
              free(tp_tmp);
            }
            free(p->itp);
          }
        }

        p->itp = NULL;
      }
    }

    q = p->nextp;
    free(p);
  }

  switch (n) {
    case IS_GLOBAL:
      global_tab = NULL;
      break;
    case IS_LOCAL:
      local_tab = NULL;
      break;
    case IS_CROSS:
      cr_tab = NULL;
      break;
  }
}

void print_tab() {
  struct ID *p;

  printf("%-*s ", TAB_SIZE_NAME, "Name");
  printf("| %-*s ", TAB_SIZE_TYPE, "Type");
  printf("| %-*s ", TAB_SIZE_DEFINE, "Define");
  printf("| %-*s\n", TAB_SIZE_REFERENCES, "References");

  for (p = cr_tab; p != NULL; p = p->nextp) {
    // Name
    char name[TAB_SIZE_NAME + 1];
    if (p->procname != NULL) {
      sprintf(name, "%s:%s", p->name, p->procname);
    } else {
      sprintf(name, "%s", p->name);
    }
    printf("%-*s ", TAB_SIZE_NAME, name);

    // Type
    if (p->itp == NULL) {
      error("Type information is misssing at print_tab()");
      return;
    }
    switch (p->itp->ttype) {
      case TPINT:
        printf("| %-*s ", TAB_SIZE_TYPE, "integer");
        break;
      case TPCHAR:
        printf("| %-*s ", TAB_SIZE_TYPE, "char");
        break;
      case TPBOOL:
        printf("| %-*s ", TAB_SIZE_TYPE, "boolean");
        break;
      case TPARRAYINT:
      case TPARRAYCHAR:
      case TPARRAYBOOL:
        char typename_array[TAB_SIZE_TYPE + 1];
        sprintf(typename_array, "array[%d] of ", p->itp->arraysize);
        switch (p->itp->ttype) {
          case TPARRAYINT:
            strcat(typename_array, "integer");
            break;
          case TPARRAYCHAR:
            strcat(typename_array, "char");
            break;
          case TPARRAYBOOL:
            strcat(typename_array, "boolean");
            break;
        }
        printf("| %-*s ", TAB_SIZE_TYPE, typename_array);
        break;
      case TPPROC:
        struct TYPE *q = p->itp->paratp;
        char procname[TAB_SIZE_TYPE * 2 + 1];
        sprintf(procname, "procedure");
        if (q != NULL) {
          strcat(procname, "(");
          for (; q != NULL; q = q->paratp) {
            switch (q->ttype) {
              case TPINT:
                strcat(procname, "integer");
                break;
              case TPCHAR:
                strcat(procname, "char");
                break;
              case TPBOOL:
                strcat(procname, "boolean");
                break;
            }
            if (q->paratp != NULL) {
              strcat(procname, ",");
            }
          }
          strcat(procname, ")");
        }
        printf("| %-*s ", TAB_SIZE_TYPE, procname);
    }

    // Define
    printf("| %-*d ", TAB_SIZE_DEFINE, p->deflinenum);

    // References
    struct LINE *lp = p->irefp;
    putchar('|');
    if (lp != NULL) {
      printf(" %d", lp->reflinenum);
      lp = lp->nextlinep;
      for (; lp != NULL; lp = lp->nextlinep) printf(",%d", lp->reflinenum);
    }
    putchar('\n');
  }
}