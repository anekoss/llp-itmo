%option noyywrap
%{
    #include <stdio.h>
    #include <string.h>
    #include "ast.h"
    #include "parser.tab.h"
%}

empty                   [ \t\r\n\f\v\s]+

%%
""                      {return EMPTY;}
"END"                   { return END; }
"("                     { return L_PARENTHESIS; }
")"                     { return R_PARENTHESIS; }
"{"                     { return L_BRACE; }
"}"                     { return R_BRACE; }
"["                     { return L_BRACKET; }
"]"                     { return R_BRACKET; }
"."                     { return DOT;}
"\""                    { return QUOTATION_MARK;}
","                     { return COMMA;}
":"                     { return COLON;}
";"                     { return SEMICOLON;}
"doc_name"              {return DOC_NAME;}
"db"                    { return DB;}
"findOne"               { return SELECT_QUERY;}
"find"                  { return SELECT_MANY_QUERY;}
"insertOne"             { return INSERT_QUERY;}
"insertMany"            { return INSERT_MANY_QUERY;}
"deleteMany"            { return DELETE_MANY_QUERY;}
"deleteOne"             { return DELETE_QUERY;}
"updateMany"            { return UPDATE_MANY_QUERY;}
"updateOne"             {return UPDATE_QUERY;}
"set"                  { return SET_UPDATE;}
"$"                      { return DOLLAR;}
"limit"                 { return LIMIT;}


{empty}          {              }



"$or"                   {yylval.opType = OP_OR; return OR;}
"$and"                  {yylval.opType = OP_AND; return AND;}
"$eq"                   {yylval.opType = OP_EQ; return EQ;}
"$ne"                   {yylval.opType = OP_NEQ; return NEQ;}
"$gt"                   {yylval.opType = OP_GT; return GT;}
"$gte"                  {yylval.opType = OP_GTE; return GTE;}
"$lt"                   {yylval.opType = OP_LT; return LT;}
"$lte"                  {yylval.opType = OP_LTE; return LTE;}
"$regex"                {yylval.opType = OP_REGEX; return REGEX;}
[+-]?[0-9]+             {yylval.integerValue = atoi(yytext); return INT_VAL; }
"true"                  {yylval.boolValue = true; return BOOL_VAL; }
"false"                 {yylval.boolValue = false; return BOOL_VAL; }
[+-]?[0-9]*\.[0-9]*     {yylval.doubleValue = atof(yytext); return DOUBLE_VAL;}
[A-Za-z_][A-Za-z0-9_]*  {yylval.string = strdup(yytext); return STRING_VAL;}

%%

