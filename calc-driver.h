#ifndef CALC_DRIVER_H__
#define CALC_DRIVER_H__

#include <string>
#include <map>
#include "calc-parser.hh"
#include "node.h"
#include "exprlist.h"
#include "arglist.h"

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

    std::string &get_filename() { return file; }
    
    bool calc(const std::string &f);

    // 変数の値を取得
    int value(const std::string *name)
    {
        return values[*name];
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

  private:
    std::map<std::string, int> values;  // 変数テーブル
    std::string file;
};

#endif

