%skeleton "lalr1.cc"

// wrote by hirok
%define parser_class_name {calc_parser}

// calc-parser.hhを生成するよう指定
%defines

%{
#include <string>
#include "node.h"
#include "exprlist.h"
#include "arglist.h"
class calc_driver;

#ifdef _MSC_VER
#pragma warning(disable: 4800)
#pragma warning(disable: 4267)
#endif
%}
// The parsing context.
%parse-param { calc_driver& driver }
%lex-param   { calc_driver& driver }

// wrote by hirok
%code requires { #include "node.h" }
%code requires { #include "exprlist.h" }
%code requires { #include "arglist.h" }

// %debug
%error-verbose
// Symbols.
%union
{
    int                 ival;
    std::string         *sval;
    cnode               *node;
    exprlist            *exprs;
    arglist             *args;
}

%{
#include "calc-driver.h"
%}

%token          TK_EOF          0   "end of file"
%token <sval>   TK_LCMNT            "lcmnt"
/* literals */
%token <sval>   TK_ID               "id"
%token <ival>   TK_INT              "int"
/* keywords */
%token          TK_PRINT            "p"
%token          TK_LIST             "l"
%token          TK_IF               "if"
%token          TK_ELSE             "else"
%token          TK_END              "end"
%token          TK_LOOP             "loop"
%token          TK_FN               "fn"
%token          TK_RET              "ret"

%type <node>    expr
%type <exprs>   exprs
%type <args>    args

%destructor { delete $$; } "id"
%destructor { delete $$; } "lcmnt"
%destructor { delete $$; } expr
%destructor { delete $$; } exprs
%destructor { delete $$; } args

%left '+' '-';
%left '*' '/';
%left NEG;
%%
%start unit;

unit    : state
        | unit state
        ;

state   : '\n'
        | lcmnt '\n'
        | state2 '\n'
        | state2 lcmnt '\n'
        ;

lcmnt   : "lcmnt"                       { driver.lcmnt($1); }
        ;

state2  : "id" '=' expr                 { driver.assign($1, $3); }
        | "p" expr                      { driver.print($2); }
        | "l"                           { driver.listvars(); }
        | "id" exprs                    { driver.call_state($1, $2); }
        | "if" expr        	            { driver.if_state($2); }
        | "else" "if" expr              { driver.elseif_state($3); }
        | "else"                        { driver.else_state(); }
        | "end"                         { driver.end_state(); }
        | "loop" expr                   { driver.loop_state($2); }
        | "loop"                        { driver.loop_state(); }
        | "fn" "id" args                { driver.declfn($2, $3); }
        | "ret" expr                    { driver.ret($2); }
        | "ret"                         { driver.ret(); }
        ;

exprs   : %empty                        { $$ = new exprlist(); }
        | expr                          { $$ = new exprlist($1); }
        | exprs ',' expr                { $$ = &($1->concat($3)); }

args    : %empty                        { $$ = new arglist(); }
        | "id"                          { $$ = new arglist($1); }
        | args ',' "id"                 { $$ = &($1->concat($3)); }
        ;

expr    : expr '-' expr                 { $$ = new cnode(OP_MINUS, $1, $3); }
        | expr '+' expr                 { $$ = new cnode(OP_PLUS, $1, $3); }
        | expr '*' expr                 { $$ = new cnode(OP_TIMES, $1, $3); }
        | expr '/' expr                 { $$ = new cnode(OP_DIVIDE, $1, $3); }
        | '-' expr %prec NEG            { $$ = new cnode(OP_NEG, $2); }
        | '(' expr ')'                  { $$ = $2; }
        | "id"                          { $$ = new cnode(OP_ID, $1); }
        | "int"                         { $$ = new cnode(OP_INT, $1); }
        ;

%%
// wrote by hirok
/*
void yy::calc_parser::error(const yy::calc_parser::location_type&, const std::string& m)
{
    driver.error(m);
}
*/
void yy::calc_parser::error(const std::string& m)
{
    driver.error(m);
}

