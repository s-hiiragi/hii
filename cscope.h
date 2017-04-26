#pragma once

#include <string>
#include <map>
#include <iostream> // for DEBUG

class cnode;

/*
 * スコープ内で定義された要素を管理
 * - 変数
 * - 関数
 * 
 * 用途
 * - 式で参照している変数/関数の名前解決
 * - a
 */
class cscope
{
  public:
    cscope() {}
    virtual ~cscope() {}

    cnode const * & at(std::string const & name) {
        std::cout << "cscope.at: " << name << std::endl;
        return items_.at(name);
    }

    cnode const * & operator[](std::string const & name) {
        return items_[name];
    }

  private:
    // 保存するもの
    // 変数: exprノード
    // 関数: statsノード
    std::map<std::string, cnode const *> items_;
};

