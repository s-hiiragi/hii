#ifndef CCLASS_H
#define CCLASS_H

#include <string>
#include <unordered_map>
#include <utility>
#include "cvalue.h"

class cclass
{
  public:
    struct cmember
    {
        int index;
        std::string typename_;
        cvalue initial_value;
        cnode const * initial_func_value;  // XXX initial_valueに統合したい
    };

    /*
    cmember const & member(int index) const
    {
        for (auto 
    }
    */

    cclass(){}

  private:
    // クラスメンバ情報
    // メンバ名 : pair<メンバのインデックス, 型名>
    // インデックスよりもメンバ名の方が引く頻度が高いと思うためメンバ名をキーとする
    std::unordered_map<std::string, std::pair<int, std::string>> fields_;
};

#endif //CCLASS_H

