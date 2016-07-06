#include <string>
#include "node.h"
#include "exprlist.h"

using namespace std;

exprlist & exprlist::concat(cnode *expr)
{
    // EMPTYリーフを探す
    cnode *p = this;
    while (p->right() != nullptr) p = p->left();

    p->set_right(expr);
    p->set_left(new cnode(OP_EMPTY));
    p->set_op(OP_NODE);

    return *this;
}

