#include <string>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include <cstdio>
#include "hii_driver.h"
#include "parser.hh"
#include "cnode.h"
#include "cleaf.h"
#include "cscope.h"
#include "clog.h"
#include "cvalue.h"

using std::cin;
using std::cout;
using std::endl;
using std::vector;
using std::string;
using my::clog;

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

bool hii_driver::resolve_names(cnode &node)
{
    vector<cscope *> scopes;

    auto on_enter = [&scopes](cnode::cctrl &ctrl, cnode &n) -> bool {
        switch (n.op())
        {
        case OP_STATS:
            // スコープ追加
            scopes.push_back(new cscope());
            cout << "on_enter: " << n.name() << " nest=" << scopes.size() << endl;
            break;
        case OP_ASSIGN:
            {
                // 変数の二重定義をチェック
                auto &name = static_cast<cleaf &>(*n.left()).sval();
                cout << "on_enter: " << n.name() << " " << name << endl;
                if (scopes.back()->has_var(name)) {
                    cout << "E: 変数" << name << "はすでに定義されています" << endl;
                    //return false;
                    return true;
                }

                // 変数を定義
                scopes.back()->add_var(name, nullptr);  // value is dummy
            }
            break;
        case OP_FUN:
            {
                // 関数の二重定義をチェック
                auto &name = static_cast<cleaf &>(*n.left()).sval();
                cout << "on_enter: " << n.name() << " " << name << endl;
                if (scopes.back()->has_fun(name)) {
                    cout << "E: 関数" << name << "はすでに定義されています" << endl;
                    //return false;
                    return true;
                }

                // 関数を定義
                scopes.back()->add_fun(name, nullptr);  // value is dummy

                // 引数を定義
                auto &args = static_cast<clist &>(*n.right()->left());
                args.each([&](cnode &a) {
                    auto &argname = dynamic_cast<cleaf &>(a).sval();
                    scopes.back()->add_var(argname, nullptr);  // value is dummy
                    return true;
                });
            }
            break;
        case OP_ARGS:
            ctrl.skip_children();
            break;
        case OP_LOOP:
            {
                if (n.left() != nullptr) {
                    auto const &name = static_cast<cleaf const *>(n.left())->sval();
                    cout << "on_enter: " << name << endl;
                    // 変数を定義
                    scopes.back()->add_var(name, nullptr);  // value is dummy
                }
            }
            break;
        case OP_ID:
            // 識別子が定義されているかチェック
            // XXX 仮引数はチェックしないようにする!
            //     -> OP_ARGSだったら子ノードを探索しない、という処理が必要?
            {
                auto &name = static_cast<cleaf &>(n).sval();
                cout << "on_enter: " << n.name() << " " << name << endl;

                // 組込関数かチェック
                if (name == "nop" ||
                    name == "input" ||
                    name == "p" ||
                    name == "print" ||
                    name == "_put_scopes") {
                    break;
                }

                bool found = false;
                for (auto it = scopes.rbegin(); it != scopes.rend(); it++)
                {
                    auto s = *it;
                    if (s->has_var(name) || s->has_fun(name)) {
                        found = true;
                        break;
                    }
                }
                if (!found) {
                    cout << "E: 識別子" << name << "は定義されていません" << endl;
                    //return false;
                    return true;
                }
            }
            break;
        }
        return true;
    };

    auto on_leave = [&scopes](cnode::cctrl &ctrl, cnode &n) -> bool {
        switch (n.op())
        {
        case OP_STATS:
            // スコープ削除
            delete scopes.back();
            scopes.pop_back();
            cout << "on_leave: " << n.name() << " nest=" << scopes.size() << endl;
            break;
        }
        return true;
    };

    // 名前解決
    node.each(on_enter, on_leave);

    // 後始末
    for (auto s : scopes) {
        delete s;
    }
    scopes.clear();
}

bool hii_driver::def_var(cnode const *node)
{
    auto &name = static_cast<cleaf const *>(node->left())->sval();
    auto expr = node->right();

    clog::d("assign name=%s", name.c_str());

    // 二重定義は不可
    if (scopes_.back()->has_var(name))
        return false;

    // 上位のスコープに定義されている場合は警告を出す
    bool defined = false;
    auto it = scopes_.rbegin();
    it++; // skip current scope
    for (; it != scopes_.rend(); it++) {
        if ((*it)->has_var(name)) {
            defined = true;
            break;
        }
    }
    if (defined) {
        fprintf(stderr, "W: 上位のスコープで定義された変数%sが隠されます\n", name.c_str());
    }

    // 式を評価
    cleaf * res = new cleaf(); // TODO newさせない
    *res = eval(expr);

    // 現在のスコープに変数を登録
    scopes_.back()->add_var(name, res);

    return true;
}

