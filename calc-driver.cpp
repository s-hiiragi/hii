#include <iostream>
#include <iomanip>
#include "calc-driver.h"
#include "calc-parser.hh"


calc_driver::calc_driver()
{
}

calc_driver::~calc_driver()
{
}

bool calc_driver::calc(const std::string &f)
{
	file = f;
	scan_begin();								// スキャナー初期化
	yy::calc_parser parser(*this);				// パーサー構築
	int result = parser.parse();				// 構文解析
	scan_end();									// スキャナー終了

	if (result != 0)
		return false;							// パーサーエラー
	return true;
}

// エラーメッセージを出力

void calc_driver::error(const std::string& m)
{
	std::cerr << m << std::endl;
}

// 代入処理

void calc_driver::assign(const std::string *value, cnode *node)
{
	values[*value] = node->expr(this);
	delete value;		// 後始末は自分で行う
	delete node;
}

void calc_driver::print(cnode *node)
{
	std::cout << node->expr(this) << std::endl;
	delete node;
}

struct list_action {
	void operator()(const std::pair<std::string, int> &it)
	{
		std::cout << it.first << " = " << it.second << std::endl;
	}
} ;

void calc_driver::list()
{
    // TODO range-based for, lambda-functionに変える
	std::for_each(values.begin(), values.end(), list_action());
}

void calc_driver::lcomment(const std::string *value)
{
    std::cout << "COMMENT: " << *value << std::endl;
    delete value;
}

void print_node(const cnode *p, int nestlev = 0)
{
    for (int i = 0; i < nestlev; i++) {
        std::cout << "  ";
    }
    
    if (nestlev >= 1) {
        std::cout << " |-- ";
    }
    
    if (p == nullptr) {
        std::cout << "nullptr" << std::endl;
        return;
    }
    
    const char * opname;
    switch (p->op()) {
    case OP_NEG:        opname = "NEG";     break;
    case OP_PLUS:       opname = "PLUS";    break;
    case OP_MINUS:      opname = "MINUS";   break;
    case OP_TIMES:      opname = "TIMES";   break;
    case OP_DIVIDE:     opname = "DIVIDE";  break;
    case OP_NAMEVAL:    opname = "NAMEVAL"; break;
    case OP_IVAL:       opname = "IVAL";    break;
    case OP_ARGS:       opname = "ARGS";    break;
    case OP_EMPTY:      opname = "EMPTY";   break;
    default:            opname = "unknown"; break;
    }
    
    std::cout << "cnode " << opname << std::endl;
    print_node(p->left(), nestlev+1);
    print_node(p->right(), nestlev+1);
}

template <class T>
void listnodes(const cnode *node, const T& fn)
{
    if (node == nullptr) return;
    listnodes(node->left(), fn);
    listnodes(node->right(), fn);
    if (node->op() == OP_NAMEVAL) {
        fn(node->string());
    }
}

// 関数定義
void calc_driver::declfn(const std::string *name, cnode *args)
{
    std::cout << "DECL_FN: " << *name << std::endl;
    
    // TODO もうちょっとすっきり表示したい
    //print_node(args);
    
    // 引数列を取得
    std::vector<std::string> names;
    listnodes(args, [&](std::string n) { names.push_back(n); });
    
    for (auto &&e : names) {
        std::cout << "arg: " << e << std::endl;
    }
    
    delete name;
    delete args;
}

void calc_driver::ret(cnode *node)
{
    std::cout << "RET: " << node->op() << std::endl;
    delete node;
}

