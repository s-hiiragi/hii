%{
#include <cstdlib>
#include <cerrno>
#include <climits>
#include <string>
#include <cstring>
#include "parser_driver.h"
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

static unsigned int column = 1;

hii::parser_driver *yymy_driver = nullptr;  // for YY_INPUT

#define YY_INPUT(buf,result,max_size) \
    do { \
        if (yymy_driver->_has_input_string()) { \
            std::string s = yymy_driver->_read_string(max_size); \
            std::strncpy(buf, s.c_str(), max_size); \
            result = s.size(); \
        } else { \
            errno=0; \
            while ( (result = fread(buf, 1, max_size, yyin))==0 && ferror(yyin)) \
                { \
                if( errno != EINTR) \
                    { \
                    YY_FATAL_ERROR( "input in flex scanner failed" ); \
                    break; \
                    } \
                errno=0; \
                clearerr(yyin); \
                } \
        } \
    } while (0)

// TODO 改行トークンのcolumnが1にならない
#define YY_USER_ACTION  do { \
        if (0) { \
        printf("YY_USER_ACTION: yytext=%s, yylineno=%d, yyleng=%d, column=%u\n", \
            yytext[0] == '\n' ? "<NL>" : yytext, yylineno, yyleng, column); \
        } \
        yylloc->begin.line = yylloc->end.line = yylineno; \
        yylloc->begin.column = column; \
        yylloc->end.column = column + yyleng - 1; \
        yylloc->begin.filename = yylloc->end.filename = &yymy_driver->filename(); \
        column += yyleng; \
    } while (0);
%}

%option noyywrap nounput batch
%option never-interactive
%option noyy_scan_buffer
%option noyy_scan_bytes
%option noyy_scan_string
%option nounistd
%option yylineno
%option bison-bridge
%option bison-locations

var    \$[a-zA-Z_][a-zA-Z_0-9]*
id     [a-zA-Z_][a-zA-Z_0-9]*
int    [1-9][0-9]*
/*
以下のようにすれば-1を単一のトークンとして扱える
int    -?[1-9][0-9]*
*/
str    \"([^\\"]|\\.)*\"
/*"*/
blank   [ \t]
rcmnt   #\{\n(([^#][^\n]*|#[^}][^\n]*|#)?\n)*#\}
lcmnt   #[^\n]*

%%
%{
    typedef yy::parser::token token;

    std::string string_buffer;
%}

"if"     return token::TK_IF;
"elif"   return token::TK_ELIF;
"else"   return token::TK_ELSE;
"sw"     return token::TK_SW;
"case"   return token::TK_CASE;
"end"    return token::TK_END;
"fun"    return token::TK_FUN;
"ret"    return token::TK_RET;
"loop"   return token::TK_LOOP;
"for"    return token::TK_FOR;
"cont"   return token::TK_CONT;
"break"  return token::TK_BREAK;

":="     return token::TK_REASSIGN;
"++"     return token::TK_INC;
"--"     return token::TK_DEC;
"+="     return token::TK_PLUS_ASSIGN;
"-="     return token::TK_MINUS_ASSIGN;
"*="     return token::TK_TIMES_ASSIGN;
"/="     return token::TK_DIVIDE_ASSIGN;

"..."    return token::TK_TDOT;
".."     return token::TK_DDOT;
"=="     return token::TK_EQ;
"!="     return token::TK_NEQ;
"<="     return token::TK_LTEQ;
">="     return token::TK_GTEQ;
"<=>"    return token::TK_SPACESHIP;
"and"    return token::TK_AND;
"or"     return token::TK_OR;
[-+*/%=(),<>\[\]@?:.]  return yy::parser::token_type(yytext[0]);

\n              {
                    column = 1;
                    return yy::parser::token_type(yytext[0]);
                }

{blank}+        ;
{rcmnt}         {
                    yylval->sval = new std::string(yytext);
                    return token::TK_RCMNT;
                }
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
                    yylval->sval = new std::string(yytext, 1, std::strlen(yytext)-2);
                    return token::TK_STR;
                }
{var}           {
                    yylval->sval = new std::string(yytext, 1, std::strlen(yytext)-1);
                    return token::TK_VAR;
                }
{id}            {
                    yylval->sval = new std::string(yytext);
                    return token::TK_ID;
                }
.               driver.error("不正な文字です。: %s\n", yytext);

%%

