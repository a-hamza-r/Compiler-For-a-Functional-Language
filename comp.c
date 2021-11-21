#include <stdio.h>
#include "y.tab.h"
#include "containers/containers.h"
#include "containers/instructions.h"

int yyparse();
int isFla(struct ast*);
int isTerm(struct ast*);

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

struct asgn_instr* first_asgns_root = NULL;
struct asgn_instr* first_asgns_tail = NULL;

struct node_istr* ifun_r = NULL;
struct node_istr* ifun_t = NULL;

struct node_int* bb_num_root = NULL;
struct node_int* bb_num_tail = NULL;

int current_bb = 0;
int current_bb_for_instrs = -1;
bool foundEntry = false;

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
   
    int tmp_bb1 = current_bb+1;
    int tmp_bb2 = current_bb+2;
    int tmp_bb3 = current_bb+3;
    current_bb+=4;
   
    struct br_instr* bri = mk_cbr(bb_num_tail->id, 999, tmp_bb1, tmp_bb2);
    push_br(bri, &bb_root, &bb_tail);
    struct br_instr* b = find_br_instr(bb_num_tail->id, bb_root);
    
    // bb for then
    push_int(tmp_bb1, &bb_num_root, &bb_num_tail);
    procRec(get_child(node, 2));  // recurse for then
    bri = mk_ubr(bb_num_tail->id, tmp_bb3);
    push_br(bri, &bb_root, &bb_tail);
    
    // bb for else
    push_int(tmp_bb2, &bb_num_root, &bb_num_tail);
    procRec(get_child(node, 3));  // recurse for else
    bri = mk_ubr(bb_num_tail->id, tmp_bb3);
    push_br(bri, &bb_root, &bb_tail);
    
    // bb for join
    push_int(tmp_bb3, &bb_num_root, &bb_num_tail);
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
  struct br_instr *bri;
  // function definition
  if (node->ntoken == FUNID || (node->ntoken == CALL && strcmp(node->token, "ENTRY") == 0))
  {
    struct ast* parent = node->parent;
    char *istr = node->ntoken == FUNID ? node->token : "print";
    // create a new CFG, with entry and exit
    push_istr(current_bb, istr, &ifun_r, &ifun_t);
    push_int(current_bb, &bb_num_root, &bb_num_tail);
    if (node->ntoken == FUNID)
      procRec(get_child(parent, get_child_num(parent)));
    else 
      procRec(get_child(node, 1));
    bri = mk_ubr(bb_num_tail->id, -1);
    push_br(bri, &bb_root, &bb_tail);

    current_bb++;
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
  if (node->parent != NULL)
  {
    struct ast* n = node->parent;
    while (n != NULL)
    {
      if (!foundEntry && n->ntoken == CALL && strcmp(n->token, "ENTRY") == 0)
      {
        current_bb_for_instrs = pop_int_front(&bb_num_root, &bb_num_tail);
        foundEntry = true;
      }
      n = n->parent;
    }
  }
  if (node->ntoken == EQUAL || node->ntoken == MINUS || node->ntoken == DIV 
    || node->ntoken == MOD || node->ntoken == LT || node->ntoken == LE 
    || node->ntoken == GT || node->ntoken == GE || node->ntoken == PLUS 
    || node->ntoken == MULT || node->ntoken == LAND || node->ntoken == LOR)
  {
    int numChild = get_child_num(node);
    struct ast* c1 = get_child(node, 1);
    int op1 = c1->id, op2;
    int lhs = node->id;
    for (int i = 2; i < numChild; i++)
    {
      struct ast* currChild = get_child(node, i);
      op2 = currChild->id;
      struct asgn_instr *asgn = mk_basgn(current_bb_for_instrs, lhs, op1, op2, node->ntoken);
      push_asgn(asgn, &asgn_root, &asgn_tail);

      op1 = lhs;
    }
    struct ast* currChild = get_child(node, numChild);
    op2 = currChild->id;
    struct asgn_instr *asgn = mk_basgn(current_bb_for_instrs, lhs, op1, op2, node->ntoken);
    push_asgn(asgn, &asgn_root, &asgn_tail);
  }
  if (node->ntoken == CONST)
  {
    int lhs = node->id;
    int op1;
    if (strcmp(node->token, "true") == 0) op1 = 1;
    else if (strcmp(node->token, "false") == 0) op1 = 0;
    else op1 = atoi(node->token);
    struct asgn_instr *asgn = mk_uasgn(current_bb_for_instrs, lhs, op1, node->ntoken);
    push_asgn(asgn, &asgn_root, &asgn_tail);
  }
  if (node->ntoken == NOT)
  {
    int lhs = node->id;
    struct ast* c1 = get_child(node, 1);
    int op1 = c1->id;
    struct asgn_instr *asgn = mk_uasgn(current_bb_for_instrs, lhs, op1, node->ntoken);
    push_asgn(asgn, &asgn_root, &asgn_tail);
  }
  if (node->ntoken == VARID)
  {
    struct node_var_str* v = find_var_str(node->id, node->token, var_r);
    struct asgn_instr *asgn = mk_uasgn(current_bb_for_instrs, node->id, v->reg_id, -1);
    push_asgn(asgn, &asgn_root, &asgn_tail);
  }
  if (node->ntoken == GETINT || node->ntoken == GETBOOL || node->ntoken == CALL)
  {
    if (node->ntoken == CALL)
    {
      for (int i = 1; i <= get_child_num(node); i++)
      {
        struct asgn_instr *asgn = mk_uasgn(current_bb_for_instrs, -i, get_child(node, i)->id, -1);
        push_asgn(asgn, &asgn_root, &asgn_tail);
      }
    }
    int lhs = node->id;
    char *tok = (node->ntoken == CALL && strcmp(node->token, "ENTRY") == 0) ? "print" : node->token;
    struct asgn_instr *asgn = mk_casgn(current_bb_for_instrs, lhs, tok);
    push_asgn(asgn, &asgn_root, &asgn_tail);
  }
  if (node->ntoken == FUNID)
  {
    struct node_int* b = bb_num_root;
    current_bb_for_instrs = pop_int_front(&bb_num_root, &bb_num_tail);
    struct ast* parent = node->parent;
    char *funcName = node->token;
    struct node_fun_str* info = find_fun_str(funcName, fun_r);
    for (int i = 1; i <= info->arity; i++)
    {
      struct ast* childNode = get_child(parent, i+1);
      int lhs = childNode->id, op1 = -i;
      struct asgn_instr *asgn = mk_uasgn(current_bb_for_instrs, lhs, op1, -1);
      push_asgn(asgn, &asgn_root, &asgn_tail);
      struct node_var_str* v = find_var_str(childNode->id, childNode->token, var_r);
      v->reg_id = lhs;
    }
  }
  if (node->ntoken == LET)
  {
    int lhs = node->id;
    struct ast* thirdChild = get_child(node, 3);
    int op1 = thirdChild->id;
    struct asgn_instr *asgn = mk_uasgn(current_bb_for_instrs, lhs, op1, -1);
    push_asgn(asgn, &asgn_root, &asgn_tail);
  }
  if (node->parent != NULL)
  {
    struct ast* parent = node->parent;
    if (parent->ntoken == LET)
    {
      struct ast* firstChild = get_child(parent, 1);
      struct ast* secondChild = get_child(parent, 2);
      if (node->id == secondChild->id)
      {
        int lhs = firstChild->id;
        struct node_var_str* v = find_var_str(firstChild->id, firstChild->token, var_r);
        v->reg_id = lhs;
        int op1 = secondChild->id;
        struct asgn_instr *asgn = mk_uasgn(current_bb_for_instrs, lhs, op1, -1);
        push_asgn(asgn, &asgn_root, &asgn_tail);
      }
    }
    // if last child of define-fun, assign rv to its associated register 
    if (parent->ntoken == DEFFUN)
    {
      struct ast* lastChild = get_child(parent, get_child_num(parent));
      if (node->id == lastChild->id)
      {
        int op1 = lastChild->id;
        struct asgn_instr *asgn = mk_uasgn(current_bb_for_instrs, 0, op1, lastChild->ntoken);
        push_asgn(asgn, &asgn_root, &asgn_tail);
      }
    }
    if (parent->ntoken == IF)
    {
      struct ast* firstChild = get_child(parent, 1);
      struct ast* secondChild = get_child(parent, 1);
      if (node->id == firstChild->id)
      {
        struct br_instr* b = find_br_instr(current_bb_for_instrs, bb_root);
        if (b != NULL) b->cond = node->id;
        current_bb_for_instrs = pop_int_front(&bb_num_root, &bb_num_tail);
      }
      else 
      {
        struct asgn_instr *asgn = mk_uasgn(current_bb_for_instrs, parent->id, node->id, -1);
        push_asgn(asgn, &asgn_root, &asgn_tail);
        current_bb_for_instrs = pop_int_front(&bb_num_root, &bb_num_tail);
      }
    }
  }
  return 0;
}


