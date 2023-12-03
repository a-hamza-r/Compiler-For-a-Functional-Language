CC = gcc
LEXFLAGS = -ll

LEX = lex
YACC = yacc

TARGET = parser

TESTCASE_DIR = testcases
GOOD_TESTCASE_DIR = $(TESTCASE_DIR)/good
BAD_TESTCASE_DIR = $(TESTCASE_DIR)/bad
GOOD_TESTCASES = $(wildcard $(GOOD_TESTCASE_DIR)/*)
BAD_TESTCASES = $(wildcard $(BAD_TESTCASE_DIR)/*)
TESTCASES = $(GOOD_TESTCASES) $(BAD_TESTCASES)
RESULTS = results

all: $(TARGET)

lex.yy.c: mylex.l
	$(LEX) mylex.l

y.tab.c y.tab.h: myyacc.y
	$(YACC) -d myyacc.y

$(TARGET): lex.yy.c y.tab.c y.tab.h
	$(CC) -o $(TARGET) lex.yy.c y.tab.c $(LEXFLAGS)

run:
	@echo "testcase: "
	@cat $(input)
	@echo
	@echo "output: "
	@./$(TARGET) < $(input)

runall:
	@for testcase in $(TESTCASES); do \
		echo "Running $$testcase"; \
		cat $$testcase; \
		echo; \
		echo "output: "; \
		./$(TARGET) < $$testcase; \
		echo; \
		echo "-----------------"; \
		echo; \
	done > $(RESULTS)/all.out 2>&1

runallgood:
	@for testcase in $(GOOD_TESTCASES); do \
		echo "Running $$testcase"; \
		cat $$testcase; \
		echo; \
		echo "output: "; \
		./$(TARGET) < $$testcase; \
		echo; \
		echo "-----------------"; \
		echo; \
	done > $(RESULTS)/good.out 2>&1

runallbad:
	@for testcase in $(BAD_TESTCASES); do \
		echo "Running $$testcase"; \
		cat $$testcase; \
		echo; \
		echo "output: "; \
		./$(TARGET) < $$testcase; \
		echo; \
		echo "-----------------"; \
		echo; \
	done > $(RESULTS)/bad.out 2>&1

clean:
	rm -f $(TARGET) lex.yy.c y.tab.c y.tab.h

help:
	@echo "Makefile targets:"
	@echo "  all        - Build the program (default target)"
	@echo "  run        - Run the compiled program with an input file argument"
	@echo "               Usage: make run input=input_file.txt"
	@echo "  runall     - Run the compiled program with all testcases"
	@echo "  runallgood - Run the compiled program with all good testcases"
	@echo "  runallbad  - Run the compiled program with all bad testcases"
	@echo "  clean      - Remove the compiled program"
	@echo "  help       - Display this help message"


.PHONY: all run clean help runall runallgood runallbad
