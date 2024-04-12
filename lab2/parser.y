%{
#include "ast.h"
int yylex();
void yyerror(const char* s);

extern int yylex();
#define NULLVALUE NULL

void yyerror(const char *s) {
    fprintf(stderr, "Parse error: %s\n", s);
}
#define NULL_STRING (char*) " "

int yywrap() {
    return 1;
}
%}

%token DB DOLLAR
%token DOC_NAME
%token SELECT_QUERY SELECT_MANY_QUERY
%token INSERT_QUERY INSERT_MANY_QUERY
%token DELETE_QUERY DELETE_MANY_QUERY
%token UPDATE_QUERY UPDATE_MANY_QUERY
%token SET_UPDATE LIMIT
%token L_PARENTHESIS R_PARENTHESIS L_BRACE R_BRACE L_BRACKET R_BRACKET QUOTATION_MARK
%token COMMA COLON SEMICOLON DOT
%token OR AND EQ NEQ LT LTE GT GTE REGEX
%token INT_VAL BOOL_VAL DOUBLE_VAL STRING_VAL
%token END EMPTY
%type <string> STRING_VAL DOC_NAME EMPTY
%type <integerValue> INT_VAL limit
%type <boolValue> BOOL_VAL
%type <doubleValue> DOUBLE_VAL
%type <opType> OR AND EQ NEQ LT LTE GT GTE REGEX cmp_operator logical_operator
%type <element> intElement doubleElement boolElement stringElement
%type <document> create_document
%type <documents> fill_document add_set_elements
%type <element> elements element
%type <query> insertOne  insertMany selectOne  selectMany  deleteOne  deleteMany  updateOne updateMany query init
%type<condition> cmp_condition query_conditions query_condition logical_condition conditions condition input

%union{
    char* string;
    bool boolValue;
    double doubleValue;
    int32_t integerValue;
    operationType_t opType;
    element_t* element;
    document_t* document;
    queryResult_t* documents;
    query_t* query;
    condition_t* condition;
}

%%

init:
    query SEMICOLON {
        yylval.query =$1;
        YYACCEPT;
    }
;


query:
    insertOne | insertMany |selectOne | selectMany | deleteOne | deleteMany | updateOne | updateMany
;

updateOne:
        DB DOT STRING_VAL DOT UPDATE_QUERY L_PARENTHESIS  query_conditions add_set_elements R_PARENTHESIS{
            $$ = createQuery(UPDATE, $3,$7, $8);
                    }
;



updateMany:
        DB DOT STRING_VAL DOT UPDATE_MANY_QUERY L_PARENTHESIS query_conditions add_set_elements R_PARENTHESIS{
                $$ = createQuery(UPDATE, $3,$7, $8);
        }
        | DB DOT STRING_VAL DOT UPDATE_MANY_QUERY L_PARENTHESIS query_conditions add_set_elements R_PARENTHESIS DOT limit{
                          $$ = createManyQuery(UPDATE, $3,$7, $8, $11);
                  }
;
deleteOne:
        DB DOT STRING_VAL DOT DELETE_QUERY input{
            $$ = createQuery(DELETE, $3,$6, NULLVALUE);
        }
;


deleteMany:
        DB DOT STRING_VAL DOT DELETE_MANY_QUERY input{
            $$ = createQuery(DELETE, $3,$6, NULLVALUE);
        }
        | DB DOT STRING_VAL DOT DELETE_MANY_QUERY input DOT limit{
                    $$ = createManyQuery(DELETE, $3,$6, NULLVALUE, $8);
                }
;

selectOne:
    DB DOT STRING_VAL DOT SELECT_QUERY input{
        $$ = createQuery(SELECT, $3,$6, NULLVALUE);
    }
;


selectMany:
     DB DOT STRING_VAL DOT SELECT_MANY_QUERY input{
                   $$ = createQuery(SELECT, $3,$6, NULLVALUE);
               }
    | DB DOT STRING_VAL DOT SELECT_MANY_QUERY input DOT limit{
            $$ = createManyQuery(SELECT, $3,$6, NULLVALUE, $8);
        }


;