void print_asgn(struct asgn_instr *asgn)
{
  if (asgn->bin == 0) 
  {
    if (asgn->type == CONST)
      printf("v%d := %d\n", asgn->lhs, asgn->op1);
    else if (asgn->type == NOT)
      printf("v%d := not v%d\n", asgn->lhs, asgn->op1);
    else if (asgn->op1 < 0)
      printf("v%d := a%d\n", asgn->lhs, -asgn->op1);
    else if (asgn->lhs == 0)
      printf("rv := v%d\n", asgn->op1);
    else if (asgn->lhs < 0)
      printf("a%d := v%d\n", -asgn->lhs, asgn->op1);
    else 
      printf("v%d := v%d\n", asgn->lhs, asgn->op1);
  }
  else if (asgn->bin == 1) 
  {
    if (asgn->type == EQUAL)
      printf("v%d := v%d = v%d\n", asgn->lhs, asgn->op1, asgn->op2);
    else if (asgn->type == PLUS)
      printf("v%d := v%d + v%d\n", asgn->lhs, asgn->op1, asgn->op2);
    else if (asgn->type == MULT)
      printf("v%d := v%d * v%d\n", asgn->lhs, asgn->op1, asgn->op2);
    else if (asgn->type == MINUS)
      printf("v%d := v%d - v%d\n", asgn->lhs, asgn->op1, asgn->op2);
    else if (asgn->type == DIV)
      printf("v%d := v%d / v%d\n", asgn->lhs, asgn->op1, asgn->op2);
    else if (asgn->type == MOD)
      printf("v%d := v%d mod v%d\n", asgn->lhs, asgn->op1, asgn->op2);
    else if (asgn->type == LT)
      printf("v%d := v%d < v%d\n", asgn->lhs, asgn->op1, asgn->op2);
    else if (asgn->type == GT)
      printf("v%d := v%d > v%d\n", asgn->lhs, asgn->op1, asgn->op2);
    else if (asgn->type == LE)
      printf("v%d := v%d <= v%d\n", asgn->lhs, asgn->op1, asgn->op2);
    else if (asgn->type == GE)
      printf("v%d := v%d >= v%d\n", asgn->lhs, asgn->op1, asgn->op2);
    else if (asgn->type == LAND)
      printf("v%d := v%d && v%d\n", asgn->lhs, asgn->op1, asgn->op2);
    else if (asgn->type == LOR)
      printf("v%d := v%d || v%d\n", asgn->lhs, asgn->op1, asgn->op2);
  }
  else if (asgn->bin == 2)
  {
    printf("call %s\n", asgn->fun);
    if (strcmp(asgn->fun, "print") != 0)
      printf("v%d := rv\n", asgn->lhs);
  }
}


