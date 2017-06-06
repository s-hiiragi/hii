%{
#include <cstdlib>
#include <cerrno>
#include <climits>
#include <string>
#include <cstring>
#include "hii_driver.h"
#include "parser.hh"

#ifdef _MSC_VER
#pragma warning(disable:4018)
#pragma warning(disable:4102)
#pragma warning(disable:4244)
#pragma warning(disable:4267)
#pragma warning(disable:4996)
#endif

#undef yywrap
#define yywrap() 1

#define yyterminate()   return token::TK_EOF
%}

%option noyywrap nounput batch
%option never-interactive
%option noyy_scan_buffer
%option noyy_scan_bytes
%option noyy_scan_string
%option nounistd

id     [a-zA-Z_][a-zA-Z_0-9]*
int    [1-9][0-9]*
str    \"([^\\"]|\\.)*\"
/*"*/
blank  [ \t]
lcmnt  #[^\n]*

%%
%{
    typedef yy::parser::token token;

    std::string string_buffer;
%}

"if"                return token::TK_IF;
"elif"              return token::TK_ELIF;
"else"              return token::TK_ELSE;
"end"               return token::TK_END;
"fun"               return token::TK_FUN;
"ret"               return token::TK_RET;
"loop"              return token::TK_LOOP;

".."                return token::TK_DDOT;
"=="                return token::TK_EQ;
"!="                return token::TK_NEQ;
"<="                return token::TK_LTEQ;
">="                return token::TK_GTEQ;
"and"               return token::TK_AND;
"or"                return token::TK_OR;
[-+*/%=()\n,<>\[\]] return yy::parser::token_type(yytext[0]);

{blank}+        ;
{lcmnt}         {
                    yylval->sval = new std::string(yytext);
                    return token::TK_LCMNT;
                }
{int}           {
                    errno = 0;
                    long n = strtol(yytext, NULL, 10);
                    if (n < LONG_MIN || n > LONG_MAX || errno == ERANGE)
                        driver.error("整数が範囲外です。: %s\n", yytext);
                    yylval->ival = n;
                    return token::TK_INT;
                }
"0"             {
                    yylval->ival = 0;
                    return token::TK_INT;
                }
{str}           {
                    // TODO ""を除去する
                    yylval->sval = new std::string(yytext, 1, std::strlen(yytext)-2);
                    return token::TK_STR;
                }
{id}            {
                    yylval->sval = new std::string(yytext);
                    return token::TK_ID;
                }
.               driver.error("不正な文字です。: %s\n", yytext);

%%

void hii_driver::scan_begin()
{
    if ((yyin = fopen(file_.c_str(), "r")) == 0)
        this->error("%s がオープンできません。\n", file_);
}

void hii_driver::scan_end()
{
    fclose(yyin);
    yylex_destroy();
}

