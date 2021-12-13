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

struct register_info* reg_info_r = NULL;
struct register_info* reg_info_t = NULL;

struct node_int_array* non_interf_vars_r = NULL;
struct node_int_array* non_interf_vars_t = NULL;

struct node_int* all_regs_r = NULL;
struct node_int* all_regs_t = NULL;

int current_bb = 0;
int current_bb_for_instrs = -1;
bool foundEntry = false;

int counter = 0;
int registers_num = 0;

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
    char *istr = node->ntoken == FUNID ? node->token : "main";
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
  if (asgn->num >= 0) printf("#%d: ", asgn->num);
  if (asgn->bin == 0) 
  {
    if (asgn->type == CONST)
      printf("r%d := %d\n", asgn->lhs, asgn->op1);
    else if (asgn->type == NOT)
      printf("r%d := not r%d\n", asgn->lhs, asgn->op1);
    else if (asgn->op1 < 0)
      printf("r%d := a%d\n", asgn->lhs, -asgn->op1);
    else if (asgn->lhs == 0)
      printf("rv := r%d\n", asgn->op1);
    else if (asgn->lhs < 0)
      printf("a%d := r%d\n", -asgn->lhs, asgn->op1);
    else 
      printf("r%d := r%d\n", asgn->lhs, asgn->op1);
  }
  else if (asgn->bin == 1) 
  {
    if (asgn->type == EQUAL)
      printf("r%d := r%d = r%d\n", asgn->lhs, asgn->op1, asgn->op2);
    else if (asgn->type == PLUS)
      printf("r%d := r%d + r%d\n", asgn->lhs, asgn->op1, asgn->op2);
    else if (asgn->type == MULT)
      printf("r%d := r%d * r%d\n", asgn->lhs, asgn->op1, asgn->op2);
    else if (asgn->type == MINUS)
      printf("r%d := r%d - r%d\n", asgn->lhs, asgn->op1, asgn->op2);
    else if (asgn->type == DIV)
      printf("r%d := r%d / r%d\n", asgn->lhs, asgn->op1, asgn->op2);
    else if (asgn->type == MOD)
      printf("r%d := r%d mod r%d\n", asgn->lhs, asgn->op1, asgn->op2);
    else if (asgn->type == LT)
      printf("r%d := r%d < r%d\n", asgn->lhs, asgn->op1, asgn->op2);
    else if (asgn->type == GT)
      printf("r%d := r%d > r%d\n", asgn->lhs, asgn->op1, asgn->op2);
    else if (asgn->type == LE)
      printf("r%d := r%d <= r%d\n", asgn->lhs, asgn->op1, asgn->op2);
    else if (asgn->type == GE)
      printf("r%d := r%d >= r%d\n", asgn->lhs, asgn->op1, asgn->op2);
    else if (asgn->type == LAND)
      printf("r%d := r%d && r%d\n", asgn->lhs, asgn->op1, asgn->op2);
    else if (asgn->type == LOR)
      printf("r%d := r%d || r%d\n", asgn->lhs, asgn->op1, asgn->op2);
  }
  else if (asgn->bin == 2)
  {
    printf("call %s\n", asgn->fun);
    if (strcmp(asgn->fun, "print") != 0)
      printf("r%d := rv\n", asgn->lhs);
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
  while (asgn != NULL && asgn->bb == bb)
  {
    if (asgn->lhs == lhs)
      ret_val = asgn;
    asgn = asgn->next;
  }
  return ret_val;
}