void print_interm(struct asgn_instr *asgn_root)
{
  struct asgn_instr* asgn = asgn_root;
  struct br_instr* br = bb_root;
  char* fun_name = find_istr(ifun_r, br->id);
  printf("function %s\n\n", fun_name);
  printf("entry:\n");

  while (asgn != NULL){
    if (asgn->bb != br->id){
      if (br->cond == 0){
        if (br->succ1 == -1) {
          printf("br exit\n\n");
        }
        else printf("br bb%d\n\n", br->succ1);
      }
      else printf("br v%d bb%d bb%d\n\n", br->cond, br->succ1, br->succ2);
      br = br->next;
      if (br == NULL) return;
      fun_name = find_istr(ifun_r, br->id);
      if (fun_name != NULL){
        printf("function %s\n\n", fun_name);
        printf("entry:\n");
      } else {
        printf("bb%d:\n", br->id);
      }
    }
    if (asgn->bb == br->id) {
      print_asgn(asgn);
      asgn = asgn->next;
    }
  }
  printf("br exit\n\n");
}


struct asgn_instr* find_asgn_in_bb(struct asgn_instr* start, int lhs, int bb)
{
  struct asgn_instr* asgn = start;
  struct asgn_instr* ret_val = NULL;
  while (asgn != NULL)
  {
    if (asgn->lhs == lhs && asgn->bb == bb)
      ret_val = asgn;
    asgn = asgn->next;
  }
  return ret_val;
}


