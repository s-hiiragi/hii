#include <algorithm>
#include <cassert>
#include <cstdio>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>
#include "hii_driver.h"
#include "parser.hh"
#include "cnode.h"
#include "cleaf.h"
#include "cscope.h"
#include "cvalue.h"
#include "error.h"
#include "clog.h"

using std::cin;
using std::cout;
using std::endl;
using std::map;
using std::vector;
using std::string;
using std::make_pair;
using std::stringstream;
using std::logic_error;
using my::clog;

/**
 * 字句解析,構文解析,意味解析,インタプリタ実行を行う
 */
bool hii_driver::exec(const string &f, vector<string> &args)
{
    if (!parse(f)) return false;

    // グローバルスコープを生成
    scopes_.push_back(cscope());
    {
        vector<cvalue> arg_values(args.size());
        std::transform(args.begin(), args.end(), arg_values.begin(),
            [](string const &s){ return cvalue(s); });

        scopes_.back().add_var(string("sys_args"), cvalue(arg_values), false);
    }

    // 構文チェック
    clog::i("### check syntax: starting ###");
    if (!check_syntax(*ast_)) {
        clog::e("check_syntax failed");
        return false;
    }
    clog::i("### check syntax: finished ###");

    // 名前解決
    clog::i("### resolve names: starting ###");
    resolve_names(*ast_);
    clog::i("### resolve names: finished ###");

    // 意味解析＆実行
    clog::d("### eval ast: starting ###");
    eval(ast_);
    clog::d("### eval ast: finished ###");

    if (exit_fun_) {
        clog::e("関数の外でret文が使用されています");
    }

    if (cont_loop_) {
        clog::e("loopの外でcont文が使用されています");
    }

    if (break_loop_) {
        clog::e("loopの外でbreak文が使用されています");
    }

    // 後始末
    //assert(scopes_.empty());
    assert(scopes_.size() == 1);

    delete ast_;
    ast_ = nullptr;

    return true;
}

bool hii_driver::parse(string const &f)
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

void hii_driver::set_ast(cnode *ast)
{
    assert(ast != nullptr);

    ast_ = ast;
    clog::d("set_ast");

    cnode::print(ast);
}

/* 構文チェック
 * 
 * チェック項目
 * - returnがfunの中にあるか
 * - break,continueがloop/swの中にあるか
 */
bool hii_driver::check_syntax(cnode &node)
{
    bool result = true;
    int fun_count = 0;
    vector<int> loop_counts = {0};
    vector<int> sw_counts = {0};

    auto on_enter = [&](cnode::cctrl &ctrl, cnode &n) -> bool {
        switch (n.op())
        {
        case OP_FUN:
            fun_count++;
            // sw_count, loop_countを退避する
            sw_counts.push_back(0);
            loop_counts.push_back(0);
            break;
        case OP_SW:
            sw_counts.back()++;
            break;
        case OP_LOOP:
            loop_counts.back()++;
            break;
        case OP_RET:
            if (fun_count <= 0) {
                clog::e("retは関数の外では使用できません: fun_count=%d", fun_count);
                result = false;
            }
            break;
        case OP_CONT:
            if (loop_counts.back() <= 0) {
                clog::e("contはloopの外では使用できません: loop_count=%d", loop_counts.back());
                result = false;
            }
            break;
        case OP_BREAK:
            if (sw_counts.back() <= 0 && loop_counts.back() <= 0) {
                clog::e("breakはswまたはloopの外では使用できません: sw_count=%d, loop_count=%d", 
                    sw_counts.back(), loop_counts.back());
                result = false;
            }
            break;
        case OP_MCOMMENT:
            {
                auto const *comments = static_cast<clist const *>(&n);
                comments->each([](cnode const &n2) {
                    auto &&v = static_cast<cleaf const &>(n2);
                    cout << "  \e[32m" << v.sval() << "\e[00m" << endl;
                    return true;
                });
            }
            break;
        }
        return true;
    };
    auto on_leave = [&](cnode::cctrl &ctrl, cnode &n) -> bool {
        switch (n.op())
        {
        case OP_FUN:
            // sw_count, loop_countを復元する
            sw_counts.pop_back();
            loop_counts.pop_back();
            fun_count--;
            break;
        case OP_SW:
            sw_counts.back()--;
            break;
        case OP_LOOP:
            loop_counts.back()--;
            break;
        }
        return true;
    };

    node.each(on_enter, on_leave);

    if (fun_count != 0) {
        clog::e("fun_count = %d", fun_count);
        return false;
    }
    if (sw_counts.size() != 1 || sw_counts.back() != 0) {
        clog::e("loop_counts: size=%zu", loop_counts.size());
        if (sw_counts.size() >= 1) {
            clog::e("loop_counts: back.count=%d", loop_counts.back());
        }
        return false;
    }
    if (loop_counts.size() != 1 || loop_counts.back() != 0) {
        clog::e("loop_counts: size=%zu", loop_counts.size());
        if (loop_counts.size() >= 1) {
            clog::e("loop_counts: back.count=%d", loop_counts.back());
        }
        return false;
    }

    return result;
}

/* 名前解決
 */
