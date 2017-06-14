# 開発メモ

[ ] "..."は省略記号(elipsis)

[ ] 合計を求める計算をどう実装するか
  - mutable x = 0 みたいな変数定義文を追加する？
    - x = 1 のような代入が、定数定義と区別がつかなくてコードが読みづらくなりそう
    - 再代入を禁止すれば良いのでは？
    - 再代入したいケースが出てくるのでは？
    - ++, +=のような演算のみサポートする？

[ ] x..yを範囲リテラルにする

[ ] cnodeを基本とすると書きづらい、Lispみたいにリストを基本としたらどうか?
  - new cnode(OP\_xxx, $1, new cnode(OP\_NODE, $2, $3))
  - これが
  - new clist(OP\_xxx, $1, $2, $3)
  - こう書ける
  - リストのインデックスで参照するより名前で参照したいなら、cmapみたいなクラスを作る
  - new cmap(OP\_xxx, {"name",$1}, {"args",$2}, {"body",$3})

[ ] @name : アノテーション
[ ] @require : 変数への型制約の追加
[ ] ? : Option＜T＞型

    # @deplicated @param T
    # @require x in 1..10
    # @require s 
    fun f s @str, x? @int
        p T + x
    end

[ ] eachメソッドから呼ぶコールバックにはイテレータを渡す
  - cnode::each(std::function＜void(cnode::iterator it)＞ &f)
  - it-＞exit, it-＞skip\_children等のメソッドを生やす

[x] 評価結果を格納するクラスをcleafからcvalueに置き換える
  - 評価結果に特有の実装をcleafに持ち込まない
    - 配列は構文木(exprs)と評価結果(cvalue配列)でデータが異なる

[ ] コーディングルールを決める
  - 修飾子の位置
    - 型 const \* 識別子
    - 型 const & 識別子
    - virtual, static, inlineは先頭に付ける
    - const > volatileの順で付ける
  - アクセス修飾子の順序
    - public > protected > privateの順
  - メンバの順序
    - コンストラクタ、デストラクタ、代入演算子を最初に書く
  - インクルードガード
    - ifndefの代わりに可能な限りpragma onceを使う
  - キャスト
    - dynamic\_cast vs static\_cast

[x] (Mac) capslockをctrlに変更する
  - システム環境設定 > キーボード > 修飾キーから変更可能

[ ] cnode, clist, cleafにto\_string()を実装したい
  - AST上でノードの表示に使う
  - rvalue reference難しい

[ ] 式の中に関数コールを書くための検討
  - ","を使う文の中に関数コールが出てくると結合順位の問題が生じる
  - 例えば、
  - 関数コールの引数に関数コールを書いた場合
      - f g 2,3
      - ==> `f (g 2),3`なのか`f (g 2,3)`なのか

[ ] cnode, clistを統一できないか？
  - 基本はrightで連結していくリスト構造
  - 値がある場合はleftに格納
  - assign   : name, expr
  - fun      : name, args, stats
  - if       : cond, stats[, cond, stats[, ...]]  # elseはcond=trueとすれば良さそう
  - call     : name[, expr[, ...]]
  - mcomment : lcomment[, lcomment[, ...]]
  - lcomment : string
  - args     : [name[, name[, ...]]]

[ ] scopestack操作を関数化する
  - 行われている操作を調べる
  - has\_var
  - add\_var
  - has\_fun
  - add\_fun
  - get\_fun
  - new cscope, scopestack.push\_back
  - scopestack.size
  - delete scope, scopestack.pop\_back
  - scopestackの末尾から先頭へのループ

[-] cleafの扱いを見直す
  - OP\_ARRAYはcleaf?

[ ] 構文エラー、意味エラーの例外クラスを定義する

[ ] Brainfu\*kを実装する


[ ] 関数の引数の数をチェックしたい

[ ] 組込関数は最初のスコープに定義する

