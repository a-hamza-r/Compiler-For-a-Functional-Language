/**
 * Author:    Grigory Fedyukovich, Subhadeep Bhattacharya
 * Created:   09/30/2020
 *
 * (c) FSU
 **/

#include "containers.h"

void push_int (int i, struct node_int** r, struct node_int** t){
  if (*r == NULL){   // If root is null - empty list
    *r = (struct node_int*)malloc(sizeof(struct node_int)); //Create a new node a assign the root pointer tp tail
    (*r)->id = i;
    (*r)->next = NULL;
    *t = *r;
  }
  else{
    struct node_int* ptr; //Non-empty list
    ptr = (struct node_int*)malloc(sizeof(struct node_int)); // Create a new node, put it after tail as a new node
    ptr->id = i;
    ptr->next = NULL;
    (*t)->next = ptr;
    (*t) = ptr;
  }
}

int pop_int(struct node_int** r, struct node_int** t) {
  if ((*r)->next == NULL) {             //Check if next node is NULL - check for only one node
    int retval = (*r)->id;              //Store the id of the current node in retval
    free(*r);
    *r = NULL;                          //Store current node to NULL
    *t = NULL;
    return retval;                      //return retval
  }

  struct node_int** r1 = r;             //Else store the pointer in r1 - if multiple node, just traverse and then pop
  while ((*r1)->next != NULL) {
    *t = *r1;
    r1 = &((*r1)->next); //traverse the tree
  }

  int retval = (*r1)->id;               //store the last element in retval
  *r1 = NULL;                           //Store current node to NULL
  return retval;                        //return retval
}

int pop_int_front(struct node_int** r, struct node_int** t) {
  if ((*r)->next == NULL) {             //Check if next node is NULL - check for only one node
    int retval = (*r)->id;              //Store the id of the current node in retval
    free(*r);
    *r = NULL;                          //Store current node to NULL
    *t = NULL;
    return retval;                      //return retval
  }

  int retval = (*r)->id;
  (*r) = (*r)->next;

  return retval;                        //return retval
}

int pop_ast(struct node_int** r, struct node_int** t) {
  if ((*r)->next == NULL) {             //Check if next node is NULL - check for only one node
    int retval = (*r)->id;              //Store the id of the current node in retval
    *r = NULL;
    *t = *r;                            //Store current node to NULL
    free(*r);
    return retval;                      //return retval
  }

  struct node_int** r1 = r;             //Else store the pointer in r1 - if multiple node, just traverse and then pop
  struct node_int** temp = NULL;
  while ((*r1)->next != NULL) {
      temp = r1;
      r1 = &((*r1)->next);              //traverse the tree
  }

  int retval = (*r1)->id;               //store the last element in retval
  *r1 = NULL;                           //Store current node to NULL
  *t = *temp;
  return retval;                        //return retval
}

int print_int(struct node_int* r){
  while (r != NULL){
    printf(" %d,", r->id);
    r = r->next;
  }
  printf("\n");
  return 1;
}

void push_var_str (int begin_id, int end_id, int type, char* name, int reg_id, struct node_var_str** r, struct node_var_str** t){
  if (*r == NULL){
    *r = (struct node_var_str*)malloc(sizeof(struct node_var_str)); //Create a new node
    (*r)->begin_id = begin_id;
    (*r)->end_id = end_id;
    (*r)->type = type;
    (*r)->name = name;
    (*r)->reg_id = reg_id;
    (*r)->next = NULL;
    *t = *r;
  }
  else {
    struct node_var_str* ptr;
    ptr = (struct node_var_str*)malloc(sizeof(struct node_var_str));  //Create a temporary node
    ptr->begin_id = begin_id;
    ptr->end_id = end_id;
    ptr->type = type;
    ptr->name = name;
    ptr->reg_id = reg_id;
    ptr->next = NULL;
    (*t)->next = ptr;                                         //Set the node after tail
    *t = ptr;
  }
}