bool hii_driver::resolve_names(cnode &node)
{
    // スコープを作成
    vector<cscope> scopes = scopes_;

    auto on_enter = [&](cnode::cctrl &ctrl, cnode &n) -> bool {
        switch (n.op())
        {
        case OP_STATS:
            // スコープ追加
            scopes.push_back(cscope());
            clog::d("on_enter: %s nest=%zu", n.name(), scopes.size());

            if (scopes.size() == 1) {
                // TODO 組込関数を定義(evalの方でも行う)
                // TODO 解放処理も必要
                // def_builtin_functions(*scopes.back());
            }
            break;
        case OP_ASSIGN:
            {
                // 変数の二重定義をチェック
                auto const &name = static_cast<cleaf &>(*n.left()).sval();
                bool is_var = n.left()->op() == OP_VAR;
                clog::d("on_enter: %s %s", n.name(), name.c_str());
                if (scopes.back().has_var(name)) {
                    clog::e("%s%sがすでに定義されているため%s%sを二重定義できません", 
                        scopes.back().is_writable(name) ? "変数" : "定数", name.c_str(), 
                        is_var ? "変数" : "定数", name.c_str());
                    //return false;
                    return true;
                }

                // 変数を定義
                scopes.back().add_var(name, cvalue(), is_var);  // value is dummy
            }
            break;
        case OP_REASSIGN:
            {
                // 変数の定義チェック
                auto const &name = n.left()->op() == OP_VAR ?
                    static_cast<cleaf *>(n.left())->sval() :
                    static_cast<cleaf *>(n.left()->left())->sval();
                
                clog::d("on_enter: %s %s", n.name(), name.c_str());
                
                if (!scopes.back().has_var(name)) {
                    clog::e("変数%sは定義されていません", name.c_str());
                    //return false;
                    return true;
                }
                if (!scopes.back().is_writable(name)) {
                    clog::e("%sは定数なので代入できません", name.c_str());
                    //return false;
                    return true;
                }
            }
            break;
        case OP_FUN:
            {
                // 関数の二重定義をチェック
                auto const &name = static_cast<cleaf &>(*n.left()).sval();
                clog::d("on_enter: %s %s", n.name(), name.c_str());
                if (scopes.back().has_fun(name)) {
                    clog::e("関数%sはすでに定義されています", name.c_str());
                    //return false;
                    return true;
                }

                // 関数を定義
                scopes.back().add_fun(name, nullptr);  // value is dummy

                // 引数を定義
                auto const &args = static_cast<clist &>(*n.right()->left());
                args.each([&](cnode const &a) {
                    auto const &argname = dynamic_cast<cleaf const &>(a).sval();
                    scopes.back().add_var(argname, cvalue(), false);  // value is dummy
                    return true;
                });
            }
            break;
        case OP_ARGS:
        case OP_DICT:
        case OP_DICTITEM:
        case OP_DICT_INDEX:
            ctrl.skip_children();
            break;
        case OP_ATTRS:
            {
                auto const &attrs = static_cast<clist const &>(n);
                if (attrs.left() != nullptr) {
                    auto const &attr1 = static_cast<cleaf const *>(attrs.left())->sval();
                    if (attr1 == "variadic") {
                        auto const &name = static_cast<cleaf const *>(attrs.right()->left())->sval();
                        scopes.back().add_var(name, cvalue(), false);  // value is dummy
                    }
                }
            }
            ctrl.skip_children();
            break;
        case OP_LOOP:
            {
                std::string name;
                if (n.left() != nullptr) {
                    name = static_cast<cleaf const *>(n.left())->sval();
                } else {
                    name = "cnt";
                }
                clog::d("on_enter: %s", n.name());
                // 変数を定義
                scopes.back().add_var(name, cvalue(), false);  // value is dummy
            }
            break;
        case OP_ID:
        case OP_VAR:
            // 識別子が定義されているかチェック
            // XXX 仮引数はチェックしないようにする!
            //     -> OP_ARGSだったら子ノードを探索しない、という処理が必要?
            {
                auto const &name = static_cast<cleaf &>(n).sval();
                clog::d("on_enter: %s %s", n.name(), name.c_str());

                // 組込定数はエラーとしない
                /*
                if (name == "sys_args") {
                    break;
                }
                */

                // 組込関数はエラーとしない
                if (name == "nop" ||
                    name == "input" ||
                    name == "p" ||
                    name == "print" ||
                    name == "d" ||
                    name == "len" ||
                    name == "assert" ||
                    name == "_put_scopes") {
                    break;
                }

                bool found = false;
                for (auto it = scopes.rbegin(); it != scopes.rend(); it++)
                {
                    auto &&s = *it;
                    if (s.has_var(name) || s.has_fun(name)) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    clog::e("識別子%sは定義されていません", name.c_str());
                    //return false;
                    return true;
                }
            }
            break;
        }
        return true;
    };

    auto on_leave = [&](cnode::cctrl &ctrl, cnode &n) -> bool {
        switch (n.op())
        {
        case OP_STATS:
            // スコープ削除
            scopes.pop_back();
            clog::d("on_leave: %s %zu", n.name(), scopes.size());
            break;
        }
        return true;
    };

    // 名前解決
    node.each(on_enter, on_leave);

    // 後始末
    scopes.clear();
}

bool hii_driver::def_var(cnode const *node)
{
    auto const &name = static_cast<cleaf const *>(node->left())->sval();
    bool is_var = node->left()->op() == OP_VAR;
    auto const *expr = node->right();

    clog::d("assign name=%s", name.c_str());

    // 二重定義は不可
    if (scopes_.back().has_var(name)) {
        clog::e("%sは%sとしてすでに定義されています", 
            static_cast<cleaf const *>(node->left())->sval().c_str(),
            scopes_.back().is_writable(name) ? "変数" : "定数");
        return false;
    }

    // 上位のスコープに定義されている場合は警告を出す
    bool defined = false;
    auto it = scopes_.rbegin();
    it++; // skip current scope
    for (; it != scopes_.rend(); it++) {
        if (it->has_var(name)) {
            defined = true;
            break;
        }
    }
    if (defined) {
        clog::i("上位のスコープで定義された%s%sが隠されます\n", 
            it->is_writable(name) ? "変数" : "定数", name.c_str());
    }

    // 式を評価
    cvalue val = eval(expr);

    // 現在のスコープに変数を登録
    scopes_.back().add_var(name, val, is_var);

    return true;
}

bool hii_driver::def_fun(cnode const *node)
{
    auto const &name = static_cast<cleaf const *>(node->left())->sval();

    clog::d("declfun name=%s", name.c_str());

    // 二重定義は不可
    if (scopes_.back().has_fun(name))
        return false;

    // 上位のスコープに定義されている場合は警告を出す
    bool defined = false;
    auto it = scopes_.rbegin();
    it++; // skip current scope
    for (; it != scopes_.rend(); it++) {
        if (it->has_fun(name)) {
            defined = true;
            break;
        }
    }
    if (defined) {
        fprintf(stderr, "W: 上位のスコープで定義された関数%sが隠されます\n", name.c_str());
    }

    // TODO 名前解決を行う

    scopes_.back().add_fun(name, node);

    return true;
}

// TODO scopeを非ポインタにする
cvalue hii_driver::eval_stats(cnode const *node)
{
    auto const &stats = *static_cast<clist const *>(node);

    clog::d("eval_stats:");

    // スコープを追加
    scopes_.push_back(cscope());

    clog::d("eval_stats: create scope %zu", scopes_.size());
   
    // 複文を実行
    cvalue res;
    stats.each([&](cnode const &n) {
        res = eval(&n);
        clog::d("eval_stats: eval %s, result=%s", n.name(), res.to_string().c_str());
        if (exit_fun_) {
            clog::d("    exit_fun=true");
            return false;
        }
        if (cont_loop_) {
            clog::d("    cont_loop=true");
            return false;
        }
        if (break_loop_) {
            clog::d("    break_loop=true");
            return false;
        }
        return true;
    });

    // スコープを削除
    scopes_.pop_back();

    clog::d("eval_stats: delete scope %zu", scopes_.size());

    return res;
}

cvalue hii_driver::eval_assign(cnode const *node)
{
    if (!def_var(node)) {
        // TODO エラー処理
    }
    return cvalue();
}

/**
 * スコープを順に遡って変数を取得する
 *
 * @param[in] varname  変数名
 *
 * @retval  cvalue*の値  取得成功
 * @retval  error_info   取得失敗 (変数が定義されていない)
 * @retval  error_info   取得失敗 (定数名が指定された)
 */
my::expected<cvalue *> hii_driver::get_var(string const &varname)
{
    // 変数を探す

    bool defined = false;
    auto it = scopes_.rbegin();
    for (; it != scopes_.rend(); it++) {
        if (it->has_var(varname)) {
            defined = true;
            break;
        }
    }
    if (!defined) {
        return my::error_info("変数%sは定義されていません", varname.c_str());
    }
    if (!it->is_writable(varname)) {
        return my::error_info("%sは定数です", varname.c_str());
    }

    return &it->get_var(varname);
}

