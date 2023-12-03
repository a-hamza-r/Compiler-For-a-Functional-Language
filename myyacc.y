
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
void funcdecl(char *, struct Node *, char *, char *);

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
	| eval
;
eval : LPAR EVAL expr RPAR { printf("%s %s", $2, $3); }
;
funcdecl : LPAR DEFFUN LPAR id args RPAR INT term RPAR {
         funcdecl($4, $5, $7, $8);
	}
    | LPAR DEFFUN LPAR id args RPAR BOOL fla RPAR {
         funcdecl($4, $5, $7, $8);
    }
;
args :  { $$ = NULL; }
	| LPAR type id RPAR args {
        size_t size = strlen($2)+1+strlen($3)+1; // +1 for space, +1 for \0
		char *cur = (char *)malloc(size*sizeof(char));
		snprintf(cur, size, "%s %s", $2, $3);
		struct Node *ret = (struct Node *)malloc(sizeof(struct Node));
		ret->tok = strdup(cur);
		ret->next = $5;
		$$ = ret;
		free(cur);
	}
;
type : INT { $$ = strdup($1); } | BOOL { $$ = strdup($1); }
;
id : IDENTIFIER	{ $$ = strdup($1); }
; 
expr : term { $$ = strdup($1); } | fla { $$ = strdup($1); }
;
fla : TRUE { $$ = strdup($1); } | FALSE { $$ = strdup($1); }
	| id { $$ = strdup($1); }
	| LPAR GETBOOL RPAR	{ $$ = strdup($2); }
	| funcall { $$ = strdup($1); }
	| LPAR LGOP fla fla multiplefla RPAR	{
		char *parsed;
		parsed = opsWithMultipleArgs($2, $3, $4, $5, parsed);
		$$ = strdup(parsed);
		free(parsed);
	}
	| LPAR CMOP term term RPAR	{ 
        size_t size = 1+strlen($3)+1+strlen($2)+1+strlen($4)+1+1; // +5 for spaces, \0, and ()
		char* cur = (char *)malloc(size*sizeof(char));
		snprintf(cur, size, "(%s %s %s)", $3, $2, $4);
		$$ = strdup(cur);
		free(cur);
	}
	| LPAR NOT fla RPAR	{
        size_t size = 1+strlen($2)+1+strlen($3)+1+1; // +4 for spaces, \0, and ()
		char* cur = (char *)malloc(size*sizeof(char));
		snprintf(cur, size, "(%s %s)", $2, $3);
		$$ = strdup(cur);
		free(cur);
	}
	| LPAR IF fla fla fla RPAR {
        size_t size = 1+strlen($2)+1+strlen($3)+6+strlen($4)+6+strlen($5)+1+1;
            // +16 for spaces, \0, and (), else
		char* cur = (char *)malloc(size*sizeof(char));
		snprintf(cur, size, "(%s %s then %s else %s)", $2, $3, $4, $5);
		$$ = strdup(cur);
		free(cur);
	}
	| LPAR LET LPAR id expr RPAR fla RPAR {
        size_t size = 1+strlen($2)+2+strlen($4)+3+strlen($5)+2+strlen($7)+1+1+1;
		char* cur = (char *)malloc(size*sizeof(char));
		snprintf(cur, size, "(%s (%s = %s, %s))", $2, $4, $5, $7);
		$$ = strdup(cur);
		free(cur);
	}
;
multiplefla : fla multiplefla {
		struct Node *n = (struct Node *)malloc(sizeof(struct Node));
		n->tok = strdup($1);
		n->next = $2;
		$$ = n;
	}
	| { $$ = NULL; }
