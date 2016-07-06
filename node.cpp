#include "node.h"
#include "calc-driver.h"
//#include "calc-parser.hh"

// ツリーの評価をしている
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

      case OP_INT:
        return ival_;

      case OP_ID:
        return driver->value(sval_);

      case OP_NEG:
        return -left_->expr(driver);

      default:
        return 0;       // error
    }
}

const char * cnode::name() const
{
    switch (op_)
    {
    case OP_NODE:   return "NODE";      break;
    case OP_NEG:    return "NEG";       break;
    case OP_PLUS:   return "PLUS";      break;
    case OP_MINUS:  return "MINUS";     break;
    case OP_TIMES:  return "TIMES";     break;
    case OP_DIVIDE: return "DIVIDE";    break;
    case OP_ID:     return "ID";        break;
    case OP_INT:    return "INT";       break;
    case OP_EMPTY:  return "EMPTY";     break;
    default:        return "unknown";   break;
    }
}