/**
 * 変数から順にインデックスを適用し要素を取得する
 * 
 * @param[in] varname  変数名
 * @param[in] indexes  インデックス(添字またはキー)のリスト
 * 
 * @retval true, cvalue*値      取得成功
 * @retval false, error_info値  取得失敗 (変数が未定義)
 * @retval false, error_info値  取得失敗 (varnameは定数であるため
 */
my::expected<cvalue *> hii_driver::get_var_element(cvalue *var, clist const &indexes)
{
    // 変更対象の要素を取得
    my::expected<> e = my::dummy_value();
    indexes.each([&](cnode const &n) {
        switch (n.op())
        {
        case OP_ARRAY_INDEX:
            {
                // 親要素が配列型かチェック
                if (!var->is_ary()) {
                    e = my::error_info("親要素が配列型ではありません (type=%s)", var->type_name().c_str());
                    return false;
                }

                // 添字を評価
                cvalue &&index = eval(n.left());

                // 添字の型をチェック
                if (!index.is_int()) {
                    e = my::error_info("添字が数値型ではありません (type=%s)", index.type_name().c_str());
                    return false;
                }

                // 添字を非負整数に変換
                size_t size = var->a().size();
                size_t forward_index = cvalue::to_positive_index(index.i(), size);

                // 添字の範囲をチェック
                if (forward_index >= size) {
                    e = my::error_info("添字が配列の範囲外です (index=%d, range=0..%zu)", index.i(), size);
                    return false;
                }

                // 要素を取得
                var = &var->a(forward_index);
            }
            break;
        case OP_DICT_INDEX:
            {
                // 親要素が辞書型かチェック
                if (!var->is_dict()) {
                    e = my::error_info("親要素が辞書型ではありません (type=%s)", var->type_name().c_str());
                    return false;
                }

                // キーが存在するかチェック
                string const &key = static_cast<cleaf const *>(n.left())->sval();
                if (var->d().find(key) == var->d().end()) {
                    e = my::error_info("キー%sが見つかりません", key.c_str());
                    return false;
                }

                // 要素を取得
                var = &var->d(key);
            }
            break;
        default:
            clog::e("invalid index node: node=%s", n.name());
            throw std::logic_error("invalid index node");
        }
        return true;
    });
    if (!e) {
        return e.error();
    }

    return var;
}

cvalue hii_driver::eval_reassign(cnode const *node)
{
    auto const &varname = static_cast<cleaf const *>(node->left()->left())->sval();
    auto const &indexes = *static_cast<clist const *>(node->left()->right());
    auto const *expr = node->right();

    clog::d("reassign name=%s", varname.c_str());

    // 変数を探す

    auto r = get_var(varname);
    if (!r) {
        // TODO varnameが定数を指している場合、定数だから代入できないと補足説明したい
        clog::e("%s", r.error().message().c_str());
        return cvalue();
    }
    cvalue *var = r.value();

    // 変更対象の要素を取得
    auto r2 = get_var_element(var, indexes);
    if (!r2) {
        clog::e("%s", r2.error().message().c_str());
        return cvalue();
    }
    cvalue *elem = r2.value();

    // 式を評価
    cvalue &&val = eval(expr);

    // 要素を更新
    *elem = val;

    return cvalue();
}

cvalue hii_driver::eval_op1stat(cnode const *node)
{
    auto const &varname = static_cast<cleaf const *>(node->left()->left())->sval();
    auto const &indexes = *static_cast<clist const *>(node->left()->right());

    clog::d("op1stat name=%s", varname.c_str());

    // 変数を取得
    auto r = get_var(varname);
    if (!r) {
        // TODO varnameが定数を指している場合、定数だから代入できないと補足説明したい
        clog::e("%s", r.error().message().c_str());
        return cvalue();
    }
    cvalue *var = r.value();

    // 変更対象の要素を取得
    auto r2 = get_var_element(var, indexes);
    if (!r2) {
        clog::e("%s", r2.error().message().c_str());
        return cvalue();
    }
    cvalue *elem = r2.value();

    // 変数の型チェック
    if (elem->type() != cvalue::INTEGER) {
        clog::e("変数%sはインクリメント/デクリメントできない型です (type=%s)", varname.c_str(), elem->type_name().c_str());
        return cvalue();
    }

    // 演算処理と変数の更新
    int res = elem->i();
    switch (node->op())
    {
    case OP_INC:
        res++;
        break;
    case OP_DEC:
        res--;
        break;
    default:
        clog::e("unknown operator (op=%s)", node->name());
        throw new logic_error(__func__);
    }
    *elem = cvalue(res);

    return cvalue();
}

cvalue hii_driver::eval_op2stat(cnode const *node)
{
    auto const &varname = static_cast<cleaf const *>(node->left()->left())->sval();
    auto const &indexes = *static_cast<clist const *>(node->left()->right());
    auto const &expr = node->right();

    clog::d("op2stat name=%s", varname.c_str());

    // 変数を取得
    auto r = get_var(varname);
    if (!r) {
        // TODO varnameが定数を指している場合、定数だから代入できないと補足説明したい
        clog::e("%s", r.error().message().c_str());
        return cvalue();
    }
    cvalue *var = r.value();

    // 変更対象の要素を取得
    auto r2 = get_var_element(var, indexes);
    if (!r2) {
        clog::e("%s", r2.error().message().c_str());
        return cvalue();
    }
    cvalue *elem = r2.value();

    // 要素の型チェック
    if (elem->type() != cvalue::INTEGER) {
        clog::e("要素が数値型ではないため計算できません (type=%s)", elem->type_name().c_str());
        return cvalue();
    }

    // 式の評価
    cvalue &&val = eval(expr);

    // 要素と式の型一致チェック
    if (elem->type() != val.type()) {
        clog::e("要素と式の型が一致しません (elem.type=%s, expr.type=%s)", elem->type_name().c_str(), val.type_name().c_str());
        return cvalue();
    }

    switch (node->op())
    {
    case OP_PLUS_ASSIGN:
        switch (elem->type())
        {
        case cvalue::INTEGER:
            {
                int i = elem->i() + val.i();
                *elem = cvalue(i);
            }
            break;
        case cvalue::STRING:
            {
                string &&s = elem->s() + val.s();
                *elem = cvalue(s);
            }
            break;
        default:
            clog::e("要素が%s型であるため+=演算子を適用できません", elem->type_name().c_str());
            break;
        }
        break;
    case OP_MINUS_ASSIGN:
        switch (elem->type())
        {
        case cvalue::INTEGER:
            {
                int i = elem->i() - val.i();
                *elem = cvalue(i);
            }
            break;
        default:
            clog::e("要素が%s型であるため-=演算子を適用できません", elem->type_name().c_str());
            break;
        }
        break;
    case OP_TIMES_ASSIGN:
        switch (elem->type())
        {
        case cvalue::INTEGER:
            {
                int i = elem->i() * val.i();
                *elem = cvalue(i);
            }
            break;
        default:
            clog::e("要素が%s型であるため*=演算子を適用できません", elem->type_name().c_str());
            break;
        }
        break;
    case OP_DIVIDE_ASSIGN:
        switch (elem->type())
        {
        case cvalue::INTEGER:
            {
                int i = elem->i() / val.i();
                *elem = cvalue(i);
            }
            break;
        default:
            clog::e("要素が%s型であるため/=演算子を適用できません", elem->type_name().c_str());
            break;
        }
        break;
    default:
        throw logic_error("invalid node type");
        break;
    }

    return cvalue();
}

