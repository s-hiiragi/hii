#include <iostream>
#include <cassert>
#include <cstdio>
#include <string>
#include <vector>
#include <cstring>
#include "parser_driver.h"
#include "hii_driver.h"
#include "cscope.h"
#include "test.h"
#include "clog.h"

using my::clog;
using std::string;
using std::vector;
using std::cin;
using std::cout;
using std::endl;

bool has_suffix(const string &str, const string &suffix)
{
    return str.size() >= suffix.size() &&
        str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

int repl()
{
    int ret;
    char line[256];
    std::vector<cscope> scopes;

    //clog::set_debug(false);

    while (true)
    {
        cout << "> ";
        cin.getline(line, sizeof(line - 1));  // XXX std::getline(cin, string)は使えないか?

        size_t len = std::strlen(line);
        if (len > 0) {
            if (line[len - 1] != '\n') {
                line[len] = '\n';
                line[len + 1] = '\0';
                len++;
            }
        }
        if (cin.eof()) {
            clog::d("-- break (EOF)");
            break;
        }
        if (cin.fail()) cin.clear();

        const string code = line;

        cnode *ast = nullptr;  // constにできないのか?

        hii::parser_driver pd;
        //cout << "-- code" << endl;
        //cout << code << endl;
        //cout << "--" << endl;
        clog::d("-- parse_string");
        ret = pd.parse_string(code, &ast);
        if (ret != 0) {
            clog::e("parse error");
            continue;
        }

        //cout << "-- print ast" << endl;
        //cnode::print(ast);
        //cout << "ast->op: " << ast->op() << endl;
        //cout << "ast->left: " << ast->left() << endl;
        //cout << "ast->right: " << ast->right() << endl;
        if (ast->op() == OP_STATS && ast->left() != nullptr && ast->right() == nullptr) {
            //cout << "expand stats" << endl;
            cnode *p = ast->release_left();
            delete ast;
            ast = p;
        }

        clog::d("-- eval_ast");
        //cout << "scopes: size=" << scopes.size() << endl;
        //if (scopes.size() >= 1) {
        //    scopes[0].print();
        //}
        hii_driver driver;
        ret = driver.eval_ast(ast, {}, scopes); // writableなscopeを受け取るようにする
        if (ret != 0) {
            clog::e("eval error");
            delete ast;
            continue;
        }
        //cout << "scopes: size=" << scopes.size() << endl;
        //if (scopes.size() >= 1) {
        //    scopes[0].print();
        //}

        delete ast;
    }

    return 0;
}

int interpret_file(string const &fname, vector<string> const &args)
{
    int ret;
    cnode *ast = nullptr;

    clog::d("parse file: %s", fname.c_str());

    hii::parser_driver pd;

    ret = pd.parse_file(fname, &ast);
    if (ret != 0) {
        return 1;
    }

    clog::d("eval_ast");

    hii_driver driver;
    std::vector<cscope> scopes;

    ret = driver.eval_ast(ast, args, scopes);
    if (ret != 0) {
        delete ast;
        return 2;
    }

    delete ast;
    return 0;
}

int main(int argc, char *argv[])
{
    // test
    //assert(test());

    if (argc <= 1) {
        std::fprintf(stderr, "usage: %s [-nd] {file} [{args}]\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];

        if (arg == "--") {
            continue;
        }
        else if (arg == "-d" || arg == "--debug") {
            clog::set_debug(true);
        }
        else if (arg == "-nd" || arg == "--nodebug") {
            clog::set_debug(false);
        }
        else if (arg == "-i" || arg == "--intaractive") {
            repl();
        }
        else {
            if (!has_suffix(arg, ".hi")) {
                arg += ".hi";
            }
            vector<string> fargs(&argv[i + 1], &argv[argc]);  // 残りの全引数を消費

            int ret = interpret_file(arg, fargs);
            if (ret != 0) {
                cout << "[E]: interpret_file failed (" << ret << ")" << endl;
            }
            return ret;
        }
    }
    return 0;
}

