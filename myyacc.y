
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

char *opsWithMultipleArgs(char *, char *, char *, struct Node *, char *);

%}

%union {
	int val; 
	char* str;
	struct Node* lst;
}

%start prog
%token CONST IDENTIFIER EVAL LPAR RPAR AROP AROPMUL GETINT GETBOOL TRUE FALSE IF LET LGOP CMOP NOT DEFFUN INT BOOL
%type<str> CONST AROP AROPMUL expr IDENTIFIER GETINT GETBOOL TRUE FALSE IF LET EVAL LGOP CMOP NOT DEFFUN type INT BOOL funcall funcdecl
%type<lst> multipleexpr args exprfun


%% 

prog : funcdecl prog
	| LPAR EVAL expr RPAR {
		char *cur;
		cur = (char *)malloc(strlen($2)+strlen($3)+10);
		printf("%s %s", $2, $3); 
		free(cur);
	}
;
funcdecl : LPAR DEFFUN LPAR IDENTIFIER args RPAR type expr RPAR {
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
		free(cur); free(tmp);
	}
;
args :  { $$ = NULL; }
	| LPAR type IDENTIFIER RPAR args {
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
type : INT { $$ = strdup($1); } | BOOL { $$ = strdup($1); }
;
expr : CONST { $$ = strdup($1); } | IDENTIFIER { $$ = strdup($1); }
  | TRUE { $$ = strdup($1); } | FALSE { $$ = strdup($1); }
	| funcall { $$ = strdup($1); }
	| LPAR IF expr expr expr RPAR {
		char* cur;
		cur = (char *)malloc(strlen($2)+strlen($3)+strlen($4)+strlen($5)+10);
		sprintf(cur, "(%s %s %s else %s)", $2, $3, $4, $5);
		$$ = strdup(cur);
		free(cur);
	}
	| LPAR LET LPAR IDENTIFIER expr RPAR expr RPAR {
		char* cur;
		cur = (char *)malloc(strlen($2)+strlen($4)+strlen($5)+strlen($7)+10);
		sprintf(cur, "(%s (%s = %s, %s))", $2, $4, $5, $7);
		$$ = strdup(cur);
		free(cur);
	}
	| LPAR GETBOOL RPAR	{ $$ = strdup($2); }
	| LPAR LGOP expr expr multipleexpr RPAR	{
		char *parsed;
		parsed = opsWithMultipleArgs($2, $3, $4, $5, parsed);
		$$ = strdup(parsed);
		free(parsed);
	}
	| LPAR CMOP expr expr RPAR	{ 
		char* cur;
		cur = (char *)malloc(strlen($2)+strlen($3)+strlen($4)+10);
		sprintf(cur, "(%s %s %s)", $3, $2, $4);
		$$ = strdup(cur);
		free(cur);
	}
	| LPAR NOT expr RPAR	{
		char* cur;
		cur = (char *)malloc(strlen($2)+strlen($3)+10);
		sprintf(cur, "(%s %s)", $2, $3);
		$$ = strdup(cur);
		free(cur);
	}
	| LPAR GETINT RPAR	{
		char *cur;
		cur = (char *)malloc(strlen($2)+10);
		sprintf(cur, "(%s)", $2);
		$$ = strdup(cur); 
		free(cur);
	}
	| LPAR AROP expr expr RPAR { 
		char* cur;
		cur = (char *)malloc(strlen($2)+strlen($3)+strlen($4)+10);
		sprintf(cur, "(%s %s %s)", $3, $2, $4);
		$$ = strdup(cur);
		free(cur);
	}
	| LPAR AROPMUL expr expr multipleexpr RPAR	{
		char *parsed;
		parsed = opsWithMultipleArgs($2, $3, $4, $5, parsed);
		$$ = strdup(parsed);
		free(parsed);
	}
;
multipleexpr : expr multipleexpr {
		struct Node *n = (struct Node *)malloc(sizeof(struct Node));
		n->tok = strdup($1);
		n->next = $2;
		$$ = n;
	}
	| { $$ = NULL; }
;
funcall : LPAR IDENTIFIER exprfun RPAR {
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

%% 

void yyerror (char *s) {fprintf(stderr, "%s\n", s);}

char *opsWithMultipleArgs(char *op, char *arg1, char *arg2, struct Node *args, char *parsed)
{	
	struct Node *tmp = args;
	u_int size = 0;
	while (tmp != NULL) 
	{
		size += strlen(tmp->tok)+strlen(op)+1;
		tmp = tmp->next;
	}
	parsed = (char *)malloc(size+strlen(arg1)+strlen(arg2)+strlen(op)+10);
	sprintf(parsed, "(%s %s %s", arg1, op, arg2);
	tmp = args;
	while (tmp != NULL && tmp->next != NULL)
	{
		sprintf(parsed, "%s %s %s", parsed, op, tmp->tok);
		tmp = tmp->next;
	}
	if (tmp != NULL) sprintf(parsed, "%s %s %s", parsed, op, tmp->tok);
	strcat(parsed, ")");
	free(tmp);
	return parsed;
}

