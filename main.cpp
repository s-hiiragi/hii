#include <iostream>
#include <cassert>
#include <cstdio>
#include <string>
#include <vector>
#include <cstring>
#include "parser_driver.h"
#include "hii_driver.h"
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

    clog::set_debug(false);

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
            cout << "-- break (EOF)" << endl;
            break;
        }
        if (cin.fail()) cin.clear();

        string code = line;

        cnode *ast = nullptr;


        hii::parser_driver pd;
        //cout << "-- code" << endl;
        //cout << code << endl;
        //cout << "--" << endl;
        cout << "-- parse_string " << endl;
        ret = pd.parse_string(code, &ast);
        if (ret != 0) {
            cout << "E: parse error" << endl;
            continue;
        }
        cout << "-- eval_ast " << endl;
        hii_driver driver;
        ret = driver.eval_ast(ast, {});
        if (ret != 0) {
            cout << "E: eval error" << endl;
            delete ast;
            continue;
        }

        delete ast;
    }

    return 0;
}

int interpret_file(string const &fname, vector<string> const &args)
{
    int ret;
    cnode *ast = nullptr;

    cout << "[D] parse_file" << endl;

    hii::parser_driver pd;

    ret = pd.parse_file(fname, &ast);
    if (ret != 0) {
        return 1;
    }

    cout << "[D] eval_ast" << endl;

    hii_driver driver;

    ret = driver.eval_ast(ast, args);
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
            break;
        }
    }
    return 0;
}

