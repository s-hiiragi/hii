#ifndef NODE_H_
#define NODE_H_

#include <string>

typedef enum node_type_
{
    OP_UNKNOWN,
    
    // なんかのノード
    OP_NODE,

    // 文
    OP_ASSIGN,
    OP_PRINT,
    OP_LIST,
    OP_CALL,
    OP_IF,
    OP_ELIF,
    OP_ELSE,
    OP_END,
    OP_LOOP,
//  OP_FN,  // 関数内での関数定義は許さないので種別は不要
    OP_RET,

    // 単項演算子
    OP_NEG,
    
    // 二項演算子
    OP_PLUS,
    OP_MINUS,
    OP_TIMES,
    OP_DIVIDE,

    // リテラル
    OP_ID,
    OP_INT,
    OP_EMPTY // 値なしリーフ
} node_type;

// ノード

class calc_driver;
class cnode {
  public:
    // 左リーフ優先で深さ優先探索
    // @param[in] callback void (*callback)(const cnode *node, unsigned int nestlev)
    template <class T>
    static void list(const cnode *node, const T &callback, unsigned int nestlev=0)
    {
        if (node == nullptr) return;
        callback(node, nestlev);
        list(node->left_, callback, nestlev+1);
        list(node->right_, callback, nestlev+1);
    }

    static void print(const cnode *node, unsigned int nestlev=0);
    
    cnode(int op, cnode *left=nullptr, cnode *right=nullptr)
        : op_(op), left_(left), right_(right) {}
    
    cnode(int op, std::string *sval)
        : op_(op), sval_(sval) {}
    
    cnode(int op, int ival)
        : op_(op), ival_(ival) {}
    
    virtual ~cnode()
    {
        delete left_;
        delete right_;
        delete sval_;
    }

    int expr(calc_driver *driver) const;
    
    const char *name() const;

    int op() const { return op_; }
    const cnode *left() const { return left_; }
    const cnode *right() const { return right_; }
    cnode *left() { return left_; }
    cnode *right() { return right_; }
    const std::string &sval() const { return *sval_; }
    int ival() const { return ival_; }

//  protected:
    void set_op(int op) { op_ = op; }
    void set_left(cnode *left) { delete left_; left_ = left; }
    void set_right(cnode *right) { delete right_; right_ = right; }
    void set_sval(std::string *sval) { delete sval_; sval_ = sval; }
    void set_ival(int ival) { ival_ = ival; }

  private:
    int op_ = OP_EMPTY;
    cnode *left_ = nullptr;
    cnode *right_ = nullptr;
    std::string *sval_ = nullptr;
    int ival_ = 0;
};

#endif //NODE_H_