void push_fun_str (char* name, int type, int arity, struct node_int* argTypes, struct node_fun_str** r, struct node_fun_str** t) {
  if (*r == NULL){
    *r = (struct node_fun_str*)malloc(sizeof(struct node_fun_str)); //Create a new node
    (*r)->name = name;
    (*r)->type = type;
    (*r)->argTypes = argTypes;
    (*r)->start = NULL;
    (*r)->end = NULL;
    (*r)->instrs = NULL;
    (*r)->next = NULL;
    (*r)->arity = arity;
    *t = *r;
  }
  else {
    struct node_fun_str* ptr;
    ptr = (struct node_fun_str*)malloc(sizeof(struct node_fun_str));  //Create a temporary node
    ptr->type = type;
    ptr->name = name;
    ptr->argTypes = argTypes;
    ptr->start = NULL;
    ptr->end = NULL;
    ptr->arity = arity;
    ptr->instrs = NULL;
    ptr->next = NULL;
    (*t)->next = ptr;                                         //Set the node after tail
    *t = ptr;
  }
}

struct node_var_str* find_var_str(int loc_id, char* name, struct node_var_str* r){
  while (r != NULL){
    if (strcmp(name, r->name) == 0 &&
        r->begin_id <= loc_id && loc_id <= r->end_id) return r;
    else r = r->next;
  }
  return NULL;
}

struct node_fun_str* find_fun_str(char* name, struct node_fun_str* r){
  while (r != NULL){
    if (strcmp(name, r->name) == 0) return r;
    else r = r->next;
  }
  return NULL;
}

void insert_parent(struct ast* p) {
  struct ast_child* temp_ast_child_root = p->child;
  while(temp_ast_child_root != NULL){
    temp_ast_child_root->id->parent = p;
    temp_ast_child_root = temp_ast_child_root->next;
  }
}

void fill_ast_node(struct ast** t, int val, char* token, bool is_leaf, int ntoken){
  (*t) = (struct ast*)malloc(sizeof(struct ast));
  (*t)->id = val;                                     //set value into id field
  (*t)->next = NULL;                                  //set next pointer to NULL
  (*t)->token = token;
  (*t)->is_leaf = is_leaf;
  (*t)->ntoken = ntoken;
  if (ast_child_root != NULL){                          //if child doesnot exist,
    (*t)->child = ast_child_root;                     //set current child root pointer to child field
    ast_child_root = NULL;                              //Set the child root to NULL as we intend to set a new list
    insert_parent(*t);
  } else {
    (*t)->child = NULL;                               //
  }
}

struct ast* find_ast_node(int id){
  struct ast* temp_root = ast_list_root;
  while(temp_root != NULL){
    if (id == temp_root->id) return temp_root;
    temp_root = temp_root->next;
  }
  return NULL;
}

void insert_node_tmp(int val, char* token, bool is_leaf, int ntoken){
  if (ast_list_root == NULL){                               //Create a new node and set the value if root is empty
    fill_ast_node(&ast_list_root, val, token, is_leaf, ntoken);
  } else {                                                  //Else create a ast graph node and assign it to next
    struct ast* ptr;                                        //Create a temp pointer
    fill_ast_node(&ptr, val, token, is_leaf, ntoken);

    //Traverse root and set the new pointer at the next of last node
    struct ast** tmp = &ast_list_root;
    while ((*tmp)->next != NULL) {
      tmp = &((*tmp)->next);    // last_node will store the pointer to last node
    }
    (*tmp)->next = ptr;
  }
}

int insert_node(char* token, int ntoken) {
  current_node_id++;
  insert_node_tmp(current_node_id, token, ast_child_root == NULL, ntoken);
  return current_node_id;
}

