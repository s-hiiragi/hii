//#include <iostream>
#include "calc-driver.h"

int main(int argc, char *argv[])
{
    for (++argv; argv[0]; ++argv) {
        calc_driver driver;
        driver.calc(*argv);
    }
    return 0;
}

