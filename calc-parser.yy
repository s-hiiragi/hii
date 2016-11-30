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

unit    : stats                         { driver.set_ast($1); }
        ;

stats   : stats2                        { $$ = $1; }
        | stats stats2                  { $$ = &($1->concat($2)); }
        ;

stats2  : %empty                        { $$ = new clist(OP_STATS); }
        | '\n'                          { $$ = new clist(OP_STATS); }
        | lcmnt '\n'                    { $$ = new clist(OP_STATS, $1); }
        | stat '\n'                     { $$ = new clist(OP_STATS, $1); }
        | stat lcmnt '\n'               { $$ = new clist(OP_STATS, $1, $2); }
        ;

stat    : stat_assign                   { $$ = $1; }
        | stat_p                        { $$ = $1; }
        | stat_l                        { $$ = $1; }
        | stat_call                     { $$ = $1; }
        | stat_if                       { $$ = $1; }
        | stat_loop                     { $$ = $1; }
        | stat_fn                       { $$ = $1; }
        ;

stat_assign : id '=' expr                   { $$ = new cnode(OP_ASSIGN, $1, $3); }
            ;
stat_p      : id_p exprs                    { $$ = new cnode(OP_CALL, $1, $2); }
            ;
stat_l      : id_l exprs                    { $$ = new cnode(OP_CALL, $1, $2); }
            ;
stat_call   : id exprs                      { $$ = new cnode(OP_CALL, $1, $2); }
            ;
stat_if     : if elifs else "end"           { $$ = new clist(OP_STATIF, $1, $2, $3); }
            ;
stat_loop   : "loop" expr '\n' stats "end"  { $$ = new cnode(OP_STATLOOP, $2, $4); }
            ;
stat_fn     : "fn" args '\n' stats "end"    { $$ = new cnode(OP_FN, $2, $3); }
            ;

if          : "if" expr '\n' stats          { $$ = new cnode(OP_IFTHEN, $2, $4); }
            ;
elifs       : %empty                        { $$ = new clist(OP_ELIFS); }
            | elif                          { $$ = new clist(OP_ELIFS, $1); }
            | elifs elif                    { $$ = $1.concat($2); }
            ;
elif        : "elif" expr '\n' stats        { $$ = new cnode(OP_ELIF, expr, stats); }
            ;
else        : %empty                        { $$ = new cleaf(OP_EMPTY); }
            | "else" '\n' stats             { $$ = new cnode(OP_ELSE, $3); }
            ;

args        : %empty                        { $$ = new clist(OP_ARGS); }
            | id                            { $$ = new clist(OP_ARGS, $1); }
            | args ',' id                   { $$ = &($1->concat($3)); }
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
            | "int"                         { $$ = new cleaf(OP_INT, $1); }
            ;

/* 末端 */
lcmnt   : "lcmnt"                       { $$ = new cleaf(OP_LCMNT, $1); }
        ;
id      : "id"                          { $$ = new cleaf(OP_ID, $1); }
        ;
id_p    : "p"                           { $$ = new cleaf(OP_ID, new std::string("p")); }
        ;
id_l    : "l"                           { $$ = new cleaf(OP_ID, new std::string("l")); }
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

