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
 * 
 */
class nodelist
{
  public:
    typedef node_type list_type;

    nodelist(list_type t)
        : type_(t) {}

    nodelist(list_type t, )
        : type_(t) {}

    ~nodelist() {}

    nodelist * concat(nodelist * l);
  private:
    node_type type_;
};

#endif

