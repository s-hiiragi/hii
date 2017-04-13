# 開発メモ

[ ] cnodeをcleafにキャストしてvalueにアクセスするのを簡単にしたい

[ ] 組込関数コールを実行するメソッドを実装する

[ ] 変数定義する実行メソッドを実装する
  スコープを扱う必要がある
  - グローバルスコープ
  - ブロックスコープ (fun, if)

  変数登録時
  - 一番近いスコープに登録する
  変数参照時
  - 一番近いスコープの変数を参照する

[ ] スコープを生成する


[ ] OP_FUNの子ノードを処理する
  -> ASTを(ASTよりも実行しやすい)中間言語に変換する必要がある

[ ] 中間言語のデータ構造を考える
  program:
    - stats: cstats
  cstats:
    - stats: vector<cstat>
  cstat:
    ccomment:
      - value: string
    cassign:
      - name: string
      - expr: cexpr
    cif:
      - cond: 
    cfun:
      - name: string
      - args: 
      - stats: cstats
    ccall:
      - exprs: vector<cexpr>
  cexpr:
    - 

[-] (cnode::listを拡張して)スコープを抜けるときcallbackを呼ぶ
  -> OP_FUNで子ノードに到達したときにスコープを作成し、子ノードの処理後、
     親ノードに戻ったときにスコープを破棄しようと考えた
  -> よく考えたらOP_FUNの子ノードを別のcallbackで処理することがおかしい
  -> OP_FUNに到達したときにcallback内で子ノードを処理する

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
-> リストのトップノードだけOP_LISTにする ?





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