bool hii_driver::def_fun(cnode const *node)
{
    auto &name = static_cast<cleaf const *>(node->left())->sval();

    clog::d("declfun name=%s", name.c_str());

    // 二重定義は不可
    if (scopes_.back()->has_fun(name))
        return false;

    // 上位のスコープに定義されている場合は警告を出す
    bool defined = false;
    auto it = scopes_.rbegin();
    it++; // skip current scope
    for (; it != scopes_.rend(); it++) {
        if ((*it)->has_fun(name)) {
            defined = true;
            break;
        }
    }
    if (defined) {
        fprintf(stderr, "W: 上位のスコープで定義された関数%sが隠されます\n", name.c_str());
    }

    // TODO 名前解決を行う

    scopes_.back()->add_fun(name, node);

    return true;
}

cleaf hii_driver::eval_stats(cnode const *node, cscope *scope)
{
    auto stats = static_cast<clist const *>(node);
 
    // スコープを追加
    cscope *s = (scope != nullptr ? scope : new cscope());
    scopes_.push_back(s);

    clog::d("create scope %zu", scopes_.size());
   
    // 複文を実行
    cleaf res;
    stats->each([&](cnode const & n) {
        res = eval(&n);
        if (exit_fun_) {
            return false;
        }
        return true;
    });

    // スコープを削除
    delete s;
    scopes_.pop_back();

    clog::d("delete scope %zu", scopes_.size());

    return res;
}

cleaf hii_driver::eval_assign(cnode const *node)
{
    cleaf res {};

    if (!def_var(node)) {
        std::printf("E: 変数%sはすでに定義されています\n", 
            static_cast<cleaf const *>(node->left())->sval().c_str());
    }
    return res;
}

cleaf hii_driver::eval_fun(cnode const *node)
{
    cleaf res {};

    if (!def_fun(node)) {
        std::printf("E: 関数%sはすでに定義されています\n", 
            static_cast<cleaf const *>(node->left())->sval().c_str());
    }
    return res;
}

cleaf hii_driver::eval_ret(cnode const *node)
{
    auto *expr = node->left();

    cleaf res;
    if (expr != nullptr) {
        res = eval(expr);
    }

    // XXX 関数コール内でない場合はエラー
    
    exit_fun_ = true;
    
    return res;
}

cleaf hii_driver::eval_if(cnode const *node)
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
        std::fprintf(stderr, "評価結果を真偽値に変換できません: op=%s\n", res.name());
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

// TODO eval_call()内に埋め込まれた組込関数の処理を分離、他の関数と共通化するために使う
// 予めtopレベルのスコープに組込関数を登録しておく
// 登録の際に、OP_BUILTIN_CALLのみを持ったOP_STATSをノードとして指定する
/*
cleaf hii_driver::eval_builtin_call(cnode const *node)
{
    cleaf res {};

    return res;
}
*/

