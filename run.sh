
lex mylex.l
yacc -d myyacc.y
gcc lex.yy.c y.tab.c -ll
file=$1
echo "Running $1"
./a.out < $1
