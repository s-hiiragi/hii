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
#include "error.h"

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

    bool exec_string(std::string const &repl_code);
    bool exec_file(std::string const &file, std::vector<std::string> const &args);

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

    std::string & filename()
    {
        return file_;
    }

    std::string const & filename() const
    {
        return file_;
    }

    bool is_repl() const { return is_repl_; }
    std::string scan_input(size_t max_size);

  private:
    bool parse(std::string const &fname);
    void scan_begin();
    void scan_end();
    bool eval_ast(std::vector<std::string> const &args);

    bool check_syntax(cnode &node);
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
    cvalue eval_dict(cnode const *node);
    cvalue eval_dictitem(cnode const *node);
    cvalue eval_slice(cnode const *node);
    cvalue eval_str(cnode const *node);
    cvalue eval(cnode const *node);

    my::expected<cvalue *> get_var(std::string const &varname);
    my::expected<cvalue *> get_var_element(cvalue *var, clist const &indexes);
    
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

    // 構文解析字情報
    std::string file_;
    bool is_repl_ = false;
    std::string repl_code_;
    cnode *ast_ = nullptr;

    // 実行時情報
    std::vector<cscope> scopes_;
    bool exit_fun_ = false;
    bool cont_loop_ = false;
    bool break_loop_ = false;
    //std::map<std::string, cnode *> builtin_functions_;
};

#endif // HII_DRIVER_H_

