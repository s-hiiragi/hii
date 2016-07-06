#include <string>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include "calc-driver.h"
#include "calc-parser.hh"
#include "node.h"
#include "exprlist.h"
#include "arglist.h"

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
    scan_begin();                               // スキャナー初期化
    yy::calc_parser parser(*this);              // パーサー構築
    int result = parser.parse();                // 構文解析
    scan_end();                                 // スキャナー終了

    if (result != 0)
        return false;                           // パーサーエラー
    return true;
}

void calc_driver::lcmnt(const string *text)
{
    cout << "LINE_COMMENT: " << *text << endl;
    delete text;
}

void calc_driver::assign(const std::string *name, cnode *expr)
{
    values[*name] = expr->expr(this);
    delete name;       // 後始末は自分で行う
    delete expr;
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
};

void calc_driver::listvars()
{
    // TODO range-based for, lambda-functionに変える
    for_each(values.begin(), values.end(), list_action());
}

void calc_driver::call_state(const std::string *name, exprlist *exprs)
{
    delete name;
    delete exprs;
}

void calc_driver::if_state(cnode *expr)
{
    cout << "IF: " << endl;
    delete expr;
}

void calc_driver::elseif_state(cnode *expr)
{
    cout << "ELSE_IF: " << endl;
    delete expr;
}

void calc_driver::else_state()
{
    cout << "ELSE: " << endl;
}

void calc_driver::end_state()
{
    cout << "END: " << endl;
}

void calc_driver::loop_state(cnode *expr)
{
    cout << "LOOP: " << endl;
    delete expr;
}

void print_node(const cnode *p, int nestlev = 0)
{
    if (p == nullptr) return;
    
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
    
    cout << "cnode " << p->name() << endl;
    print_node(p->left(), nestlev+1);
    print_node(p->right(), nestlev+1);
}

void calc_driver::declfn(const string *name, arglist *args)
{
    cout << "DECL_FN: " << *name << endl;
    
    // TODO もうちょっとすっきり表示したい
    print_node(args);
    
    // 引数列を取得
    vector<string> names;
    //listnodes(args, [&](string n) { names.push_back(n); });
    cnode::list(args, [&](const cnode *n, unsigned int nestlev) {
        if (n->op() == OP_ID) names.push_back(n->sval());
    });
    
    for (auto &&e : names) {
        cout << "arg: " << e << endl;
    }
    
    delete name;
    delete args;
}

void calc_driver::ret(cnode *expr)
{
    if (expr != nullptr) {
        cout << "RET: op=" << expr->name() << endl;
        delete expr;
    } else {
        cout << "RET: op=null" << endl;
    }
}

// エラーメッセージを出力
void calc_driver::error(const string& m, const string& text)
{
    cerr << m << ": " << text << endl;
}

