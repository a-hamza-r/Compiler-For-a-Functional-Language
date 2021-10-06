#include "../y.tab.h"
#include "containers.h"
int yyparse();

int main (int argc, char **argv) {
  int retval = yyparse();
  if (retval == 0) print_ast();      // run `dot -Tpdf ast.dot -o ast.pdf` to create a PDF
  // TODO: add semantic checks
  free_ast();
  return retval;
}
