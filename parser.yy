%skeleton "lalr1.cc"

// wrote by hirok
%define parser_class_name {parser}

// parser.hhを生成するよう指定
%defines

%{
#include <string>
#include "cnode.h"
#include "cleaf.h"
#include "clist.h"
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
%code requires { #include "clist.h" }

// %debug
%error-verbose
// Symbols.
%union
{
    clist               *list;
    cnode               *node;
    int                 ival;
    std::string         *sval;
}

%{
#include "hii_driver.h"
%}

%token          TK_EOF          0   "end of file"
%token <sval>   TK_LCMNT            "lcmnt"
/* literals */
%token <sval>   TK_ID               "id"
%token <ival>   TK_INT              "int"
%token <sval>   TK_STR              "str"
/* keywords */
%token          TK_PRINT            "p"
%token          TK_IF               "if"
%token          TK_ELIF             "elif"
%token          TK_ELSE             "else"
%token          TK_END              "end"
%token          TK_FUN              "fun"
%token          TK_RET              "ret"

%type <list>    stats
%type <node>    stat
%type <list>    comment
%type <node>    assign_stmt
%type <node>    if_stmt
%type <node>    elifs
%type <node>    fun_stmt
%type <node>    call_stmt
%type <node>    expr
%type <list>    exprs
%type <list>    args
%type <node>    id
%type <node>    lcmnt

%destructor { delete $$; } "id"
%destructor { delete $$; } "lcmnt"
%destructor { delete $$; } stats
%destructor { delete $$; } stat
/* TODO たのトークンも追加する */

%left '+' '-';
%left '*' '/';
%left NEG;
%%
%start unit;

unit    : stats                         { driver.set_ast($1); }
        ;

stats   : '\n'                          { $$ = new clist(OP_STATS); }
        | stat                          { $$ = new clist(OP_STATS, $1); }
        | stats '\n'                    { $$ = $1; }
        | stats stat                    { $1->add($2); $$ = $1; }
        ;

stat    : comment                       { $$ = $1; }
        | assign_stmt '\n'              { $$ = $1; }
        | if_stmt '\n'                  { $$ = $1; }
        | fun_stmt '\n'                 { $$ = $1; }
        | call_stmt '\n'                { $$ = $1; }
        ;

comment     : lcmnt '\n'                { $$ = new clist(OP_MCOMMENT, $1); }
            | comment lcmnt '\n'        { $1->add($2); $$ = $1; }
            ;

assign_stmt : id '=' expr                   { $$ = new cnode(OP_ASSIGN, $1, $3); }
            ;

if_stmt     : "if" expr '\n' stats elifs "end"  { $$ = new cnode(OP_IF, $2, new cnode(OP_NODE, $4, $5)); }
            ;

elifs       : %empty                        { $$ = nullptr; }
            | "elif" expr '\n' stats elifs  { $$ = new cnode(OP_ELIF, $2, new cnode(OP_NODE, $4, $5)); }
            | "else" '\n' stats             { $$ = new cnode(OP_ELSE, $3); }
            ;

fun_stmt    : "fun" id args '\n' stats "end"    { $$ = new cnode(OP_FUN, $2, new cnode(OP_NODE, $3, $5)); }
            ;

args        : %empty                        { $$ = new clist(OP_ARGS); }
            | id                            { $$ = new clist(OP_ARGS, $1); }
            | args ',' id                   { $1->add($3); $$ = $1; }
            ;

call_stmt   : id exprs                      { $$ = new cnode(OP_CALL, $1, $2); }
            ;

exprs       : %empty                        { $$ = new clist(OP_EXPRS); }
            | expr                          { $$ = new clist(OP_EXPRS, $1); }
            | exprs ',' expr                { $1->add($3); $$ = $1; }
            ;

expr        : expr '-' expr                 { $$ = new cnode(OP_MINUS, $1, $3); }
            | expr '+' expr                 { $$ = new cnode(OP_PLUS, $1, $3); }
            | expr '*' expr                 { $$ = new cnode(OP_TIMES, $1, $3); }
            | expr '/' expr                 { $$ = new cnode(OP_DIVIDE, $1, $3); }
            | '-' expr %prec NEG            { $$ = new cnode(OP_NEG, $2); }
            | '(' expr ')'                  { $$ = $2; }
            | id                            { $$ = $1; }
            | "int"                         { $$ = new cleaf(OP_INT, $1); }
            | "str"                         { $$ = new cleaf(OP_STR, $1); }
            ;

/* 末端 */
id      : "id"                          { $$ = new cleaf(OP_ID, $1); }
        ;
lcmnt   : "lcmnt"                       { $$ = new cleaf(OP_LCOMMENT, $1); }
        ;

%%
// wrote by hirok
/*
void yy::parser::error(const yy::parser::location_type&, const std::string& m)
{
    driver.error(m);
}
*/

void yy::parser::error(const std::string & m)
{
    driver.error(m.c_str());
}