cleaf hii_driver::eval_call(cnode const *node)
{
    cleaf res {};

    auto &name = static_cast<cleaf const *>(node->left())->sval();
    auto exprs = static_cast<clist const *>(node->right());

    // 引数を評価
    // XXX 関数の名前解決前に引数を評価するの微妙...
    // XXX 何度もコピーが発生している
    vector<cleaf> values;
    exprs->each([&](cnode const & n){
        cleaf v = eval(&n);
        values.push_back(v);
        return true;
    });

    // 関数呼び出しを実行
    // input  : vector<cleaf> values, context(scopes_, exit_fun_, eval_stats)
    // output : cleaf
    // 組込関数->ユーザー定義関数の順に名前解決する
    if (name == "nop") {
        // do nothing
    }
    else if (name == "input") {
        int ival;
        if (cin >> ival) {
            res = cleaf(OP_INT, ival);
        }
        else {
            cin.clear();
            string *sval = new string();
            if (cin >> *sval) {
                res = cleaf(OP_STR, sval);
            }
            else {
                assert(0);  // BUG!!
            }
        }
    }
    else if (name == "p" || name == "print") {
        auto it = values.begin();

        if (it != values.end()) {
            switch (it->op()) {
            case OP_INT:
                cout << it->ival();
                break;
            case OP_STR:
                cout << it->sval();
                break;
            default:
                cout << "<unknown>";
                break;
            }
            it++;
            for (; it != values.end(); it++) {
                cout << " ";
                switch (it->op()) {
                case OP_INT:
                    cout << it->ival();
                    break;
                case OP_STR:
                    cout << it->sval();
                    break;
                default:
                    cout << "<unknown>";
                    break;
                }
            }
        }
        if (name == "p") {
            cout << endl;
        }
    }
    else if (name == "_put_scopes") {
        print_scopes();
    }
    else {
        // スコープを上へ辿っていって関数を探す
        auto b = find_if(scopes_.rbegin(), scopes_.rend(), 
            [&](cscope * s){ return s->has_fun(name); });
        
        if (b == scopes_.rend()) {
            std::fprintf(stderr, "E: 関数%sは未定義です\n", name.c_str());
            return res;
        }

        cnode const * fun = (*b)->get_fun(name);
        auto args = static_cast<clist const *>(fun->right()->left());
        auto stats = static_cast<clist const *>(fun->right()->right());

        // scopes_      [u][v][w]
        //                  ^
        //                  \-- b
        // V
        // scopes       [u][v]
        // orig_scopes  [u][v][w]
        // V
        // scopes       [u][v][x]

        // スコープスタックを複製
        auto count = count_if(b, scopes_.rend(), [](cscope *){ return true; });
        vector<cscope *> scopes(count);
        copy(b, scopes_.rend(), scopes.rbegin());

        // スコープスタックを退避・差し替え
        auto orig_scopes = scopes_;
        scopes_ = scopes;

        clog::d("swap scopes: %zu -> %zu", orig_scopes.size(), scopes_.size());

        // 実引数の定義用のスコープを作成
        cscope *s = new cscope();
        scopes_.push_back(s);

        // 実引数を定義
        int i=0;
        args->each([&](cnode const &n) {
            auto &name = static_cast<cleaf const &>(n).sval();
            cleaf *value = new cleaf(values[i]);
            scopes_.back()->add_var(name, value);
            i++;
            return true;
        });

        // 複文を実行
        res = eval_stats(stats, s);
        if (res.has_sval()) {
            clog::d("ret %s (%s)", res.name(), res.sval());
        } else {
            clog::d("ret %s (%d)", res.name(), res.ival());
        }
        if (exit_fun_) {
            exit_fun_ = false;
        }

        // スコープスタックを復元
        scopes_ = orig_scopes;

        clog::d("restore scopes: %zu", scopes_.size());
    }

    return res;
}

cleaf hii_driver::eval_loop(cnode const *node)
{
    cnode const *id = node->left(); // nullable
    cnode const *arg1 = node->right()->left();
    cnode const *arg2 = node->right()->right()->left(); // nullable
    cnode const *stats = node->right()->right()->right();

    assert(arg1 != nullptr);
    assert(stats != nullptr);

    string cnt_name = (id != nullptr ? static_cast<cleaf const *>(id)->sval() : "cnt");

    // loop(num|collection)のパラメータをloop(begin,end)に統一
    // XXX なんでres_が頭についているのか忘れた
    bool const is_range = (arg2 != nullptr);
    cleaf res_times, res_begin, res_end;
    if (is_range) {
        res_begin = eval(arg1);
        res_end   = eval(arg2);
    } else {
        res_times = eval(arg1);
    }
    int num_begin, num_end;
    if (is_range) {
        num_begin = res_begin.ival();
        num_end   = res_end.ival();
    } else {
        num_begin = 1;
        switch (res_times.op()) {
        case OP_INT:
            num_end = res_times.ival();
            break;
        case OP_STR:
            num_end = res_times.sval().size();
            break;
        default:
            assert(0);
        }
    }

    cleaf res;
    for (int i = num_begin; i <= num_end; i++)
    {
        // ループカウンタを定義
        cscope *s = new cscope();
        if (is_range || res_times.op() == OP_INT) {
            s->add_var(cnt_name, new cleaf(OP_INT, i));
        } else {
            // 文字列のn番目の文字をセット
            s->add_var(cnt_name, new cleaf(OP_STR, res_times.sval().at(i-1)));
        }

        res = eval_stats(stats, s);
        if (exit_fun_) {
            break;
        }
    }

    return res;
}

