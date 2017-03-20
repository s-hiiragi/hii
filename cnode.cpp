#include <iostream>
#include "cnode.h"
#include "calc-driver.h"
//#include "calc-parser.hh"

using namespace std;

void cnode::print(const cnode *node, unsigned int nestlev)
{
    if (node == nullptr) return;
    
    for (int i = 0; i < nestlev; i++) {
        cout << "  ";
    }
    
    if (nestlev >= 1) {
        cout << "|-- ";
    }
    
    cout << node->name();
    switch (node->op())
    {
    case OP_ID:
        cout << " " << dynamic_cast<const cleaf *>(node)->sval();
        break;
    case OP_INT:
        cout << " " << dynamic_cast<const cleaf *>(node)->ival();
        break;
    }
    cout << endl;

    cnode::print(node->left(), nestlev+1);
    cnode::print(node->right(), nestlev+1);
}

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
        return dynamic_cast<const cleaf *>(this)->ival();

      case OP_ID:
        return driver->value(dynamic_cast<const cleaf *>(this)->sval());

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
    case OP_ASSIGN: return "ASSIGN";    break;
    case OP_PRINT:  return "PRINT";     break;
    case OP_LIST:   return "LIST";      break;
    case OP_CALL:   return "CALL";      break;
    case OP_IFTHEN: return "IF";        break;
    case OP_ELIF:   return "ELIF";      break;
    case OP_ELSE:   return "ELSE";      break;
    case OP_END:    return "END";       break;
    case OP_LOOP:   return "LOOP";      break;
    case OP_RET:    return "RET";       break;
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

