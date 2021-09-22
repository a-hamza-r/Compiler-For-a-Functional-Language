# A compiler for a functional language from scratch (in progress)

This branch shows the work till the development of lexer and parser. It includes <b>mylex.l</b> that contains the code for the lexer, while <b>myyacc.y</b> contains the code for the parser. Further, it contains the directory <b>testcases/</b> containing good and bad test cases. Good testcases should print the operations after running the lexer and parser, while bad testcases should print syntax error. The script <b>benchs.sh</b> along with the arguments <b>good</b> or <b>bad</b> runs respective benchmarks for the lexer and parser, while <b>run.sh</b> runs a single benchmark when provided with the path for the benchmark. Some output files in directory <b>testcases/</b> contain the outputs of the script <b>benchs.sh</b> in some format. Currently, all good testcases print the appropriate results, while bad testcases print syntax error. The pdf manual shows the course assignment that resulted in this work.  