cleaf hii_driver::eval_op1(cnode const *node)
{
    auto l_value = eval(node->left());

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

cleaf hii_driver::eval_op2(cnode const *node)
{
    auto l_value = eval(node->left());
    auto r_value = eval(node->right());

    if (l_value.op() != OP_INT) {
        std::runtime_error("左辺が数値ではありません");
    }
    if (r_value.op() != OP_INT) {
        std::runtime_error("右辺が数値ではありません");
    }

    int val;
    switch (node->op())
    {
    case OP_PLUS:
        // TODO オーバーフローチェック
        val = l_value.ival() + r_value.ival();
        break;
    case OP_MINUS:
        val = l_value.ival() - r_value.ival();
        break;
    case OP_TIMES:
        val = l_value.ival() * r_value.ival();
        break;
    case OP_DIVIDE:
        // TODO 0除算エラーチェック
        val = l_value.ival() / r_value.ival();
    case OP_MODULO:
        // TODO 0除算エラーチェック, 負数チェック
        val = l_value.ival() % r_value.ival();
        break;
    case OP_EQ:
        val = (l_value.ival() == r_value.ival() ? 1 : 0);
        break;
    case OP_NEQ:
        val = (l_value.ival() != r_value.ival() ? 1 : 0);
        break;
    case OP_LT:
        val = (l_value.ival() < r_value.ival() ? 1 : 0);
        break;
    case OP_LTEQ:
        val = (l_value.ival() <= r_value.ival() ? 1 : 0);
        break;
    case OP_GT:
        val = (l_value.ival() > r_value.ival() ? 1 : 0);
        break;
    case OP_GTEQ:
        val = (l_value.ival() >= r_value.ival() ? 1 : 0);
        break;
    case OP_AND:
        val = (l_value.ival() && r_value.ival() ? 1 : 0);
        break;
    case OP_OR:
        val = (l_value.ival() || r_value.ival() ? 1 : 0);
        break;
    default:
        std::logic_error("未定義の演算子です");
    }
    cleaf res = cleaf(OP_INT, val);

    return res;
}

cleaf hii_driver::eval_id(cnode const *node)
{
    auto &name = static_cast<cleaf const *>(node)->sval();

    auto b = find_if(scopes_.rbegin(), scopes_.rend(), 
        [&](cscope *s){ return s->has_var(name); });
   
    if (b != scopes_.rend()) {
        return (*b)->get_var(name);
    }

    // 組込関数かチェック
    if (name == "nop" ||
        name == "input" ||
        name == "p" ||
        name == "print" ||
        name == "_put_scopes")
    {
        // call_stmtのノード構造に変換する
        cnode n(OP_CALL, new cleaf(*static_cast<cleaf const *>(node)), new clist(OP_EXPRS));

        // 関数コールを評価
        cleaf res = eval_call(&n);
        return res;
    }

    // 変数が見つからない場合は関数を探す
    auto b2 = find_if(scopes_.rbegin(), scopes_.rend(), 
        [&](cscope *s){ return s->has_fun(name); });
   
    if (b2 != scopes_.rend()) {
        // call_stmtのノード構造に変換する
        cnode n(OP_CALL, new cleaf(*static_cast<cleaf const *>(node)), new clist(OP_EXPRS));

        // 関数コールを評価
        cleaf res = eval_call(&n);
        return res;
    }

    std::fprintf(stderr, "E: 変数or関数%sが定義されていません\n", name.c_str());
    throw std::runtime_error("変数or関数が定義されていません");
}

cleaf hii_driver::eval_array(cnode const * node)
{
    cleaf res;

    clist const & exprs = *static_cast<clist const *>(node->left());

    // TODO 配列リテラル(exprのリスト)を評価し、値に変換する
    vector<cleaf> elements;

    exprs.each([&](cnode const &n){
        elements.push_back(eval(&n));
        return true;
    });

    // cvalue ary(elements);
    // return ary;

    return res;
}

cleaf hii_driver::eval(cnode const *node)
{
    clog::d("eval %s", node->name());

    cvalue v;

    switch (node->op())
    {
    case OP_STATS:
        return eval_stats(node);
    case OP_ASSIGN:
        return eval_assign(node);
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
    case OP_CALL:
        return eval_call(node);
    case OP_LOOP:
        return eval_loop(node);
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
        return eval_op2(node);
    case OP_CALLEXPR:
        return eval_call(node);
    case OP_LCOMMENT:
        return cleaf();
    case OP_MCOMMENT:
        {
            auto comments = static_cast<clist const *>(node);
            comments->each([](cnode const &n) {
                auto v = static_cast<cleaf const &>(n);
                cout << v.sval() << endl;
                return true;
            });
        }
        return cleaf();
    // 変数
    case OP_ID:
        return eval_id(node);
    // リテラル
    case OP_INT:
    case OP_STR:
        return *static_cast<cleaf const *>(node);
    case OP_ARRAY:
        return eval_array(node);
    default:
        std::fprintf(stderr, "評価方法が未定義です: op=%s\n", node->name());
        throw std::logic_error("評価方法が未定義です");
    }
}

/**
 * 意味解析＆実行
 */
bool hii_driver::exec(const string &f)
{
    if (!parse(f)) return false;

    // グローバルスコープを生成
    // -> ASTのルート要素OP_STATSの評価時に生成される

    // 名前解決
    clog::d("### resolve names: starting ###");
    resolve_names(*ast_);
    clog::d("### resolve names: finished ###");

    // 意味解析＆実行
    clog::d("### eval ast: starting ###");
    eval(ast_);
    clog::d("### eval ast: finished ###");

    if (exit_fun_) {
        clog::e("関数の外でret文が使用されています");
    }

    // 後始末
    assert(scopes_.empty());

    delete ast_;
    ast_ = nullptr;

    return true;
}

