#ifndef CALC_DRIVER_H__
#define CALC_DRIVER_H__

#include <string>
#include <map>
#include "parser.hh"
#include "cnode.h"
#include "cleaf.h"
#include "cscope.h"

class hii_driver;

#define YY_DECL                                    \
    yy::parser::token_type                         \
    yylex(yy::parser::semantic_type* yylval,       \
         hii_driver& driver)

YY_DECL;

/*
 * 字句解析＆構文解析
 * Flex&Bisonにより構文木を生成する
 * parse()
 *  |-- set_ast()
 * 
 * 意味解析＆実行
 * exec()
 */
class hii_driver {
  public:
    hii_driver() {}
    virtual ~hii_driver() {}

    bool parse(std::string const &file);
    void set_ast(cnode *ast);

    bool resolve_names(cnode &node);

    bool def_var(cnode const *node);
    bool def_fun(cnode const *node);

    cleaf eval_stats(cnode const *node);
    cleaf eval_assign(cnode const *node);
    cleaf eval_fun(cnode const *node);
    cleaf eval_if(cnode const *node);
    cleaf eval_call(cnode const *node);
    cleaf eval_op1(cnode const *node);
    cleaf eval_op2(cnode const *node);
    cleaf eval_id(cnode const *node);
    cleaf eval(cnode const *node);

    bool exec(std::string const &file);

    // 変数の値を取得
    int value(const std::string &name)
    {
        // TODO 未定義チェックを行う
        // TODO 実装する
        //return nullptr;
        return 0;
    }
    
    // 構文にマッチした時のアクション

    // Error handling.
    void error(char const *message)
    {
        std::printf("%s", message);
    }

    //void error(const std::string& m, const std::string& text = "");
    template<class... Args>
    void error(char const *format, Args const &... args)
    {
        std::printf(format, args...);
    }

    // for debug
    void print_scopes() {
        using std::cout;
        using std::endl;
        cout << "D: put scopes (size=" << scopes_.size() << ")" << endl;
        cout << "  --" << endl;
        for (auto const s : scopes_) {
            s->print();
        }
    }

  private:
    void scan_begin();
    void scan_end();

  private:
    std::string file_;
    cnode *ast_ = nullptr;
    std::vector<cscope *> scopes_;
};

#endif

