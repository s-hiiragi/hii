#include <iostream>
#include <iomanip>
#include "calc-driver.h"
#include "calc-parser.hh"

// コンストラクタ

calc_driver::calc_driver()
{
}

// デストラクタ

calc_driver::~calc_driver()
{
}

// 計算

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
	std::for_each(values.begin(), values.end(), list_action());
}

