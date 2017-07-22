#pragma once

#include <string>
#include <functional>

typedef enum node_type_
{
    OP_UNKNOWN,
    
    // なんかのノード
    OP_NODE,

    // リストの要素
    OP_LISTITEM,  // clistで使ってるっぽい

    // TODO OP_LIST を追加した方がいいような。。。
    // メリット
    // - リストかどうか判定しやすい
    // - 新しいリストを追加するたびに種別を増やさなくて良い
    // 
    // ==> リストの種別をどう判定するかの問題がある
    // - トップレベルのノード種別をOP_LISTとして別途種別を持たせるか、
    // - トップレベルのノード種別をOP_<anytype>として別途リストフラグを持たせるか

    // 複文
    OP_STATS,

    // 文
    OP_ASSIGN,
    OP_REASSIGN,
    OP_INC,
    OP_DEC,
    OP_CALL,
    OP_IF,
    OP_ELIF,
    OP_ELSE,
    OP_SW,
    OP_SWCASES,
    OP_SWCASE,
    OP_SWELSE,
    OP_FUN,
    OP_RET,
    OP_LOOP,
    OP_CONT,
    OP_BREAK,

    OP_EXPRS,
    OP_ARGS,

    // 単項演算子
    OP_NEG,
    
    // 二項演算子
    OP_PLUS,
    OP_MINUS,
    OP_TIMES,
    OP_DIVIDE,
    OP_MODULO,
    OP_EQ,
    OP_NEQ,
    OP_LT,
    OP_LTEQ,
    OP_GT,
    OP_GTEQ,
    OP_AND,
    OP_OR,

    // 関数コール式
    OP_CALLEXPR,

    // 配列の要素
    OP_ELEMENT,

    // スライス
    OP_SLICE,

    // リテラル
    OP_INT,
    OP_STR,
    OP_ARRAY,

    // 識別子
    OP_ID,
    OP_VAR,

    // コメント
    OP_LCOMMENT, // line comment
    OP_TCOMMENT, // tail comment
    OP_MCOMMENT,  // multi line comment
    OP_RCOMMENT, // range comment

    // アノテーション
    OP_ATTRS,

    OP_EMPTY // 値なしリーフ
} node_type;


typedef enum node_group_
{
    NG_NODE,
    NG_LEAF,
    NG_LIST
} node_group;


class cleaf;
class clist;

class cnode
{
  public:
    /**
     * 左リーフ優先で深さ優先探索
     * 
     * @param[in] node
     * @param[in] f  bool (*)(const cnode *node, unsigned int nestlev)
     * @param[in] nestlev
     */
    template <class T>
    static void list(const cnode * node, const T & callback, unsigned int nestlev = 0)
    {
        if (node == nullptr) return;
        bool cont = callback(node, nestlev);
        if (cont) {
            list(node->left_, callback, nestlev+1);
            list(node->right_, callback, nestlev+1);
        }
    }

    static void print(cnode const * node, unsigned int nestlev = 0);

    cnode()
    {
    }

    cnode(int op, cnode * left = nullptr, cnode * right = nullptr)
        : op_(op), left_(left), right_(right)
    {
    }

    cnode(cnode const & obj)
    {
        copy_members(obj);
    }

    virtual ~cnode()
    {
        delete left_;
        delete right_;
    }

    cnode & operator=(cnode const & obj)
    {
        if (&obj == this) {
            // XXX x=xはエラーとすべき？
            return *this;
        }
        copy_members(obj);
        return *this;
    }

    int group() const { return group_; }
    int op() const { return op_; }
    char const * name() const;

    cnode * left() { return left_; }
    cnode * right() { return right_; }
    cnode const * left() const { return left_; }
    cnode const * right() const { return right_; }
   
    // ノードのシンプルな文字列表現を返す
    virtual std::string to_string() const
    {
        return "cnode(" + std::string(name()) + ")";
    }

    class cctrl
    {
        friend cnode;
      public:
        void do_break() { action_ = action_do_break; }
        void skip_children() { action_ = action_skip_children; }
        cctrl() : action_(noaction) {}
      private:
        enum eaction { noaction, action_do_break, action_skip_children };
        eaction action_;
    };

    bool each(std::function<bool(cnode & node)> const & on_enter);
    bool each(std::function<bool(cnode const & node)> const & on_enter) const;

    bool each(std::function<bool(cctrl &ctrl, cnode &node)> const &on_enter, std::function<bool(cctrl &ctrl, cnode &node)> const &on_leave);
    bool each(std::function<bool(cctrl &ctrl, cnode const &node)> const &on_enter, std::function<bool(cctrl &ctrl, cnode const &node)> const &on_leave) const;

  //protected:
    void set_left(cnode * left)
    {
        delete left_;
        left_ = left;
    }

    void set_right(cnode * right)
    {
        delete right_;
        right_ = right;
    }

  protected:
    int group_ = NG_NODE;

  private:
    void copy_members(cnode const & obj)
    {
        // free memory of memebers
        if (left_ != nullptr) delete left_;
        if (right_ != nullptr) delete right_;
        // copy members
        op_ = obj.op();
        group_ = obj.group();
        if (obj.left() != nullptr) {
            left_ = new cnode(*obj.left());
        } else {
            left_ = nullptr;
        }
        if (obj.right() != nullptr) {
            right_ = new cnode(*obj.right());
        } else {
            right_ = nullptr;
        }
    }

    int op_ = OP_EMPTY;
    cnode * left_ = nullptr;
    cnode * right_ = nullptr;
};

