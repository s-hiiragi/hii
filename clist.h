#ifndef NODELIST_H_
#define NODELIST_H_

#include "cnode.h"

/**
 * ノードリスト
 * 
 * 用途
 * - 文のリスト
 * - 仮引数のリスト
 * - 実引数のリスト
 * 
 * NOTE:
 * - 簡単のために、リストの子ノードはリストではなくOP_NODEとしている
 * 
 * ノード構成の例
 * clist * list = new clist(OP_STATS, node1);
 * list->add(node2);
 * list->add(node3);
 * 
 * clist op=OP_STATS
 *  |-- node1
 *  |-- cnode op=NODE
 *       |-- node2
 *       |-- cnode op=NODE
 *            |-- node3
 *            |-- nullptr
 * 
 */
class clist : public cnode
{
  public:
    clist(int op, cnode * node)
        : cnode(op, node, nullptr) {}

    int add(cnode * node) {

        // 追加先ノードを探す
        cnode * p = this;
        while (true) {
            if (p->left() == nullptr) {
                break;
            }
            if (p->right() == nullptr) {
                break;
            } else {
                p = p->right();
            }
        }

        // ノードを追加
        if (p->left() == nullptr) {
            p->set_left(node);

        } else if (p->right() == nullptr) {
            cnode * n = new cnode(OP_NODE, node, nullptr);
            p->set_right(n);
        }

        return 0;
    }

    // TODO リスト走査関数を定義する
};

#endif

