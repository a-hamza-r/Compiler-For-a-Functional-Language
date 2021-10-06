make

files="testcases/$1/*"
outfile="testcases/$1.out"
for f in $files
do
	echo "Running $f"
	./comp < $f
done &> $outfile

cat $outfile | grep "Running\|error" &> "$outfile.grepped"
cat "$outfile.grepped"
