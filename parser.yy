%skeleton "lalr1.cc"

%define parser_class_name {parser}

// parser.hhを生成するよう指定
%defines
%locations

%{
#include <string>
#include "cnode.h"
#include "cleaf.h"
#include "clist.h"

#ifdef _MSC_VER
#pragma warning(disable: 4800)
#pragma warning(disable: 4267)
#endif
%}
// The parsing context.
%parse-param { hii_driver& driver }
%lex-param   { hii_driver& driver }

%code requires { #include "cnode.h" }
%code requires { #include "clist.h" }
%code requires { class hii_driver; }

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
%token <sval>   TK_RCMNT            "rcmnt"
/* literals */
%token <sval>   TK_ID               "id"
%token <sval>   TK_VAR              "var"
%token <ival>   TK_INT              "int"
%token <sval>   TK_STR              "str"
/* keywords */
%token          TK_IF               "if"
%token          TK_ELIF             "elif"
%token          TK_ELSE             "else"
%token          TK_END              "end"
%token          TK_FUN              "fun"
%token          TK_RET              "ret"
%token          TK_LOOP             "loop"
%token          TK_CONT             "cont"
%token          TK_BREAK            "break"
%token          TK_REASSIGN         ":="
%token          TK_TDOT             "..."
%token          TK_DDOT             ".."
%token          TK_EQ               "=="
%token          TK_NEQ              "!="
%token          TK_LTEQ             "<="
%token          TK_GTEQ             ">="
%token          TK_AND              "and"
%token          TK_OR               "or"

%type <list>    stats
%type <node>    stat
%type <list>    comment
%type <node>    assign_stmt
%type <node>    reassign_stmt
%type <node>    lsassign_stmt
%type <node>    if_stmt
%type <node>    elifs
%type <node>    fun_stmt
%type <node>    ret_stmt
%type <node>    call_stmt
%type <node>    loop_stmt
%type <node>    cont_stmt
%type <node>    break_stmt
%type <node>    expr
%type <node>    slice_expr
%type <list>    exprs
%type <list>    some_exprs
%type <list>    args
%type <node>    id
%type <node>    var
%type <node>    idvar
%type <node>    lcmnt
%type <node>    rcmnt
/*
%type <list>    attrs
*/

%destructor { delete $$; } "id"
%destructor { delete $$; } "var"
%destructor { delete $$; } "str"
%destructor { delete $$; } "lcmnt"
%destructor { delete $$; } "rcmnt"
%destructor { delete $$; } stats
%destructor { delete $$; } stat
%destructor { delete $$; } comment
%destructor { delete $$; } assign_stmt
%destructor { delete $$; } reassign_stmt
%destructor { delete $$; } lsassign_stmt
%destructor { delete $$; } if_stmt
%destructor { delete $$; } elifs
%destructor { delete $$; } fun_stmt
%destructor { delete $$; } ret_stmt
%destructor { delete $$; } call_stmt
%destructor { delete $$; } loop_stmt
%destructor { delete $$; } cont_stmt
%destructor { delete $$; } break_stmt
%destructor { delete $$; } expr
%destructor { delete $$; } slice_expr
%destructor { delete $$; } exprs
%destructor { delete $$; } some_exprs
%destructor { delete $$; } args
%destructor { delete $$; } id
%destructor { delete $$; } var
%destructor { delete $$; } idvar
%destructor { delete $$; } lcmnt

%left "or";
%left "and";
%left "==" "!=" '<' "<=" '>' ">=";
%left '+' '-';
%left '*' '/' '%';
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
        | rcmnt '\n'                    { $$ = $1; }
        | assign_stmt '\n'              { $$ = $1; }
        | reassign_stmt '\n'            { $$ = $1; }
        | lsassign_stmt '\n'            { $$ = $1; }
        | if_stmt '\n'                  { $$ = $1; }
        | fun_stmt '\n'                 { $$ = $1; }
        | ret_stmt '\n'                 { $$ = $1; }
        | call_stmt '\n'                { $$ = $1; }
        | loop_stmt '\n'                { $$ = $1; }
        | cont_stmt '\n'                { $$ = $1; }
        | break_stmt '\n'               { $$ = $1; }
        ;

comment     : lcmnt '\n'                { $$ = new clist(OP_MCOMMENT, $1); }
            | comment lcmnt '\n'        { $1->add($2); $$ = $1; }
            ;

assign_stmt : idvar '=' expr            { $$ = new cnode(OP_ASSIGN, $1, $3); }
            ;

reassign_stmt : var ":=" expr           { $$ = new cnode(OP_REASSIGN, $1, $3); }
              ;

lsassign_stmt : var '[' expr ']' '=' expr { $$ = new cnode(OP_LSASSIGN, $1, new cnode(OP_NODE, $3, $6)); }

if_stmt     : "if" expr '\n' stats elifs "end"  { $$ = new cnode(OP_IF, $2, new cnode(OP_NODE, $4, $5)); }
            ;

elifs       : %empty                        { $$ = nullptr; }
            | "elif" expr '\n' stats elifs  { $$ = new cnode(OP_ELIF, $2, new cnode(OP_NODE, $4, $5)); }
            | "else" '\n' stats             { $$ = new cnode(OP_ELSE, $3); }
            ;

/*
fun_stmt    : "fun" id args '\n' stats "end"           { $$ = new cnode(OP_FUN, $2, new cnode(OP_NODE, $3, $5)); }
            ;
*/
fun_stmt    : "fun" id args '\n' stats "end"               { $$ = new cnode(OP_FUN, $2, new cnode(OP_NODE, $3, new cnode(OP_NODE, new clist(OP_ATTRS), $5))); }
            | "fun" id args ',' "..." id '\n' stats "end"  { $$ = new cnode(OP_FUN, $2, new cnode(OP_NODE, $3, 
                                                                new cnode(OP_NODE, &(new clist(OP_ATTRS, new cleaf(OP_ID, "variadic")))->add($6), $8))); }
            ;

ret_stmt    : "ret"                         { $$ = new cnode(OP_RET); }
            | "ret" expr                    { $$ = new cnode(OP_RET, $2); }
            ;

args        : %empty                        { $$ = new clist(OP_ARGS); }
            | id                            { $$ = new clist(OP_ARGS, $1); }
            | args ',' id                   { $1->add($3); $$ = $1; }
            ;

call_stmt   : id exprs                      { $$ = new cnode(OP_CALL, $1, $2); }
            ;

loop_stmt   : "loop" expr '\n' stats "end"                  { $$ = new cnode(OP_LOOP, nullptr, new cnode(OP_NODE, $2, new cnode(OP_NODE, nullptr, $4))); }
            | "loop" expr ".." expr '\n' stats "end"        { $$ = new cnode(OP_LOOP, nullptr, new cnode(OP_NODE, $2, new cnode(OP_NODE, $4,      $6))); }
            | "loop" id ',' expr '\n' stats "end"           { $$ = new cnode(OP_LOOP, $2,      new cnode(OP_NODE, $4, new cnode(OP_NODE, nullptr, $6))); }
            | "loop" id ',' expr ".." expr '\n' stats "end" { $$ = new cnode(OP_LOOP, $2,      new cnode(OP_NODE, $4, new cnode(OP_NODE, $6,      $8))); }
            ;

cont_stmt   : "cont"                        { $$ = new cnode(OP_CONT, nullptr); }
            | "cont" expr                   { $$ = new cnode(OP_CONT, $2); }
            ;

break_stmt  : "break"                       { $$ = new cnode(OP_BREAK, nullptr); }
            | "break" expr                  { $$ = new cnode(OP_BREAK, $2); }
            ;

exprs       : %empty                        { $$ = new clist(OP_EXPRS); }
            | expr                          { $$ = new clist(OP_EXPRS, $1); }
            | exprs ',' expr                { $1->add($3); $$ = $1; }
            ;

some_exprs  : expr                          { $$ = new clist(OP_EXPRS, $1); }
            | some_exprs ',' expr           { $1->add($3); $$ = $1; }
            ;

expr        : expr '-' expr                 { $$ = new cnode(OP_MINUS, $1, $3); }
            | expr '+' expr                 { $$ = new cnode(OP_PLUS, $1, $3); }
            | expr '*' expr                 { $$ = new cnode(OP_TIMES, $1, $3); }
            | expr '/' expr                 { $$ = new cnode(OP_DIVIDE, $1, $3); }
            | expr '%' expr                 { $$ = new cnode(OP_MODULO, $1, $3); }
            | '-' expr %prec NEG            { $$ = new cnode(OP_NEG, $2); }
            | expr "==" expr                { $$ = new cnode(OP_EQ, $1, $3); }
            | expr "!=" expr                { $$ = new cnode(OP_NEQ, $1, $3); }
            | expr '<' expr                 { $$ = new cnode(OP_LT, $1, $3); }
            | expr "<=" expr                { $$ = new cnode(OP_LTEQ, $1, $3); }
            | expr '>' expr                 { $$ = new cnode(OP_GT, $1, $3); }
            | expr ">=" expr                { $$ = new cnode(OP_GTEQ, $1, $3); }
            | expr "and" expr               { $$ = new cnode(OP_AND, $1, $3); }
            | expr "or" expr                { $$ = new cnode(OP_OR, $1, $3); }
            | '(' expr ')'                  { $$ = $2; }
            | '[' exprs ']'                 { $$ = new cnode(OP_ARRAY, $2); }
            | expr '[' expr ']'             { $$ = new cnode(OP_ELEMENT, $1, $3); }
            | slice_expr                    { $$ = $1; }
            | id some_exprs                 { $$ = new cnode(OP_CALLEXPR, $1, $2); }
            | idvar                         { $$ = $1; }
            | "int"                         { $$ = new cleaf(OP_INT, $1); }
            | "str"                         { $$ = new cleaf(OP_STR, $1); }
            ;

slice_expr  : expr '[' ':' expr ']'         { $$ = new cnode(OP_SLICE, $1, new cnode(OP_NODE, nullptr, $4)); }
            | expr '[' expr ':' ']'         { $$ = new cnode(OP_SLICE, $1, new cnode(OP_NODE, $3, nullptr)); }
            | expr '[' expr ':' expr ']'    { $$ = new cnode(OP_SLICE, $1, new cnode(OP_NODE, $3, $5)); }
            ;

/*
x - 1   ... 減算 or 関数xの呼び出し -> 減算を優先して欲しい
x [ 1 ] ... 要素の参照 or 関数xの呼び出し -> 要素の参照を優先して欲しい
*/

/* 末端 */
idvar   : id                            { $$ = $1; }
        | var                           { $$ = $1; }
        ;

id      : "id"                          { $$ = new cleaf(OP_ID, $1); }
        ;

var     : "var"                         { $$ = new cleaf(OP_VAR, $1); }
        ;

lcmnt   : "lcmnt"                       { $$ = new cleaf(OP_LCOMMENT, $1); }
        ;

rcmnt   : "rcmnt"                       { $$ = new cleaf(OP_RCOMMENT, $1); }
        ;

/* TODO id : "id" attrs, lcmnt : "lcmnt" attrs みたいに自由に属性を付けたい*/
/*
attrs   : %empty                        { $$ = new clist(OP_ATTRS); }
        | '?'                           { $$ = new clist(OP_ATTRS, new cleaf(OP_ID, "optional")); }
        | '@' "id"                      { $$ = new clist(OP_ATTRS, $2); }
        | attrs '@' "id"                { $1->add($3); $$ = $1; }
        ;
*/

%%

void yy::parser::error(const yy::parser::location_type &l, const std::string &m)
{
    driver.error("%p:%u:%u: error: %s", l.begin.filename, l.begin.line, l.begin.column, m.c_str());
    //driver.error("%s:%u:%u: error: %s", l.begin.filename->c_str(), l.begin.line, l.begin.column, m.c_str());
}

/*
void yy::parser::error(const std::string & m)
{
    driver.error(m.c_str());
}
*/