;
funcall : LPAR id exprfun RPAR {
		u_int size = 0;
		struct Node *tmp = $3;
		while (tmp != NULL)
		{
			size += strlen(tmp->tok);
            size += 2; // for ", "
			tmp = tmp->next;
		}
        size += 2; // for "()"
        size += 1; // for \0
		char* cur = (char *)malloc(size*sizeof(char));
		snprintf(cur, 2, "(");
		tmp = $3;
		while (tmp != NULL && tmp->next != NULL)
		{
			snprintf(cur+strlen(cur), size-strlen(cur), "%s, ", tmp->tok);
			tmp = tmp->next;
		}
		if (tmp != NULL) snprintf(cur+strlen(cur), size-strlen(cur), "%s", tmp->tok);
        snprintf(cur+strlen(cur), 2, ")");
        size = size+strlen($2)+3;   // +3 for spaces, and ()
		char *ret = (char *)malloc(size*sizeof(char));
		snprintf(ret, size, "(%s %s)", $2, cur);
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
term : CONST { $$ = strdup($1); } | id { $$ = strdup($1); }
	| LPAR GETINT RPAR {
        size_t size = 1+strlen($2)+1+1;
		char *cur = (char *)malloc(size*sizeof(char));
		snprintf(cur, size, "(%s)", $2);
		$$ = strdup(cur); 
		free(cur);
	}
	| funcall { $$ = strdup($1); }
	| LPAR AROP term term RPAR { 
        size_t size = 1+strlen($3)+1+strlen($2)+1+strlen($4)+1+1;
		char* cur = (char *)malloc(size*sizeof(char));
		snprintf(cur, size, "(%s %s %s)", $3, $2, $4);
		$$ = strdup(cur);
		free(cur);
	}
	| LPAR AROPMUL term term multipleterm RPAR	{
		char *parsed;
		parsed = opsWithMultipleArgs($2, $3, $4, $5, parsed);
		$$ = strdup(parsed);
		free(parsed);
	}
	| LPAR IF fla term term RPAR {
        size_t size = 1+strlen($2)+1+strlen($3)+6+strlen($4)+6+strlen($5)+1+1;
		char* cur = (char *)malloc(size*sizeof(char));
		snprintf(cur, size, "(%s %s then %s else %s)", $2, $3, $4, $5);
		$$ = strdup(cur);
		free(cur);
	}
	| LPAR LET LPAR id expr RPAR term RPAR {
        size_t size = 1+strlen($2)+2+strlen($4)+3+strlen($5)+2+strlen($7)+1+1+1;
		char* cur = (char *)malloc(size*sizeof(char));
		snprintf(cur, size, "(%s (%s = %s, %s))", $2, $4, $5, $7);
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
	| { $$ = NULL; }
;

%% 

void yyerror (char *s) {fprintf(stderr, "%s\n", s);}

void funcdecl(char *id, struct Node *args, char *type, char *expr) {
    struct Node *tmp = args;
    u_int size = 0;
    while (tmp != NULL)
    {
        size += strlen(tmp->tok);
        size += 2; // ", "
        tmp = tmp->next;
    }
    size += 2; // for "()"
    size += 1; // for \0
    char *cur = (char *)malloc(size*sizeof(char));
    snprintf(cur, 2, "(");
    tmp = args;
    while (tmp != NULL && tmp->next != NULL)
    {
        snprintf(cur+strlen(cur), size-strlen(cur), "%s, ", tmp->tok);
        tmp = tmp->next;
    }
    if (tmp != NULL) snprintf(cur+strlen(cur), size-strlen(cur), "%s", tmp->tok);
    snprintf(cur+strlen(cur), 2, ")");
    printf("%s %s %s : %s", type, id, cur, expr);
    free(cur); free(args); free(tmp);
}

char *opsWithMultipleArgs(char *op, char *arg1, char *arg2, struct Node *args, char *parsed)
{
	struct Node *tmp = args;
	u_int size = 0;
    size += strlen(arg1)+1+strlen(op)+1+strlen(arg2);   // arg1 op arg2
	while (tmp != NULL) 
	{
		size += strlen(op)+1+strlen(tmp->tok);    // op arg
		tmp = tmp->next;
	}
    size += 2; // for ()
    size += 1; // for \0
	parsed = (char *)malloc(size*sizeof(char));
	snprintf(parsed, size, "(%s %s %s", arg1, op, arg2);
	tmp = args;
	while (tmp != NULL && tmp->next != NULL)
	{
		snprintf(parsed+strlen(parsed), size-strlen(parsed), " %s %s", op, tmp->tok);
		tmp = tmp->next;
	}
	if (tmp != NULL) snprintf(parsed+strlen(parsed), size-strlen(parsed), " %s %s", op, tmp->tok);
    snprintf(parsed+strlen(parsed), 2, ")");
	free(tmp);
	return parsed;
}

int main() {
	yyparse();
	return 0;
}
