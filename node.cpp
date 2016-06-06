#include "node.h"
#include "calc-driver.h"
#include "calc-parser.hh"

int cnode::expr(calc_driver *driver) const
{
	switch (op_) {
	  case OP_PLUS:
		return left_->expr(driver) + right_->expr(driver);

	  case OP_MINUS:
		return left_->expr(driver) - right_->expr(driver);

	  case OP_TIMES:
		return left_->expr(driver) * right_->expr(driver);

	  case OP_DIVIDE:
		return left_->expr(driver) / right_->expr(driver);

	  case OP_IVAL:
		return value_;

	  case OP_VALUE:
		return driver->value(string_);

	  case OP_NEG:
		return -left_->expr(driver);

	  default:
		return 0;		// error
	}
}

