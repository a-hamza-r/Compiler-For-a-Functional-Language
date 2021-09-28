
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
%type<str> CONST AROP AROPMUL expr IDENTIFIER GETINT GETBOOL TRUE FALSE IF LET fla id term EVAL LGOP CMOP NOT DEFFUN type INT BOOL funcall funcdecl
%type<lst> multiplefla multipleterm args exprfun


%% 

prog : funcdecl prog
	| LPAR EVAL expr RPAR {
		char *cur;
		cur = (char *)malloc(strlen($2)+strlen($3)+10);
		printf("%s %s", $2, $3); 
		free(cur);
	}
;
funcdecl : LPAR DEFFUN LPAR id args RPAR type expr RPAR {
		struct Node *tmp = $5;
		u_int size = 0;
		while (tmp != NULL)
		{
			size += strlen(tmp->tok)+2;
			tmp = tmp->next;
		}
		char *cur;
		cur = (char *)malloc(size+10);
		strcat(cur, "(");
		tmp = $5;
		while (tmp != NULL && tmp->next != NULL)
		{
			sprintf(cur, "%s%s, ", cur, tmp->tok);
			tmp = tmp->next;
		}
		if (tmp != NULL) sprintf(cur, "%s%s", cur, tmp->tok);
		strcat(cur, ")");
		printf("%s %s %s : %s", $7, $4, cur, $8);
		free(cur);
		free(tmp);
	}
;
args :  { $$ = NULL; }
	| LPAR type id RPAR args {
		char *cur;
		cur = (char *)malloc(strlen($2)+strlen($3)+10);
		sprintf(cur, "%s %s", $2, $3);
		struct Node *ret = (struct Node *)malloc(sizeof(struct Node));
		ret->tok = strdup(cur);
		ret->next = $5;
		$$ = ret;
		free(cur);
	}
;
type : INT | BOOL 		{ $$ = strdup($1); };
id : IDENTIFIER			{ $$ = strdup($1); }; 
expr : term | fla		{ $$ = strdup($1); }
;
fla : TRUE | FALSE | id	{ $$ = strdup($1); }
	| LPAR GETBOOL RPAR	{ $$ = strdup($2); }
	| funcall		{ $$ = strdup($1); }
	| LPAR LGOP fla multiplefla RPAR	{
		char* cur;
		struct Node *tmp = $4;
		u_int size = 0;
		while (tmp != NULL) 
		{
			size += strlen(tmp->tok)+strlen($2);
			tmp = tmp->next;
		}
		cur = (char *)malloc(size+strlen($3)+10);
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
		cur = (char *)malloc(strlen($2)+strlen($3)+strlen($4)+10);
		sprintf(cur, "(%s %s %s)", $3, $2, $4);
		$$ = strdup(cur);
		free(cur);
	}
	| LPAR NOT fla RPAR	{
		char* cur;
		cur = (char *)malloc(strlen($2)+strlen($3)+10);
		sprintf(cur, "(%s %s)", $2, $3);
		$$ = strdup(cur);
		free(cur);
	}
	| LPAR IF fla fla fla RPAR {
		char* cur;
		cur = (char *)malloc(strlen($2)+strlen($3)+strlen($4)+strlen($5)+10);
		sprintf(cur, "(%s %s %s else %s)", $2, $3, $4, $5);
		$$ = strdup(cur);
		free(cur);
	}
	| LPAR LET LPAR id expr RPAR fla RPAR {
		char* cur;
		cur = (char *)malloc(strlen($2)+strlen($4)+strlen($5)+strlen($7)+10);
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
funcall : LPAR id exprfun RPAR {
		char* cur;
		u_int size = 0;
		struct Node *tmp = $3;
		while (tmp != NULL)
		{
			size += strlen(tmp->tok)+2;
			tmp = tmp->next;
		}
		cur = (char *)malloc(size+10);
		strcat(cur, "(");
		tmp = $3;
		while (tmp != NULL && tmp->next != NULL)
		{
			sprintf(cur, "%s%s, ", cur, tmp->tok);
			tmp = tmp->next;
		}
		if (tmp != NULL) sprintf(cur, "%s%s", cur, tmp->tok);
		strcat(cur, ")");
		char *ret = (char *)malloc(size+strlen($2)+10);
		sprintf(ret, "(%s %s)", $2, cur);
		$$ = strdup(ret);
		free(cur); free(ret); free(tmp);
	}
;
exprfun : { $$ = NULL; }
	| expr exprfun {
		struct Node *ret = (struct Node *)malloc(sizeof(struct Node));
		ret->tok = $1;
		ret->next = $2;
		$$ = ret;
	}
;
term : CONST | id		{ $$ = strdup($1); }
	| LPAR GETINT RPAR	{
		char *cur;
		cur = (char *)malloc(strlen($2)+10);
		sprintf(cur, "(%s)", $2);
		$$ = strdup(cur); 
		free(cur);
	}
	| funcall		{ $$ = strdup($1); }
	| LPAR AROP term term RPAR { 
		char* cur;
		cur = (char *)malloc(strlen($2)+strlen($3)+strlen($4)+10);
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
			size += strlen(tmp->tok)+strlen($2);
			tmp = tmp->next;
		}
		cur = (char *)malloc(size+strlen($3)+10);
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
		cur = (char *)malloc(strlen($2)+strlen($3)+strlen($4)+strlen($5)+10);
		sprintf(cur, "(%s %s %s else %s)", $2, $3, $4, $5);
		$$ = strdup(cur);
		free(cur);
	}
	| LPAR LET LPAR id expr RPAR term RPAR {
		char* cur;
		cur = (char *)malloc(strlen($2)+strlen($4)+strlen($5)+strlen($7)+10);
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
