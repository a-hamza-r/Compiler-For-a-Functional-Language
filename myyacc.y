
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
%token CONST IDENTIFIER EVAL LPAR RPAR GETINT GETBOOL TRUE FALSE IF LET NOT DEFFUN INT BOOL PLUS MINUS MULT DIV MOD EQUAL GT LT GE LE LAND LOR CALL VARID FUNID INTDECL BOOLDECL VARDECL
%type<str> CONST IDENTIFIER GETINT GETBOOL TRUE FALSE IF LET EVAL NOT DEFFUN INT BOOL PLUS MINUS MULT DIV MOD EQUAL GT LT GE LE LAND LOR
%type<val> expr exprs args typefun funid var intdecl booldecl

%% 

var : IDENTIFIER { $$ = insert_node($1, VARDECL); }         // var use identifier
intdecl : IDENTIFIER { $$ = insert_node($1, INTDECL); }         // int declaration
booldecl : IDENTIFIER { $$ = insert_node($1, BOOLDECL); }         // bool declaration
funid : IDENTIFIER { $$ = insert_node($1, FUNID); }             // function identifier 
prog : LPAR DEFFUN LPAR funid args RPAR typefun expr RPAR prog { // function declaration
    insert_child($4);
    for (u_int i = 0; i < $5; i++)
      insert_child(pop_int(&tmp_r, &tmp_t));
    insert_child($7);
    insert_child($8);
    insert_node($2, DEFFUN);
	}
  | LPAR EVAL expr RPAR {                             // eval 
    insert_child($3);
    insert_node("ENTRY", CALL);
  }
;
args : LPAR INT intdecl RPAR args { push_int($3, &tmp_r, &tmp_t); $$ = $5+1; }  // args functions
  | LPAR BOOL booldecl RPAR args { push_int($3, &tmp_r, &tmp_t); $$ = $5+1; }  // args functions
  | { $$ = 0; }
;
typefun : INT { $$ = insert_node("ret INT", INT); }   // int type function
  | BOOL { $$ = insert_node("ret BOOL", BOOL); }      // bool type function
;
expr : CONST { $$ = insert_node($1, CONST); }         // numbers
  | IDENTIFIER { $$ = insert_node($1, VARID); }       // variables
  | TRUE { $$ = insert_node($1, CONST); }              // true 
  | FALSE { $$ = insert_node($1, CONST); }            // false
  | LPAR IDENTIFIER exprs RPAR {                      // function call
    for (u_int i = 0; i < $3; i++)
      insert_child(pop_int(&tmp_r, &tmp_t));
    $$ = insert_node($2, CALL);
	}
	| LPAR IF expr expr expr RPAR {                     // if
	  insert_children(3, $3, $4, $5);
    $$ = insert_node($2, IF);
  }
	| LPAR LET LPAR var expr RPAR expr RPAR {         // let
	  insert_children(3, $4, $5, $7);
    $$ = insert_node($2, LET);
  }
	| LPAR GETBOOL RPAR	{ $$ = insert_node("GET-BOOL", GETBOOL); }  // get-bool
	| LPAR LAND expr expr exprs RPAR	{                 // Logical AND
    insert_children(2, $3, $4);
    for (u_int i = 0; i < $5; i++)
      insert_child(pop_int(&tmp_r, &tmp_t));
    $$ = insert_node($2, LAND);
	}
	| LPAR LOR expr expr exprs RPAR	{               // Logical OR
    insert_children(2, $3, $4);
    for (u_int i = 0; i < $5; i++)
      insert_child(pop_int(&tmp_r, &tmp_t));
    $$ = insert_node($2, LOR);
  }
	| LPAR EQUAL expr expr RPAR	{                       // EQUAL
	  insert_children(2, $3, $4);
    $$ = insert_node($2, EQUAL);
  }
	| LPAR GT expr expr RPAR	{                       // GT
	  insert_children(2, $3, $4);
    $$ = insert_node($2, GT);
  }
	| LPAR LT expr expr RPAR	{                       // LT 
	  insert_children(2, $3, $4);
    $$ = insert_node($2, LT);
  }
	| LPAR GE expr expr RPAR	{                       // GE 
	  insert_children(2, $3, $4);
    $$ = insert_node($2, GE);
  }
	| LPAR LE expr expr RPAR	{                       // LE 
	  insert_children(2, $3, $4);
    $$ = insert_node($2, LE);
  }
	| LPAR NOT expr RPAR	{                           // logical not
	  insert_child($3);
    $$ = insert_node($2, NOT);
  }
	| LPAR GETINT RPAR	{ $$ = insert_node("GET-INT", GETINT); }  // get-int
	| LPAR MOD expr expr RPAR {                       // MOD
	  insert_children(2, $3, $4);
    $$ = insert_node($2, MOD);
	}
	| LPAR DIV expr expr RPAR	{                       // DIV 
	  insert_children(2, $3, $4);
    $$ = insert_node($2, DIV);
  }
	| LPAR MINUS expr expr exprs RPAR	{               // MINUS
    insert_children(2, $3, $4);
    for (u_int i = 0; i < $5; i++)
      insert_child(pop_int(&tmp_r, &tmp_t));
    $$ = insert_node($2, MINUS);
	}
	| LPAR PLUS expr expr exprs RPAR	{               // PLUS
    insert_children(2, $3, $4);
    for (u_int i = 0; i < $5; i++)
      insert_child(pop_int(&tmp_r, &tmp_t));
    $$ = insert_node($2, PLUS);
  }
	| LPAR MULT expr expr exprs RPAR	{               // MULT
    insert_children(2, $3, $4);
    for (u_int i = 0; i < $5; i++)
      insert_child(pop_int(&tmp_r, &tmp_t));
    $$ = insert_node($2, MULT);
  }
;
exprs : expr exprs { push_int($1, &tmp_r, &tmp_t); $$ = $2+1; } // multiple exprs
	| { $$ = 0; }
;

%% 

void yyerror (char *s) {fprintf(stderr, "%s\n", s);}

