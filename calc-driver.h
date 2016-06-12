#ifndef CALC_DRIVER_H__
#define CALC_DRIVER_H__

#include "calc-parser.hh"
#include "node.h"

// Forward declarations.
class calc_driver;

#define	YY_DECL											\
	yy::calc_parser::token_type							\
	yylex(yy::calc_parser::semantic_type* yylval,		\
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
	void assign(const std::string *value, cnode *node);
	void print(cnode *node);
	void list();
    void lcomment(const std::string *value);
    void declfn(const std::string *name, cnode *args);
    void ret(cnode *node);

	// Error handling.
	void error(const std::string& m);

  private:
	void scan_begin();
	void scan_end();

  private:
	std::map<std::string, int> values;	// 変数テーブル

	std::string file;
} ;

#endif

