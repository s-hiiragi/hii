#include <string>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdio>
#include "hii_driver.h"
#include "parser.hh"
#include "cnode.h"
#include "cleaf.h"

using namespace std;

void hii_driver::set_ast(cnode *ast)
{
    std::cout << "set_ast" << std::endl;

    cnode::print(ast);

    // OP_ASSIGN or OP_FUN なら、識別子とノードを登録する
    cnode::list(ast, [](cnode const *node, unsigned int nestlev) {
        switch (node->op())
        {
        // 現在のスコープに変数を追加する
        case OP_ASSIGN:
            cout << "declare variable \"" << static_cast<cleaf const *>(node->left())->sval() << "\"" << endl;
            // this->scope_[name] = node;
            break;
        // 関数の場合は現在のスコープに関数を追加する
        // スコープを作る
        case OP_FUN:
            cout << "declare function \"" << static_cast<cleaf const *>(node->left())->sval() << "\"" << endl;

            // 現在のスコープに関数を追加
            // this->scope_[name] = node;

            // 現在のスコープスタックに関数スコープを追加
            // cscope s = new cscope();
            // this->scopes_.push(s);
            // this->scope_ = s;
            break;
        case OP_IF:
            break;
        }
    });
}

bool hii_driver::exec(const string &f)
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

// エラーメッセージを出力
/*
void hii_driver::error(const string& m, const string& text)
{
    cerr << m << ": " << text << endl;
}
*/