void remove_bb(struct asgn_instr* asgn, int to_remove, struct br_instr* br)
{
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
    //printf("checking cond: bb%d v%d bb%d bb%d", br->id, br->cond, br->succ1, br->succ2);
    struct asgn_instr* cond_asgn = find_asgn_in_bb(asgn, br->cond, br->id);
    if (cond_asgn != NULL && cond_asgn->type == CONST)
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


int same_branches(struct br_instr* br)
{
  struct asgn_instr* a1 = asgn_root, *a2 = asgn_root;

  while (a1->bb != br->succ1) a1 = a1->next;
  while (a2->bb != br->succ2) a2 = a2->next;

  while (a1 != NULL && a2 != NULL)
  {
    if (a1->lhs != a2->lhs || a1->bin != a2->bin) return 1;
    if (a1->bin == 0 && a1->op1 != a2->op1) return 1;
    if (a1->bin == 1 && (a1->op1 != a2->op1 || a1->op2 != a2->op2)) return 1;
    if (a1->bin < 2 && a1->type != a2->type) return 1;
    if (a1->bin == 2 && strcmp(a1->fun, a2->fun) != 0) return 1;
    a1 = a1->next; a2 = a2->next;
    if (a1->bb != br->succ1 && a2->bb != br->succ2) return 0;
    else if (a1->bb != br->succ1 || a2->bb != br->succ2) return 1;
  }
  return 0;
}

int detect_same_branches(struct br_instr* br, struct asgn_instr* asgn)
{
  if (br->cond != 0)
  {
    if (same_branches(br) == 0)
    {
      br->cond = 0;
      remove_bb(asgn, br->succ2, br);
    }

  }
  return 1;
}


void compute_line_number(struct br_instr* br, struct asgn_instr* asgn)
{
  asgn->num = counter++;
}

void compute_live_range(struct br_instr* br, struct asgn_instr* asgn)
{
  if ((asgn->bin == 0 && asgn->lhs > 0) || asgn->bin > 0)
  {
    if (!find_int(asgn->lhs, all_regs_r)) push_int(asgn->lhs, &all_regs_r, &all_regs_t);
    int last_end_val = -1;
    struct register_info* r = find_reg_info(reg_info_r, asgn->lhs);
    struct asgn_instr* a = asgn;
    struct br_instr* b = br;
    while (a != NULL)
    {
      if (a->bb != b->id)
      {
        if (b->cond == 0 && b->succ1 == -1)
          break;
        b = b->next;
      }
      else 
      {
        if (a->type != CONST && (a->op1 == asgn->lhs || a->op2 == asgn->lhs))
          last_end_val = a->num;
        a = a->next;
      }
    }
    
    int end_val = last_end_val == -1 ? asgn->num : last_end_val;
    if (r == NULL) push_reg_info(asgn->lhs, asgn->num, end_val, -1, &reg_info_r, &reg_info_t);
    else r->end = end_val;
    //printf("range of v%d is [%d - %d]\n", asgn->lhs, asgn->num, end_val);
  }
}



void registers_live_at(int line_num)
{
  struct node_int* live_regs_r = NULL;
  struct node_int* live_regs_t = NULL;

  struct register_info* reg = reg_info_r; 

  while (reg != NULL)
  {
    //printf("v%d has start end: %d, %d\n", reg->id, reg->st, reg->end);
    if (reg->st <= line_num && line_num < reg->end)
      push_int(reg->id, &live_regs_r, &live_regs_t);
    reg = reg->next;
  }

  /*
  printf("registers live at line %d are: ", line_num);
  struct node_int* n = live_regs_r;
  while (n != NULL)
  {
    printf("v%d ", n->id);
    n = n->next;
  }
  printf("\n");
  */

  struct node_int* live_regs = live_regs_r;
  while (live_regs != NULL)
  {
    struct node_int* live_regs_next = live_regs->next;
    while (live_regs_next != NULL)
    {
      struct node_int* p_r = NULL;
      struct node_int* p_t = NULL;
      push_int(live_regs->id, &p_r, &p_t);
      push_int(live_regs_next->id, &p_r, &p_t);

      push_int_array(p_r, &non_interf_vars_r, &non_interf_vars_t);
      live_regs_next = live_regs_next->next;
    }
    live_regs = live_regs->next;
  }
}


void interference_graph()
{
  struct asgn_instr* asgn = asgn_root;
  struct br_instr* br = bb_root;
 
  for (int i = 0; i < counter; i++)
  {
    registers_live_at(i);
  }
  /*
  struct node_int_array* r = non_interf_vars_r;
  while (r != NULL)
  {
    struct node_int* r1 = r->id;
    while (r1 != NULL)
    {
      printf("%d ", r1->id);
      r1 = r1->next;
    }
    printf("\n");
    r = r->next;
  }
  */
}


bool check_smt_file(char *str, bool get_values)
{
  FILE *fp;
  char command[100];
  char path[1024];
  sprintf(command, "z3 %s", str);
  fp = popen(command, "r");
  if (fp != NULL)
  {
    fgets(path, sizeof(path), fp);
    if (!get_values)
    {
      pclose(fp);
      char parsed[4];
      memcpy(parsed, path, 3);
      if (strcmp(parsed, "sat") == 0) return true;
      else return false;
    }
    struct node_int* all_regs = all_regs_r;
    while (fgets(path, sizeof(path), fp) != NULL) 
    {
      struct register_info* reg_info = find_reg_info(reg_info_r, all_regs->id);
      if (reg_info != NULL) reg_info->new_id = atoi(path);
      all_regs = all_regs->next;
    }
  }
  pclose(fp);
  return true;
}


void create_smt_constraints()
{
  int threshhold = 10;
  FILE *fp;
  int i = threshhold;
  while (i > 0)
  {
    struct node_int_array* arr = non_interf_vars_r;
    char str[20];
    sprintf(str, "constraints_%d.smt2", i);
    fp = fopen(str, "w");
    printf("\n");
    if (fp != NULL)
    {
      struct node_int* all_regs = all_regs_r;
      while (all_regs != NULL)
      {
        fprintf(fp, "(declare-var v%d Int)\n", all_regs->id);
        all_regs = all_regs->next;
      }
      fprintf(fp, "\n");
      while (arr != NULL)
      {
        struct node_int* n = arr->id;
        fprintf(fp, "(assert (not (=");
        while (n != NULL)
        {
          fprintf(fp, " v%d", n->id);
          n = n->next;
        }
        fprintf(fp, ")))\n");
        arr = arr->next;
      }

      all_regs = all_regs_r;
      fprintf(fp, "\n");
      while (all_regs != NULL)
      {
        fprintf(fp, "(assert (and (> v%d 0) (<= v%d %d)))\n", all_regs->id, all_regs->id, i);
        all_regs = all_regs->next;
      }
      fprintf(fp, "\n(check-sat)\n");
    }
    fclose(fp);

    if (!check_smt_file(str, false))
    {
      registers_num = i+1;
      if (registers_num > 7) 
      {
        printf("Compiler does not allow more than 7 registers\n");
        exit(1);
      }
      sprintf(str, "constraints_%d.smt2", registers_num);
      fp = fopen(str, "a");
      if (fp != NULL)
      {
        struct node_int* all_regs = all_regs_r;
        while (all_regs != NULL)
        {
          fprintf(fp, "(eval v%d)\n", all_regs->id);
          all_regs = all_regs->next;
        }
      }
      fclose(fp);
      check_smt_file(str, true);
      return;
    }
    i--;
  }
}


void replace_variables(struct br_instr* br, struct asgn_instr* asgn)
{ 
  if (asgn->bin == 0)
  {
    if (asgn->lhs > 0) 
    {
      struct register_info* reg = find_reg_info(reg_info_r, asgn->lhs);
      if (reg != NULL) asgn->lhs = reg->new_id;
    }
    if (asgn->type != CONST && asgn->op1 >= 0)
    {
      struct register_info* reg = find_reg_info(reg_info_r, asgn->op1);
      if (reg != NULL) asgn->op1 = reg->new_id;
    }
  }
  else if (asgn->bin == 1)
  {
    struct register_info* reg = find_reg_info(reg_info_r, asgn->lhs); 
    if (reg != NULL) asgn->lhs = reg->new_id;
    reg = find_reg_info(reg_info_r, asgn->op1); 
    if (reg != NULL) asgn->op1 = reg->new_id;
    reg = find_reg_info(reg_info_r, asgn->op2); 
    if (reg != NULL) asgn->op2 = reg->new_id;
  }
  else 
  {
    if (strcmp(asgn->fun, "main") != 0)
    {
      struct register_info* reg = find_reg_info(reg_info_r, asgn->lhs); 
      if (reg != NULL) asgn->lhs = reg->new_id;
    }
  }
}


void fix_variable_naming(struct br_instr* br, struct asgn_instr* asgn)
{
  struct asgn_instr* asgn_next = asgn->next;
  if (asgn_next == NULL) return;
  if (asgn_next->bin == 1)
  {
    if (asgn_next->lhs != asgn_next->op1)
    {
      if (asgn_next->lhs == asgn_next->op2)
      {
        int reg_to_use = -1;
        for (int i = 1; i <= registers_num; i++)
        {
          bool found = false;
          struct asgn_instr* a = asgn_next;
          while (true)
          {
            if (a->op1 == i || a->op2 == i) break;
            if (a->lhs == i) 
            {
              found = true;
              break;
            }
            a = a->next;
          }
          if (found) 
          {
            reg_to_use = i;
            break;
          }
        }
        if (reg_to_use > 0) 
        {
          struct asgn_instr* a = mk_uasgn(asgn_next->bb, reg_to_use, asgn_next->op1, -1);
          a->next = asgn_next;
          asgn->next = a;
          asgn = asgn->next;

          a = mk_uasgn(asgn_next->bb, asgn_next->op1, asgn_next->op2, -1);
          a->next = asgn_next;
          asgn->next = a;
          asgn = asgn->next;

          a = mk_uasgn(asgn_next->bb, asgn_next->op2, reg_to_use, -1);
          a->next = asgn_next;
          asgn->next = a;
          asgn = asgn->next;
          
          int tmp = asgn_next->op1;
          asgn_next->op1 = asgn_next->op2;
          asgn_next->op2 = tmp;
        }
        else printf("DIDN'T WORK\n");
      }
      else 
      {
        struct asgn_instr* a = mk_uasgn(asgn_next->bb, asgn_next->lhs, asgn_next->op1, -1);
        a->next = asgn_next;
        asgn->next = a;
        asgn_next->op1 = asgn_next->lhs;
      }
    }
  }
}


void print_x86_instructions(struct asgn_instr* asgn)
{
  printf("\t");
  if (asgn->bin == 0) 
  {
    if (asgn->type == CONST)
      printf("movq $%d, %d(%%rbp)\n", asgn->op1, -8*asgn->lhs);
    else if (asgn->type == NOT)
    {
      if (asgn->op1 != asgn->lhs)
        printf("movq %d(%%rbp), %d(%%rbp)\n", -8*asgn->op1, -8*asgn->lhs);
      printf("notq %d(%%rbp)\n", asgn->lhs);
    }
    else if (asgn->op1 < 0)
      printf("movq %s, %d(%%rbp)\n", x86inputs[-asgn->op1], -8*asgn->lhs);
    else if (asgn->lhs == 0)
    {
      printf("movq %d(%%rbp), %%rax\n", -8*asgn->op1);
      printf("leave\n");
      printf("ret\n");
    }
    else if (asgn->lhs < 0)
      printf("movq %d(%%rbp), %s\n", -8*asgn->op1, x86inputs[-asgn->lhs]);
    else 
      printf("movq %d(%%rbp), %d(%%rbp)\n", -8*asgn->op1, -8*asgn->lhs);
  }
  else if (asgn->bin == 1) 
  {
    if (asgn->type == EQUAL || asgn->type == LT || asgn->type == GT
      || asgn->type == LE || asgn->type == GE)
    {
      printf("cmpq %d(%%rbp), %d(%%rbp)\n", -8*asgn->op1, -8*asgn->op2);
      if (asgn->type == EQUAL)
        printf("\tsete %%al\n");
      else if (asgn->type == LT)
        printf("\tsetl %%al\n");
      else if (asgn->type == LE)
        printf("\tsetle %%al\n");
      else if (asgn->type == GT)
        printf("\tsetg %%al\n");
      else if (asgn->type == GE)
        printf("\tsetge %%al\n");
      printf("\tmovzbl %%al, %d(%%rbp)\n", -8*asgn->lhs);
    }
    else if (asgn->type == PLUS)
      printf("addq %d(%%rbp), %d(%%rbp)\n", -8*asgn->op2, -8*asgn->op1);
    else if (asgn->type == MULT)
      printf("mulq %d(%%rbp), %d(%%rbp)\n", -8*asgn->op2, -8*asgn->op1);
    else if (asgn->type == MINUS)
      printf("subq %d(%%rbp), %d(%%rbp)\n", -8*asgn->op2, -8*asgn->op1);
    else if (asgn->type == DIV || asgn->type == MOD)
    {
      printf("movq %d(%%rbp), %%rax\n", -8*asgn->op1);
      printf("movq $0, %%rdx\n");
      printf("divq %d(%%rbp)\n", -8*asgn->op2);
      if (asgn->type == DIV)
        printf("movq %%rax, %d(%%rbp)\n", -8*asgn->lhs);
      else
        printf("movq %%rdx, %d(%%rbp)\n", -8*asgn->lhs);
    }
    else if (asgn->type == LAND)
      printf("andq %d(%%rbp), %d(%%rbp)\n", -8*asgn->op2, -8*asgn->op1);
    else if (asgn->type == LOR)
      printf("orq %d(%%rbp), %d(%%rbp)\n", -8*asgn->op2, -8*asgn->op1);
  }
  else if (asgn->bin == 2)
  {
    printf("call %s\n", asgn->fun);
    if (strcmp(asgn->fun, "print") != 0)
      printf("\tmovq %%rax, %d(%%rbp)\n", 8*asgn->lhs);
  } 
}


void print_to_x86()
{
  struct asgn_instr* asgn = asgn_root;
  struct br_instr* br = bb_root;
  
  char* fun_name = find_istr(ifun_r, br->id);

  printf("\t.section .rodata\n");
  printf("msg0:\n");
  printf("\t.string \"%%d\"\n");
  printf("msg1:\n");
  printf("\t.string \"%%d\\n\"\n");
  printf("\t.text\n");
  printf("\t.global main\n");
  printf("\t.type main, @function\n");

  printf("print:\n");
  printf(".bbp\n");
  printf("\tpushq %%rbp\n");
  printf("\tmovq $msg1, %%rdi\n");
  printf("\tcall printf\n");
  printf("\tpopq %%rbp\n");
  printf("\tret\n\n");

  printf("get_int:\n");
  printf(".bbg:\n");
  printf("\tpusbq %%rbp\n");
  printf("\tmovq %%rsp, %%rbp\n");
  printf("\tsubq $16, %%rsp\n");
  printf("\tleaq -8(%%rbp), %%rax\n");
  printf("\tmovq %%rax, %%rsi\n");
  printf("\tmovq $msg0, %%rdi\n");
  printf("\tmovq $0, %%rax\n");
  printf("\tcall scanf\n");
  printf("\tmovq -8(%%rbp), %%rax\n");
  printf("\tleave\n");
  printf("\tret\n\n");

  printf("%s:\n", fun_name);
  printf(".bb%d:\n", br->id);
  printf("\tpushq %%rbp\n");
  printf("\tmovq %%rsp, %%rbp\n");
  printf("\tsubq $56, %%rsp\n");

  while (asgn != NULL){
    if (asgn->bb != br->id){
      if (br->cond != 0)
      {
        printf("\tcmp $0, %%r%d\n", br->cond);
        printf("\tje .bb%d\n", br->succ2);
        printf("\tjmp .bb%d\n", br->succ1);
      }
      br = br->next;
      if (br == NULL) return;
      fun_name = find_istr(ifun_r, br->id);
      if (fun_name != NULL){
        printf("%s:\n", fun_name);
        printf(".bb%d:\n", br->id);
        printf("\tpushq %%rbp\n");
        printf("\tmovq %%rsp, %%rbp\n");
        printf("\tsubq $56, %%rsp\n");
      } else {
        printf(".bb%d:\n", br->id);
      }
    }
    if (asgn->bb == br->id) {
      print_x86_instructions(asgn);
      asgn = asgn->next;
    }
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
  /*
  struct asgn_instr* asgn = asgn_root;
  struct br_instr* br = bb_root;
  if (simp_cfg) 
  {
    printf("After applying optimizations:\n\n");
    visit_instr(br, asgn, cond_to_uncond_check);

    asgn = asgn_root;
    br = bb_root;
    visit_instr(br, asgn, detect_same_branches);

    asgn = asgn_root;
    br = bb_root;
    visit_instr(br, asgn, merge_with_pred);
  }
  */
  visit_instr2(bb_root, asgn_root, compute_line_number);
  visit_instr2(bb_root, asgn_root, compute_live_range);
  pop_int(&all_regs_r, &all_regs_t);
  interference_graph();
  create_smt_constraints();
  visit_instr2(bb_root, asgn_root, replace_variables);
  visit_instr2(bb_root, asgn_root, fix_variable_naming);
  print_to_x86();
  print_cfg(ifun_r, bb_root, asgn_root);
  //print_interm(asgn_root);
  
  clean_fun_str(&fun_r);
  clean_var_str(&var_r);
  free_ast();
  clean_istr(&ifun_r);
  clean_bbs(&bb_root);
  clean_asgns(&asgn_root);

  return retval;
}
