#ifndef NODELIST_H_
#define NODELIST_H_

#include "node.h"

/**
 * ノードリスト
 * 
 * 用途
 * - 文のリスト
 * - 仮引数のリスト
 * - 実引数のリスト
 * 
 * NOTE
 * 型安全のためにnode_typeをクラスにして
 * nodelist<T>コレクションにしたい
 * 
 * TODO nodeを継承、Lispのようなリストにする？
 * 
 */
class clist
{
  public:
    typedef cnode::type list_type;

    clist(list_type t)
        : type_(t) {}

    ~clist() {}

    clist * concat(clist * l);
  private:
    list_type type_;
};

#endif