cvalue hii_driver::eval_fun(cnode const *node)
{
    if (!def_fun(node)) {
        clog::e("関数%sはすでに定義されています", 
            static_cast<cleaf const *>(node->left())->sval().c_str());
    }
    return cvalue();
}

cvalue hii_driver::eval_ret(cnode const *node)
{
    auto const *expr = node->left();

    clog::d("eval_ret: Enter (has_expr=%d)", expr != nullptr);

    // TODO 関数コール内でない場合はエラー

    cvalue res;
    if (expr != nullptr) {
        res = eval(expr);
    }
    
    exit_fun_ = true;

    clog::d("eval_ret: result=%s", res.to_string().c_str());
    
    return res;
}

cvalue hii_driver::eval_if(cnode const *node)
{
    auto const *cond = node->left();
    auto const *stats = node->right()->left();
    auto const *elifs = node->right()->right();
    
    // condを評価
    cvalue &&res = eval(cond);

    bool result;
    switch (res.type()) {
    // 真:0以外 偽:0
    case cvalue::INTEGER:
        result = (res.i() != 0);
        break;
    // 真:""以外 偽:""
    case cvalue::STRING:
        result = (res.s() != "");
        break;
    default:
        clog::e("評価結果を真偽値に変換できません: type=%d", res.type());
        throw std::logic_error("評価結果を真偽値に変換できません");
    }

    if (result) {
        return eval(stats);
    } else {
        if (elifs == nullptr) {
            return cvalue();
        } else {
            return eval(elifs);
        }
    }
}

cvalue hii_driver::eval_sw(cnode const *node)
{
    // TODO 実装する
    auto const *sw_expr = node->left();
    auto const *cases = static_cast<clist const *>(node->right());

    cvalue sw_value = eval(sw_expr);

    cvalue res;
    cases->each([&](cnode const &n){
        auto const *expr = n.left();
        auto const *stats = n.right();

        if (expr != nullptr) {
            cvalue v = eval(expr);

            if (sw_value.type() != v.type()) {
                clog::e("swに指定された値の型(%s)とcaseの値の型(%s)が一致しません", 
                    sw_value.type_name().c_str(), v.type_name().c_str());
                return false;
            }

            // swの値とcaseの値を比較し、複文の実行有無を決める
            bool result;
            switch (sw_value.type()) {
            case cvalue::INTEGER:
                result = (sw_value.i() == v.i());
                break;
            case cvalue::STRING:
                result = (sw_value.s() == v.s());
                break;
            default:
                clog::e("swに指定された値とcaseの値を比較できません");
                return false;
            }
            // 比較結果が偽なら複文を実行しない、かつ、次のcaseを処理する
            if (!result) {
                return true;
            }
        }

        res = eval(stats);

        // すぐ抜けるのでexit,break,continueを処理する必要はない

        if (break_loop_) {
            clog::d("eval_sw: break_loop=true");
            assert(res.is_int());

            res = cvalue(res.i() - 1);
            if (res.i() <= 0) {
                break_loop_ = false;
            }
        }

        // case/else節を処理したので抜ける
        return false;
    });

    return res;
}

// TODO eval_call()内に埋め込まれた組込関数の処理を分離、他の関数と共通化するために使う
// 予めtopレベルのスコープに組込関数を登録しておく
// 登録の際に、OP_BUILTIN_CALLのみを持ったOP_STATSをノードとして指定する
/*
cvalue hii_driver::eval_builtin_call(cnode const *node)
{
    
}
*/

cvalue hii_driver::eval_call(cnode const *node)
{
    cvalue res;
    auto const &name = static_cast<cleaf const *>(node->left())->sval();
    auto const *exprs = static_cast<clist const *>(node->right());

    clog::d("eval_call: Enter");

    // 引数を評価
    // XXX 関数の名前解決前に引数を評価するの微妙...
    vector<cvalue> values;
    exprs->each([&](cnode const &n){
        cvalue &&v = eval(&n);

        clog::d("eval_call: eval arg_value=%s", v.to_string().c_str());

        values.push_back(v);
        return true;
    });

    // 関数呼び出しを実行
    // 組込関数->ユーザー定義関数の順に名前解決する
    if (name == "nop") {
        // do nothing
    }
    else if (name == "input") {
        int i;
        if (cin >> i) {
            res = cvalue(i);
        }
        else {
            cin.clear();
            string s;
            if (cin >> s) {
                res = cvalue(s);
            }
            else {
                assert(0);  // 標準入力から何らかの原因で文字列を読み取れなかった
            }
        }
    }
    else if (name == "p" || name == "print") {
        auto it = values.begin();

        if (it != values.end()) {
            cout << it->to_string();
            it++;
            for (; it != values.end(); it++) {
                cout << ", " << it->to_string();
            }
        }
        if (name == "p") {
            cout << endl;
        }
    }
    else if (name == "d") {
        auto it = values.cbegin();

        cout << "D: ";
        if (it != values.end()) {
            cout << it->to_string();
            it++;
            for (; it != values.end(); it++) {
                cout << ", " << it->to_string();
            }
        }
        cout << endl;
    }
    else if (name == "len") {
        if (values.size() < 1) {
            clog::e("引数の数が足りません (size=%zu)", values.size());
            return cvalue();
        }
        auto const &v = values.at(0);
        cvalue res;
        switch (v.type())
        {
        case cvalue::STRING:
            res = cvalue(static_cast<int>(v.s().size()));
            break;
        case cvalue::ARRAY:
            res = cvalue(static_cast<int>(v.a().size()));
            break;
        default:
            // XXX 文字列、配列以外は1としておく
            // XXX 型チェックで弾いた方が良い？
            res = cvalue(1);
            break;
        }
        return res;
    }
    else if (name == "assert") {
        if (values.size() < 2) {
            clog::e("引数の数が足りません (size=%zu)", values.size());
            return cvalue();
        }

        auto const &actual = values.at(0);
        auto const &expected = values.at(1);

        if (actual != expected) {
            // メッセージを取得
            string message;
            if (values.size() >= 3) {
                if (!values.at(2).is_str()) {
                    clog::e("メッセージ(引数3)の型が文字列ではありません", values.at(2).type_name().c_str());
                    message = values.at(2).to_string();
                } else {
                    message = values.at(2).s();
                }
            }

            clog::e("assert: failed: %s\n" \
                "      actual   : %s\n" \
                "      expected : %s\n", message.c_str(), 
                actual.to_string().c_str(), expected.to_string().c_str());
            return cvalue();
        }
    }
    else if (name == "_put_scopes") {
        print_scopes();
    }
    else {
        // スコープを上へ辿っていって関数を探す
        auto b = find_if(scopes_.rbegin(), scopes_.rend(), 
            [&](cscope &s){ return s.has_fun(name); });
        
        if (b == scopes_.rend()) {
            clog::e("関数%sは未定義です", name.c_str());
            return res;
        }

        cnode const *fun = b->get_fun(name);
        auto const *args = static_cast<clist const *>(fun->right()->left());
        auto const *attrs = static_cast<clist const *>(fun->right()->right()->left());
        auto const *stats = static_cast<clist const *>(fun->right()->right()->right());

        // scopes_      [u][v][w]
        //                  ^
        //                  \-- b
        // V
        // scopes       [u][v]
        // orig_scopes  [u][v][w]
        // V
        // scopes       [u][v][x]

        // スコープスタックを複製
        auto count = count_if(b, scopes_.rend(), [](cscope &){ return true; });
        vector<cscope> scopes(count);
        copy(b, scopes_.rend(), scopes.rbegin());

        // スコープスタックを退避・差し替え
        auto orig_scopes = scopes_;
        scopes_ = scopes;

        clog::d("eval_call: swap scopes: %zu -> %zu", orig_scopes.size(), scopes_.size());

        // 実引数の定義用のスコープを作成
        scopes_.push_back(cscope());

        clog::d("eval_call: create scope: %zu", scopes_.size());

        // 実引数を定義
        int i=0;
        args->each([&](cnode const &n) {
            auto const &name = static_cast<cleaf const &>(n).sval();
            scopes_.back().add_var(name, values[i], false);
            i++;
            return true;
        });
        if (attrs->left() != nullptr) {
            auto const &attr1 = static_cast<cleaf const *>(attrs->left())->sval();
            if (attr1 == "variadic") {
                auto const &name = static_cast<cleaf const *>(attrs->right()->left())->sval();
                
                // i..values.size()-1をrestとして配列化
                vector<cvalue> rest(values.begin()+i, values.end());
                clog::d("eval_call: rest.size %zu", rest.size());

                scopes_.back().add_var(name, cvalue(rest), false);
            }
        }

        // 複文を実行
        res = eval_stats(stats);
        clog::d("eval_call: eval_stats result=%s", res.to_string().c_str());

        if (exit_fun_) {
            clog::d("    exit_fun=true");
            exit_fun_ = false;
        }

        // スコープスタックを復元
        scopes_ = orig_scopes;

        clog::d("eval_call: restore scopes: %zu", scopes_.size());
    }

    return res;
}

