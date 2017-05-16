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
    ast_ = ast;

    cout << "DEBUG: set_ast" << endl;
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
                });
            }
            break;
        case OP_ARGS:
            ctrl.skip_children();
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
                    name == "p" ||
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

    cout << "### resolve_names ###" << endl;
    node.each(on_enter, on_leave);
    cout << "####################" << endl;

    for (auto s : scopes) {
        delete s;
    }
    scopes.clear();
}

bool hii_driver::def_var(cnode const *node)
{
    auto &name = static_cast<cleaf const *>(node->left())->sval();
    auto expr = node->right();

    cout << "DEBUG: define variable \"" << name << "\"" << endl;

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

    cout << "declare function \"" << name << "\"" << endl;

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

cleaf hii_driver::eval_stats(cnode const *node)
{
    auto stats = static_cast<clist const *>(node);
 
    // スコープを追加
    cscope *s = new cscope();
    scopes_.push_back(s);

    cout << "DEBUG: create scope: level=" << scopes_.size() << endl;
   
    // 複文を実行
    stats->each([&](cnode const & n) {
        eval(&n);
    });

    // スコープを削除
    delete s;
    scopes_.pop_back();

    cout << "DEBUG: delete scope: level=" << scopes_.size() << endl;

    return cleaf();
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
    });

    // 関数呼び出しを実行
    // 組込関数->ユーザー定義関数の順に名前解決する
    if (name == "nop") {
        // do nothing
    }
    else if (name == "p") {
        for (auto const & v : values) {
            switch (v.op()) {
            case OP_INT:
                cout << v.ival();
                break;
            case OP_STR:
                cout << v.sval();
                break;
            default:
                cout << "<unknown>";
                break;
            }
            cout << " ";
        }
        cout << endl;
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

        // スコープスタックを複製
        auto count = count_if(b, scopes_.rend(), [](cscope *){ return true; });
        vector<cscope *> scopes(count);
        copy(b, scopes_.rend(), scopes.rbegin());

        // スコープスタックを退避・差し替え
        auto orig_scopes = scopes_;
        scopes_ = scopes;

        cout << "DEBUG: swap scopes: level=" << orig_scopes.size() << " to " << scopes_.size() << endl;

        // スコープを追加
        cscope *s = new cscope();
        scopes_.push_back(s);

        cout << "DEBUG: create scope: level=" << scopes_.size() << endl;
       
        // 実引数を定義
        int i=0;
        args->each([&](cnode const &n){
            auto &name = static_cast<cleaf const &>(n).sval();
            cleaf * value = new cleaf(values[i]);
            scopes_.back()->add_var(name, value);
            i++;
        });

        // 複文を実行
        stats->each([&](cnode const & n) {
            eval(&n);
        });

        // スコープを削除
        delete s;
        scopes_.pop_back();

        cout << "DEBUG: delete scope: level=" << scopes_.size() << endl;
        
        // スコープスタックを復元
        scopes_ = orig_scopes;

        cout << "DEBUG: restore scopes: level=" << scopes_.size() << endl;
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

cleaf hii_driver::eval_id(cnode const *node)
{
    auto &name = static_cast<cleaf const *>(node)->sval();

    auto b = find_if(scopes_.rbegin(), scopes_.rend(), 
        [&](cscope *s){ return s->has_var(name); });
    
    if (b == scopes_.rend()) {
        std::fprintf(stderr, "E: 変数%sは定義されていません\n", name.c_str());
        throw std::runtime_error("変数が定義されていません");
    }

    return (*b)->get_var(name);
}

cleaf hii_driver::eval(cnode const *node)
{
    std::printf("DEBUG: eval %s\n", node->name());

    switch (node->op())
    {
    case OP_STATS:
        return eval_stats(node);
    // 現在のスコープに変数を追加する
    case OP_ASSIGN:
        return eval_assign(node);
    // 関数の場合は現在のスコープに関数を追加する
    case OP_FUN:
        return eval_fun(node);
    // 条件分岐
    case OP_IF:
        return eval_if(node);
    case OP_ELIF:
        return eval_if(node);
    case OP_ELSE:
        return eval(node->left());
    case OP_CALL:
        return eval_call(node);
    // 1項演算子
    case OP_NEG:
        return eval_op1(node);
    // 2項演算子
    case OP_PLUS:
    case OP_MINUS:
    case OP_TIMES:
    case OP_DIVIDE:
        return eval_op2(node);
    case OP_LCOMMENT:
        return cleaf();
    case OP_MCOMMENT:
        {
            auto comments = static_cast<clist const *>(node);
            comments->each([](cnode const &n){
                auto v = static_cast<cleaf const &>(n);
                cout << v.sval() << endl;
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
    cout << "DEBUG: resolve names: starting" << endl;
    resolve_names(*ast_);
    cout << "DEBUG: resolve names: finished" << endl;

    // 意味解析＆実行
    cout << "DEBUG: eval ast: starting" << endl;
    //eval(ast_);
    cout << "DEBUG: eval ast: finished" << endl;

    // 後始末
    if (!scopes_.empty()) {
        cout << "BUG!! scopestack.size != 0: size=" << scopes_.size() << endl;
    }
    delete ast_;
    ast_ = nullptr;

    return true;
}

