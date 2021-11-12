#include <stdio.h>
#include "y.tab.h"
#include "containers/containers.h"
#include "containers/instructions.h"

int yyparse();
int isFla(struct ast*);
int isTerm(struct ast*);
void print_asgn_smt(struct asgn_instr*);

// symbol table
struct node_fun_str* fun_r = NULL;
struct node_fun_str* fun_t = NULL;
struct node_var_str* var_r = NULL;
struct node_var_str* var_t = NULL;

// CFG data structures
struct br_instr* bb_root = NULL;
struct br_instr* bb_tail = NULL;

struct asgn_instr* asgn_root = NULL;
struct asgn_instr* asgn_tail = NULL;

struct node_istr* ifun_r = NULL;
struct node_istr* ifun_t = NULL;

struct node_int* assert_num_root = NULL;
struct node_int* assert_num_tail = NULL;

struct asgn_instr_r_t* assert_instrs_r = NULL;
struct asgn_instr_r_t* assert_instrs_t = NULL;

int current_assert = 0;
bool foundEntry = false;

int total_assert_blocks = 0;

int retun_val_counter = 0;

// get function and variable declarations
int get_funs_and_vars(struct ast* node)
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
  push_var_str(firstChild->id, lastChild->id, type, node->token, -1, &var_r, &var_t);
  return 0;
}


