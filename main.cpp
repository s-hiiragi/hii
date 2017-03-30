#include "hii_driver.h"
#include <cstdio>

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

int main(int argc, char *argv[])
{
    if (argc <= 1) {
        my_fprintf(stderr, "usage: %s {file} ...\n", argv[0]);
        return 1;
    }

    for (++argv; argv[0]; ++argv) {
        hii_driver driver;
        driver.exec(*argv);
    }
    return 0;
}

