#include <iostream>
#include "cnode.h"
#include "cleaf.h"
#include "hii_driver.h"

using namespace std;

void cnode::print(const cnode *node, unsigned int nestlev)
{
    if (node == nullptr) return;
    
    if (node->op() == OP_LISTITEM) {
        nestlev--;
        goto skip;
    }

    // インデントを出力
    for (int i = 0; i < nestlev; i++) {
        cout << "  ";
    }
    
    if (nestlev >= 1) {
        cout << "|-- ";
    }
    
    // ノード名を出力
    // 値がある場合は値も表示
    // TODO ノード毎に名前と値を出力する関数を実装する
    cout << node->name();
    switch (node->op())
    {
    case OP_LCOMMENT:
        cout << " \"" << dynamic_cast<const cleaf *>(node)->sval() << "\"";
        break;
    case OP_ID:
        cout << " " << dynamic_cast<const cleaf *>(node)->sval();
        break;
    case OP_INT:
        cout << " " << dynamic_cast<const cleaf *>(node)->ival();
        break;
    case OP_STR:
        cout << " \"" << dynamic_cast<const cleaf *>(node)->sval() << "\"";
        break;
    }
    cout << endl;

skip:
    // 再帰的に子ノードを表示
    // TODO leftがnullでrightがnullでない場合、leftがnullでないときと同じ出力になってしまう
    cnode::print(node->left(), nestlev+1);
    cnode::print(node->right(), nestlev+1);
}

int cnode::expr(hii_driver *driver) const
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
    case OP_NODE:     return "NODE";     break;
    case OP_STATS:    return "STATS";    break;
    case OP_ASSIGN:   return "ASSIGN";   break;
    case OP_PRINT:    return "PRINT";    break;
    case OP_LIST:     return "LIST";     break;
    case OP_CALL:     return "CALL";     break;
    case OP_IF:       return "IF";       break;
    case OP_ELIF:     return "ELIF";     break;
    case OP_ELSE:     return "ELSE";     break;
    case OP_END:      return "END";      break;
    case OP_FUN:      return "FUN";      break;
    case OP_RET:      return "RET";      break;
    case OP_EXPRS:    return "EXPRS";    break;
    case OP_ARGS:     return "ARGS";     break;
    case OP_NEG:      return "NEG";      break;
    case OP_PLUS:     return "PLUS";     break;
    case OP_MINUS:    return "MINUS";    break;
    case OP_TIMES:    return "TIMES";    break;
    case OP_DIVIDE:   return "DIVIDE";   break;
    case OP_MCOMMENT: return "MCOMMENT"; break;
    case OP_LCOMMENT: return "LCOMMENT"; break;
    case OP_ID:       return "ID";       break;
    case OP_INT:      return "INT";      break;
    case OP_STR:      return "STR";      break;
    case OP_EMPTY:    return "EMPTY";    break;
    default:          return "unknown";  break;
    }
}

