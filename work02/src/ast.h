#ifndef AST_H
#define AST_H

#include <stdlib.h>
#include <string.h>

typedef struct _ASTNode {
  char *type;
  char *value;
  struct _ASTNode *left;
  struct _ASTNode *right;
  struct _ASTNode *next;
} ASTNode;

ASTNode *create_node(char *type, char *value, ASTNode *left, ASTNode *right);

#endif