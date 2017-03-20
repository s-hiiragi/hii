#include <string>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include "calc-driver.h"
#include "calc-parser.hh"
#include "cnode.h"
#include "exprlist.h"
#include "arglist.h"
#include "cfn.h"

using namespace std;

calc_driver::calc_driver()
{
}

calc_driver::~calc_driver()
{
}

void calc_driver::set_ast(cnode *ast)
{
    std::cout << "set_ast" << std::endl;
}

bool calc_driver::calc(const string &f)
{
    file_ = f;
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
    values_[*name] = expr->expr(this);
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
    for_each(values_.begin(), values_.end(), list_action());
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

void calc_driver::declfn(const string *name, arglist *arglst)
{
    cout << "DECL_FN: " << *name << endl;

    if (curfn_ != "") {
        cerr << "ERROR: " << *name << " is already declared." << endl;
        return;
    }
    
    cnode::print(arglst);
    
    // 引数列を取得
    vector<string> args;
    //listnodes(args, [&](string n) { names.push_back(n); });
    cnode::list(arglst, [&](const cnode *n, unsigned int nestlev) {
        if (n->op() == OP_ID) args.push_back(dynamic_cast<const cleaf *>(n)->sval());
    });
    
    for (auto &&a : args) {
        cout << "arg: " << a << endl;
    }

    // 関数名とシグネチャを登録する
    cfn fn(*name, args);
    add_fn(*name, fn);

    // 現在のスコープの関数名に名前を設定する
    curfn_ = *name;

    // 以後のコードはDECL_FNの定義に追加する(ENDがくるまで)
    
    delete name;
    delete arglst;
}

void calc_driver::ret(cnode *expr)
{
    cout << "RET: op=" << (expr != nullptr ? expr->name() : "null") << endl;

    if (curfn_ == "") {
        cerr << "ERROR: 関数定義の外ではretは使えません" << endl;
        delete expr;
        return;
    }

    // retステートメントを関数のボディに追加
    // XXX 下でセグフォする
    get_fn(curfn_).add_stat(cnode(OP_RET, expr));

    // 現在のスコープの関数名を空にする
    curfn_ = "";

    delete expr;
}

// エラーメッセージを出力
void calc_driver::error(const string& m, const string& text)
{
    cerr << m << ": " << text << endl;
}

