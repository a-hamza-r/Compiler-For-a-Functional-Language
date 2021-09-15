
%{

#include <stdio.h>
#include "y.tab.h"
%}


%% 

0|([1-9]+[0-9]*) 	{printf("read number %s\n", yytext); yylval.str = strdup(yytext); return CONST;};
0([0-9]+)	{printf("wrong number %s\n", yytext);}
"eval"		{return EVAL;}
"("		{return LPAR;}
")"		{return RPAR;}
"+"|"-"|"*"	{ yylval.str = strdup(yytext); return AROP;}
.		{;}

%% 

