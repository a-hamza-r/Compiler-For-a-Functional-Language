
%{
#include <stdio.h>
#include <string.h>
void yyerror(char* s);
int yylex();
%}

%union {int val; char* str;}

%start prog
%token CONST EVAL LPAR RPAR AROP
%type<str> CONST AROP expr

%% 

prog : LPAR EVAL expr RPAR {printf("evaluate %s\n", $3);};

expr : CONST { $$ = strdup($1); }
| LPAR AROP expr expr RPAR { 
	char* cur;
	cur = malloc(sizeof($2) + sizeof($3) + sizeof($4) + 100);
	sprintf(cur, "(%s %s %s)", $3, $2, $4);
	$$ = strdup(cur);
	free(cur);
};

%% 

void yyerror (char *s) {fprintf(stderr, "%s\n", s);}

int main() {
	yyparse();
	return 0;
}
