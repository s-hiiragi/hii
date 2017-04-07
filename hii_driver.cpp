#include <string>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cstdio>
#include "hii_driver.h"
#include "parser.hh"
#include "cnode.h"

using namespace std;

void hii_driver::set_ast(cnode *ast)
{
    std::cout << "set_ast" << std::endl;

    cnode::print(ast);
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

