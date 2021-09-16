
lex mylex.l
yacc -d myyacc.y
gcc lex.yy.c y.tab.c -ll
files="testcases/$1/*"
outfile="testcases/$1.out"
for f in $files
do
	echo "Running $f"
	./a.out < $f
done &> $outfile

cat $outfile | grep "Running\|error" &> "$outfile.grepped"
cat "$outfile.grepped"
