#ifndef CALC_DRIVER_H__
#define CALC_DRIVER_H__

#include <string>
#include <map>
#include "parser.hh"
#include "cnode.h"

class hii_driver;

#define YY_DECL                                    \
    yy::parser::token_type                         \
    yylex(yy::parser::semantic_type* yylval,       \
         hii_driver& driver)

YY_DECL;

class hii_driver {
  public:
    hii_driver() {}
    virtual ~hii_driver() {}

    void set_ast(cnode *ast);
    
    bool exec(const std::string &f);

    // 変数の値を取得
    int value(const std::string & name)
    {
        // TODO 未定義チェックを行う
        // TODO 実装する
        //return nullptr;
        return 0;
    }
    
    // 構文にマッチした時のアクション

    // Error handling.
    void error(char const * message)
    {
        std::printf("%s", message);
    }

    //void error(const std::string& m, const std::string& text = "");
    template<class... Args>
    void error(char const * format, Args const &... args)
    {
        std::printf(format, args...);
    }

  private:
    void scan_begin();
    void scan_end();

  private:
    std::string file_;
    //std::vector<cscope> scopes_;
    //std::vector<cscope> current_scopes_; 
    std::map<std::string, cnode *> identifiers_;
};

#endif

