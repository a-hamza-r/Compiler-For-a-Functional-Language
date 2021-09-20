
%{

#include <stdio.h>
#include <string.h>
void yyerror(char* s);
int yylex();

struct Node {
	char *tok;
	struct Node *next;
};
typedef unsigned int u_int;

%}

%union {
	int val; 
	char* str;
	struct Node* lst;
}

%start prog
%token CONST IDENTIFIER EVAL LPAR RPAR AROP AROPMUL GETINT GETBOOL TRUE FALSE IF LET LGOP CMOP NOT DEFFUN INT BOOL
%type<str> CONST AROP AROPMUL expr IDENTIFIER GETINT GETBOOL TRUE FALSE IF LET fla var term EVAL prog LGOP CMOP NOT DEFFUN fun type INT BOOL funcall
%type<lst> multiplefla multipleterm args exprfun


%% 

prog : LPAR DEFFUN LPAR fun args RPAR type expr RPAR prog {
		struct Node *tmp = $5;
		u_int size = 0;
		while (tmp != NULL)
		{
			size += sizeof(tmp->tok)+2;
			tmp = tmp->next;
		}
		char *cur;
		cur = (char *)malloc(size+100);
		strcat(cur, "(");
		tmp = $5;
		while (tmp != NULL && tmp->next != NULL)
		{
			sprintf(cur, "%s%s, ", cur, tmp->tok);
			tmp = tmp->next;
		}
		if (tmp != NULL) sprintf(cur, "%s%s", cur, tmp->tok);
		strcat(cur, ")");
		printf("%s %s %s : %s\n", $7, $4, cur, $8);
	}
	| LPAR EVAL expr RPAR 	{ printf("%s %s\n", $2, $3); }
;
args : /* empty */ { $$ = NULL; }
	| LPAR type var RPAR args {
		char *cur;
		cur = (char *)malloc(sizeof($2)+sizeof($3)+100);
		sprintf(cur, "%s %s", $2, $3);
		struct Node *ret = (struct Node *)malloc(sizeof(struct Node));
		ret->tok = strdup(cur);
		ret->next = $5;
		$$ = ret;
		free(cur);
	}
;
type : INT | BOOL 		{ $$ = strdup($1); };
fun : IDENTIFIER		{ $$ = strdup($1); }; 
expr : term | fla		{ $$ = strdup($1); }
	| funcall		{ $$ = strdup($1); }
;
fla : TRUE | FALSE | var	{ $$ = strdup($1); }
	| LPAR GETBOOL RPAR	{ $$ = strdup($2); }
	| LPAR LGOP fla multiplefla RPAR	{
		char* cur;
		struct Node *tmp = $4;
		u_int size = 0;
		while (tmp != NULL) 
		{
			size += sizeof(tmp->tok)+sizeof($2);
			tmp = tmp->next;
		}
		cur = (char *)malloc(size+sizeof($3)+100);
		sprintf(cur, "(%s", $3);
		tmp = $4;
		while (tmp->next != NULL)
		{
			sprintf(cur, "%s %s %s", cur, $2, tmp->tok);
			tmp = tmp->next;
		}
		sprintf(cur, "%s %s %s)", cur, $2, tmp->tok);
		$$ = strdup(cur);
		free(tmp); free(cur);
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
multiplefla : fla multiplefla 	{
		struct Node *n = (struct Node *)malloc(sizeof(struct Node));
		n->tok = strdup($1);
		n->next = $2;
		$$ = n;
	}
	| fla		{ 
		struct Node *n = (struct Node *)malloc(sizeof(struct Node));		
		n->tok = strdup($1);
		n->next = NULL;
		$$ = n; 
	}
;
funcall : LPAR fun exprfun RPAR {
		char* cur;
		u_int size = 0;
		struct Node *tmp = $3;
		while (tmp != NULL)
		{
			size += sizeof(tmp->tok)+2;
			tmp = tmp->next;
		}
		cur = (char *)malloc(size+100);
		strcat(cur, "(");
		while (tmp != NULL && tmp->next != NULL)
		{
			sprintf(cur, "%s%s, ", cur, tmp->tok);
			tmp = tmp->next;
		}
		if (tmp != NULL) sprintf(cur, "%s%s)", cur, tmp->tok);
		sprintf(cur, "(%s %s)", $2, cur);
		$$ = strdup(cur);
		free(cur);
	}
;
exprfun : /* empty */ { $$ = NULL; }
	| expr exprfun {
		struct Node *ret = (struct Node *)malloc(sizeof(struct Node));
		ret->tok = $1;
		ret->next = $2;
		$$ = ret;
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
	| LPAR AROPMUL term multipleterm RPAR	{
		char* cur;
		struct Node *tmp = $4;
		u_int size = 0;
		while (tmp != NULL) 
		{
			size += sizeof(tmp->tok)+sizeof($2);
			tmp = tmp->next;
		}
		cur = (char *)malloc(size+sizeof($3)+100);
		sprintf(cur, "(%s", $3);
		tmp = $4;
		while (tmp->next != NULL)
		{
			sprintf(cur, "%s %s %s", cur, $2, tmp->tok);
			tmp = tmp->next;
		}
		sprintf(cur, "%s %s %s)", cur, $2, tmp->tok);
		$$ = strdup(cur);
		free(tmp); free(cur);
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
multipleterm : term multipleterm {
		struct Node *n = (struct Node *)malloc(sizeof(struct Node));
		n->tok = strdup($1);
		n->next = $2;
		$$ = n;
	}
	| term	{ 
		struct Node *n = (struct Node *)malloc(sizeof(struct Node));		
		n->tok = strdup($1);
		n->next = NULL;
		$$ = n; 
	}
;

%% 

void yyerror (char *s) {fprintf(stderr, "%s\n", s);}

int main() {
	yyparse();
	return 0;
}
