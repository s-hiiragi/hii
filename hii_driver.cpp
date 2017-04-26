#include <string>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdio>
#include "hii_driver.h"
#include "parser.hh"
#include "cnode.h"
#include "cleaf.h"
#include "cscope.h"

using namespace std;

bool hii_driver::parse(string const & f)
{
    file_ = f;
    scan_begin();                 // スキャナー初期化
    yy::parser parser(*this);     // パーサー構築
    int result = parser.parse();  // 構文解析
    scan_end();                   // スキャナー終了
    if (result != 0)
        return false;             // パーサーエラー
    return true;
}

void hii_driver::set_ast(cnode * ast)
{
    ast_ = ast;

    std::cout << "set_ast" << std::endl;
    cnode::print(ast);

}

bool hii_driver::def_var(cnode const * node)
{
    auto name = static_cast<cleaf const *>(node->left())->sval();
    auto expr = node->right();

    cout << "declare variable \"" << name << "\"" << endl;

    cleaf * res = new cleaf();
    *res = eval(expr);

    (*scope_)[name] = res;
    vars_.push_back(res);

    return true;
}

bool hii_driver::def_fun(cnode const * node)
{
    auto name = static_cast<cleaf const *>(node->left())->sval();

    cout << "declare function \"" << name << "\"" << endl;

    // 現在のスコープに関数を追加
    (*scope_)[name] = node;

    return true;
}

cleaf hii_driver::eval_assign(cnode const * node)
{
    cleaf res {};
    if (!this->def_var(node)) {
        std::printf("ERROR: var \"%s\" is already defined.\n", 
            static_cast<cleaf const *>(node->left())->sval().c_str());
    }
    return res;
}

cleaf hii_driver::eval_fun(cnode const * node)
{
    cleaf res {};
    if (!this->def_fun(node)) {
        std::printf("ERROR: fun \"%s\" is already defined.\n", 
            static_cast<cleaf const *>(node->left())->sval().c_str());
    }
    return res;
}

cleaf hii_driver::eval_if(cnode const * node)
{
    auto cond = node->left();
    auto stats = node->right()->left();
    auto elifs = node->right()->right();
    
    // condを評価
    cleaf res = this->eval(cond);

    bool result;
    switch (res.op()) {
    // 真:""以外 偽:""
    case OP_STR:
        result = (res.sval() != "");
        break;
    // 真:0以外 偽:0
    case OP_INT:
        result = (res.ival() != 0);
        break;
    default:
        std::fprintf(stderr, "評価結果を真偽値に変換できません: op=%d\n", res.op());
        throw new std::logic_error("評価結果を真偽値に変換できません");
    }

    if (result) {
        return eval(stats);
    } else {
        if (elifs == nullptr) {
            return cleaf();
        } else {
            return eval(elifs);
        }
    }
}

cleaf hii_driver::eval_call(cnode const * node)
{
    cleaf res {};

    auto name = static_cast<cleaf const *>(node->left())->sval();
    auto exprs = node->right();

    // TODO 関数呼び出しを実施

    return res;
}

cleaf hii_driver::eval_op1(cnode const * node)
{
    auto l_value = this->eval(node->left());

    if (l_value.op() != OP_INT) {
        std::runtime_error("左辺が数値ではありません");
    }

    int val;
    switch (node->op())
    {
    case OP_NEG:
        val = -l_value.ival();
        break;
    default:
        std::logic_error("未定義の演算子です");
    }
    cleaf res = cleaf(OP_INT, val);

    return res;
}

cleaf hii_driver::eval_op2(cnode const * node)
{
    auto l_value = this->eval(node->left());
    auto r_value = this->eval(node->right());

    if (l_value.op() != OP_INT) {
        std::runtime_error("左辺が数値ではありません");
    }
    if (r_value.op() != OP_INT) {
        std::runtime_error("右辺が数値ではありません");
    }

    // TODO 演算を実施
    int val;
    switch (node->op())
    {
    case OP_PLUS:
        val = l_value.ival() + r_value.ival();
        break;
    case OP_MINUS:
        val = l_value.ival() - r_value.ival();
        break;
    case OP_TIMES:
        val = l_value.ival() * r_value.ival();
        break;
    case OP_DIVIDE:
        val = l_value.ival() / r_value.ival();
        break;
    default:
        std::logic_error("未定義の演算子です");
    }
    cleaf res = cleaf(OP_INT, val);

    return res;
}

cleaf hii_driver::eval_id(cnode const * node)
{
    cleaf res {};
    auto name = static_cast<cleaf const *>(node->left())->sval();
    // TODO スコープを考慮して値を取り出す
    return res;
}

cleaf hii_driver::eval(cnode const * node)
{
    std::printf("eval op=%s\n", node->name());

    switch (node->op())
    {
    case OP_STATS:
        {
            auto stats = static_cast<clist const *>(node);
            stats->each([&](cnode const & n) {
                eval(&n);
            });
        }
        return cleaf();
    // 現在のスコープに変数を追加する
    case OP_ASSIGN:
        //return eval_assign(node);
        return cleaf();
    // 関数の場合は現在のスコープに関数を追加する
    // スコープを作る
    case OP_FUN:
        //return eval_fun(node);
        return cleaf();
    // スコープを作る
    case OP_IF:
        return eval_if(node);
    case OP_ELIF:
        return eval_if(node);
    case OP_ELSE:
        return eval(node->left());
    case OP_CALL:
        //return eval_call(node);
        return cleaf();
    // 1項演算子
    case OP_NEG:
        //return eval_op1(node);
        return cleaf();
    // 2項演算子
    case OP_PLUS:
    case OP_MINUS:
    case OP_TIMES:
    case OP_DIVIDE:
        //return eval_op2(node);
        return cleaf();
    case OP_LCOMMENT:
    case OP_MCOMMENT:
        return cleaf();
    // 変数
    // スコープを考慮して値を取り出す
    case OP_ID:
        //return eval_id(node);
        return cleaf();
    // リテラル
    case OP_INT:
    case OP_STR:
        return *static_cast<cleaf const *>(node);
    default:
        std::fprintf(stderr, "評価方法が未定義です: op=%d\n", node->op());
        throw std::logic_error("評価方法が未定義です");
    }
}

/*
 * 意味解析＆実行
 * 
 * 
 */
bool hii_driver::exec(const string &f)
{
    if (!parse(f)) return false;

    cscope * s = new cscope();
    scope_ = s;
    scopestack_.push_back(s);

    cout << "eval ast: starting" << endl;
    cleaf res = eval(this->ast_);
    cout << "eval ast: finished" << endl;

    cout << "scopestack: count=" << scopestack_.size() << endl;

    delete scope_;
    scope_ = nullptr;
    scopestack_.clear();
    vars_.clear();
    delete ast_;
    ast_ = nullptr;

    return true;
}

