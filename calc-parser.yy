%skeleton "lalr1.cc"

// wrote by hirok
%define parser_class_name {calc_parser}

%defines
%{
#include <string>
#include "node.h"
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

// %debug
%error-verbose
// Symbols.
%union
{
	int                 ival;
	std::string        *sval;
    cnode              *expr;
    std::string       *lcmnt;
}

%{
#include "calc-driver.h"
%}

%token          TK_EOF          0   "end of file"
/* literals */
%token <ival>   TK_IVAL             "ival"
%token <sval>   TK_IDENTIFIER       "id"
/* keywords */
%token          TK_PRINT            "p"
%token          TK_LIST             "l"
%token <lcmnt>  TK_LCOMMENT         "lcmnt"

%type <expr>		expr

%destructor { delete $$; } "id"
%destructor { delete $$; } expr
%destructor { delete $$; } "lcmnt"

%left '+' '-';
%left '*' '/';
%left NEG;
%%
%start unit;

unit	: state
		| unit state
		;

/* modified by hirok */
state	: '\n'                          { ; }
        | "lcmnt" '\n'                  { driver.lcomment($1); }
        | state2 '\n'                   { ; }
        | state2 "lcmnt" '\n'           { driver.lcomment($2); }
		;

state2	: "id" '=' expr              	{ driver.assign($1, $3); }
		| "p" expr  		    		{ driver.print($2); }
		| "l"   	    				{ driver.list(); }
        | "fn" 
		;

expr	: expr '-' expr					{ $$ = new cnode(OP_MINUS, $1, $3); }
		| expr '+' expr					{ $$ = new cnode(OP_PLUS, $1, $3); }
		| expr '*' expr					{ $$ = new cnode(OP_TIMES, $1, $3); }
		| expr '/' expr					{ $$ = new cnode(OP_DIVIDE, $1, $3); }
		| '-' expr %prec NEG			{ $$ = new cnode(OP_NEG, $2); }
		| '(' expr ')'					{ $$ = $2; }
		| "id"		        			{ $$ = new cnode(OP_VALUE, $1); }
		| "ival"						{ $$ = new cnode(OP_IVAL, $1); }
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