input:
    L_PARENTHESIS query_conditions R_PARENTHESIS{
        $$=$2;
    }
    | L_PARENTHESIS R_PARENTHESIS{
        $$ = NULLVALUE;
}
insertOne:
    DB DOT STRING_VAL DOT INSERT_QUERY L_PARENTHESIS fill_document R_PARENTHESIS{
        $$ = createQuery(INSERT, $3, NULLVALUE, $7);
    }
;
insertMany:
    DB DOT STRING_VAL DOT INSERT_MANY_QUERY L_PARENTHESIS L_BRACKET fill_document R_BRACKET R_PARENTHESIS{
        $$ = createQuery(INSERT, $3, NULLVALUE, $8);
    }
;


add_set_elements:
     L_BRACE DOLLAR SET_UPDATE COLON  L_BRACE elements R_BRACE R_BRACE{
            $$ = addQueryResult(createQueryResult(),addElements(createDocument("0"), $6
            ));
    }
;


fill_document:
    L_BRACE create_document COMMA elements R_BRACE{
       $$ = addQueryResult(createQueryResult(), addElements($2, $4));
    }
    | L_BRACE create_document COMMA elements R_BRACE COMMA fill_document{
        $$ =  addQueryResult($7,addElements($2, $4));
    }
;


create_document:
    DOC_NAME COLON STRING_VAL {
        $$ = createDocument($3);
    }
;

limit:
     LIMIT L_PARENTHESIS INT_VAL R_PARENTHESIS{
        $$ = $3;
    }
;



elements:
    element {
        $$ = $1;
    }
    | element COMMA elements {
        $1->next = $3;
        $$ =$1;
    }

;

element:
    intElement | doubleElement | boolElement | stringElement
;
intElement:
    STRING_VAL COLON INT_VAL{
        $$ = intElement($1, $3);
    }
;

doubleElement:
    STRING_VAL COLON DOUBLE_VAL{
        $$ = doubleElement($1, $3);
    }
;
boolElement:
    STRING_VAL COLON BOOL_VAL{
            $$ = booleanElement($1, $3);
    }
;
stringElement:
    STRING_VAL COLON QUOTATION_MARK STRING_VAL QUOTATION_MARK{
            $$ = stringElement($1, $4);
    }
;

query_conditions:
        query_condition {
            $$=$1;
        }
        | query_condition COMMA query_conditions{
            $$ = createCondition(OP_SAME_LEVEL, $1, $3);
        }
        | EMPTY {
            $$ = NULLVALUE;
        }
;


query_condition:
        L_BRACE conditions R_BRACE{
            $$ = $2;
        }
        | L_BRACE conditions R_BRACE COMMA{
                    $$ = $2;
                }
;

conditions:
    condition COMMA conditions{
            $$ = createCondition(OP_SAME_LEVEL, $1, $3);
        }
    | condition{
        $$ =$1;
    }

;
condition:
    logical_condition | cmp_condition
;
logical_condition:
    logical_operator COLON L_BRACKET query_condition COMMA query_conditions R_BRACKET{
            $$ = createCondition($1, $6, $4);
    }
    | logical_operator COLON L_BRACKET query_conditions R_BRACKET{
            $$ = createCondition($1, $4, NULLVALUE);
    }
;
cmp_condition:
     element {
        $$ = createCondition(OP_EQ,$1, NULLVALUE);
    }
    | STRING_VAL COLON L_BRACE cmp_operator COLON QUOTATION_MARK STRING_VAL QUOTATION_MARK R_BRACE {
        $$ = createCondition($4, stringElement($1, $7), NULLVALUE);
    }
    | STRING_VAL COLON L_BRACE cmp_operator COLON BOOL_VAL  R_BRACE {
            $$ = createCondition($4, booleanElement($1, $6), NULLVALUE);
    }
    | STRING_VAL COLON L_BRACE cmp_operator COLON DOUBLE_VAL  R_BRACE {
            $$ = createCondition($4, doubleElement($1, $6), NULLVALUE);
    }
    | STRING_VAL COLON L_BRACE cmp_operator COLON INT_VAL  R_BRACE {
            $$ = createCondition($4, intElement($1, $6), NULLVALUE);
        }
;
logical_operator:
    OR | AND
;
cmp_operator:
    EQ | NEQ | LT | LTE | GT | GTE | REGEX
;




%%


