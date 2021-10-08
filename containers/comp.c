#include <stdio.h>
#include "../y.tab.h"
#include "containers.h"

int yyparse();

struct node_fun_str* fun_r = NULL;
struct node_fun_str* fun_t = NULL;
struct node_var_str* var_r = NULL;
struct node_var_str* var_t = NULL;


int getFunsAndVars()
{
  /*
  if (node->ntoken == DEFFUN)
  {
    char *funName = get_child(node, 1);
  }
  */
  return 0;
}

int isTerm(struct ast *node)
{
  int arr[] = {CONST, GETINT, PLUS, MINUS, MULT, DIV, MOD, VARID}; 
  for (u_int i = 0; i < 8; i++)
    if (node->ntoken == arr[i]) return 0;

  if (node->ntoken == IF) 
    return !(isTerm(get_child(node, 2)) == 0 && isTerm(get_child(node, 3)) == 0);
  else if (node->ntoken == CALL)    // yet to fix
    return 0;
  else if (node->ntoken == LET)
    return isTerm(get_child(node, 3));
  return 1;
}

int isFla(struct ast *node)
{
  int arr[] = {TRUE, FALSE, VARID, GETBOOL, EQUAL, LT, GT, LE, GE, NOT, LAND, LOR};
  for (u_int i = 0; i < 12; i++)
    if (node->ntoken == arr[i]) return 0;

  if (node->ntoken == IF)
    return !(isFla(get_child(node, 2)) == 0 && isFla(get_child(node, 3)) == 0);
  else if (node->ntoken == CALL)    // yet to fix
    return 0;
  else if (node->ntoken == LET)
    return isFla(get_child(node, 3));
}

int tc(struct ast* node)
{
  if (node->ntoken == PLUS || node->ntoken == MINUS || node->ntoken == MULT 
    || node->ntoken == DIV || node->ntoken == MOD || node->ntoken == EQUAL
    || node->ntoken == LT || node->ntoken == LE || node->ntoken == GT 
    || node->ntoken == GE)
  {
    struct ast_child *child = node->child;
    while (child != NULL)
    {
      if (isTerm(child->id) != 0) return 1;
      child = child->next;
    }
  }
  else if (node->ntoken == NOT || node->ntoken == LAND || node->ntoken == LOR)
  {
    struct ast_child *child = node->child;
    while (child != NULL)
    {
      if (isFla(child->id) != 0) return 1;
      child = child->next;
    }
  }
  else if (node->ntoken == IF)
  {
    return !(isFla(get_child(node, 1)) == 0 && 
      ((isTerm(get_child(node, 2)) == 0 && isTerm(get_child(node, 3)) == 0) || 
      (isFla(get_child(node, 2)) == 0 && isFla(get_child(node, 3)) == 0)));
  }

  return 0;
}

int typecheck()
{
  return visit_ast(tc);
}

int main (int argc, char **argv) {
  int retval = yyparse();
  if (retval != 0) return 1;
  visit_ast(getFunsAndVars);
  if (retval == 0) retval = typecheck();
  if (retval == 0) print_ast();      // run `dot -Tpdf ast.dot -o ast.pdf` to create a PDF
  else printf("Semantic error\n");
  free_ast();
  return retval;
}
