#pragma once

#include <string>
#include <map>
#include <iostream> // for DEBUG
#include <utility>
#include <stdexcept>
#include "cnode.h"
#include "cclass.h"
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
    cscope()
    {
        //std::cout << "cscope: construct" << std::endl;
    }

    virtual ~cscope()
    {
        //std::cout << "cscope: destruct" << std::endl;
    }

    void add_var(std::string const &name, cvalue const &value, bool writable)
    {
        //std::cout << "add_var: " << name << std::endl;
        vars_[name] = std::make_pair(value, writable);
    }

    void add_type(std::string const &name, cnode const *type_node)
    {
        //types_[name] = type_node;  // どんなデータを入れる?
        /*
         * 型の用途
         * - インスタンスの生成
         * 
         * 名前、クラス先頭からの位置、型、その他(値、ノードポインタ)
         * class classinfo {
         *   std::unordered_map<
         *     std::string, 
         *     std::pair<int, ...>
         *   > members_;
         * };
         */
        throw std::logic_error("not implemented");
    }

    bool has_var(std::string const &name) const
    {
        return vars_.find(name) != vars_.end();
    }

    bool is_writable(std::string const &name) const
    {
        return vars_.find(name) != vars_.end() && vars_.at(name).second == true;
    }

    // XXX writableでない変数を返せてしまうのでは？
    cvalue & get_var(std::string const &name)
    {
        return vars_.at(name).first;
    }

    cvalue const & get_var(std::string const &name) const
    {
        return vars_.at(name).first;
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

        cout << "  --" << endl;
    }

  private:
    // 保存するもの
    // 変数: cvalue, 変数フラグ
    //   ids_とvars_にmapを分ける方法も考えられる
    // 関数: funノード(仮) ... 名前解決した情報を登録する必要がある?
    std::map<std::string, std::pair<cvalue, bool>> vars_;
    std::map<std::string, cclass> classes_;
};

