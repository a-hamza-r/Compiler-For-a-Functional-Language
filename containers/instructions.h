/**
 * Author:    Grigory Fedyukovich, Subhadeep Bhattacharya
 * Created:   09/30/2020
 *
 * (c) FSU
 **/


#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>

struct node_int_array { struct node_int* id; struct node_int_array* next; };
void push_int_array (struct node_int* id, struct node_int_array** r, struct node_int_array** t);

struct node_istr { int id1; char* id2; struct node_istr* next; };

void push_istr (int c1, char* c2, struct node_istr** r, struct node_istr** t);
char* find_istr(struct node_istr* r, int key);
void clean_istr(struct node_istr** r);

struct register_info { int id; int st; int end; int new_id; struct register_info* next; };
void push_reg_info (int id, int st, int end, int new_id, struct register_info** r, struct register_info** t);
struct register_info* find_reg_info(struct register_info* r, int key);

struct br_instr { int id; int cond; int succ1; int succ2; struct br_instr* next;};
struct br_instr* mk_cbr(int id, int cond, int succ1, int succ2);
struct br_instr* mk_ubr(int id, int succ1);
struct br_instr* find_br_instr(int id, struct br_instr* r);
void push_br (struct br_instr* i, struct br_instr** r, struct br_instr** t);
void clean_bbs (struct br_instr** r);

struct asgn_instr {int bb; int lhs; int bin; int op1; int op2; int type; char* fun; int num; struct asgn_instr* next; };
struct asgn_instr* mk_asgn(int bb, int lhs, int bin, int op1, int op2, int type);
struct asgn_instr* mk_basgn(int bb, int lhs, int op1, int op2, int type);
struct asgn_instr* mk_uasgn(int bb, int lhs, int op, int type);
struct asgn_instr* mk_casgn(int bb, int lhs, char* fun);
void push_asgn (struct asgn_instr* i, struct asgn_instr** r, struct asgn_instr** t);
void rm_asgn (struct asgn_instr* i, struct asgn_instr** r, struct asgn_instr** t);
void clean_asgns (struct asgn_instr** r);

int visit_instr(struct br_instr* br, struct asgn_instr* asgn, int (*f)(struct br_instr* br, struct asgn_instr* asgn));
void visit_instr2(struct br_instr* br, struct asgn_instr* asgn, void (*f)(struct br_instr* br, struct asgn_instr* asgn));

void print_cfg(struct node_istr* ifun_r, struct br_instr* bb_root, struct asgn_instr* asgn_root);