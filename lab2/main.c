#include "ast.h"
#include "parser.tab.h"



int main() {
    yyparse();
    query_t *query = yylval.query;
    printQuery(query);

}


