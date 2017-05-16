#ifndef CSCOPE_H_
#define CSCOPE_H_

#include <string>
#include <map>
#include <iostream> // for DEBUG
#include "cnode.h"
#include "cleaf.h"

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
    virtual ~cscope() {
        for (auto & e : vars_) {
            if (e.second != nullptr) {
                delete e.second;
            }
        }
    }

    void add_var(std::string const & name, cleaf * value) {
        vars_[name] = value;
    }

    bool has_var(std::string const & name) {
        return vars_.find(name) != vars_.end();
    }

    cleaf get_var(std::string const & name) const {
        return *vars_.at(name);
    }

    void add_fun(std::string const & name, cnode const * fun_node) {
        funs_[name] = fun_node;
    }

    bool has_fun(std::string const & name) {
        return funs_.find(name) != funs_.end();
    }

    cnode const * get_fun(std::string const & name) const {
        return funs_.at(name);
    }

    // for debug
    void print() const {
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
            for (auto const & e : vars_) {
                cout << e.first << " ";
            }
        }
        cout << endl;

        cout << "  funs: ";
        if (funs_.size() >= 1) {
            for (auto const & e : funs_) {
                cout << e.first << " ";
            }
        }
        cout << endl;

        cout << "  --" << endl;
    }

  private:
    // 保存するもの
    // 変数: exprノード
    // 関数: funノード(仮) ... 名前解決した情報を登録する必要がある?
    std::map<std::string, cleaf *> vars_;
    std::map<std::string, cnode const *> funs_;
};

#endif //CSCOPE_H_