cvalue hii_driver::eval_loop(cnode const *node)
{
    cnode const *id = node->left(); // nullable
    cnode const *arg1 = node->right()->left();
    cnode const *arg2 = node->right()->right()->left(); // nullable
    cnode const *stats = node->right()->right()->right();

    assert(arg1 != nullptr);
    assert(stats != nullptr);

    auto &&cnt_name = (id != nullptr ? static_cast<cleaf const *>(id)->sval() : "cnt");

    // loop(num|collection)のパラメータをloop(begin,end)に統一
    bool const is_range = (arg2 != nullptr);

    cvalue loop_times, loop_begin, loop_end;
    if (is_range) {
        loop_begin = eval(arg1);
        loop_end   = eval(arg2);
    } else {
        loop_times = eval(arg1);
    }
    int num_begin, num_end;
    if (is_range) {
        num_begin = loop_begin.i();
        num_end   = loop_end.i() + 1;
    } else {
        num_begin = 0;  // XXX ループ開始は0の方が良い？
        switch (loop_times.type()) {
        case cvalue::INTEGER:
            num_end = loop_times.i();
            break;
        case cvalue::STRING:
            num_end = loop_times.s().size();  // 文字数をループ回数とする
            break;
        case cvalue::ARRAY:
            num_end = loop_times.a().size();
            break;
        default:
            assert(0);
        }
    }

    cvalue res;
    for (int i = num_begin; i < num_end; i++)
    {
        // ループカウンタを定義
        cscope s;
        if (is_range || loop_times.is_int()) {
            s.add_var(cnt_name, cvalue(i), false);
        } else if (loop_times.is_str()) {
            // 文字列のn番目の文字をセット
            s.add_var(cnt_name, cvalue(string(1, loop_times.s().at(i))), false);
        } else if (loop_times.is_ary()) {
            s.add_var(cnt_name, loop_times.a(i), false);
        }

        scopes_.push_back(s);
        res = eval_stats(stats);
        scopes_.pop_back();

        if (exit_fun_) {
            clog::d("eval_loop: exit_fun=true");
            break;
        }

        if (cont_loop_) {
            clog::d("eval_loop: cont_loop=true");
            assert(res.is_int() || res.is_void());

            // ループカウンタを変更
            if (res.is_int()) {
                if (res.i() < num_begin || num_end+1 < res.i()) {
                    clog::i("cont文に指定されたループカウンタ値(%d)が%d..%d+1の範囲外です", res.i(), num_begin, num_end);
                }
                i = res.i() - 1;
            }

            cont_loop_ = false;
        }

        if (break_loop_) {
            clog::d("eval_loop: break_loop=true");
            assert(res.is_int());

            res = cvalue(res.i() - 1);
            if (res.i() <= 0) {
                break_loop_ = false;
            }
            break;
        }
    }

    return res;  // XXX 最後に実行した複文の値を返す
}

cvalue hii_driver::eval_cont(cnode const *node)
{
    auto *expr = node->left();

    cvalue res;
    if (expr != nullptr) {
        res = eval(expr);
        clog::d("eval_cont: result=%s", res.to_string().c_str());
        if (!res.is_int() && !res.is_void()) {
            clog::e("ループカウンタに数値以外(%s)が指定されています", res.to_string().c_str());
            res = cvalue();  // TODO 例外処理する
        }
    }

    cont_loop_ = true;

    return res;
}

cvalue hii_driver::eval_break(cnode const *node)
{
    auto *expr = node->left();

    cvalue res;
    if (expr != nullptr) {
        res = eval(expr);
        clog::d("eval_break: result=%s", res.to_string().c_str());
        if (!res.is_int() && !res.is_void()) {
            clog::e("数値以外(%s)が指定されています", res.to_string().c_str());
            res = cvalue();  // TODO 例外処理する
        }
        if (res.is_void()) {
            res = cvalue(1);
        }
    } else {
        res = cvalue(1);
    }

    break_loop_ = true;

    return res;
}

cvalue hii_driver::eval_op1(cnode const *node)
{
    auto &&l_value = eval(node->left());

    if (!l_value.is_int()) {
        std::runtime_error("左辺が数値ではありません");
    }

    int val;
    switch (node->op())
    {
    case OP_NEG:
        val = -l_value.i();
        break;
    default:
        std::logic_error("未定義の演算子です");
    }
    return cvalue(val);
}