[ ] 繰り返し文を実装する
  - [x] 構文検討する
  - [x] n回繰り返し
      loop {number} ~ end
      loop {id}, {number} ~ end
  - [ ] 条件が真の間繰り返し
      loop {condition} ~ end
      loop {id}, {condition} ~ end
  - [ ] 各要素に対して繰り返し
      loop {collection} ~ end

[ ] コメント非表示オプションをつける
  - コメントをevalしてprintするオプションがあると良さそう

loop 10
end
loop i, 10
end
loop i, i<10
end
loop ls
end
loop 1..10
end

[x] 色をつけたい (curses/ANSI escape sequenceを使う?)

[x] 関数内では関数が定義される前に定義済みだった識別子のみ参照できること
  - 関数が定義されているスコープ(同階層)の識別子が不正に参照できてしまう

[x] 定義されている識別子一覧を表示するデバッグ関数\_put\_scopesを定義する

[x] 上位のスコープの関数を参照できるようにする
[x] 上位のスコープの変数を参照できるようにする

[x] 関数がすでに上位のスコープで定義されている場合に警告を表示
[x] 変数がすでに上位のスコープで定義されている場合に警告を表示

[x] 変数の評価を実装する
[x] 組込関数p(print)を実装する

[x] レキシカルスコープを実装する
[x] stats評価時にスコープを生成
  生成タイミング
  - unit
  - fun
  - if-then
  - elif-then
  - else
  eval\_stats()を実装する

[x] expr式のevalを実装する
[x] if文のevalを実装する

[-] cnodeをcleafにキャストしてvalueにアクセスするのを簡単にしたい
  - castをラップするのは臭いものに蓋をしてるだけなのでやめておく

[x] 組込関数コールを実行するメソッドを実装する

[x] 変数定義する実行メソッドを実装する
  スコープを扱う必要がある
  - グローバルスコープ
  - ブロックスコープ (fun, if)

  変数登録時
  - 一番近いスコープに登録する
  変数参照時
  - 一番近いスコープの変数を参照する

[x] スコープを生成する


[x] OP\_FUNの子ノードを処理する
  -> ASTを(ASTよりも実行しやすい)中間言語に変換する必要がある

[-] (vim)関数一覧を表示してジャンプしたい

[x] eval()の実装状況
  - STATS      : ok (値なし; 最後の文の値で上書きされる?)
  - ASSIGN     : ok (値なし)
  - FUN        : ok (値なし)
  - IF         : ok (最後に実行した文の値?)
  - ELIF       : ok (最後に実行した文の値?)
  - ELSE       : ok (最後に実行した文の値?)
  - CALL       : ok (値なし)
  - 単項演算子 : ok (値あり)
  - 二項演算子 : ok (値あり)
  - リテラル   : ok (値あり)
  - ID         : ok (値あり)
  - コメント   : ok (値なし)

[x] スコープの生成タミングを決める
  - プログラム実行時, 関数定義時(fun)
  - 簡単のため条件分岐(if)ではスコープを生成しない

[ ] eval()の中で探索とres返却を行ってるのは何かおかしい気がする

[ ] 構文の記載順序を統一する
  - 静的->動的の並び
  - assign, fun, if, call

[ ] eval: 探索から実行処理を分ける
  - assign : 何もしない
  - if     : stats, elifsを探索 ... 完全には分けられない...
  - fun    : 何もしない
  - call   : exprsを探索

[ ] eval()の戻り値をcleafからcvalueに変える
[ ] 値型のcvalueクラスを定義する
[ ] eval()に失敗したら例外を投げる
  - 評価結果を戻り値で返したい (戻り値同士の計算が楽)
  - 失敗するケースとは?
    - 変数未定義、未初期化変数の参照、演算不可能な型同士の演算
    - 滅多に起きないなら、例外の方が扱いやすい

