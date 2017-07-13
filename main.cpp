#include <cassert>
#include <cstdio>
#include <string>
#include <vector>
#include "hii_driver.h"
#include "test.h"
#include "clog.h"

using my::clog;
using std::string;
using std::vector;

bool has_suffix(const string &str, const string &suffix)
{
    return str.size() >= suffix.size() &&
        str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
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
        
        if (arg == "-nd") {
            clog::set_debug(false);
        }
        else {
            string fname = arg;
            if (!has_suffix(fname, ".hi")) {
                fname += ".hi";
            }
            vector<string> args(&argv[i+1], &argv[argc]);
            hii_driver driver;
            driver.exec(fname, args);
            break;
        }
    }
    return 0;
}

