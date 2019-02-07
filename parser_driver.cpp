#include <cassert>
#include <cstdio>
#include "parser.hh"
#include "parser_driver.h"
#include "cnode.h"
#include "clog.h"

using std::string;
using my::clog;
using std::cout;
using std::endl;

//extern FILE *yyin;
extern FILE *yyget_in ( void );
extern void yyset_in  ( FILE * _in_str  );
extern int yylex_destroy (void);
extern hii::parser_driver *yymy_driver;

namespace hii
{

int parser_driver::parse_file(const std::string &filename, cnode **ast)
{
    if (ast == nullptr) {
        return 1;
    }

    if (scan_begin(filename) != 0) {
        return 1;
    }

    yy::parser parser{*this};
#if YYDEBUG
    parser.set_debug_level(1);
#endif
    int result = parser.parse();
    scan_end();

    if (result != 0) {
        // パーサが失敗した
        if (ast_ != nullptr) {
            // ASTが構築済みなら解放する
            delete ast_;
            ast_ = nullptr;
        }
        return 1;
    }

    assert(ast_ != nullptr);
    *ast = ast_;
    ast_ = nullptr;

    return 0;
}

int parser_driver::parse_string(string const &code, cnode **ast)
{
    if (ast == nullptr) {
        return 1;
    }

    //if (scan_begin("__repl__.hi") != 0) {
    if (scan_begin("") != 0) {
	cout << "2 ここでエラー" << endl;
	return 1;
    }

    has_inputstr_ = true;
    inputstr_ = code;

    yy::parser parser(*this);
    int result = parser.parse();

    scan_end();

    has_inputstr_ = false;
    inputstr_ = "";

    if (result != 0) {
        if (ast_ != nullptr) {
            delete ast_;
        }
	cout << "3" << endl;
        return 1;
    }

    *ast = ast_;
    ast_ = nullptr;

    return 0;
}

string parser_driver::_read_string(size_t max_size)
{
    if (!_has_input_string()) {
        return "";
    }

    if (inputstr_ == "") {
        return "";
    }

    string ret(inputstr_, 0, max_size);
    inputstr_.erase(0, max_size);
    return ret;
}

void parser_driver::_set_ast(cnode *ast)
{
    assert(ast != nullptr);
    ast_ = ast;

    if (clog::is_debug()) {
        clog::d("set_ast");
        cnode::print(ast);
    }
}

int parser_driver::scan_begin(const string &filename)
{
    yyset_in(nullptr);
    if (filename != "") {
        FILE *fp = fopen(filename.c_str(), "r");
        if (fp == nullptr) {
            return 1;
        }
        yyset_in(fp);
    }

    yymy_driver = this;
    filename_ = filename;

    return 0;
}

void parser_driver::scan_end()
{
    if (yyget_in() != nullptr && yyget_in() != stdin) {
        //cout << "fclose: yyin=" << yyget_in() << endl;
        fclose(yyget_in());
    }

    yylex_destroy();

    yymy_driver = nullptr;
    filename_ = "";
}

} // end of hii namespace

