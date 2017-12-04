#include <iostream>
#include <cassert>
#include <cstdio>
#include <string>
#include <vector>
#include <cstring>
#include "hii_driver.h"
#include "test.h"
#include "clog.h"

using my::clog;
using std::string;
using std::vector;
using std::cin;
using std::cout;

bool has_suffix(const string &str, const string &suffix)
{
    return str.size() >= suffix.size() &&
        str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

int repl()
{
    hii_driver driver;
    char line[256];

    clog::set_debug(false);

    while (true)
    {
        cout << "> ";
        cin.getline(line, sizeof(line - 1));
        size_t len = std::strlen(line);
        line[len] = '\n';
        line[len+1] = '\0';
        driver.exec_string(line);
    }
    return 0;
}

int interpret_file(string const &fname, vector<string> const &args)
{
    hii_driver driver;
    bool ret = driver.exec_file(fname, args);
    if (!ret) {
        return 1;
    }
    return 0;
}

int main(int argc, char *argv[])
{
    // test
    assert(test());

    if (argc <= 1) {
        std::fprintf(stderr, "usage: %s [-nd] {file} [{args}]\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        string arg = argv[i];

        if (arg == "--") {
            continue;
        }
        else if (arg == "-nd" || arg == "--nodebug") {
            clog::set_debug(false);
        }
        else if (arg == "-i" || arg == "--intaractive") {
            repl();
        }
        else {
            if (!has_suffix(arg, ".hi"))
                arg += ".hi";
            vector<string> fargs(&argv[i + 1], &argv[argc]);  // 残りの全引数を消費
            interpret_file(arg, fargs);
            break;
        }
    }
    return 0;
}

