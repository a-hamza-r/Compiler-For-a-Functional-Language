/**
 * Author:    Grigory Fedyukovich, Subhadeep Bhattacharya
 * Created:   09/30/2020
 *
 * (c) FSU
 **/

#include "instructions.h"

void push_int_array (struct node_int* i, struct node_int_array** r, struct node_int_array** t){
  if (*r == NULL){   // If root is null - empty list
    *r = (struct node_int_array*)malloc(sizeof(struct node_int_array)); //Create a new node a assign the root pointer tp tail
    (*r)->id = i;
    (*r)->next = NULL;
    *t = *r;
  }
  else{
    struct node_int_array* ptr; //Non-empty list
    ptr = (struct node_int_array*)malloc(sizeof(struct node_int_array)); // Create a new node, put it after tail as a new node
    ptr->id = i;
    ptr->next = NULL;
    (*t)->next = ptr;
    (*t) = ptr;
  }
}

void push_istr (int c1, char* c2, struct node_istr** r, struct node_istr** t){
  if (*r == NULL) {                          //If root node is null
    *r = (struct node_istr*)malloc(sizeof(struct node_istr)); //Create a new node
    (*r)->id1 = c1;                           //set the id for the new node
    (*r)->id2 = c2;                           //set the id for the new node
    (*r)->next = NULL;                                      //set next pointer to null
    *t = *r;                                                //Set tail pointer == root pointer
  }
  else {                                    //If root node is not null
    struct node_istr* ptr;
    ptr = (struct node_istr*)malloc(sizeof(struct node_istr));  //Create a temporary node
    ptr->id1 = c1;                                            //Set id for the new node
    ptr->id2 = c2;                                            //Set id for the new node
    ptr->next = NULL;                                         //Set next pointer to NULL
    (*t)->next = ptr;                                         //Set the node after tail
    *t = ptr;                                                 //Set tail as the new pointer
  }
}

char* find_istr(struct node_istr* r, int key){
  while (r != NULL){
    if (r->id1 == key) return r->id2;
    r = r->next;
  }
  return NULL;
}

void push_reg_info (int id, int st, int end, int new_id, struct register_info** r, struct register_info** t){
  if (*r == NULL) {                          //If root node is null
    *r = (struct register_info*)malloc(sizeof(struct register_info)); //Create a new node
    (*r)->id = id;                          
    (*r)->st = st;                           
    (*r)->end = end;                           
    (*r)->new_id = new_id;                           
    (*r)->next = NULL;                                      //set next pointer to null
    *t = *r;                                                //Set tail pointer == root pointer
  }
  else {                                    //If root node is not null
    struct register_info* ptr;
    ptr = (struct register_info*)malloc(sizeof(struct register_info));  //Create a temporary node
    ptr->id = id;                          
    ptr->st = st;                           
    ptr->end = end;                           
    ptr->new_id = new_id; 
    (*t)->next = ptr;                                         //Set the node after tail
    *t = ptr;                                                 //Set tail as the new pointer
  }
}

struct register_info* find_reg_info(struct register_info* r, int key){
  while (r != NULL){
    if (r->id == key) return r;
    r = r->next;
  }
  return NULL;
}

struct br_instr* mk_cbr(int id, int cond, int succ1, int succ2){
  struct br_instr* tmp = (struct br_instr*)malloc(sizeof(struct br_instr));
  tmp->id = id;
  tmp->cond = cond;
  tmp->succ1 = succ1;
  tmp->succ2 = succ2;
  tmp->next = NULL;
  return tmp;
}

struct br_instr* mk_ubr(int id, int succ1){
  struct br_instr* tmp = (struct br_instr*)malloc(sizeof(struct br_instr));
  tmp->id = id;
  tmp->cond = 0;
  tmp->succ1 = succ1; // succ1 == -1 if goes to `exit`
  tmp->next = NULL;
  return tmp;
}

void push_br (struct br_instr* i, struct br_instr** r, struct br_instr** t){
  if (*r == NULL){
    *r = i;
    *t = *r;
  }
  else {
    (*t)->next = i;
    (*t) = i;
  }
}

struct br_instr* find_br_instr(int id, struct br_instr* r)
{
  struct br_instr *r1 = r;
  while (r1 != NULL)
  {
    if (id == r1->id) return r1;
    r1 = r1->next;
  }
  return NULL;
}

struct asgn_instr* mk_asgn(int bb, int lhs, int bin, int op1, int op2, int type){
  struct asgn_instr* tmp = (struct asgn_instr*)malloc(sizeof(struct asgn_instr));
  tmp->bb = bb;
  tmp->bin = bin;
  tmp->lhs = lhs;
  tmp->op1 = op1;
  tmp->op2 = op2;
  tmp->type = type;
  tmp->num = -1;
  tmp->next = NULL;
  return tmp;
}

