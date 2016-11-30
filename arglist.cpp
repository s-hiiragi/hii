#include <string>
#include "node.h"
#include "arglist.h"

using namespace std;

// 左リーフにIDノードを加える
// ノードは右側に伸びていく
// 左リーフ優先で深さ優先探索することで、IDノード一覧を列挙できる
arglist & arglist::concat(string *name)
{
    // EMPTYリーフを探す
    cnode *p = this;
    while (p->right() != nullptr) p = p->right();

    p->set_left(new cnode(OP_ID, name));
    p->set_right(new cnode(OP_EMPTY));
    p->set_op(OP_NODE);

    return *this;
}

