#pragma once

#include <string>
#include <map>
#include <iostream> // for DEBUG
#include "cnode.h"
#include "cvalue.h"

/*
 * スコープ内で定義された要素を管理
 * - 変数
 * - 関数
 * 
 * 用途
 * - 式で参照している変数/関数の名前解決
 */
class cscope
{
    /*
  private:
    struct varinfo
    {
        varinfo(cleaf *value, is_value_mutable = false)
            : value(value), is_value_mutable(is_value_mutable) {}

        cleaf *value;
        bool is_value_mutable;
    };
    */

  public:
    cscope() {}
    virtual ~cscope() {}

    // valueはnewして渡すこと (所有権を渡すこと)
    void add_var(std::string const &name, cvalue const &value)
    {
        vars_[name] = value;
    }

    void add_fun(std::string const &name, cnode const *fun_node)
    {
        funs_[name] = fun_node;
    }

    bool has_var(std::string const &name)
    {
        return vars_.find(name) != vars_.end();
    }

    bool has_fun(std::string const &name)
    {
        return funs_.find(name) != funs_.end();
    }

    cvalue & get_var(std::string const &name)
    {
        return vars_.at(name);
    }

    cvalue const & get_var(std::string const &name) const
    {
        return vars_.at(name);
    }

    cnode const * get_fun(std::string const &name) const
    {
        return funs_.at(name);
    }

    // for debug
    void print() const
    {
        using std::cout;
        using std::endl;

        cout << "D: put scope" << endl;

        cout << "  vars: ";
        if (vars_.size() >= 1) {
            // tmp
            if (vars_.size() >= 3) {
                cout << endl;
                cout << "0: key=" << ((*vars_.begin()).first) << endl;       // ==> c
                cout << "1: key=" << ((*(++vars_.begin())).first) << endl;   // ==> i
                cout << "2: key=" << ((*(++++vars_.begin())).first) << endl; // ==> (何も表示されない)
                cout << "    ";
            }
            for (auto &&e : vars_) {
                cout << e.first << " ";
            }
        }
        cout << endl;

        cout << "  funs: ";
        if (funs_.size() >= 1) {
            for (auto &&e : funs_) {
                cout << e.first << " ";
            }
        }
        cout << endl;

        cout << "  --" << endl;
    }

  private:
    // 保存するもの
    // 変数: cvalue
    // 関数: funノード(仮) ... 名前解決した情報を登録する必要がある?
    std::map<std::string, cvalue> vars_;
    std::map<std::string, cnode const *> funs_;
};

