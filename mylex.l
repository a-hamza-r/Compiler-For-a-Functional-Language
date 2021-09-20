
%{

#include <stdio.h>
#include "y.tab.h"
%}


%% 

0|([1-9]+[0-9]*)	{yylval.str = strdup(yytext); return CONST;};
0([0-9]+)		{printf("wrong number %s\n", yytext);}
"eval"			{yylval.str = strdup(yytext); return EVAL;}
"("			{return LPAR;}
")"			{return RPAR;}
"get-int"		{yylval.str = strdup(yytext); return GETINT;}
"get-bool"		{yylval.str = strdup(yytext); return GETBOOL;}
"true"			{yylval.str = strdup(yytext); return TRUE;}
"false"			{yylval.str = strdup(yytext); return FALSE;}
"if"			{yylval.str = strdup(yytext); return IF;}
"let"			{yylval.str = strdup(yytext); return LET;}
"int"			{yylval.str = strdup(yytext); return INT;}
"bool"			{yylval.str = strdup(yytext); return BOOL;}
"define-fun"			{yylval.str = strdup(yytext); return DEFFUN;}
"+"|"*"				{yylval.str = strdup(yytext); return AROPMUL;}
"-"|"div"|"mod"			{yylval.str = strdup(yytext); return AROP;}
"="|"<"|">"|">="|"<="		{yylval.str = strdup(yytext); return CMOP;}
"and"|"or"			{yylval.str = strdup(yytext); return LGOP;}
"not"				{yylval.str = strdup(yytext); return NOT;}
[a-zA-Z][a-zA-Z0-9]*		{yylval.str = strdup(yytext); return IDENTIFIER;};
.			{;}

%% 

