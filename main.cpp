#include "hii_driver.h"
#include <cstdio>
#include <string>
#include "test.h"

template<typename... Args>
void my_printf(char const * const format,
        Args const & ... args) noexcept
{
    std::printf(format, args ...);
}

template<typename... Args>
void my_fprintf(std::FILE * fp, char const * const format,
        Args const & ... args) noexcept
{
    std::fprintf(fp, format, args ...);
}

bool has_suffix(const std::string &str, const std::string &suffix)
{
    return str.size() >= suffix.size() &&
        str.compare(str.size() - suffix.size(), suffix.size(), suffix) == 0;
}

int main(int argc, char *argv[])
{
    // test
    if (!test()) return 1;

    if (argc <= 1) {
        my_fprintf(stderr, "usage: %s {file} ...\n", argv[0]);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        std::string fname = argv[i];

        if (!has_suffix(fname, ".hi")) {
            fname += ".hi";
        }

        hii_driver driver;
        driver.exec(fname);
    }
    return 0;
}

