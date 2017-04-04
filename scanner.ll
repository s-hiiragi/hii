%{
#include <cstdlib>
#include <cerrno>
#include <climits>
#include <string>
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

id    [a-zA-Z_][a-zA-Z_0-9]*
int   [1-9][0-9]*
blank [ \t]
lcmnt #[^\n]*

%%
%{
    typedef yy::parser::token token;

    std::string string_buffer;
%}

"if"        return token::TK_IF;
"elif"      return token::TK_ELIF;
"else"      return token::TK_ELSE;
"end"       return token::TK_END;
"fun"       return token::TK_FUN;
"ret"       return token::TK_RET;

[-+*/=()\n,]    return yy::parser::token_type(yytext[0]);

{blank}+        ;
{int}           {
                    errno = 0;
                    long n = strtol(yytext, NULL, 10);
                    if (n < LONG_MIN || n > LONG_MAX || errno == ERANGE)
                        driver.error("整数が範囲外です。");
                    yylval->ival = n;
                    return token::TK_INT;
                }
"0"             {
                    yylval->ival = 0;
                    return token::TK_INT;
                }
{id}            {
                    yylval->sval = new std::string(yytext);
                    return token::TK_ID;
                }
{lcmnt}         {
                    yylval->sval = new std::string(yytext);
                    return token::TK_LCMNT;
                }
.               driver.error("この文字を識別子で使用することはできません。", yytext);

%%

void hii_driver::scan_begin()
{
    if ((yyin = fopen(file_.c_str(), "r")) == 0)
        error(file_ + " がオープンできません。");
}

void hii_driver::scan_end()
{
    fclose(yyin);
    yylex_destroy();
}