cvalue hii_driver::eval_op2(cnode const *node)
{
    auto &&l_value = eval(node->left());
    auto &&r_value = eval(node->right());

    clog::d("eval_op2: Enter (lvalue=%s, rvalue=%s)", l_value.to_string().c_str(), r_value.to_string().c_str());

    cvalue res;
    switch (node->op())
    {
    case OP_PLUS:
        // TODO オーバーフローチェック
        // int + int -> int
        // int + str -> str
        // str + int -> str
        // str + str -> str
        if (l_value.is_int()) {
            if (r_value.is_int()) {
                // int + int
                res = cvalue(l_value.i() + r_value.i());
            } else if (r_value.is_str()) {
                // int + str
                size_t idx = 0;
                int r = std::stoi(r_value.s(), &idx);
                if (idx < r_value.s().size()) {
                    clog::e("右辺を数値に変換できません(rvalue=%s)", r_value.s().c_str());
                    std::runtime_error("右辺を数値に変換できません");
                }
                res = cvalue(l_value.i() + r);
            } else {
                clog::e("右辺が数値または文字列ではありません");
                std::runtime_error("右辺が数値または文字列ではありません");
            }
        } else if (l_value.is_str()) {
            if (r_value.is_int()) {
                // str + int
                size_t idx = 0;
                int l = std::stoi(l_value.s(), &idx);
                if (idx < l_value.s().size()) {
                    clog::e("左辺を数値に変換できません(rvalue=%s)", l_value.s().c_str());
                    std::runtime_error("左辺を数値に変換できません");
                }
                res = cvalue(l + r_value.i());
            } else if (r_value.is_str()) {
                // str + str
                res = cvalue(l_value.s() + r_value.s());
            } else {
                clog::e("右辺が数値または文字列ではありません");
                std::runtime_error("右辺が数値または文字列ではありません");
            }
        } else {
            clog::e("左辺が数値または文字列ではありません (type=%d, to_s=%s)", l_value.type(), l_value.to_string().c_str());
            std::runtime_error("左辺が数値または文字列ではありません");
        }
        break;
    case OP_MINUS:
        if (!l_value.is_int()) {
            clog::e("左辺が数値ではありません");
            std::runtime_error("左辺が数値ではありません");
        }
        if (!r_value.is_int()) {
            clog::e("右辺が数値ではありません");
            std::runtime_error("右辺が数値ではありません");
        }
        res = cvalue(l_value.i() - r_value.i());
        break;
    case OP_TIMES:
        if (!l_value.is_int()) {
            clog::e("左辺が数値ではありません");
            std::runtime_error("左辺が数値ではありません");
        }
        if (!r_value.is_int()) {
            clog::e("右辺が数値ではありません");
            std::runtime_error("右辺が数値ではありません");
        }
        res = cvalue(l_value.i() * r_value.i());
        break;
    case OP_DIVIDE:
        if (!l_value.is_int()) {
            clog::e("左辺が数値ではありません");
            std::runtime_error("左辺が数値ではありません");
        }
        if (!r_value.is_int()) {
            clog::e("右辺が数値ではありません");
            std::runtime_error("右辺が数値ではありません");
        }
        if (r_value.i() == 0) {
            clog::e("0除算エラーです");
            std::runtime_error("0除算エラーです");
        }
        res = cvalue(l_value.i() / r_value.i());
    case OP_MODULO:
        if (!l_value.is_int()) {
            clog::e("左辺が数値ではありません");
            std::runtime_error("左辺が数値ではありません");
        }
        if (!r_value.is_int()) {
            clog::e("右辺が数値ではありません");
            std::runtime_error("右辺が数値ではありません");
        }
        if (r_value.i() == 0) {
            clog::e("0除算エラーです");
            std::runtime_error("0除算エラーです");
        } else if (r_value.i() < 0) {
            clog::e("負の数で割った余りを求めることはできません");
            std::runtime_error("負の数で割った余りを求めることはできません");
        }
        res = cvalue(l_value.i() % r_value.i());
        break;
    case OP_EQ:
        res = cvalue(l_value == r_value);
        break;
    case OP_NEQ:
        res = cvalue(l_value != r_value);
        break;
    case OP_LT:
        if (l_value.type() != r_value.type()) {
            clog::e("右辺と左辺の型が異なります");
            std::runtime_error("右辺と左辺の型が異なります");
        }
        switch (l_value.type()) {
        case cvalue::INTEGER:
            res = cvalue(l_value.i() < r_value.i() ? 1 : 0);
            break;
        default:
            clog::e("比較不可能な型が指定されています");
            std::runtime_error("比較不可能な型が指定されています");
            break;
        }
        break;
    case OP_LTEQ:
        if (l_value.type() != r_value.type()) {
            clog::e("右辺と左辺の型が異なります");
            std::runtime_error("右辺と左辺の型が異なります");
        }
        switch (l_value.type()) {
        case cvalue::INTEGER:
            res = cvalue(l_value.i() <= r_value.i() ? 1 : 0);
            break;
        default:
            clog::e("比較不可能な型が指定されています");
            std::runtime_error("比較不可能な型が指定されています");
            break;
        }
        break;
    case OP_GT:
        if (l_value.type() != r_value.type()) {
            clog::e("右辺と左辺の型が異なります");
            std::runtime_error("右辺と左辺の型が異なります");
        }
        switch (l_value.type()) {
        case cvalue::INTEGER:
            res = cvalue(l_value.i() > r_value.i() ? 1 : 0);
            break;
        default:
            clog::e("比較不可能な型が指定されています");
            std::runtime_error("比較不可能な型が指定されています");
            break;
        }
        break;
    case OP_GTEQ:
        if (l_value.type() != r_value.type()) {
            clog::e("右辺と左辺の型が異なります");
            std::runtime_error("右辺と左辺の型が異なります");
        }
        switch (l_value.type()) {
        case cvalue::INTEGER:
            res = cvalue(l_value.i() >= r_value.i() ? 1 : 0);
            break;
        default:
            clog::e("比較不可能な型が指定されています");
            std::runtime_error("比較不可能な型が指定されています");
            break;
        }
        break;
    case OP_AND:
        switch (l_value.type()) {
        case cvalue::INTEGER:
            res = cvalue(l_value.i() != 0 ? 1 : 0);
            break;
        case cvalue::STRING:
            res = cvalue(l_value.s() != "" ? 1 : 0);
            break;
        case cvalue::ARRAY:
            res = cvalue(l_value.a().size() != 0 ? 1 : 0);
            break;
        default:
            clog::e("左辺に未知の型が指定されています(%d)", l_value.type());
            std::logic_error("左辺に未知の型が指定されています");
            break;
        }
        // 短絡評価 (左辺が真の場合のみ右辺を評価)
        if (res.i() != 0) {
            switch (r_value.type()) {
            case cvalue::INTEGER:
                res = cvalue(l_value.i() != 0 ? 1 : 0);
                break;
            case cvalue::STRING:
                res = cvalue(l_value.s() != "" ? 1 : 0);
                break;
            case cvalue::ARRAY:
                res = cvalue(l_value.a().size() != 0 ? 1 : 0);
                break;
            default:
                clog::e("左辺に未知の型が指定されています(%d)", l_value.type());
                std::logic_error("左辺に未知の型が指定されています");
                break;
            }
        }
        break;
    case OP_OR:
        switch (l_value.type()) {
        case cvalue::INTEGER:
            res = cvalue(l_value.i() != 0 ? 1 : 0);
            break;
        case cvalue::STRING:
            res = cvalue(l_value.s() != "" ? 1 : 0);
            break;
        case cvalue::ARRAY:
            res = cvalue(l_value.a().size() != 0 ? 1 : 0);
            break;
        default:
            clog::e("左辺に未知の型が指定されています(%d)", l_value.type());
            std::logic_error("左辺に未知の型が指定されています");
            break;
        }
        // 短絡評価 (左辺が偽の場合のみ右辺を評価)
        if (res.i() == 0) {
            switch (r_value.type()) {
            case cvalue::INTEGER:
                res = cvalue(l_value.i() != 0 ? 1 : 0);
                break;
            case cvalue::STRING:
                res = cvalue(l_value.s() != "" ? 1 : 0);
                break;
            case cvalue::ARRAY:
                res = cvalue(l_value.a().size() != 0 ? 1 : 0);
                break;
            default:
                clog::e("左辺に未知の型が指定されています(%d)", l_value.type());
                std::logic_error("左辺に未知の型が指定されています");
                break;
            }
        }
        break;
    case OP_ELEMENT:
        if (!l_value.is_str() && !l_value.is_ary()) {
            clog::e("左辺が文字列及び配列ではありません(%s)", l_value.to_string().c_str());
            return cvalue();
        }
        if (!r_value.is_int()) {
            clog::e("添字が数値ではありません(%s)", r_value.to_string().c_str());
            return cvalue();
        }
        {
            int i = r_value.i();
            if (l_value.is_str()) {
                size_t size = l_value.s().size();
                size_t index = cvalue::to_positive_index(i, size);
                if (index >= size) {
                    clog::e("添字(%d)が範囲外です", i);
                    break;
                }
                res= cvalue(string(1, l_value.s().at(index)));
            } else {
                size_t size = l_value.a().size();
                size_t index = cvalue::to_positive_index(i, size);
                if (index >= size) {
                    clog::e("添字(%d)が範囲外です", i);
                    break;
                }
                res = l_value.a(index);
            }
        }
        break;
    default:
        std::logic_error("未定義の演算子です");
    }

    return res;
}

