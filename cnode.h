#ifndef NODE_H_
#define NODE_H_

#include <string>

typedef enum node_type_
{
    OP_UNKNOWN,
    
    // なんかのノード
    OP_NODE,

    // 複文
    OP_STATS,

    // 文
    OP_ASSIGN,
    OP_PRINT,
    OP_LIST,
    OP_CALL,
    OP_IF,
    OP_IFTHEN,
    OP_ELIF,
    OP_ELSE,
    OP_END,
    OP_LOOP,
    OP_FN,  // 右のコメントは何？// 関数内での関数定義は許さないので種別は不要
    OP_RET,

    OP_EXPRS,
    OP_ARGS,

    // 単項演算子
    OP_NEG,
    
    // 二項演算子
    OP_PLUS,
    OP_MINUS,
    OP_TIMES,
    OP_DIVIDE,

    // コメント
    OP_LCOMMENT,

    // リテラル
    OP_ID,
    OP_INT,
    OP_EMPTY // 値なしリーフ
} node_type;

// ノード

class hii_driver;
class exprlist;
class arglist;

class cnode {

    friend exprlist;
    friend arglist;

  public:
    // 左リーフ優先で深さ優先探索
    // @param[in] node
    // @param[in] callback void (*callback)(const cnode *cnode, unsigned int nestlev)
    // @param[in] nestlev
    /*
    template <class T>
    static void list(const cnode *node, const T &callback, unsigned int nestlev=0)
    {
        if (node == nullptr) return;
        callback(node, nestlev);
        list(node->left_, callback, nestlev+1);
        list(node->right_, callback, nestlev+1);
    }
    */
    //static void print(const cnode *node, unsigned int nestlev=0);

    cnode() {}

    cnode(int op, cnode *left=nullptr, cnode *right=nullptr)
        : op_(op), left_(left), right_(right) {}

    virtual ~cnode()
    {
        delete left_;
        delete right_;
    }

    int expr(hii_driver *driver) const;

    int op() const { return op_; }
    const cnode *left() const { return left_; }
    const cnode *right() const { return right_; }
    cnode *left() { return left_; }
    cnode *right() { return right_; }

    //void set_right(cnode *right) { delete right_; right_ = right; }

  //protected:
    void set_left(cnode *left) { delete left_; left_ = left; }
    void set_right(cnode *right) { delete right_; right_ = right; }

  private:
    int op_ = OP_EMPTY;
    cnode *left_ = nullptr;
    cnode *right_ = nullptr;
};

#endif //NODE_H_

