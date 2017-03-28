%skeleton "lalr1.cc"

// wrote by hirok
%define parser_class_name {parser}

// parser.hhを生成するよう指定
%defines

%{
#include <string>
#include "cnode.h"
class hii_driver;

#ifdef _MSC_VER
#pragma warning(disable: 4800)
#pragma warning(disable: 4267)
#endif
%}
// The parsing context.
%parse-param { hii_driver& driver }
%lex-param   { hii_driver& driver }

// wrote by hirok
%code requires { #include "cnode.h" }

// %debug
%error-verbose
// Symbols.
%union
{
    int                 ival;
    std::string         *sval;
    cnode               *node;
}

%{
#include "hii_driver.h"
%}

%token          TK_EOF          0   "end of file"
%token <sval>   TK_LCMNT            "lcmnt"
/* literals */
%token <sval>   TK_ID               "id"
%token <ival>   TK_INT              "int"
/* keywords */
%token          TK_PRINT            "p"
%token          TK_IF               "if"
%token          TK_ELIF             "elif"
%token          TK_ELSE             "else"
%token          TK_END              "end"
%token          TK_FN               "fun"
%token          TK_RET              "ret"

%type <node>    expr
%type <node>   exprs
%type <node>    args
%type <node>    stats
%type <node>    stat
%type <node>    assign_stmt
%type <node>    if_stmt
%type <node>    fun_stmt
%type <node>    call_stmt
%type <node>    print_stmt
%type <node>    elifs
%type <node>    id
%type <node>    id_p

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

unit    : stats                         { driver.set_ast($1); }
        ;

stats   : stat                          { $$ = new cnode(OP_STATS, $1); }
        | stats stat                    { cnode * p = new cnode(OP_STATS, $2); $1->set_right(p); $$ = p; }
        ;

/* 以下のようなツリーを構築したいが、
 * 構文ルール上、構築できるツリーの形が決まってしまう？
 * STATS
 *  |-- stat
 *  |-- STATS
 *       |-- stat
 *       |-- STATS
 */

stat    : '\n'                          { $$ = new cnode(); }
        | "lcmnt" '\n'                  { $$ = new cleaf(OP_LCOMMENT, $1); }
        | assign_stmt '\n'              { $$ = $1; }
        | if_stmt '\n'                  { $$ = $1; }
        | fun_stmt '\n'                 { $$ = $1; }
        | call_stmt '\n'                { $$ = $1; }
        | print_stmt '\n'               { $$ = $1; }
        ;

assign_stmt : id '=' expr                   { $$ = new cnode(OP_ASSIGN, $1, $3); }
            ;

if_stmt     : "if" expr '\n' stats elifs "end"  { $$ = new cnode(OP_IF, $2, new cnode(OP_IFTHEN, $4, $5)); }
            ;

elifs       : %empty                        { $$ = nullptr; }
            | "elif" expr '\n' stats elifs  { $$ = new cnode(OP_ELIF, $2, new cnode(OP_IFTHEN, $4, $5)); }
            | "else" stats                  { $$ = new cnode(OP_ELSE, $2); }
            ;

fun_stmt    : "fun" id args '\n' stats "end"    { $$ = new cnode(OP_FN, $2, new cnode(OP_NODE, $3, $5)); }
            ;

args        : %empty                        { $$ = nullptr; }
            | id                            { $$ = new cnode(OP_NODE, $1); }
            | args ',' id                   { cnode * p = new cnode(OP_NODE, $3); $1->set_right(p); $$ = p; }
            ;

call_stmt   : id exprs                      { $$ = new cnode(OP_CALL, $1, $2); }
            ;

print_stmt  : id_p exprs                    { $$ = new cnode(OP_CALL, $1, $2); }
            ;

exprs       : %empty                        { $$ = new cnode(OP_NODE); }
            | expr                          { $$ = new cnode(OP_NODE, $1); }
            | exprs ',' expr                { cnode * p = new cnode(OP_NODE, $3); $1->set_right(p); $$ = p; }
            ;

expr        : expr '-' expr                 { $$ = new cnode(OP_MINUS, $1, $3); }
            | expr '+' expr                 { $$ = new cnode(OP_PLUS, $1, $3); }
            | expr '*' expr                 { $$ = new cnode(OP_TIMES, $1, $3); }
            | expr '/' expr                 { $$ = new cnode(OP_DIVIDE, $1, $3); }
            | '-' expr %prec NEG            { $$ = new cnode(OP_NEG, $2); }
            | '(' expr ')'                  { $$ = $2; }
            | id                            { $$ = $1; }
            | "int"                         { $$ = new cleaf(OP_INT, $1); }
            ;

/* 末端 */
id      : "id"                          { $$ = new cleaf(OP_ID, $1); }
        ;
id_p    : "p"                           { $$ = new cleaf(OP_ID, new std::string("p")); }
        ;

%%
// wrote by hirok
/*
void yy::parser::error(const yy::parser::location_type&, const std::string& m)
{

}
*/
void yy::parser::error(const std::string& m)
{
    driver.error(m);
}