[ ] まずインタプリタを作る
  ノードに到達した時の処理
  stats:
    何もしない (子ノードの処理に任せる)
  stat:
    comment:
      何もしない
    assign:
      式を評価して値を得る
      変数を現在のスコープに追加
    if:
      条件式を評価して値を得る
      値が真の場合:
        statsを実行 (list(stats))
      値が偽の場合:
        elifsを実行 (list(elifs))
    fun:
      関数を登録
      # 注:インタプリタなので、関数コールしないと構文チェックされない
    call:
      各式を評価
      関数を実行
  elif:
  else:
    スコープを追加
    statsを実行
    スコープを削除
  
  必要な関数
    - 変数/関数の定義済みチェック(name) -> bool
    - 変数を登録(name, value)
    - 関数を登録(name, node)
    - 式を評価(node) -> value
    - 関数実行(name, values)
  
  スコープについて
    - スコープはいつdeleteするか？
    - スコープに追加したノードはいつ破棄するか？

[-] 中間言語のデータ構造を考える
  -> まだバイトコード生成を考える段階ではないため保留
  program:
    - @stats: cstats
    - run: stats->run();
  cstats:
    - @stats: vector<cstat>
    - run: for (auto && stat : stats) stat->run();
  cstat:
    ccomment:
      - @value: string
      - run: ;
    cassign:
      - @name: string
      - @expr: cexpr
      - run: cexprの評価結果を変数にセットする
    cif:
      - @cond: cexpr
      - @then: stats
      - @elifs: vector<cif>
      - @else: stats
      - run: cexprを評価, 真ならthenを実行, 偽ならelifsを順に実行, 全て偽ならelseを実行
    cfun:
      - @name: string
      - @args: 
      - @stats: cstats
      - run: 関数を定義
    ccall:
      - @name: string
      - @exprs: vector<cexpr>
      - run: 各cexprを評価, 評価結果を引数にして関数をコール
  cexpr:
    - @expr: cnode
    - eval: 

[x] list()に子ノードを探索するかどうかcallbackから通知する仕組みを入れる
  -> goの戻り値でboolを返すのが良さそう

[-] (cnode::listを拡張して)スコープを抜けるときcallbackを呼ぶ
  -> OP\_FUNで子ノードに到達したときにスコープを作成し、子ノードの処理後、
     親ノードに戻ったときにスコープを破棄しようと考えた
  -> よく考えたらOP\_FUNの子ノードを別のcallbackで処理することがおかしい
  -> OP\_FUNに到達したときにcallback内で子ノードを処理する

[x] ASTを走査して変数定義・関数定義時に名前を表示する
  どうやって走査するか？
  -> 深さ優先探索(cnode::list)を使う

[x] 文字列リテラルを追加する

[x] driver.errorをprintf仕様にする
  note: printfのformatに文字列リテラル以外を指定すると警告が出る(?)

[-] 直接コードを渡すオプションをつける
  -> lexerにファイルではなく文字列を渡す方法がわからないため中止

[-] intをnumに改名する
  -> トークンはintとして区別した方がよい

[x] リストを簡潔に表示したい

現状のclistではリストがネストしている場合に
ネストしていることがわからない
-> リストのトップノードだけOP\_LISTにする ?





TODO
Makefile改善
- [x] 生成ファイルをobjディレクトリに移動するようMakefileを変更する
- [x] 依存関係を自動生成する

方針
- cnodeだけでAST構築する (2017/03/08)

クラス構成
- cnode        ... ノード
- cleaf        ... 
- clist        ... 
- cfn          ... 何者？







