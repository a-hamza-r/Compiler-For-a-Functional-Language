# A compiler for a functional language from scratch (in progress)

This branch shows the work till the development of lexer and parser. It includes <b>mylex.l</b> that contains the code for the lexer, and <b>myyacc.y</b> contains the code for the parser. 
Further, it contains the directory <b>testcases/</b> containing good and bad test cases. 
Good testcases should print the operations after running the lexer and parser, while bad testcases should print syntax error.
The <b>Makefile</b> can be used to compile the parser/lexer, run a single benchmark, run multiple benchmarks (all benchmarks, all good benchmarks, all bad benchmarks) -- use <b>make help</b> to learn more.
All good benchmarks should print an output result, while all bad benchmarks should output <i>syntax error</i>.
The directory <b>results</b> contains the results of multiple benchmark runs (in files with extensions <b>.out</b>, while the single benchmark runs are shown on standard output.
The pdf manual shows the course assignment that resulted in this work.  
