#pragma once

#include "cnode.h"

/**
 * ノードリスト
 * 
 * 用途
 * - 文のリスト
 * - 仮引数のリスト
 * - 実引数のリスト
 * 
 * ノード構成の例
 * clist * list = new clist(OP_STATS, node1);
 * list->add(node2);
 * list->add(node3);
 * 
 * clist op=OP_STATS
 *  |-- node1
 *  |-- cnode op=OP_LISTITEM
 *       |-- node2
 *       |-- cnode op=OP_LISTITEM
 *            |-- node3
 *            |-- nullptr
 * 
 */
class clist : public cnode
{
  public:
    template<class T>
    void each(T const & fn)
    {
        if (this->left() == nullptr) return;

        cnode * p = this;
        do {
            fn(*p->left());
            p = p->right();
        }
        while (p != nullptr);
    }

    template<class T>
    void each(T const & fn) const
    {
        if (this->left() == nullptr) return;

        cnode const * p = this;
        do {
            fn(*p->left());
            p = p->right();
        }
        while (p != nullptr);
    }

    clist(int op)
        : cnode(op, nullptr, nullptr) { group_ = NG_LIST; }

    clist(int op, cnode * node)
        : cnode(op, node, nullptr) { group_ = NG_LIST; }

    int add(cnode * node);
};