// check if a given node is a term, i.e., has a type INT
int isTerm(struct ast *node)
{
  int arr[] = {CONST, GETINT, PLUS, MINUS, MULT, DIV, MOD}; 
  for (unsigned int i = 0; i < sizeof(arr)/sizeof(arr[0]); i++)
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
  for (unsigned int i = 0; i < sizeof(arr)/sizeof(arr[0]); i++)
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


void procRec(struct ast* node)
{
  if (node->ntoken == IF)
  {
    procRec(get_child(node, 1));  // recurse for guard 
   
    // then branch
    push_int(assert_num_tail->id+1, &assert_num_root, &assert_num_tail);
    procRec(get_child(node, 2));  // recurse for then
    
    // else branch
    push_int(assert_num_tail->id+1, &assert_num_root, &assert_num_tail);
    procRec(get_child(node, 3));  // recurse for else
    
    // join
    push_int(assert_num_tail->id+1, &assert_num_root, &assert_num_tail);
  }
  else if (node->ntoken == CALL)
  {
    push_int(assert_num_tail->id+1, &assert_num_root, &assert_num_tail);
    struct node_fun_str* f = find_fun_str(node->token, fun_r);
    int numChild = get_child_num(node);
    for (int i = 1; i <= numChild; i++)
      procRec(get_child(node, i));
    procRec(f->end);
    /*struct ast* temp = f->start;
    while (temp->id <= f->end->id)
    {
      printf("func tokens: %s\n", temp->token);
      temp = temp->next;
    }
    */
    /* struct asgn_instr* instrs = f->instrs;
    while (instrs != NULL)
    {
      print_asgn_smt(instrs);
      instrs = instrs->next;
    }
    */
  }
  else 
  {
    int numChild = get_child_num(node);
    for (int i = 1; i <= numChild; i++)
      procRec(get_child(node, i));
  }
}


int compute_br_structure(struct ast* node)
{
  // function definition
  if (node->ntoken == CALL && strcmp(node->token, "ENTRY") == 0)
  {
    struct ast* parent = node->parent;
    // create a new CFG, with entry and exit
    push_int(0, &assert_num_root, &assert_num_tail);
    procRec(get_child(node, 1));

    push_int(assert_num_tail->id+1, &assert_num_root, &assert_num_tail);

    total_assert_blocks = assert_num_tail->id+1;
    for (int i = 0; i < total_assert_blocks; i++)
      push_asgn_instr_r_t(NULL, NULL, &assert_instrs_r, &assert_instrs_t);
  }

  return 0;
}

int get_register_val(struct ast* c)
{
  // if the node is a variable use, then use the register id of where it's declared
  // otherwise, just return node id
  if (c->ntoken == VARID)
  {
    struct node_var_str* v = find_var_str(c->id, c->token, var_r);
    return v->reg_id;
  }
  return c->id;
}


int fill_instrs(struct ast* node)
{
  struct ast* n = node;
  struct asgn_instr* asgn;
  struct asgn_instr_r_t* current = get(assert_instrs_r, current_assert);
  
  while (n->parent != NULL)
  {
    if (n->parent->ntoken == DEFFUN) return 0;
    n = n->parent;
  }

  if (node->ntoken == CALL && strcmp(node->token, "ENTRY") == 0)
  {
    struct asgn_instr* asgn = mk_uasgn(current_assert, 0, get_child(node, 1)->id, INT, EVAL);
    push_asgn(asgn, &current->asgn_r, &current->asgn_t);
  }

  if (node->parent != NULL)
  {
    struct ast* n = node->parent;
    while (n != NULL)
    {
      if (!foundEntry && n->ntoken == CALL && strcmp(n->token, "ENTRY") == 0)
      {
        current_assert = pop_int_front(&assert_num_root, &assert_num_tail);
        foundEntry = true;
      }
      n = n->parent;
    }
  }
  if (node->ntoken == CONST)
  {
    int lhs = node->id;
    int op1 = atoi(node->token);
    asgn = mk_uasgn(current_assert, lhs, op1, INT, node->ntoken);
    push_asgn(asgn, &current->asgn_r, &current->asgn_t);
  }
  if (node->ntoken == TRUE || node->ntoken == FALSE)
  {
    int lhs = node->id;
    asgn = mk_uasgn(current_assert, lhs, -1, BOOL, node->ntoken);
    push_asgn(asgn, &current->asgn_r, &current->asgn_t);
  }
  
  if (node->ntoken == EQUAL || node->ntoken == MINUS || node->ntoken == DIV
    || node->ntoken == MOD || node->ntoken == LT || node->ntoken == LE
    || node->ntoken == GT || node->ntoken == GE || node->ntoken == PLUS
    || node->ntoken == MULT || node->ntoken == LAND || node->ntoken == LOR)
  {
    int numChild = get_child_num(node);
    struct ast* c1 = get_child(node, 1);
    int op1 = get_register_val(c1), op2;
    int lhs = node->id;
    int type = (node->ntoken == MINUS || node->ntoken == DIV || node->ntoken == MOD 
      || node->ntoken == PLUS || node->ntoken == MULT) ? INT : BOOL;
    for (int i = 2; i < numChild; i++)
    {
      struct ast* currChild = get_child(node, i);
      op2 = get_register_val(currChild);
      asgn = mk_basgn(current_assert, lhs, op1, op2, type, node->ntoken);
      push_asgn(asgn, &current->asgn_r, &current->asgn_t);

      op1 = lhs;
    }
    struct ast* currChild = get_child(node, numChild);
    op2 = get_register_val(currChild);
    asgn = mk_basgn(current_assert, lhs, op1, op2, type, node->ntoken);
    push_asgn(asgn, &current->asgn_r, &current->asgn_t);
  }
  if (node->ntoken == NOT)
  {
    int lhs = node->id;
    struct ast* c1 = get_child(node, 1);
    int op1 = get_register_val(c1);
    asgn = mk_uasgn(current_assert, lhs, op1, BOOL, node->ntoken);
    push_asgn(asgn, &current->asgn_r, &current->asgn_t);
  }
  if (node->ntoken == GETINT || node->ntoken == GETBOOL)
  {
    int lhs = node->id;
    char *tok = node->token;
    asgn = mk_casgn(current_assert, lhs, tok);
    push_asgn(asgn, &current->asgn_r, &current->asgn_t);
  }
  /*
  if (node->ntoken == CALL && strcmp(node->token, "ENTRY") != 0)
  {
    for (int i = 1; i <= get_child_num(node); i++)
    {
      asgn = mk_uasgn(current_assert, -i, get_child(node, i)->id, -1);
      push_asgn(asgn, &current->asgn_r, &current->asgn_t);
    }
    int lhs = node->id;
    char *tok = node->token;
    asgn = mk_casgn(current_assert, lhs, tok);
    push_asgn(asgn, &current->asgn_r, &current->asgn_t);
  }
  */
  if (node->ntoken == LET)
  {
    int lhs = node->id;
    struct ast* thirdChild = get_child(node, 3);
    int op1 = get_register_val(thirdChild);
    asgn = mk_uasgn(current_assert, lhs, op1, (isFla(thirdChild) == 0) ? BOOL : INT, -1);
    push_asgn(asgn, &current->asgn_r, &current->asgn_t);
  }
  if (node->parent != NULL)
  {
    struct ast* parent = node->parent;
    if (parent->ntoken == CALL && strcmp(parent->token, "ENTRY") == 0)
    {
      current_assert = pop_int_front(&assert_num_root, &assert_num_tail);
    }

    if (parent->ntoken == LET)
    {
      struct ast* firstChild = get_child(parent, 1);
      struct ast* secondChild = get_child(parent, 2);
      if (node->id == secondChild->id)
      {
        int lhs = firstChild->id;
        struct node_var_str* v = find_var_str(firstChild->id, firstChild->token, var_r);
        v->reg_id = lhs;
        int op1 = get_register_val(secondChild);
        asgn = mk_uasgn(current_assert, lhs, op1, (isFla(secondChild) == 0) ? BOOL : INT, -1);
        push_asgn(asgn, &current->asgn_r, &current->asgn_t);
      }
    }
    // if last child of define-fun, assign rv to its associated register
    /*
    if (parent->ntoken == DEFFUN)
    {
      struct ast* lastChild = get_child(parent, get_child_num(parent));
      if (node->id == lastChild->id)
      {
        int op1 = get_register_val(lastChild);
        asgn = mk_uasgn(current_assert, 0, op1, lastChild->ntoken);
        push_asgn(asgn, &current->asgn_r, &current->asgn_t);
      }
    }
    */
    if (parent->ntoken == IF)
    {
      struct ast* firstChild = get_child(parent, 1);
      struct ast* secondChild = get_child(parent, 2);
      int type = (isFla(secondChild) == 0) ? BOOL : INT;
      if (node->id == firstChild->id)
      {
        current_assert = pop_int_front(&assert_num_root, &assert_num_tail);
      }
      else if (node->id == secondChild->id)
      {
        asgn = mk_uasgn(current_assert, parent->id, node->id, type, -1);
        push_asgn(asgn, &current->asgn_r, &current->asgn_t);
        current->cond = firstChild->id;
        current->neg = 1;
        current_assert = pop_int_front(&assert_num_root, &assert_num_tail);
      }
      else
      {
        asgn = mk_uasgn(current_assert, parent->id, node->id, 999, -1);
        push_asgn(asgn, &current->asgn_r, &current->asgn_t);
        current->cond = firstChild->id;
        current->neg = -1;
        current_assert = pop_int_front(&assert_num_root, &assert_num_tail);
      }
    }
  }

  /*
  if (node->ntoken == FUNID)
  {
    //current_bb_for_instrs = pop_int_front(&assert_num_root, &assert_num_tail);
    struct ast* parent = node->parent;
    char *funcName = node->token;
    struct node_fun_str* info = find_fun_str(funcName, fun_r);
    for (int i = 1; i <= info->arity; i++)
    {
      struct ast* childNode = get_child(parent, i+1);
      int lhs = childNode->id, op1 = -i;
      struct asgn_instr *asgn = mk_uasgn(current_bb_for_instrs, lhs, op1, -1);
      push_asgn(asgn, &asgn_root, &asgn_tail);
      //struct node_var_str* v = find_var_str(childNode->id, childNode->token, var_r);
      //v->reg_id = lhs;
    }
  }
  */
  return 0;
}


void print_asgn_smt(struct asgn_instr *asgn)
{
  printf("\t");
  if (asgn->bin == 0) 
  {
    if (asgn->type == CONST)
      printf("(= v%d %d)\n", asgn->lhs, asgn->op1);
    else if (asgn->type == TRUE)
      printf("(= v%d true)\n", asgn->lhs);
    else if (asgn->type == FALSE)
      printf("(= v%d false)\n", asgn->lhs);
    else if (asgn->type == EVAL)
      printf("(not v%d)\n", asgn->op1);
    else if (asgn->type == NOT)
      printf("(= v%d (not v%d))\n", asgn->lhs, asgn->op1);
    else if (asgn->op1 < 0)
      printf("(= v%d a%d)\n", asgn->lhs, -asgn->op1);
    else if (asgn->lhs == 0)
      printf("(= rv v%d)\n", asgn->op1);
    else if (asgn->lhs < 0)
      printf("(= a%d v%d)\n", -asgn->lhs, asgn->op1);
    else 
      printf("(= v%d v%d)\n", asgn->lhs, asgn->op1);
  }
  else if (asgn->bin == 1) 
  {
    if (asgn->type == EQUAL)
      printf("(= v%d (= v%d v%d))\n", asgn->lhs, asgn->op1, asgn->op2);
    else if (asgn->type == PLUS)
      printf("(= v%d (+ v%d v%d))\n", asgn->lhs, asgn->op1, asgn->op2);
    else if (asgn->type == MULT)
      printf("(= v%d (* v%d v%d)\n", asgn->lhs, asgn->op1, asgn->op2);
    else if (asgn->type == MINUS)
      printf("(= v%d (- v%d v%d))\n", asgn->lhs, asgn->op1, asgn->op2);
    else if (asgn->type == DIV)
      printf("(= v%d (div v%d v%d))\n", asgn->lhs, asgn->op1, asgn->op2);
    else if (asgn->type == MOD)
      printf("(= v%d (mod v%d v%d))\n", asgn->lhs, asgn->op1, asgn->op2);
    else if (asgn->type == LT)
      printf("(= v%d (< v%d v%d))\n", asgn->lhs, asgn->op1, asgn->op2);
    else if (asgn->type == GT)
      printf("(= v%d (> v%d v%d))\n", asgn->lhs, asgn->op1, asgn->op2);
    else if (asgn->type == LE)
      printf("(= v%d (<= v%d v%d))\n", asgn->lhs, asgn->op1, asgn->op2);
    else if (asgn->type == GE)
      printf("(= v%d (>= v%d v%d))\n", asgn->lhs, asgn->op1, asgn->op2);
    else if (asgn->type == LAND)
      printf("(= v%d (and v%d v%d))\n", asgn->lhs, asgn->op1, asgn->op2);
    else if (asgn->type == LOR)
      printf("(= v%d (or v%d v%d))\n", asgn->lhs, asgn->op1, asgn->op2);
  }
  else if (asgn->bin == 2)
  {
    //printf("call %s\n", asgn->fun);
    if (strcmp(asgn->fun, "print") != 0)
      printf("(= v%d rv%d)\n", asgn->lhs, retun_val_counter++);
  }
}

void print_asgndecl_smt(struct asgn_instr *asgn)
{
  if (asgn->bin != 2) 
  {
    if (asgn->lhs_type == INT)
    {
      printf("(declare-var v%d Int)\n", asgn->lhs);
    }
    else if (asgn->lhs_type == BOOL)
    {
      printf("(declare-var v%d Bool)\n", asgn->lhs);
    }
  }
}

void print_stmts_smt(struct asgn_instr *asgn_root)
{
  struct asgn_instr_r_t* temp = assert_instrs_r;
  while (temp != NULL)
  {
    printf("(assert\n");
    
    if (temp->cond != -1)
    {
      if (temp->neg == 1)
        printf("(=> v%d\n", temp->cond);
      else 
        printf("(=> (not v%d)\n", temp->cond);
    }
    printf("(and\n");
    struct asgn_instr* temp2 = temp->asgn_r;
    while (temp2 != NULL)
    {
      print_asgn_smt(temp2);
      temp2 = temp2->next;
    }
    printf(")\n");
    
    if (temp->cond != -1)
      printf(")\n");
    
    printf(")\n\n");
    temp = temp->next;
  }

  printf("(check-sat)\n");
}

void print_decls_smt(struct asgn_instr *asgn_root)
{
  struct asgn_instr_r_t* temp = assert_instrs_r;
  while (temp != NULL)
  {
    struct asgn_instr* temp2 = temp->asgn_r;
    while (temp2 != NULL)
    {
      print_asgndecl_smt(temp2);
      temp2 = temp2->next;
    }
    temp = temp->next;
  }
  printf("\n");
}

int compute_functions(struct ast* node)
{
  if (node->ntoken == DEFFUN)
  {
    struct ast* lastChild = get_child(node, get_child_num(node));
    struct ast* secondLastChild = get_child(node, get_child_num(node)-1);
    char *funcName = get_child(node, 1)->token;
    struct node_fun_str* info = find_fun_str(funcName, fun_r);
    info->start = secondLastChild->next;
    info->end = lastChild;
    /* 
    struct asgn_instr *instrs_root, *instrs_tail;
    struct asgn_instr *asgn;

    int op1, lhs;
    for (int i = 1; i <= info->arity; i++)
    {
      struct ast* childNode = get_child(node, i+1);
      lhs = childNode->id, op1 = -i;
      
      asgn = mk_uasgn(current_bb_for_instrs, lhs, op1, -1);
      push_asgn(asgn, &instrs_root, &instrs_tail);
      
      struct node_var_str* v = find_var_str(childNode->id, childNode->token, var_r);
      v->reg_id = lhs;
    }

    struct ast* secondLastChild = get_child(node, get_child_num(node)-1);
    struct ast* temp = secondLastChild->next;
    while (temp->id <= lastChild->id)
    {
      fill_instrs_simple(temp, instrs_root, instrs_tail);
      temp = temp->next;
    }
    op1 = get_register_val(lastChild);
    asgn = mk_uasgn(current_bb_for_instrs, 0, op1, lastChild->ntoken);
    push_asgn(asgn, &instrs_root, &instrs_tail);

    struct asgn_instr *t = instrs_root;
    while (t != NULL)
    {
      printf("1\n");
      t = t->next;
    }
  
    info->instrs = instrs_root;
    */
  }
  return 0;
}


int main (int argc, char **argv) {
  int retval = yyparse();
  if (retval != 0) return 1;
  retval = visit_ast(get_funs_and_vars);
  if (retval == 0) print_ast();      // run `dot -Tpdf ast.dot -o ast.pdf` to create a PDF
  else 
  {
    printf("Semantic error\n");
    return 1;
  }
  visit_ast(compute_functions);
  visit_ast(compute_br_structure);
  visit_ast(fill_instrs);
  print_decls_smt(asgn_root);
  print_stmts_smt(asgn_root);

  clean_fun_str(&fun_r);
  clean_var_str(&var_r);
  free_ast();

  clean_istr(&ifun_r);
  clean_bbs(&bb_root);
  clean_asgns(&asgn_root);

  return retval;
}
