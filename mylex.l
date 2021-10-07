
%{

#include <stdio.h>
#include "y.tab.h"
%}


%% 

0|[1-9][0-9]*		{yylval.str = strdup(yytext); return CONST;};
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
"bool"				{yylval.str = strdup(yytext); return BOOL;}
"define-fun"			{yylval.str = strdup(yytext); return DEFFUN;}
"+"			  {yylval.str = strdup(yytext); return PLUS;}
"-"			  {yylval.str = strdup(yytext); return MINUS;}
"*"			  {yylval.str = strdup(yytext); return MULT;}
"div"			{yylval.str = strdup(yytext); return DIV;}
"mod"			{yylval.str = strdup(yytext); return MOD;}
"="			{yylval.str = strdup(yytext); return EQUAL;}
"<"			{yylval.str = strdup(yytext); return LT;}
">"			{yylval.str = strdup(yytext); return GT;}
"<="			{yylval.str = strdup(yytext); return LE;}
">="		{yylval.str = strdup(yytext); return GE;}
"or"			{yylval.str = strdup(yytext); return LOR;}
"and"			{yylval.str = strdup(yytext); return LAND;}
"not"				{yylval.str = strdup(yytext); return NOT;}
[0][a-zA-Z0-9]+			{;}
[a-zA-Z][a-zA-Z0-9]*		{yylval.str = strdup(yytext); return IDENTIFIER;};
;[^\n]*\n			{;}
[\s ]				{;}

%% 

