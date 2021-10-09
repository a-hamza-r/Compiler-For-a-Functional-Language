#include <stdio.h>
#include "../y.tab.h"
#include "containers.h"

int yyparse();
int isFla(struct ast*);
int isTerm(struct ast*);

struct node_fun_str* fun_r = NULL;
struct node_fun_str* fun_t = NULL;
struct node_var_str* var_r = NULL;
struct node_var_str* var_t = NULL;


int getFunsAndVars(struct ast* node)
{
  if (node->ntoken == DEFFUN)
  {
    char *funName = get_child(node, 1)->token;
    if (find_fun_str(funName, fun_r) != NULL)
    {
      printf("double declaration\n");
      return 1;
    }
    int numChild = get_child_num(node);
    
    struct node_int *argTypes_r = NULL;
    struct node_int *argTypes_t = NULL;

    for (int i = 2; i < numChild-1; i++)
    {
      struct ast* n = get_child(node, i);
      push_int(n->ntoken, &argTypes_r, &argTypes_t);
    }

    push_fun_str(funName, get_child(node, numChild-1)->ntoken, numChild-3, 
      argTypes_r, &fun_r, &fun_t);
  }
  else if (node->ntoken == INTDECL || node->ntoken == BOOLDECL)
  {
    struct ast* parent = node->parent;
    //printf("found variable decl: %s\n", node->token);
    int type = node->ntoken == INTDECL ? INT : BOOL;
    int numChild = get_child_num(parent);
    struct ast* lastChild = get_child(parent, numChild);
    push_var_str(node->id, lastChild->id, type, node->token, &var_r, &var_t);
  }
  else if (node->ntoken == VARDECL)
  {
    struct ast* parent = node->parent;
    struct ast* c2 = get_child(parent, 2);
    struct ast* c3 = get_child(parent, 3);
    int type = isFla(c2) ? INT : BOOL;
    push_var_str(node->id, c3->id, type, node->token, &var_r, &var_t);
  }
  return 0;
}

int isTerm(struct ast *node)
{
  int arr[] = {CONST, GETINT, PLUS, MINUS, MULT, DIV, MOD}; 
  for (u_int i = 0; i < 7; i++)
    if (node->ntoken == arr[i]) return 0;

  if (node->ntoken == IF) 
    return !(isTerm(get_child(node, 2)) == 0 && isTerm(get_child(node, 3)) == 0);
  else if (node->ntoken == LET)
    return isTerm(get_child(node, 3));
  else if (node->ntoken == VARID)
  {
    struct node_var_str* v = find_var_str(node->id, node->token, var_r);
    if (v != NULL) return !(v->type == INT);
  }
  else if (node->ntoken == CALL)
  {
    struct node_fun_str* f = find_fun_str(node->token, fun_r);
    if (f != NULL) return !(f->type == INT);
  }
  return 1;
}

int isFla(struct ast *node)
{
  int arr[] = {TRUE, FALSE, GETBOOL, EQUAL, LT, GT, LE, GE, NOT, LAND, LOR};
  for (u_int i = 0; i < 11; i++)
    if (node->ntoken == arr[i]) return 0;

  if (node->ntoken == IF)
    return !(isFla(get_child(node, 2)) == 0 && isFla(get_child(node, 3)) == 0);
  else if (node->ntoken == LET)
    return isFla(get_child(node, 3));
  else if (node->ntoken == VARID)
  {
    struct node_var_str* v = find_var_str(node->id, node->token, var_r);
    if (v != NULL) return !(v->type == BOOL);
  }
  else if (node->ntoken == CALL)
  {
    struct node_fun_str* f = find_fun_str(node->token, fun_r);
    if (f != NULL) return !(f->type == BOOL);
  }
  return 1;
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
  else if (node->ntoken == VARID)
  {
    if (find_var_str(node->id, node->token, var_r) == NULL)
    {
      printf("Variable '%s' is out of scope\n", node->token);
      return 1;
    }
  }
  else if (node->ntoken == CALL)
  {
    struct node_fun_str* info = find_fun_str(node->token, fun_r);
    if (info == NULL)
    {
      printf("Function %s is not declared\n", node->token);
      return 1;
    }
    if (info->arity != get_child_num(node)) 
    {
      printf("Number of arguments does not match the function declaration\n");
      return 1;
    }
    struct ast_child* c = node->child;
    struct node_int* types = info->argTypes;
    while (c != NULL && types != NULL)
    {
      if (!((isTerm(c->id) == 0 && types->id == INTDECL) 
        || (isFla(c->id) == 0 && types->id == BOOLDECL)))
      {
        return 1;
      }
      c = c->next; types = types->next;
    }
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
  if (visit_ast(getFunsAndVars) == 1) return 1;
  if (retval == 0) retval = typecheck();
  if (retval == 0) print_ast();      // run `dot -Tpdf ast.dot -o ast.pdf` to create a PDF
  else printf("Semantic error\n");
  free_ast();
  return retval;
}