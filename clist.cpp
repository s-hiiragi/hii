#include <functional>
#include "cnode.h"
#include "clist.h"

/**
 * リストに要素を追加
 * 
 * 空のリスト
 * list
 *  |-- nullptr
 *  |-- nullptr
 * 要素1のリスト
 * list
 *  |-- item
 *  |-- nullptr
 * 要素2のリスト
 * list
 *  |-- item1
 *  |-- node
 *       |-- item2
 *       |-- nullptr
 * 要素3のリスト
 * list
 *  |-- item1
 *  |-- node
 *       |-- item2
 *       |-- node
 *            |-- item3
 *            |-- nullptr
 * つまり、
 * - left == nullptr なら空のリスト
 * - left != nullptr ならrightを末端まで辿るとright == nullptr
 */
clist & clist::add(cnode * node)
{
    if (this->left() == nullptr) {
        // リストが空の場合
        this->set_left(node);
    }
    else {
        // リストが空でない場合

        // 末端のノードを探す
        cnode * p = this;
        while (p->right() != nullptr) p = p->right();

        // 要素を追加
        cnode * n = new cnode(OP_LISTITEM, node, nullptr);
        p->set_right(n);
    }
    return *this;
}

bool clist::each(std::function<bool(cnode &)> const & fn)
{
    if (this->left() == nullptr)
        return true;

    bool ret = true;
    cnode *p = this;
    do {
        ret = fn(*p->left());
        if (!ret) break;
        p = p->right();
    }
    while (p != nullptr);

    return ret;
}

bool clist::each(std::function<bool(cnode const &)> const & fn) const
{
    if (this->left() == nullptr)
        return true;

    bool ret = true;
    cnode const * p = this;
    do {
        ret = fn(*p->left());
        if (!ret) break;
        p = p->right();
    }
    while (p != nullptr);
}

