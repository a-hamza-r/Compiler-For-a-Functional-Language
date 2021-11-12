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

struct asgn_instr_r_t { struct asgn_instr* asgn_r; struct asgn_instr* asgn_t; int cond; int neg; struct asgn_instr_r_t* next; };
void push_asgn_instr_r_t (struct asgn_instr* asgn_r, struct asgn_instr* asgn_t, struct asgn_instr_r_t** r, struct asgn_instr_r_t** t);
struct asgn_instr_r_t* get(struct asgn_instr_r_t* r, int i);

struct node_istr { int id1; char* id2; struct node_istr* next; };

void push_istr (int c1, char* c2, struct node_istr** r, struct node_istr** t);
char* find_istr(struct node_istr* r, int key);
void clean_istr(struct node_istr** r);

struct br_instr { int id; int cond; int succ1; int succ2; struct br_instr* next;};
struct br_instr* mk_cbr(int id, int cond, int succ1, int succ2);
struct br_instr* mk_ubr(int id, int succ1);
struct br_instr* find_br_instr(int id, struct br_instr* r);
void push_br (struct br_instr* i, struct br_instr** r, struct br_instr** t);
void clean_bbs (struct br_instr** r);

struct asgn_instr {int bb; int lhs; int bin; int op1; int op2; int type; int lhs_type; char* fun; struct asgn_instr* next; };
struct asgn_instr* mk_asgn(int bb, int lhs, int bin, int op1, int op2, int lhs_type, int type);
struct asgn_instr* mk_basgn(int bb, int lhs, int op1, int op2, int lhs_type, int type);
struct asgn_instr* mk_uasgn(int bb, int lhs, int op, int lhs_type, int type);
struct asgn_instr* mk_casgn(int bb, int lhs, char* fun);
void push_asgn (struct asgn_instr* i, struct asgn_instr** r, struct asgn_instr** t);
void rm_asgn (struct asgn_instr* i, struct asgn_instr** r, struct asgn_instr** t);
void clean_asgns (struct asgn_instr** r);

int visit_instr(struct br_instr* br_root, struct asgn_instr* asgn,
                int (*f)(struct asgn_instr* asgn, struct br_instr* br, int arg1, int arg2),
                int arg1, int arg2); // arg1 or arg2 could be empty

void print_cfg(struct node_istr* ifun_r, struct br_instr* bb_root, struct asgn_instr* asgn_root);