/*
 * スライスの評価
 * 
 * スライスを取得可能な値をここではシーケンスと呼ぶ
 * 現状では以下の型の値が相当する
 * - 文字列
 * - 配列
 * 
 * アルゴリズム
 *
 * 1. シーケンスの要素数(size)を取得する
 *   1.1 文字列の場合
 *     (1) 内部プロパティ[size]の値をsizeに設定する
 *   1.2 配列の場合
 *     (1) 内部プロパティ[size]の値をsizeに設定する
 * 
 * 2. 始端の添字(start)を評価する
 *   2.1 始端の添字が指定されていない場合
 *     (1) startに0を設定する
 *   2.2 始端の添字が指定されている場合
 *     (1) 始端の添字を評価し、評価結果をstartに設定する
 *     (2) startが数値型ではない場合、エラーを返す
 *       TODO 静的に式の型チェックを行う
 *     TODO 添字の範囲外チェックを行う
 *     (3) startが負の場合、正の整数に変換する
 *       start = size + start
 * 
 * 3. 終端の添字(end)を評価する
 *   3.1 終端の添字が指定されていない場合
 *     (1) endに0を設定する
 *   3.2 終端の添字が指定されている場合
 *     (1) 終端の添字を評価し、評価結果をendに設定する
 *     (2) endが数値型ではない場合、エラーを返す
 *       TODO 静的に式の型チェックを行う
 *     TODO 添字の範囲外チェックを行う
 *     (3) endが負の場合、正の整数に変換する
 *       end = size + end
 * 
 * 4. 戻り値(res)を計算する
 *   4.1 start <= endの場合
 *     4.1.1 配列の場合
 *       (1) [start .. end]の範囲の要素をコピーした配列をresに設定する
 *     4.1.2 文字列の場合
 *       (1) [start .. end]の範囲の文字をコピーした文字列をresに設定する
 *   4.2 start >  endの場合
 *     4.2.1 配列の場合
 *       (1) [end .. start]の範囲の要素をコピーした配列をresに設定する
 *     4.2.2 文字列の場合
 *       (1) [end .. start]の範囲の文字をコピーした文字列をresに設定する
 * 
 * Note:
 * - 現状ではシーケンスのコピーが作られる
 */
cvalue hii_driver::eval_slice(cnode const *node)
{
    auto const *obj_expr = node->left();
    auto const *start_expr = node->right()->left();
    auto const *end_expr = node->right()->right();

    assert(obj_expr != nullptr);

    auto &&obj = eval(obj_expr);

    if (!obj.is_ary() && !obj.is_str()) {
        clog::e("配列,文字列以外の型からスライスを作成することはできません (type=%s)", obj.type_name().c_str());
        return cvalue();
    }

    size_t size;
    switch (obj.type())
    {
    case cvalue::STRING:
        size = obj.s().size();
        break;
    case cvalue::ARRAY:
        size = obj.a().size();
        break;
    }

    if (size == 0) {
        clog::e("サイズが0の配列または文字列からスライスを作ることはできません");
        return cvalue();
    }

    // start, endを評価

    // [:end]   -> 0, end
    // [start:] -> start, size-1
    size_t start, end;

    if (start_expr == nullptr) {
        start = 0;
    } else {
        auto &&v = eval(start_expr);
        if (!v.is_int()) {
            clog::e("スライスの始端が数値型ではありません (type=%s)", v.type_name().c_str());
            return cvalue();
        }
        start = cvalue::to_positive_index(v.i(), size);
        if (start >= size) {
            clog::e("スライスの始端が範囲外です (index=%s, size=%zu)", v.i(), size);
            return cvalue();
        }
    }

    if (end_expr == nullptr) {
        end = size - 1;
    } else {
        auto &&v = eval(end_expr);
        if (!v.is_int()) {
            clog::e("スライスの終端が数値型ではありません (type=%s)", v.type_name().c_str());
            return cvalue();
        }
        end = cvalue::to_positive_index(v.i(), size);
        if (end >= size) {
            clog::e("スライスの始端が範囲外です (index=%s, size=%zu)", v.i(), size);
            return cvalue();
        }
    }

    cvalue res;
    if (start <= end) {
        if (obj.is_ary()) {
            res = cvalue(vector<cvalue>(obj.a().begin() + start, obj.a().begin() + end + 1));
        } else {
            res = cvalue(string(obj.s().begin() + start, obj.s().begin() + end + 1));
        }
    } else {
        size_t rstart = size - 1 - start;
        size_t rend   = size - 1 - end;
        if (obj.is_ary()) {
            res = cvalue(vector<cvalue>(obj.a().rbegin() + rstart, obj.a().rbegin() + rend + 1));
        } else {
            res = cvalue(string(obj.s().rbegin() + rstart, obj.s().rbegin() + rend + 1));
        }
    }

    return res;
}

