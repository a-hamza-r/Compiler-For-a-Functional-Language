CC	= gcc
YACC	= yacc
LEX	= lex

comp:	y.tab.c lex.yy.c containers/containers.c comp.c 
	$(CC) lex.yy.c y.tab.c containers/containers.c comp.c containers/instructions.c -o comp -ll

lex.yy.c: mylex.l y.tab.h
	$(LEX) mylex.l

y.tab.c: myyacc.y
	$(YACC) -d myyacc.y

clean: 
	rm comp lex.yy.c y.tab.c y.tab.h
