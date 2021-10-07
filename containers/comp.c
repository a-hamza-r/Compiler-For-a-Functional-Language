#include <stdio.h>
#include "../y.tab.h"
#include "containers.h"

int yyparse();

int isTerm(struct ast *node)
{
  return 0;
}

int tc(struct ast* node)
{
  /* 
  printf("visit: %s\n", node->token);
  struct ast_child *child = node->child;
  while (child != NULL)
  {
    printf("  has child: %s\n", child->id->token);
    child = child->next;
  }

  if (node->ntoken == AROP)
  {
    struct ast_child *child = node->child;
    while (child != NULL)
    {
      if (isTerm(child->id) != 0) return 1;
      child = child->next;
    }
  }
  */

  return 0;
}

int typecheck()
{
  return visit_ast(tc);
}

int main (int argc, char **argv) {
  int retval = yyparse();
  if (retval == 0) retval = typecheck();
  else return 1;
  if (retval == 0) print_ast();      // run `dot -Tpdf ast.dot -o ast.pdf` to create a PDF
  else printf("Semantic error\n");
  free_ast();
  return retval;
}
