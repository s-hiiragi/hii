#pragma once

#include "parser.hh"
#include "cnode.h"

namespace hii {
class parser_driver;
}

#define YY_DECL                                     \
    yy::parser::token_type                          \
    yylex(yy::parser::semantic_type* yylval_param,  \
          yy::parser::location_type* yylloc_param,  \
          hii::parser_driver& driver)

#define YYSTYPE yy::parser::semantic_type
#define YYLTYPE yy::parser::location_type

YY_DECL;

namespace hii
{

/*
 * sample:
 *   cnode *ast = nullptr;
 *
 *   parser_driver pd;
 *   if (!pd.parse_file("hoge.hi", &ast)) {
 *     return 1;
 *   }
 */

class parser_driver
{
public:
    int parse_file(const std::string &filename, cnode **ast);
    int parse_string(const std::string &code, cnode **ast);

    // 以下はscanner/driver向けに公開するメソッド
    // privateにしたいが'_'プレフィクスで妥協する
   
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

    std::string & filename() {
        return filename_;
    }

    bool _has_input_string() const {
        return has_inputstr_;
    }

    std::string _read_string(size_t max_size);

    void _set_ast(cnode *ast);

    void scan_end();
private:
    int scan_begin(const std::string &filename);

    std::string filename_;
    bool has_inputstr_ = false;
    std::string inputstr_;

    cnode *ast_ = nullptr;
};

} // end of hii namespace

