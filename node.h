#ifndef NODE_H_
#define NODE_H_

//継承を使うには使う必然性が必要

#include <string>
//#include <vector>
//#include <map>
//#include <functional>
//#include <algorithm>

// ノードの種類
enum {
    OP_UNKNOWN,
    
    // なんかのノード
    OP_NODE,

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
};

// ノード

class calc_driver;
class cnode {
  public:
    template <class T> // T is const cnode *
    static void list(const cnode *node, const T &callback, unsigned int nestlev=0)
    {
        if (node == nullptr) return;
        callback(node, nestlev);
        list(node->left_, callback, nestlev+1);
        list(node->right_, callback, nestlev+1);
    }
    
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

    // ツリーの評価をしている
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

