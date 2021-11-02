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

// get function and variable declarations
int getFunsAndVars(struct ast* node)
{
  // function declarations
  if (node->ntoken == DEFFUN)
  {
    char *funName = get_child(node, 1)->token;
    if (find_fun_str(funName, fun_r) != NULL)
    {
      printf("double declaration of functions\n");
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
 
    return 0;
  }

  // Variable declarations
  int type;
  struct ast* parent = node->parent;
  if (node->ntoken == INTDECL || node->ntoken == BOOLDECL)    // variable decls inside function
    type = node->ntoken == INTDECL ? INT : BOOL;
  else if (node->ntoken == VARDECL)                           // variable decls inside let
    type = isFla(get_child(parent, 2)) ? INT : BOOL;
  else return 0;

  int numChild = get_child_num(parent);
  struct ast* firstChild = parent->child->id;
  struct ast* lastChild = get_child(parent, numChild);
  if (find_var_str(node->id, node->token, var_r) != NULL) 
  {
    printf("variable %s already declared in this scope\n", node->token);
    return 1;
  }
  push_var_str(firstChild->id, lastChild->id, type, node->token, &var_r, &var_t);
  return 0;
}


// check if a given node is a term, i.e., has a type INT
int isTerm(struct ast *node)
{
  int arr[] = {CONST, GETINT, PLUS, MINUS, MULT, DIV, MOD}; 
  for (u_int i = 0; i < sizeof(arr)/sizeof(arr[0]); i++)
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


// check if a given node is a formula, i.e. has a type BOOL
int isFla(struct ast *node)
{
  int arr[] = {TRUE, FALSE, GETBOOL, EQUAL, LT, GT, LE, GE, NOT, LAND, LOR};
  for (u_int i = 0; i < sizeof(arr)/sizeof(arr[0]); i++)
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

int typecheck(struct ast* node)
{
  // handle operations that have all children as terms
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
  // handle operations that have all children as flas
  else if (node->ntoken == NOT || node->ntoken == LAND || node->ntoken == LOR)
  {
    struct ast_child *child = node->child;
    while (child != NULL)
    {
      if (isFla(child->id) != 0) return 1;
      child = child->next;
    }
  }
  // handle if-statement
  else if (node->ntoken == IF)
  {
    return !(isFla(get_child(node, 1)) == 0 && 
      ((isTerm(get_child(node, 2)) == 0 && isTerm(get_child(node, 3)) == 0) || 
      (isFla(get_child(node, 2)) == 0 && isFla(get_child(node, 3)) == 0)));
  }
  // handle variable use
  else if (node->ntoken == VARID)
  {
    if (find_var_str(node->id, node->token, var_r) == NULL)
    {
      printf("Variable '%s' is out of scope\n", node->token);
      return 1;
    }
  }
  // handle function calls
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
        printf("Type mismatch for variables of function %s\n", node->token);
        return 1;
      }
      c = c->next; types = types->next;
    }
  }
  // check the return type of a declared function
  else if (node->ntoken == DEFFUN)
  {
    char *funcName = node->child->id->token;
    struct ast* lastChild = get_child(node, get_child_num(node));
    struct node_fun_str* info = find_fun_str(funcName, fun_r);
    if (!(info->type == INT && isTerm(lastChild) == 0 
      || info->type == BOOL && isFla(lastChild) == 0))
    {
      printf("Return type of %s does not match the body\n", funcName);
      return 1;
    }
  }
  // check if any declared variable is not the same as function 
  if (node->ntoken == INTDECL || node->ntoken == BOOLDECL || node->ntoken == VARDECL)
  {
    if (find_fun_str(node->token, fun_r) != NULL)
    {
      printf("variable %s is declared as a function\n", node->token);
      return 1;
    }
  }

  return 0;
}

int main (int argc, char **argv) {
  int retval = yyparse();
  if (retval != 0) return 1;
  retval = visit_ast(getFunsAndVars);
  if (retval == 0) retval = visit_ast(typecheck);
  if (retval == 0) print_ast();      // run `dot -Tpdf ast.dot -o ast.pdf` to create a PDF
  else printf("Semantic error\n");
  
  clean_fun_str(&fun_r);
  clean_var_str(&var_r);
  free_ast();

  return retval;
}
