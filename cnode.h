#pragma once

#include <string>
#include <vector>
#include <functional>
#include <cassert>

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
    OP_PLUS_ASSIGN,
    OP_MINUS_ASSIGN,
    OP_TIMES_ASSIGN,
    OP_DIVIDE_ASSIGN,
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
    OP_FOR,
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
	OP_SPACESHIP,
    OP_AND,
    OP_OR,

    // 関数コール式
    OP_CALLEXPR,

    // 配列の要素
    OP_ELEMENT,

    // スライス
    OP_SLICE,

    // 辞書関連
    OP_DICT,
    OP_PAIRS,
    OP_PAIR,

    // 辞書の要素
    OP_DICTITEM,

    // 変数関連
    OP_VAR_EXPR,
    OP_INDEXES,
    OP_ARRAY_INDEX,
    OP_DICT_INDEX,

    // リテラル
    OP_INT,
    OP_STR,
    OP_ARRAY,

    // 識別子
    OP_ID,
    OP_VAR,

    // コメント
    OP_LCOMMENT, // line comment
    OP_TCOMMENT, // tail comment (構文はLCOMMENTと同じ)
    OP_MCOMMENT,  // multi line comment (複数行のLCOMMENT)
    OP_RCOMMENT, // range comment (範囲コメント)

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
class cnode_iterator;

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

    // on_enterがfalseを返したら探索を終了する
    bool each(std::function<bool(cnode & node)> const & on_enter);
    bool each(std::function<bool(cnode const & node)> const & on_enter) const;

    // on_enterがfalseを返したら探索を終了する
    // ctrl.do_break()がコールされると、探索を終了する
    // ctrl.skip_children()がコールされると、子要素の探索をスキップする
    bool each(std::function<bool(cctrl &ctrl, cnode &node)> const &on_enter, std::function<bool(cctrl &ctrl, cnode &node)> const &on_leave);
    bool each(std::function<bool(cctrl &ctrl, cnode const &node)> const &on_enter, std::function<bool(cctrl &ctrl, cnode const &node)> const &on_leave) const;

    cnode_iterator begin();
    cnode_iterator end();

    bool operator ==(cnode const &obj) const {
        if (group_ != obj.group_) return false;
        if (op_ != obj.op_) return false;
        if (left_ != obj.left_) return false;
        if (right_ != obj.right_) return false;
        return true;
    }

    bool operator !=(cnode const &obj) const {
        return !operator==(obj);
    }

  //protected:
    void set_left(cnode * left)
    {
        delete left_;
        left_ = left;
    }

    cnode * release_left()
    {
        cnode * p = left_;
        left_ = nullptr;
        return p;
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

class cnode_iterator
{
public:
    cnode_iterator()
        {}

    cnode_iterator(cnode *obj)
        : stack_({ obj }) {}

    cnode_iterator(cnode_iterator const &it)
        : stack_(it.stack_) {}

    cnode_iterator & operator =(cnode_iterator const &it)
    {
        stack_ = it.stack_;
        return *this;
    }

    cnode & operator *() const
    {
        assert(!stack_.empty());
        return const_cast<cnode &>(*stack_.back());
    }

    cnode * operator ->() const
    {
        assert(!stack_.empty());
        return const_cast<cnode *>(stack_.back());
    }

    cnode_iterator & operator ++()
    {
        assert(!stack_.empty());
        cnode &n = *stack_.back();

        cnode *left = n.left();
        cnode *right = n.right();

        stack_.pop_back();

        // leftを優先して探索するので、rightを先に入れる
        if (right != nullptr) stack_.push_back(right);
        if (left != nullptr) stack_.push_back(left);

        return *this;
    }

    cnode_iterator operator ++(int)
    {
        assert(!stack_.empty());
        cnode_iterator it { *this };
        this->operator++();
        return it;
    }

    bool operator ==(cnode_iterator const &it) const
    {
        return stack_ == it.stack_;
    }

    bool operator !=(cnode_iterator const &it) const
    {
        return !operator==(it);
    }

private:
    std::vector<cnode *> stack_;
};

inline cnode_iterator cnode::begin() { return { this }; }
inline cnode_iterator cnode::end() { return {}; }