・fnをどう実装するか？
  ・stateをノードとして扱い、
    構文解析時にfn/if/loopをノードにしてしまう
      
      cfn [name]
       |-- arglist
       |-- cbody
      cif
       |-- cond
       |-- node
            |-- cbody
            |-- node
                 |-- celsif
                 |    |-- cond
                 |    |-- node
                 |         |-- cbody
                 |-- else
      cloop
       |-- cond
       |-- cbody

  ・意味解析アクションでモードを切り替え、
    以降の構文マッチ時にノードにしてしまう
      
      // fnの場合
      state2 : "fn" "id" args  { mode=fnにし、"id"と"args"を保存 }
      /* 以降の構文マッチ時には実行せずにノードにして保存
       * 以下の構文はエラー
       * "fn"
       */
      state2 : "ret" expr  { mode=globalにし、exprを保存 }

      // loopの場合
      state2 : "loop" expr  { stackにloopを積み、exprを保存 }
      /* 以降の構文マッチ時には実行せずにノードにして保存
       * 以下の構文はエラー
       * "fn"
       */
      state2 : "end"


    スクリプト全体が__main(args)関数内と考える
      - スコープを関数と同様に扱える
      - スクリプトをretで終了できる
      - コマンドライン引数にargsでアクセスできる

    cvar
      - name : string
      - init_val : expr

    cblock::type : enum
      + FN // fn ... end
      + IF // if ... elsif ... end
      + LOOP // loop ... end
    
    cblock
      + vars : map<string, cvar>
      + stats : vector<cnode>
    
    cfunc : cblock
      + name : string
      + args : arglist
    
    cif : cblock
      + cond : cnode
    
    cloop::type : enum
      + TIMES // ex) n
      + COND  // ex) n >= 0

    cloop : block
      - type : cloop::type
      - cond : cnode
      - index : int

    calc_driver
      + blocklev : int = nest_blocks.count()
      - nest_blocks : vector<cblock>


## TODO

実装関連
- 関数定義
- 関数コール/ret
- 関数式
- if/else if/else/end
- loop/end

その他
- Makefileで再ビルドしないようにする
- QuickRun的物がほしい
- autocompleteほしい
- Githubで管理すべき


## 方針

1. 新規機能の追加
  - 辞書データ型

2. 既存機能の強化
  - JavaScriptのように扱いやすい配列
  - 正規表現リテラル
  - 強力な文字列処理ライブラリ


## 既知の問題
・1文字キーワード"l"を定義すると、変数名とかに"l"を使えなくなる
  => "l", "p"は組込関数にする


## ノード種別 (括弧は予定)
  演算子  ：OP_NEG, OP_PLUS, OP_MINUS, OP_TIMES, OP_DIVIDE
  識別子  ：OP_NAMEVAL
  リテラル：OP_IVAL, (OP_SVAL)
  関数    ：OP_FNSTATE, OP_FNARGS, OP_FNBODY, OP_FNFOOT


## ノード毎のメンバ使用状況

- cnodeはノードまたはリーフのいずれかとして使う
- ノードはleftまたはrightに子ノードを持っている
- リーフはleftとrightがnullptr

=> ノード/リーフ判定メソッドを用意する

下記表からvalue,stringは殆ど使われていない
ノードの探索にも関係ないので、派生クラスのメンバにしてもいいかも
op      | left   | right  | value | string || extends
--------|--------|--------|-------|--------||---------
NEG     | node   | null   | -     | null   || -
PLUS    | node   | node   | -     | null   || -
NAME    | null   | null   | -     | string || -
IVAL    | null   | null   | any   | null   || -
FNSTATE | FNBODY | FNFOOT | -     | null   || name, args
FNARGS  | null   | null   | -     | -      || args
FNBODY  | node   | node   | -     | -      || -
FNFOOT  | node   | null   | -     | null   || -
EMPTY   | null   | null   | -     | null   || -


## ノード構造

fn(stmt)
 |-- args
 |    |-- 
 |    |-- 
 |-- rettype
 |-- fnbody
      |-- 

NEG  : node -+- node(IVAL)
             L- null
PLUS : node -+- node(IVAL | NAMEVAL | NEG | PLUS...)
             L- node(IVAL | NAMEVAL | NEG | PLUS...)
IVAL : node(IVAL)
ARGS : node -+- node(ARGS)
             L- node(NAMEVAL) | null


