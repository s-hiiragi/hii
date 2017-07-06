#pragma once

#include <string>
#include <map>
#include <iostream> // for DEBUG
#include <utility>
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

    void add_var(std::string const &name, cvalue const &value, bool writable)
    {
        vars_[name] = std::make_pair(value, writable);
    }

    void add_fun(std::string const &name, cnode const *fun_node)
    {
        funs_[name] = fun_node;
    }

    bool has_var(std::string const &name)
    {
        return vars_.find(name) != vars_.end();
    }

    bool is_writable(std::string const &name)
    {
        return vars_.find(name) != vars_.end() && vars_[name].second == true;
    }

    bool has_fun(std::string const &name)
    {
        return funs_.find(name) != funs_.end();
    }

    cvalue & get_var(std::string const &name)
    {
        return vars_.at(name).first;
    }

    cvalue const & get_var(std::string const &name) const
    {
        return vars_.at(name).first;
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
            for (auto &&e : vars_) {
                cout << (e.second.second ? "$" : "") << e.first << " ";
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
    // 変数: cvalue, 変数フラグ
    //   ids_とvars_にmapを分ける方法も考えられる
    // 関数: funノード(仮) ... 名前解決した情報を登録する必要がある?
    std::map<std::string, std::pair<cvalue, bool>> vars_;
    std::map<std::string, cnode const *> funs_;
};

