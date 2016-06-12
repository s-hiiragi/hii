#ifndef NODE_H__
#define NODE_H__

#include <string>
#include <vector>
#include <map>
#include <functional>
#include <algorithm>

// ノードの命令
enum {
	OP_NEG,
	OP_PLUS,
	OP_MINUS,
	OP_TIMES,
	OP_DIVIDE,
	OP_VALUE,
	OP_IVAL
} ;

// ノード

class calc_driver;
class cnode {
  public:
    // a @ b, @a : a+b, a-b, a*b, a/b, -a
	cnode(int op, cnode *left, cnode *right=nullptr)
		: op_(op), left_(left), right_(right), value_(0), string_(0)
	{
	}
    
    // "ival"
	cnode(int op, int value)
		: op_(op), left_(0), right_(0), value_(value), string_(0)
	{
	}
    
    // "id"
	cnode(int op, std::string *str)
		: op_(op), left_(0), right_(0), value_(0), string_(str)
	{
	}
    
	virtual ~cnode()
	{
		delete left_;
		delete right_;
		delete string_;
	}

    // ツリーの評価をしている
	int expr(calc_driver *driver) const;

	int op() const { return op_; }
	int value() const { return value_; }
	const std::string &string() const { return *string_; }
	const cnode *left() const { return left_; }
	const cnode *right() const { return right_; }

  protected:
	int op_;
	int value_;
	std::string *string_;
	cnode *left_;
	cnode *right_;
} ;

#endif

