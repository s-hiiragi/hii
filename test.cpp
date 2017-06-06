#include <iostream>
#include <cstdio>
#include "cnode.h"

using namespace std;

#define assert_eq(a, b)  assert_eq_((a), (b), #a " == " #b)
#define assert_ne(a, b)  assert_ne_((a), (b), #a " != " #b)

template <class T>
bool assert_eq_(T const &x, T const &y, char const *msg) {
    if (!(x == y)) {
        printf("%s : \e[01;31mNG\e[00m\n", msg);
        return false;
    } else {
        printf("%s : \e[01;32mOK\e[00m\n", msg);
    } 
    return true;
}

template <class T>
bool assert_ne_(T const &x, T const &y, char const *msg) {
    if (!(x != y)) {
        printf("%s : \e[01;31mNG\e[00m\n", msg);
        return false;
    } else {
        printf("%s : \e[01;32mOK\e[00m\n", msg);
    }
    return true;
}

int test_cnode_clone()
{
    cnode * n1 = new cnode();
    cnode * n2 = new cnode();
    cnode n3(OP_NODE, n1, n2);

    cnode n4 = n3;

    if (!assert_eq(n4.op(), n3.op())) return 1;
    if (!assert_eq(n4.group(), n3.group())) return 1;
    if (!assert_ne(n4.left(), n3.left())) return 1;
    if (!assert_ne(n4.right(), n3.right())) return 1;

    return 0;
}

int test(void)
{
    if (!test_cnode_clone()) return 1;
    return 0;
}

