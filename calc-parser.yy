%skeleton "lalr1.cc"

// wrote by hirok
%define parser_class_name {calc_parser}

// calc-parser.hhを生成するよう指定
%defines

%{
#include <string>
#include "cnode.h"
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
%code requires { #include "cnode.h" }
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
%token          TK_IF               "if"
%token          TK_ELIF             "elif"
%token          TK_ELSE             "else"
%token          TK_END              "end"
%token          TK_FN               "fun"
%token          TK_RET              "ret"

%type <node>    expr
%type <exprs>   exprs
%type <args>    args
%type <node>    stats
%type <node>    stat
%type <node>    assign_stmt
%type <node>    if_stmt
%type <node>    fun_stmt
%type <node>    call_stmt
%type <node>    print_stmt
%type <node>    if
%type <node>    elifs
%type <node>    elif
%type <node>    else
%type <node>    lcmnt
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
        | lcmnt '\n'                    { $$ = new cnode(OP_LCOMMENT, $1); }
        | assign_stmt '\n'              { $$ = $1; }
        | if_stmt '\n'                  { $$ = $1; }
        | fun_stmt '\n'                 { $$ = $1; }
        | call_stmt '\n'                { $$ = $1; }
        | print_stmt '\n'               { $$ = $1; }
        ;

assign_stmt : id '=' expr                   { $$ = new cnode(OP_ASSIGN, $1, $3); }
            ;
/*
if_stmt     : if elifs else "end"           { $$ = new clist(OP_STATIF, $1, $2, $3); }
            ;
*/
if_stmt     : "if" expr '\n' stats elifs    { $$ = new cnode(OP_IF, $2, new cnode(OP_IFTHEN, $4, $5); }
            ;

elifs       : "elif" expr '\n' stats elifs  { $$ = new cnode(OP_ELIF, $2, new cnode(OP_IFTHEN, $4, $5); }
            : "else" stats                  { $$ = new cnode(OP_ELSE, $2); }
            : "end"                         { $$ = null; }
            ;




fun_stmt    : "fun" args '\n' stats "end"   { $$ = new cnode(OP_FN, $2, $4); }
            ;
print_stmt  : id_p exprs                    { $$ = new cnode(OP_CALL, $1, $2); }
            ;
call_stmt   : id exprs                      { $$ = new cnode(OP_CALL, $1, $2); }
            ;

if          : "if" expr '\n' stats          { $$ = new cnode(OP_IFTHEN, $2, $4); }
            ;

elifs       : %empty                        { $$ = new clist(OP_ELIFS); }
            | elif                          { $$ = new clist(OP_ELIFS, $1); }
            | elifs elif                    { $$ = $1->concat($2); }
            ;

elif        : "elif" expr '\n' stats        { $$ = new cnode(OP_ELIF, $2, $4); }
            ;

else        : %empty                        { $$ = new cnode(); }
            | "else" '\n' stats             { $$ = new cnode(OP_ELSE, $3); }
            ;

args        : %empty                        { $$ = new clist(OP_ARGS); }
            | id                            { $$ = new clist(OP_ARGS, $1); }
            | args ',' id                   { $$ = &($1->concat(new cnode(OP_ID, $3))); }
            ;

exprs       : %empty                        { $$ = new clist(OP_EXPRS); }
            | expr                          { $$ = new clist(OP_EXPRS, $1); }
            | exprs ',' expr                { $$ = &($1->concat($3)); }
            ;

expr        : expr '-' expr                 { $$ = new cnode(OP_MINUS, $1, $3); }
            | expr '+' expr                 { $$ = new cnode(OP_PLUS, $1, $3); }
            | expr '*' expr                 { $$ = new cnode(OP_TIMES, $1, $3); }
            | expr '/' expr                 { $$ = new cnode(OP_DIVIDE, $1, $3); }
            | '-' expr %prec NEG            { $$ = new cnode(OP_NEG, $2); }
            | '(' expr ')'                  { $$ = $2; }
            | id                            { $$ = $1; }
            | "int"                         { $$ = new cnode(OP_INT, $1); }
            ;

/* 末端 */
lcmnt   : "lcmnt"                       { $$ = new cnode(OP_LCOMMENT, $1); }
        ;
id      : "id"                          { $$ = new cnode(OP_ID, $1); }
        ;
id_p    : "p"                           { $$ = new cnode(OP_ID, new std::string("p")); }
        ;

%%
// wrote by hirok
/*
void yy::calc_parser::error(const yy::calc_parser::location_type&, const std::string& m)
{

}
*/
void yy::calc_parser::error(const std::string& m)
{
    driver.error(m);
}

