#pragma once

#include <functional>
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
    clist(int op)
        : cnode(op, nullptr, nullptr) { group_ = NG_LIST; }

    clist(int op, cnode *node)
        : cnode(op, node, nullptr) { group_ = NG_LIST; }

    clist(clist const &obj)
        : cnode(obj) { group_ = NG_LIST; }

    clist & add(cnode *node);

    bool each(std::function<bool(cnode &)> const &fn);
    bool each(std::function<bool(cnode const &)> const &fn) const;

    //bool each(std::function<bool(cnode &, int)> const &fn);
    //bool each(std::function<bool(cnode const &, int)> const &fn) const;
};

