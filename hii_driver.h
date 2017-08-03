#ifndef HII_DRIVER_H_
#define HII_DRIVER_H_

#include <string>
#include <vector>
#include <map>
#include "parser.hh"
#include "cnode.h"
#include "cleaf.h"
#include "cscope.h"
#include "cvalue.h"

class hii_driver;

#define YY_DECL                                     \
    yy::parser::token_type                          \
    yylex(yy::parser::semantic_type* yylval_param,  \
          yy::parser::location_type* yylloc_param,  \
          hii_driver& driver)

#define YYSTYPE yy::parser::semantic_type
#define YYLTYPE yy::parser::location_type

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
class hii_driver
{
  public:
    hii_driver() {}
    virtual ~hii_driver() {}

    bool exec(std::string const &file, std::vector<std::string> &args);

  // for parser
    void set_ast(cnode *ast);

    void error(char const *message)
    {
        std::fprintf(stderr, "%s", message);
    }

    //void error(const std::string& m, const std::string& text = "");
    template<class... Args>
    void error(char const *format, Args const &... args)
    {
        std::fprintf(stderr, format, args...);
    }

  private:
    void scan_begin();
    void scan_end();

    bool parse(std::string const &file);

    bool resolve_names(cnode &node);

    bool def_var(cnode const *node);
    bool def_fun(cnode const *node);

    cvalue eval_stats(cnode const *node);
    cvalue eval_assign(cnode const *node);
    cvalue eval_reassign(cnode const *node);
    cvalue eval_op1stat(cnode const *node);
    cvalue eval_op2stat(cnode const *node);
    cvalue eval_fun(cnode const *node);
    cvalue eval_ret(cnode const *node);
    cvalue eval_if(cnode const *node);
    cvalue eval_sw(cnode const *node);
    cvalue eval_call(cnode const *node);
    cvalue eval_loop(cnode const *node);
    cvalue eval_cont(cnode const *node);
    cvalue eval_break(cnode const *node);
    cvalue eval_op1(cnode const *node);
    cvalue eval_op2(cnode const *node);
    cvalue eval_id(cnode const *node);
    cvalue eval_array(cnode const *node);
    cvalue eval_slice(cnode const *node);
    cvalue eval_str(cnode const *node);
    cvalue eval(cnode const *node);
    
    // for debug
    void print_scopes()
    {
        using std::cout;
        using std::endl;
        cout << "D: print scopes (size=" << scopes_.size() << ")" << endl;
        cout << "  --" << endl;
        for (auto &&s : scopes_) {
            s.print();
        }
    }

    std::string file_;
    cnode *ast_ = nullptr;

    // 実行時情報
    std::vector<cscope> scopes_;
    bool exit_fun_ = false;
    bool cont_loop_ = false;
    bool break_loop_ = false;
    //std::map<std::string, cnode *> builtin_functions_;
};

#endif // HII_DRIVER_H_

