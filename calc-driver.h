#ifndef CALC_DRIVER_H__
#define CALC_DRIVER_H__

#include <string>
#include <map>
#include "calc-parser.hh"
#include "cnode.h"
#include "exprlist.h"
#include "arglist.h"
#include "cfn.h"

// Forward declarations.
class calc_driver;

#define YY_DECL                                         \
    yy::calc_parser::token_type                         \
    yylex(yy::calc_parser::semantic_type* yylval,       \
         calc_driver& driver)

YY_DECL;

class calc_driver {
  public:
    calc_driver();
    virtual ~calc_driver();

    std::string &get_filename() { return file_; }

    void set_ast(cnode *ast);
    
    bool calc(const std::string &f);

    // 変数の値を取得
    int value(const std::string & name)
    {
        // 未定義チェックを行う
        return values_[name];
    }
    
    // 構文にマッチした時のアクション
    void lcmnt(const std::string *text);
    void assign(const std::string *name, cnode *expr);
    void print(cnode *node);
    void listvars();
    void call_state(const std::string *name, exprlist *exprs);

    void if_state(cnode *expr);
    void elseif_state(cnode *expr);
    void else_state();
    void end_state();
    void loop_state(cnode *expr = nullptr);
    void declfn(const std::string *name, arglist *args);
    void ret(cnode *expr = nullptr);

    // Error handling.
    void error(const std::string& m, const std::string& text = "");

  private:
    void scan_begin();
    void scan_end();

    void add_fn(const std::string & name, const cfn & fn)
    {
        // 二重定義をチェックする
        fns_[name] = fn;
    }

    cfn & get_fn(const std::string & name)
    {
        // 未定義チェックを行う
        return fns_[name];
    }

  private:
    std::string file_;
    std::map<std::string, int> values_;  // 変数テーブル
    std::map<std::string, cfn> fns_;
    std::string curfn_;
};

#endif

