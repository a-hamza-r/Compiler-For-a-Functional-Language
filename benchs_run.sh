make

files="testcases/$1/*"
outfile="testcases/$1.out"
for f in $files
do
  if [ ! -d "$f" ]; then 
    filename="$(basename -- $f)"
	  echo "Running $f"
	  ./comp $2 < $f
    if [ "$1" = "good" ]; then 
      dot -Tpdf ast.dot -o testcases/$1/asts/$filename.pdf
      dot -Tpdf cfg.dot -o testcases/$1/cfgs/$filename.pdf
    fi
  fi

done &> $outfile

cat $outfile | grep -i "Running\|error\|segmentation\|aborted" &> "$outfile.grepped"
cat "$outfile.grepped"