cvalue hii_driver::eval_id(cnode const *node)
{
    auto const &name = static_cast<cleaf const *>(node)->sval();

    clog::d("eval_id: Enter (var=%s)", name.c_str());

    // 変数を探す
    auto b = find_if(scopes_.rbegin(), scopes_.rend(), 
        [&](cscope &s){ return s.has_var(name); });
   
    if (b != scopes_.rend()) {
        // 変数の値を返す
        auto &&v = b->get_var(name);

        clog::d("eval_id: retval=%s", v.to_string().c_str());
        return v;
    }

    // 変数が見つからないので、以降は関数を探す

    // 組込関数かチェック
    // XXX トップレベルスコープに予め組込関数を定義できれば、この処理は以降の処理と共通化できる
    if (name == "nop" ||
        name == "input" ||
        name == "p" ||
        name == "print" ||
        name == "d" ||
        name == "len" ||
        name == "assert" ||
        name == "_put_scopes")
    {
        // call_statのノード構造に変換する
        cnode n(OP_CALL, new cleaf(*static_cast<cleaf const *>(node)), new clist(OP_EXPRS));

        // 関数コールを評価し、評価結果を返す
        cvalue res = eval_call(&n);

        clog::d("eval_id: eval_call result=%s", res.to_string().c_str());

        return res;
    }

    // 関数を探す
    auto b2 = find_if(scopes_.rbegin(), scopes_.rend(), 
        [&](cscope &s){ return s.has_fun(name); });
   
    if (b2 != scopes_.rend()) {
        // call_statのノード構造に変換する
        cnode n(OP_CALL, new cleaf(*static_cast<cleaf const *>(node)), new clist(OP_EXPRS));

        // 関数コールを評価
        cvalue res = eval_call(&n);

        clog::d("eval_id: eval_call result=%s", res.to_string().c_str());

        return res;
    }

    clog::e("変数or関数%sが定義されていません\n", name.c_str());
    throw std::runtime_error("変数or関数が定義されていません");
}

cvalue hii_driver::eval_array(cnode const *node)
{
    clist const &exprs = *static_cast<clist const *>(node->left());

    // 配列リテラル(exprs)を評価し、値に変換する
    vector<cvalue> elements;

    exprs.each([&](cnode const &n){
        elements.push_back(eval(&n));
        return true;
    });

    return cvalue(elements);
}

cvalue hii_driver::eval_dict(cnode const *node)
{
    clist const &pair_list = *static_cast<clist const *>(node->left());

    map<string, cvalue> pairs;

    pair_list.each([&](cnode const &n){
        string const & key = static_cast<cleaf const *>(n.left())->sval();
        auto && value = eval(n.right());
        pairs.insert(make_pair(key, value));
        return true;
    });

    return cvalue(pairs);
}

cvalue hii_driver::eval_dictitem(cnode const *node)
{
    string const & varname = static_cast<cleaf const *>(node->left())->sval();
    string const & key = static_cast<cleaf const *>(node->right())->sval();

    clog::d("eval_dictitem: Enter (var=%s, key=%s)", varname.c_str(), key.c_str());

    // 変数を探す
    auto b = find_if(scopes_.rbegin(), scopes_.rend(), 
        [&](cscope &s){ return s.has_var(varname); });

    if (b == scopes_.rend()) {
        clog::e("変数%sが見つかりません", varname.c_str());
        return cvalue();
    }

    auto &&v = b->get_var(varname);

    if (!v.is_dict()) {
        clog::e("指定された変数は辞書型ではありません (type=%s)", v.type_name().c_str());
        return cvalue();
    }

    if (v.d().find(key) == v.d().end()) {
        clog::e("指定したキーが見つかりません");
        return cvalue();
    }

    return v.d(key);
}

cvalue hii_driver::eval_str(cnode const *node)
{
    auto const &str = static_cast<cleaf const *>(node)->sval();

    // エスケープシーケンスの処理
    stringstream ss;
    auto it = str.begin();
    while (it != str.end())
    {
        if (*it != '\\') {
            ss << *it;
            it++;
            continue;
        }

        it++;
        if (it == str.end()) {
            clog::i("文字列の末尾のエスケープ文字は削除されます");
            break;
        }

        switch (*it)
        {
        case 't': ss << '\t'; it++; break;
        case 'r': ss << '\r'; it++; break;
        case 'n': ss << '\n'; it++; break;
        case '{':
            // 埋め込まれた変数を展開する
            it++;
            if (it == str.end()) {
                ss << '{';
                break;
            }
            {
                stringstream id;
                while (it != str.end() && *it != '}') {
                    id << *it;
                    it++;
                }
                if (id.str().size() == 0) {
                    // '{'の直後に'}'が来た
                    ss << "{}";
                    break;
                }
                if (it == str.end()) {
                    // '{'の後に'}'がなく文字列が終了した
                    ss << '{' << id.str();
                    break;
                }
                it++;
                // TODO 変数or関数を評価
                cleaf n(OP_ID, new string(id.str()));
                cvalue v = eval_id(&n);
                ss << v.to_string();
            }
            break;
        default:
            ss << *it;
            it++;
            break;
        }
    }

    return cvalue(ss.str());

    //return cvalue(str.sval());
}

cvalue hii_driver::eval(cnode const *node)
{
    clog::d("eval: node=%s", node->name());

    switch (node->op())
    {
    case OP_STATS:
        return eval_stats(node);
    case OP_ASSIGN:
        return eval_assign(node);
    case OP_REASSIGN:
        return eval_reassign(node);
    case OP_INC:
    case OP_DEC:
        return eval_op1stat(node);
    case OP_PLUS_ASSIGN:
    case OP_MINUS_ASSIGN:
    case OP_TIMES_ASSIGN:
    case OP_DIVIDE_ASSIGN:
        return eval_op2stat(node);
    case OP_FUN:
        return eval_fun(node);
    case OP_RET:
        return eval_ret(node);
    // 条件分岐
    case OP_IF:
        return eval_if(node);
    case OP_ELIF:
        return eval_if(node);
    case OP_ELSE:
        return eval(node->left());
    case OP_SW:
        return eval_sw(node);
    case OP_CALL:
    case OP_CALLEXPR:
        return eval_call(node);
    case OP_LOOP:
        return eval_loop(node);
    case OP_CONT:
        return eval_cont(node);
    case OP_BREAK:
        return eval_break(node);
    // 1項演算子
    case OP_NEG:
        return eval_op1(node);
    // 2項演算子
    case OP_PLUS:
    case OP_MINUS:
    case OP_TIMES:
    case OP_DIVIDE:
    case OP_MODULO:
    case OP_EQ:
    case OP_NEQ:
    case OP_LT:
    case OP_LTEQ:
    case OP_GT:
    case OP_GTEQ:
    case OP_AND:
    case OP_OR:
    case OP_ELEMENT:
        return eval_op2(node);
    case OP_SLICE:
        return eval_slice(node);
    case OP_LCOMMENT:
    case OP_TCOMMENT:
        return cvalue();
    case OP_MCOMMENT:
        {
            auto const *comments = static_cast<clist const *>(node);
            comments->each([](cnode const &n) {
                auto &&v = static_cast<cleaf const &>(n);
                cout << "  \e[32m" << v.sval() << "\e[00m" << endl;
                return true;
            });
        }
        return cvalue();
    case OP_RCOMMENT:
        {
            auto const &comment = static_cast<cleaf const *>(node)->sval();
            cout << "\e[42m" << comment << "\e[00m" << endl;
        }
        return cvalue();
    // 定数、変数
    case OP_ID:
    case OP_VAR:
        return eval_id(node);
    // リテラル
    case OP_INT:
        return cvalue(static_cast<cleaf const *>(node)->ival());
    case OP_STR:
        return eval_str(node);
    case OP_ARRAY:
        return eval_array(node);
    case OP_DICT:
        return eval_dict(node);
    case OP_DICTITEM:
        return eval_dictitem(node);
    default:
        std::fprintf(stderr, "評価方法が未定義です: op=%s\n", node->name());
        throw std::logic_error("評価方法が未定義です");
    }
}

