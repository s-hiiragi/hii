# 開発メモ

[ ] 気になる: OP\_ELEMENTはeval\_op2とeval\_op2statで異なる評価が行われている
    - eval\_op2だと、評価結果はcvalue(コピー値)となる

[ ] 負の添字->正の添字変換ロジックを一箇所にまとめる
    - 現状の変換箇所
        - OP_ELEMENT    eval_op2 (e = a[index])
        - OP_SLICE      eval_slice (e = a[start:end])
        - OP_xxx_ASIGN  eval_op2stat (e[index] = value)
    - まとめ先
        - cvalue::a(int index);

[ ] cvalue.a(index)を定義する (cvalue.a().at(index)と書くのが煩わしいため)

[ ] stmtをstatに統一する part2

[x] 式をimmutableな式とmutableな式に分ける
    - 現状の問題
        - `a = $b + $c`といった式を書けてしまう
        - この式以降に変数b,cが変更されても、定数aの値は変わらない
        - 定数aが定義された時点で値が決まるので問題ない?

[x] 以下の代入文を統一する
    - reassign\_stmt
    - lsassign\_stmt

[ ] デバッグプリント関数dを追加

[ ] switch文でbreak使えて欲しいかもしれない

[x] loop文に文字列を指定した時にループカウンタに数字が入るバグを修正

[ ] cont,break,retは戻り値を制御に使うので、
    これらのフラグを見て関数を抜けるときは適切な戻り値を返す必要がある
    - statsを評価する関数で注意する必要がある

[ ] cvalueをbool値に変換するロジックを1箇所にまとめる

[ ] exit,break,continueを処理するロジックを1箇所にまとめる

[x] stmtをstatに統一する

[ ] Brainfuckプログラムを作成する
    - [x] 同じロジックをC++で作成する

[ ] 定数の値に変更が及ばないようにする
    - 定数=変数+定数のような演算をチェックする
        - 変数=変数+定数なら許可
        - 変数を含む式は変数にしか代入不可

[x] 配列のスライス式を実装する
    - 構文
        - a[start:end]
    - 配列のcvalueを拡張
        - start_pos, end_pos

[x] 配列の要素の代入文を実装する
    - 構文は`$a[index] = value`

[x] break文を実装する
    - 構文は`break {expr}`
    - 多重ループを抜ける場合、抜けたいループ数を引数に指定する

[x] continue文を実装する
    - 構文は`cont {expr}`
    - 引数には次のループカウンタを指定する

[-] 1行に複数の文を書く
    - 構文は`stat1: stat2: ...`
    - `def f x: ret x*2: end`の"end"を書くのが冗長
      専用の構文を用意した方が短く書けそう
      ==> 却下する

[ ] \(Vim) IMEのモード、挿入モードによってカーソルの色とか何か変えたい

[ ] シェルコマンド実行文を追加する
    - 構文はbashと同じ$(command)

[x] 範囲コメント文を追加する
    - #begin .. #endを範囲コメントとする
        - (問題) #endがない場合、#beginが行コメントとなり、構文エラーとならない
        - Rubyの場合、=begin ... =endという構文になっているため、この問題は生じない
          先頭の=を消すとbegin .. endとなり、実行コードにできる
        - C/C++の場合、/* .. */が範囲コメントとなる
          /* .. //*/ と書き、開始記号を//*とすることで実行コードにできる
        - #begin \n hoge \n fuga \n #end を正しく解析できない問題がある
          #begin .. #end の".."部分に任意の文字を許容すると、#begin .. #endが複数ある場合に正しく解析できない
          ==> 正しく解析するために構文を#{ .. #}に変更

[ ] cnodeにトークン位置を記録する
    - filename, line, column

[ ] 構文エラー位置を表示する
    - ファイル名が表示できない問題がある(yy:parser::errorのlocにnullが設定されている)
    - 行、列番号も1のまま

[ ] 単項マイナス問題
    - `x - a`がx(-a)の構文木になってしまう問題
        - aが整数の場合は、(Haskellみたいに?)`-a`を1つの負数としてスキャナで処理すれば良さそう

[-] "..."は省略記号(elipsis)

[ ] 合計を求める計算をどう実装するか
  - mutable x = 0 みたいな変数定義文を追加する？
    - x = 1 のような代入が、定数定義と区別がつかなくてコードが読みづらくなりそう
    - 再代入を禁止すれば良いのでは？
    - 再代入したいケースが出てくるのでは？
    - ++, +=のような演算のみサポートする？

[x] 変数を追加する
  - `$id`
  - cscope::add\_var/idを用意
  - eval\_assignで重複定義エラーを避ける & 登録時にadd\_var/idを呼ぶ
  - 課題: 親スコープと子スコープで同じ名前の変数を定義しようとすると別物になってしまう
    - 代入文が変数定義なのか再代入なのか区別できないため、新たな変数を定義してしまう
    - 解決策1: `var $v = expr`のような変数定義文を導入し、再代入と区別する
    - 解決策2: 代入先の変数が上位スコープに定義されていたら再定義しない
    - 解決策3: 再代入は許さず、+=演算子等を用意する
    - 解決策4: 再代入は許さず、再代入演算子:=を用意する
        - これを採用してみる

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

[x] Makefile改善
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
  演算子  ：OP\_NEG, OP\_PLUS, OP\_MINUS, OP\_TIMES, OP\_DIVIDE
  識別子  ：OP\_NAMEVAL
  リテラル：OP\_IVAL, (OP\_SVAL)
  関数    ：OP\_FNSTATE, OP\_FNARGS, OP\_FNBODY, OP\_FNFOOT


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

