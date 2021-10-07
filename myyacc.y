
%{

#include <stdio.h>
#include <string.h>
#include "containers/containers.h"

typedef unsigned int u_int;

int yylex();
void yyerror(char *s);

struct node_int* tmp_r;
struct node_int* tmp_t;
%}

%union {
	int val; 
	char* str;
}


%start prog
%token CONST IDENTIFIER EVAL LPAR RPAR AROP AROPMUL GETINT GETBOOL TRUE FALSE IF LET LGOP CMOP NOT DEFFUN INT BOOL
%type<str> CONST AROP AROPMUL IDENTIFIER GETINT GETBOOL TRUE FALSE IF LET EVAL LGOP CMOP NOT DEFFUN INT BOOL type
%type<val> expr exprs args id typefun

%% 

id : IDENTIFIER { $$ = insert_node($1, IDENTIFIER); }
prog : LPAR DEFFUN LPAR id args RPAR typefun expr RPAR prog {
    insert_child($4);
    for (u_int i = 0; i < $5; i++)
      insert_child(pop_int(&tmp_r, &tmp_t));
    insert_child($7);
    insert_child($8);
    insert_node($2, DEFFUN);
	}
  | LPAR EVAL expr RPAR {
    insert_node("main", $3+1);
    insert_child($3+1);
    insert_child($3);
    insert_node("ENTRY", EVAL);
  }
;
args : LPAR type id RPAR args { push_int($3, &tmp_r, &tmp_t); $$ = $5+1; }
  | { $$ = 0; }
;
typefun : INT { $$ = insert_node("ret INT", INT); } 
  | BOOL { $$ = insert_node("ret BOOL", BOOL); } 
;
type : INT { $$ = strdup($1); } | BOOL { $$ = strdup($1); }
;
expr : CONST { $$ = insert_node($1, CONST); } 
  | IDENTIFIER { $$ = insert_node($1, IDENTIFIER); }
  | TRUE { $$ = insert_node($1, TRUE); } | FALSE { $$ = insert_node($1, FALSE); }
  | LPAR IDENTIFIER exprs RPAR {
    for (u_int i = 0; i < $3; i++)
      insert_child(pop_int(&tmp_r, &tmp_t));
    $$ = insert_node($2, IDENTIFIER);
	}
	| LPAR IF expr expr expr RPAR {
	  insert_children(3, $3, $4, $5);
    $$ = insert_node($2, IF);
  }
	| LPAR LET LPAR id expr RPAR expr RPAR {
	  insert_children(3, $4, $5, $7);
    $$ = insert_node($2, LET);
  }
	| LPAR GETBOOL RPAR	{ $$ = insert_node("GET-BOOL", GETBOOL); }
	| LPAR LGOP expr expr exprs RPAR	{
    insert_children(2, $3, $4);
    for (u_int i = 0; i < $5; i++)
      insert_child(pop_int(&tmp_r, &tmp_t));
    $$ = insert_node($2, LGOP);
	}
	| LPAR CMOP expr expr RPAR	{ 
	  insert_children(2, $3, $4);
    $$ = insert_node($2, CMOP);
  }
	| LPAR NOT expr RPAR	{
	  insert_child($3);
    $$ = insert_node($2, NOT);
  }
	| LPAR GETINT RPAR	{ $$ = insert_node("GET-INT", GETINT); }
	| LPAR AROP expr expr RPAR { 
	  insert_children(2, $3, $4);
    $$ = insert_node($2, AROP);
	}
	| LPAR AROPMUL expr expr exprs RPAR	{
    insert_children(2, $3, $4);
    for (u_int i = 0; i < $5; i++)
      insert_child(pop_int(&tmp_r, &tmp_t));
    $$ = insert_node($2, LGOP);
	}
;
exprs : expr exprs { push_int($1, &tmp_r, &tmp_t); $$ = $2+1; }
	| { $$ = 0; }
;

%% 

void yyerror (char *s) {fprintf(stderr, "%s\n", s);}

