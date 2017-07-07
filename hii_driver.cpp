#include <string>
#include <iostream>
#include <iomanip>
#include <algorithm>
#include <cassert>
#include <cstdio>
#include <sstream>
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
using std::stringstream;
using my::clog;

/**
 * 字句解析,構文解析,意味解析,インタプリタ実行を行う
 */
bool hii_driver::exec(const string &f)
{
    if (!parse(f)) return false;

    // グローバルスコープを生成
    // -> ASTのルート要素OP_STATSの評価時に生成される
    // XXX グローバルスコープに予め何か定義したい場合に不便...

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
    assert(scopes_.empty());

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

/*
void hii_driver::def_builtin_fun(cscope &scope, string const &name, clist *args)
{
    cnode *body = new clist(OP_STATS);
    body->add(new cleaf(OP_BUILTIN_CALL, new string(name)));
    
    cnode *fun = new cnode(OP_FUN, new cleaf(OP_ID, new string(name)), new cnode(OP_NODE, args, body));
    
    builtin_functions_.push_back(fun);
    scope.add_fun(name, fun);
}

void hii_driver::def_builtin_functions(cscope &scope)
{
    cnode *nop_args = new clist(OP_ARGS);
    def_builtin_fun(scope, "nop", nop_args);

    cnode *input_args = new clist(OP_ARGS);
    def_builtin_fun(scope, "input", input_args);

    cnode *p_args = new clist(OP_ARGS);  // 可変長引数
    def_builtin_fun(scope, "p", p_args);

    cnode *print_args = new clist(OP_ARGS);  // 可変長引数
    def_builtin_fun(scope, "print", print_args);

    cnode *_put_scopes_args = new clist(OP_ARGS);
    def_builtin_fun(scope, "_put_scopes", _put_scopes_args);
}

void hii_driver::free_builtin_functions()
{
    for (auto &&e : builtin_functions_) {
        // 解放
        delete e;
    }
}
*/

/* 名前解決
 */
bool hii_driver::resolve_names(cnode &node)
{
    vector<cscope> scopes;

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
                auto const &name = static_cast<cleaf &>(*n.left()).sval();
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

cvalue hii_driver::eval_reassign(cnode const *node)
{
    auto const &name = static_cast<cleaf const *>(node->left())->sval();
    auto const *expr = node->right();

    clog::d("reassign name=%s", name.c_str());

    // 変数を探す
    bool defined = false;
    auto it = scopes_.rbegin();
    for (; it != scopes_.rend(); it++) {
        if (it->has_var(name)) {
            defined = true;
            break;
        }
    }
    if (!defined) {
        clog::e("変数%sは定義されていません", name.c_str());
        return cvalue();
    }
    if (!it->is_writable(name)) {
        clog::e("%sは定数のため代入できません", name.c_str());
        return cvalue();
    }

    // 式を評価
    cvalue val = eval(expr);

    // 変数の値を上書き
    it->add_var(name, val, true);

    return cvalue();
}

cvalue hii_driver::eval_lsassign(cnode const *node)
{
    auto &&varname = static_cast<cleaf const *>(node->left())->sval();
    auto const *index_expr = node->right()->left();
    auto const *value_expr = node->right()->right();

    clog::d("lsassign name=%s", varname.c_str());

    // 変数を取得

    bool defined = false;
    auto it = scopes_.rbegin();
    for (; it != scopes_.rend(); it++) {
        if (it->has_var(varname)) {
            defined = true;
            break;
        }
    }
    if (!defined) {
        clog::e("変数%sは定義されていません", varname.c_str());
        return cvalue();
    }
    if (!it->is_writable(varname)) {
        clog::e("%sは定数のため代入できません", varname.c_str());
        return cvalue();
    }

    auto &var = it->get_var(varname);

    // 配列型変数かチェック
    if (!var.is_ary()) {
        clog::e("%sは配列型変数ではありません", varname.c_str());
        return cvalue();
    }

    // 添字の評価
    cvalue index = eval(index_expr);

    // 添字の型チェック
    if (!index.is_int()) {
        clog::e("添字の式の型(%s)が数値ではありません", index.type_name().c_str());
        return cvalue();
    }

    // 添字のオーバーフローチェック
    if (index.i() < 0 || var.a().size() <= index.i()) {
        clog::e("添字(%d)が配列(0..%zu)の範囲外です", index.i(), var.a().size());
        return cvalue();
    }

    // 値の評価
    cvalue value = eval(value_expr);

    // XXX 配列の型と値の型の比較
    // ==> 現状では配列型が要素の型を保持していないため比較できない

    // 要素の変更
    var.a().at(index.i()) = value;

    return cvalue();
}

cvalue hii_driver::eval_op1stmt(cnode const *node)
{
    auto const &varname = static_cast<cleaf const *>(node->left())->sval();

    clog::d("op1stmt name=%s", varname.c_str());

    // 変数を取得

    bool defined = false;
    auto it = scopes_.rbegin();
    for (; it != scopes_.rend(); it++) {
        if (it->has_var(varname)) {
            defined = true;
            break;
        }
    }
    if (!defined) {
        clog::e("変数%sは定義されていません", varname.c_str());
        return cvalue();
    }
    if (!it->is_writable(varname)) {
        clog::e("%sは定数のため代入できません", varname.c_str());
        return cvalue();
    }

    auto &var = it->get_var(varname);
    
    if (!var.is_int()) {
        clog::e("変数%sは数値型ではないため計算できません", varname.c_str());
        return cvalue();
    }

    int res;
    switch (node->op())
    {
    case OP_INC:
        res = var.i() + 1;
        it->add_var(varname, cvalue(res), true);
        break;
    case OP_DEC:
        res = var.i() - 1;
        it->add_var(varname, cvalue(res), true);
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
    else if (name == "_put_scopes") {
        print_scopes();
    }
    else {
        // スコープを上へ辿っていって関数を探す
        auto b = find_if(scopes_.rbegin(), scopes_.rend(), 
            [&](cscope &s){ return s.has_fun(name); });
        
        if (b == scopes_.rend()) {
            std::fprintf(stderr, "E: 関数%sは未定義です\n", name.c_str());
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
        num_end   = loop_end.i();
    } else {
        num_begin = 1;  // XXX ループ開始は0の方が良い？
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
    for (int i = num_begin; i <= num_end; i++)
    {
        // ループカウンタを定義
        cscope s;
        if (is_range || loop_times.is_int()) {
            s.add_var(cnt_name, cvalue(i), false);
        } else if (loop_times.is_str()) {
            // 文字列のn番目の文字をセット
            s.add_var(cnt_name, cvalue(loop_times.s().at(i-1)), false);
        } else if (loop_times.is_ary()) {
            s.add_var(cnt_name, loop_times.a().at(i-1), false);
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
            clog::e("ループカウンタに数値以外(%s)が指定されています", res.to_string().c_str());
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
        if (l_value.type() != r_value.type()) {
            clog::e("右辺と左辺の型が異なります");
            std::runtime_error("右辺と左辺の型が異なります");
        }
        switch (l_value.type()) {
        case cvalue::INTEGER:
            res = cvalue(l_value.i() == r_value.i() ? 1 : 0);
            break;
        case cvalue::STRING:
            res = cvalue(l_value.s() == r_value.s() ? 1 : 0);
            break;
        default:
            clog::e("比較不可能な型が指定されています");
            std::runtime_error("比較不可能な型が指定されています");
            break;
        }
        break;
    case OP_NEQ:
        if (l_value.type() != r_value.type()) {
            clog::e("右辺と左辺の型が異なります");
            std::runtime_error("右辺と左辺の型が異なります");
        }
        switch (l_value.type()) {
        case cvalue::INTEGER:
            res = cvalue(l_value.i() != r_value.i() ? 1 : 0);
            break;
        case cvalue::STRING:
            res = cvalue(l_value.s() != r_value.s() ? 1 : 0);
            break;
        default:
            clog::e("比較不可能な型が指定されています");
            std::runtime_error("比較不可能な型が指定されています");
            break;
        }
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
        // TODO 文字列にも[]演算子を適用可能にする
        if (!l_value.is_ary()) {
            clog::e("左辺が配列ではありません(type=%d)", l_value.type());
            return cvalue();
        }
        if (!r_value.is_int()) {
            clog::e("添字が数値ではありません");
            return cvalue();
        }
        res = l_value.a().at(r_value.i());
        break;
    default:
        std::logic_error("未定義の演算子です");
    }

    return res;
}

cvalue hii_driver::eval_slice(cnode const *node)
{
    auto const *array_expr = node->left();
    auto const *start_expr = node->right()->left();
    auto const *end_expr = node->right()->right();

    assert(array_expr != nullptr);

    // arrayを評価
    cvalue array_value = eval(array_expr);

    if (!array_value.is_ary() && array_value.is_str()) {
        clog::e("配列,文字列以外(%s)からスライスを作成することはできません", array_value.type_name().c_str());
        return cvalue();
    }

    auto &array = array_value.a();

    // start, endを評価

    // [:end]   -> 0, end
    // [start:] -> start, length-1
    cvalue start_value;
    cvalue end_value;

    if (start_expr == nullptr) {
        start_value = cvalue(0);
    } else {
        start_value = eval(start_expr);
        if (!start_value.is_int()) {
            clog::e("スライスの開始番号(%s)が数値ではありません", start_value.type_name().c_str());
        }
        if (start_value.i() < 0) {
            // len + (-1) = len - 1
            // len + (-2) = len - 2
            start_value = cvalue(array.size() + start_value.i());
        }
    }

    if (end_expr == nullptr) {
        end_value = cvalue(array.size() - 1);
    } else {
        end_value = eval(end_expr);
        if (!end_value.is_int()) {
            clog::e("スライスの終了番号(%s)が数値ではありません", end_value.type_name().c_str());
        }
        if (end_value.i() < 0) {
            // len + (-1) = len - 1
            // len + (-2) = len - 2
            end_value = cvalue(array.size() + end_value.i());
        }
    }

    clog::d("start=%d, end=%d", start_value.i(), end_value.i());

    cvalue res;
    if (start_value.i() <= end_value.i()) {
        res = cvalue(vector<cvalue>(array.begin() + start_value.i(), array.begin() + end_value.i() + 1));
    } else {
        int rstart = array.size() - 1 - start_value.i();
        int rend   = array.size() - 1 - end_value.i();
        clog::d("rstart=%d, rend=%d", rstart, rend);
        res = cvalue(vector<cvalue>(array.rbegin() + rstart, array.rbegin() + rend + 1));
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
        name == "_put_scopes")
    {
        // call_stmtのノード構造に変換する
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
        // call_stmtのノード構造に変換する
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

cvalue hii_driver::eval_str(cnode const *node)
{
    auto const &str = static_cast<cleaf const *>(node)->sval();

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
    case OP_LSASSIGN:
        return eval_lsassign(node);
    case OP_INC:
    case OP_DEC:
        return eval_op1stmt(node);
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
    default:
        std::fprintf(stderr, "評価方法が未定義です: op=%s\n", node->name());
        throw std::logic_error("評価方法が未定義です");
    }
}