struct asgn_instr* mk_basgn(int bb, int lhs, int op1, int op2, int type){
  struct asgn_instr* tmp = (struct asgn_instr*)malloc(sizeof(struct asgn_instr));
  tmp->bb = bb;
  tmp->lhs = lhs;
  tmp->bin = 1;
  tmp->op1 = op1;
  tmp->op2 = op2;
  tmp->type = type;
  tmp->num = -1;
  tmp->next = NULL;
  return tmp;
}

struct asgn_instr* mk_uasgn(int bb, int lhs, int op, int type){
  struct asgn_instr* tmp = (struct asgn_instr*)malloc(sizeof(struct asgn_instr));
  tmp->bb = bb;
  tmp->lhs = lhs;
  tmp->bin = 0;
  tmp->op1 = op;
  tmp->op2 = -1;
  tmp->type = type;
  tmp->num = -1;
  tmp->next = NULL;
  return tmp;
}

struct asgn_instr* mk_casgn(int bb, int lhs, char* fun){
  struct asgn_instr* tmp = (struct asgn_instr*)malloc(sizeof(struct asgn_instr));
  tmp->bb = bb;
  tmp->lhs = lhs;
  tmp->bin = 2;
  tmp->fun = fun;
  tmp->next = NULL;
  return tmp;
}

void rm_asgn (struct asgn_instr* i, struct asgn_instr** r, struct asgn_instr** t){
  struct asgn_instr* pred = (*r);
  while (pred != NULL && pred->next != i) pred = pred->next;
  if (pred == NULL) return;
  if (i == *t) (*t) = pred;
  else (*pred).next = i->next;
  free(i);
}

void push_asgn (struct asgn_instr* i, struct asgn_instr** r, struct asgn_instr** t){
  if (*r == NULL){
    *r = i;
    *t = *r;
  }
  else {
    (*t)->next = i;
    (*t) = i;
  }
}

void clean_asgns (struct asgn_instr** r){
  while (*r != NULL){
    struct asgn_instr* tmp = *r;
    (*r) = (*r)->next;
    free(tmp);
  }
}

void clean_bbs (struct br_instr** r){
  while (*r != NULL){
    struct br_instr* tmp = *r;
    (*r) = (*r)->next;
    free(tmp);
  }
}

void clean_istr(struct node_istr** r){
  while (*r != NULL){
    struct node_istr* tmp = *r;
    (*r) = (*r)->next;
    free(tmp);
  }
}

int visit_instr(struct br_instr* br, struct asgn_instr* asgn, int (*f)(struct br_instr* br, struct asgn_instr* asgn)){
  while (f (br, asgn) == 0);

  // goes to the end of the function
  while (asgn != NULL){
    if (asgn->bb != br->id){
      br = br->next;
      if (br == NULL) return 0;
      while (f (br, asgn) == 0);
    }
    if (asgn->bb == br->id) asgn = asgn->next;
  }
  return 0;
}

void visit_instr2(struct br_instr* br, struct asgn_instr* asgn, void (*f)(struct br_instr* br, struct asgn_instr* asgn))
{
  while (asgn != NULL){
    if (asgn->bb != br->id){
      br = br->next;
      if (br == NULL) return;
    }
    if (asgn->bb == br->id) 
    {
      f(br, asgn);
      asgn = asgn->next;
    }
  }
}

void print_cfg(struct node_istr* ifun_r, struct br_instr* bb_root, struct asgn_instr* asgn_root) {
  FILE *fp;
  fp = fopen("cfg.dot", "w");
  fprintf(fp, "digraph print {\n ");
  int num = 0;
  char cur_name[128];
  snprintf(cur_name, sizeof(cur_name), "\"entry [function %s]\"", find_istr(ifun_r, bb_root->id));

  struct asgn_instr* asgn = asgn_root;
  struct br_instr* br = bb_root;

  while (asgn != NULL){
    if (asgn->bb != br->id){
      if (br->cond == 0){
        if (br->succ1 == -1) {
          fprintf(fp, "%d [label=\"exit\"]\n", num);
          fprintf(fp, "%s -> %d\n", cur_name, num);
          num++;
        }
        else
          fprintf(fp, "%s -> bb%d\n", cur_name, br->succ1);
      }
      else {
        fprintf(fp, "%s -> bb%d\n", cur_name, br->succ1);
        fprintf(fp, "%s -> bb%d\n", cur_name, br->succ2);
      }
      br = br->next;
      if (br == NULL) return;
      char* fun_name = find_istr(ifun_r, br->id);
      if (fun_name != NULL){
        snprintf(cur_name, sizeof(cur_name), "\"entry [function %s]\"", fun_name);
      } else {
        snprintf(cur_name, sizeof(cur_name), "bb%d", br->id);
      }
    }
    if (asgn->bb == br->id) {
      asgn = asgn->next;
    }
  }
  fprintf(fp, "%d [label=\"exit\"]\n", num);
  fprintf(fp, "%s -> %d\n", cur_name, num);
  fprintf(fp, "}\n ");
  fclose(fp);
  system("dot -Tpdf cfg.dot -o cfg.pdf");
}
