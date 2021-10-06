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

// basic data structure
struct node_int { int id; struct node_int* next; };
void push_int (int i, struct node_int** r, struct node_int** t);
int find_int(int c, struct node_int* r);
void clean_int (struct node_int** r);
int pop_int (struct node_int** r, struct node_int** t);
int print_int(struct node_int* r);

// data structure used for vars in symbol tables
struct node_var_str { int begin_id; int end_id; int type; char* name; struct node_var_str* next; };
void push_var_str (int begin_id, int end_id, int type, char* name, struct node_var_str** r, struct node_var_str** t);
struct node_var_str* find_var_str(int loc_id, char* name, struct node_var_str* r);
void clean_var_str(struct node_var_str** r);

// data structure used for funs in symbol tables
struct node_fun_str { char* name; int type; int arity; struct node_var_str* args; struct node_fun_str* next; };
void push_fun_str (char* name, int type, int arity, struct node_var_str* args, struct node_fun_str** r, struct node_fun_str** t);
struct node_fun_str* find_fun_str(char* name, struct node_fun_str* r);
void clean_fun_str(struct node_fun_str** r);

// data structures for AST
struct ast;
struct ast_child {struct ast* id; struct ast_child* next; }; //Dynamic data structure for the list of children
struct ast {int id; char *token; bool is_leaf; int ntoken; char *fun; struct ast_child* child; struct ast* parent; struct ast* next;}; //Dynamic data structure for the AST. Each node points to a terminal/nonterminal and its children

int pop_ast(struct node_int** r, struct node_int** t);
void insert_child(int val);       // Helper methods to specify child/children for the upcoming node
void insert_children(int, ...);   // Variadic version (expects the # of children to be given in the first place)
                                  // Note that it is not necessary to insert all children at once. You can call either insert_child or insert_children many times -- they all will be aggregated and used during the next call of insert_node/insert_node_tmpvoid insert_node_tmp(int val, char * token, bool is_leaf, int ntoken);
int insert_node(char* token, int ntoken);
struct ast* get_child(struct ast* ast_node, int id);
int get_child_num(struct ast* ast_node);

int visit_ast(int (*f)(struct ast* ast_node));
void print_ast(); //    run "dot -Tpdf ast.dot -o ast.pdf" to create a PDF. Requires a preinstalled graphviz package (https://graphviz.org/download/)
void free_ast();

// static data structres used in the lab:

static int current_node_id = 0;
static struct ast_child* ast_child_root = NULL;
static struct ast* ast_list_root = NULL;
