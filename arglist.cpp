#include <string>
#include "node.h"
#include "arglist.h"

using namespace std;

arglist & arglist::concat(string *name)
{
    // EMPTYリーフを探す
    cnode *p = this;
    while (p->right() != nullptr) p = p->left();

    p->set_right(new cnode(OP_ID, name));
    p->set_left(new cnode(OP_EMPTY));
    p->set_op(OP_NODE);

    return *this;
}

