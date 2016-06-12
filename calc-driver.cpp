#include <iostream>
#include <iomanip>
#include "calc-driver.h"
#include "calc-parser.hh"

using namespace std;

calc_driver::calc_driver()
{
}

calc_driver::~calc_driver()
{
}

bool calc_driver::calc(const string &f)
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

void calc_driver::error(const string& m)
{
	cerr << m << endl;
}

// 代入処理

void calc_driver::assign(const string *value, cnode *node)
{
	values[*value] = node->expr(this);
	delete value;		// 後始末は自分で行う
	delete node;
}

void calc_driver::print(cnode *node)
{
	cout << node->expr(this) << endl;
	delete node;
}

struct list_action {
	void operator()(const pair<string, int> &it)
	{
		cout << it.first << " = " << it.second << endl;
	}
} ;

void calc_driver::list()
{
    // TODO range-based for, lambda-functionに変える
	for_each(values.begin(), values.end(), list_action());
}

void calc_driver::lcomment(const string *value)
{
    cout << "COMMENT: " << *value << endl;
    delete value;
}

void print_node(const cnode *p, int nestlev = 0)
{
    for (int i = 0; i < nestlev; i++) {
        cout << "  ";
    }
    
    if (nestlev >= 1) {
        cout << " |-- ";
    }
    
    if (p == nullptr) {
        cout << "nullptr" << endl;
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
    
    cout << "cnode " << opname << endl;
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
void calc_driver::declfn(const string *name, cnode *args)
{
    cout << "DECL_FN: " << *name << endl;
    
    // TODO もうちょっとすっきり表示したい
    //print_node(args);
    
    // 引数列を取得
    vector<string> names;
    listnodes(args, [&](string n) { names.push_back(n); });
    
    for (auto &&e : names) {
        cout << "arg: " << e << endl;
    }
    
    delete name;
    delete args;
}

void calc_driver::ret(cnode *node)
{
    cout << "RET: " << node->op() << endl;
    delete node;
}