void remove_bb(struct asgn_instr* asgn, int to_remove, struct br_instr* br)
{
  // TODO: corner cases like asgn's next is NULL
  struct br_instr* temp_br = br;
  while (temp_br != NULL)
  {
    if (temp_br->next != NULL && temp_br->next->id == to_remove)
    {
      struct br_instr* next = temp_br->next;
      temp_br->next = next->next;
      free(next);
    }
    else
      temp_br = temp_br->next;
  }
  struct asgn_instr* temp = asgn;
  while (temp != NULL)
  {
    if (temp->next != NULL && temp->next->bb == to_remove)
    {
      struct asgn_instr* next = temp->next;
      temp->next = next->next;
      free(next);
    }
    else
      temp = temp->next;
  }
}


int cond_to_uncond_check(struct br_instr* br, struct asgn_instr* asgn)
{
  if (br->cond != 0)
  {
    struct asgn_instr* cond_asgn = find_asgn_in_bb(asgn, br->cond, br->id);
    if (cond_asgn != NULL)
    {
      if (cond_asgn->op1 == 1)
      {
        br->cond = 0;
        remove_bb(cond_asgn, br->succ2, br);
      }
      else if (cond_asgn->op1 == 0)
      {
        br->cond = 0;
        remove_bb(cond_asgn, br->succ1, br);
        br->succ1 = br->succ2;
      }
    } 
  }
  return 1;
}

int count_preds(int br_id)
{
  int count = 0;
  struct br_instr* br = bb_root;
  while (br != NULL)
  {
    if (br->cond == 0)
      if (br->succ1 == br_id) count++;
    else 
    {
      if (br->succ1 == br_id) count++;
      if (br->succ2 == br_id) count++;
    }
    br = br->next;
  }
  return count;
}

int merge_with_pred(struct br_instr* br, struct asgn_instr* asgn)
{
  if (br->cond == 0)
  {
    if (count_preds(br->succ1) == 1)
    {
      while (asgn != NULL)
      {
        if (asgn->bb == br->succ1)
          asgn->bb = br->id;
        asgn = asgn->next;
      }
      struct br_instr* b = bb_root;
      while (b != NULL)
      {
        if (b->next != NULL && b->next->id == br->succ1)
        {
          struct br_instr* next = b->next;
          br->succ1 = next->succ1;
          br->succ2 = next->succ2;
          br->cond = next->cond;
          b->next = next->next;
          free(next);
          return 0;
        }
        else
          b = b->next;
      }
    }
  }
  return 1;
}

void get_first_instrs()
{
  struct asgn_instr* asgn = asgn_root;
  struct br_instr* br = bb_root;

  while (asgn != NULL) 
  {
    if (asgn->bb != br->id)
    {
      br = br->next;
      if (asgn->bb == br->id)
      {
        //print_asgn(asgn);
        push_asgn(asgn, &first_asgns_root, &first_asgns_tail);
        if (br->next == NULL)
        {
          first_asgns_tail->next = NULL;
        }
      }
    }

    if (asgn->bb == br->id) asgn = asgn->next;
  }
}


int main (int argc, char **argv) {
  const char *opt_simplify = "--simpcfg";
  bool simp_cfg = false;
  for (int i = 1; i < argc; i++)
  {
    if (strcmp(argv[i], opt_simplify) == 0) 
    {
      simp_cfg = true;
      break;
    }
  }
  int retval = yyparse();
  if (retval != 0) return 1;
  retval = visit_ast(get_funs_and_vars);
  if (retval == 0) print_ast();      // run `dot -Tpdf ast.dot -o ast.pdf` to create a PDF
  else 
  {
    printf("Semantic error\n");
    return 1;
  }
  visit_ast(compute_br_structure);
  visit_ast(fill_instrs);
  print_cfg(ifun_r, bb_root, asgn_root);
  print_interm(asgn_root);
  //get_first_instrs();
  if (simp_cfg) 
  {
    printf("After applying optimizations:\n\n");
    struct asgn_instr* asgn = asgn_root;
    struct br_instr* br = bb_root;
    visit_instr(br, asgn, cond_to_uncond_check);

    asgn = asgn_root;
    br = bb_root;
    visit_instr(br, asgn, merge_with_pred);
  }
  print_interm(asgn_root);
  
  clean_fun_str(&fun_r);
  clean_var_str(&var_r);
  free_ast();
  clean_istr(&ifun_r);
  clean_bbs(&bb_root);
  clean_asgns(&asgn_root);

  return retval;
}