void insert_child(int val){        //This function helps to fill the child list
  struct ast* node = find_ast_node(val);
  if (ast_child_root == NULL) {     //If null list
    (ast_child_root) = (struct ast_child*)malloc(sizeof(struct ast_child));  //allocate memory
    // Set id and next field of the ast_child_root
    (ast_child_root)->id = node;
    (ast_child_root)->next = NULL;
  } else {
    //Create the node
    struct ast_child* ptr;
    ptr = (struct ast_child*)malloc(sizeof(struct ast_child));
    ptr->id = node;
    ptr->next = NULL;

    //Traverse the list and set the node at top
    struct ast_child** temp = &ast_child_root;     // Set child root in a temporary variable
    struct ast_child** last_child_node = NULL;
    while ((*temp)->next != NULL) {           // Until next is NULL traverse the list
      temp = &((*temp)->next);              // temp will hold the address of next node
    }
    (*temp)->next = ptr;
  }
}

void insert_children (int n, ...){
  va_list vl;
  va_start(vl,n);
  for (int i = 0; i < n; i++)
    insert_child(va_arg(vl, int));
  va_end(vl);
}

struct ast* get_child(struct ast* ast_node, int id){
  struct ast_child* temp_child_root = ast_node->child;
  int child_num = 0;
  while(temp_child_root != NULL) {
    child_num++;
    if (child_num == id) {
      return temp_child_root->id;
    }
    temp_child_root = temp_child_root->next;
  }
  return NULL;
}

int get_child_num(struct ast* ast_node){
  struct ast_child* temp_child_root = ast_node->child;
  int child_num = 0;
  while(temp_child_root != NULL) {
    child_num++;
    temp_child_root = temp_child_root->next;
  }
  return child_num;
}

int visit_ast(int (*f)(struct ast* ast_node)){
  struct ast* temp_root = ast_list_root;
  while(temp_root != NULL){
    if (f (temp_root) != 0) return 1;
    temp_root = temp_root->next;
  }
  return 0;
}

void clean_var_str(struct node_var_str** r){
  while (*r != NULL){
    struct node_var_str* tmp = *r;
    (*r) = (*r)->next;
    free(tmp);
  }
}

void clean_fun_str(struct node_fun_str** r){
  while (*r != NULL){
    struct node_fun_str* tmp = *r;
    (*r) = (*r)->next;
    free(tmp);
  }
}

FILE *fp;
int print(struct ast* temp_root) {
  if (! temp_root->is_leaf){
    fprintf(fp, "%d [label=\"%s:%d\", fontname=\"monospace\", style=filled, fillcolor=mintcream];\n ", temp_root->id, temp_root->token, temp_root->id);
  } else {
    fprintf(fp, "%d [label=\"%s:%d\", fontname=\"monospace\"];\n ", temp_root->id, temp_root->token, temp_root->id);
  }
  if (temp_root->child != NULL){
    struct ast_child* temp_ast_child_root = temp_root->child;
    while(temp_ast_child_root != NULL){
      fprintf(fp, "%d->%d\n ", temp_root->id, temp_ast_child_root->id->id);
      temp_ast_child_root = temp_ast_child_root->next;
    }
  }
  return 0;
}

void print_ast(){
  fp = fopen("ast.dot", "w");
  fprintf(fp, "digraph print {\n ");
  visit_ast(print);
  fprintf(fp, "}\n ");
  fclose(fp);
  system("dot -Tpdf ast.dot -o ast.pdf");
}

void free_ast() {
  struct ast* temp_root = ast_list_root;
  struct ast* current_root = NULL;
  while(temp_root != NULL){
    if (temp_root->child != NULL){
      struct ast_child* temp_ast_child_root = temp_root->child;
      struct ast_child* current_child = NULL;
      while(temp_ast_child_root != NULL){
        current_child = temp_ast_child_root;
        temp_ast_child_root = temp_ast_child_root->next;
        free(current_child);
      }
    }
    current_root = temp_root;
    temp_root = temp_root->next;
    free(current_root);
  }
}
