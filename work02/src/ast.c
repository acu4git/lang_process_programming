#include "ast.h"

char *copy(const char *src) {
  if (!src) return NULL;
  size_t len = strlen(src) + 1;
  char *copy = (char *)malloc(len);
  if (copy) strcpy(copy, src);
  return copy;
}

ASTNode *create_node(char *type, char *value, ASTNode *left, ASTNode *right) {
  ASTNode *node = (ASTNode *)malloc(sizeof(ASTNode));
  if (!node) return NULL;
  node->type = copy(type);
  node->value = copy(value);
  node->left = left;
  node->right = right;
  node->next = NULL;
  return node;
}