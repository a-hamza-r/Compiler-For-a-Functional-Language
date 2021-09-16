
%{
#include <stdio.h>
#include <string.h>
void yyerror(char* s);
int yylex();
%}

%union {int val; char* str;}

%start prog
%token CONST IDENTIFIER EVAL LPAR RPAR AROP GETINT GETBOOL TRUE FALSE IF LET LGOP CMOP NOT
%type<str> CONST AROP expr IDENTIFIER GETINT GETBOOL TRUE FALSE IF LET fla var term EVAL prog LGOP CMOP NOT

%% 

prog : LPAR EVAL expr RPAR 	{ printf("%s %s\n", $2, $3); };
expr : term | fla		{ $$ = strdup($1); };
fla : TRUE | FALSE | var	{ $$ = strdup($1); }
	| LPAR GETBOOL RPAR	{ $$ = strdup($2); }
	| LPAR LGOP fla fla RPAR	{
		char* cur;
		cur = (char *)malloc(sizeof($2)+sizeof($3)+sizeof($4)+100);
		sprintf(cur, "(%s %s %s)", $3, $2, $4);
		$$ = strdup(cur);
		free(cur);
	}
	| LPAR CMOP term term RPAR	{ 
		char* cur;
		cur = (char *)malloc(sizeof($2)+sizeof($3)+sizeof($4)+100);
		sprintf(cur, "(%s %s %s)", $3, $2, $4);
		$$ = strdup(cur);
		free(cur);
	}
	| LPAR NOT fla RPAR	{
		char* cur;
		cur = (char *)malloc(sizeof($2)+sizeof($3)+100);
		sprintf(cur, "(%s %s)", $2, $3);
		$$ = strdup(cur);
		free(cur);
	}
	| LPAR IF fla fla fla RPAR {
		char* cur;
		cur = (char *)malloc(sizeof($2)+sizeof($3)+sizeof($4)+sizeof($5)+100);
		sprintf(cur, "(%s (%s) %s else %s)", $2, $3, $4, $5);
		$$ = strdup(cur);
		free(cur);
	}
	| LPAR LET LPAR var expr RPAR fla RPAR {
		char* cur;
		cur = (char *)malloc(sizeof($2)+sizeof($4)+sizeof($5)+sizeof($7)+100);
		sprintf(cur, "(%s (%s = %s, %s))", $2, $4, $5, $7);
		$$ = strdup(cur);
		free(cur);
	}
;
var : IDENTIFIER		{ $$ = strdup($1); };
term : CONST | var		{ $$ = strdup($1); }
	| LPAR GETINT RPAR	{ $$ = strdup($2); }
	| LPAR AROP term term RPAR { 
		char* cur;
		cur = (char *)malloc(sizeof($2)+sizeof($3)+sizeof($4)+100);
		sprintf(cur, "(%s %s %s)", $3, $2, $4);
		$$ = strdup(cur);
		free(cur);
	}
	| LPAR IF fla term term RPAR {
		char* cur;
		cur = (char *)malloc(sizeof($2)+sizeof($3)+sizeof($4)+sizeof($5)+100);
		sprintf(cur, "(%s (%s) %s else %s)", $2, $3, $4, $5);
		$$ = strdup(cur);
		free(cur);
	}
	| LPAR LET LPAR var expr RPAR term RPAR {
		char* cur;
		cur = (char *)malloc(sizeof($2)+sizeof($4)+sizeof($5)+sizeof($7)+100);
		sprintf(cur, "(%s (%s = %s, %s))", $2, $4, $5, $7);
		$$ = strdup(cur);
		free(cur);
	}
;

%% 

void yyerror (char *s) {fprintf(stderr, "%s\n", s);}

int main() {
	yyparse();
	return 0;
}
