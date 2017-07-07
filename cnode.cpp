#include <iostream>
#include <functional>
#include "cnode.h"
#include "cleaf.h"
#include "hii_driver.h"

using std::cout;
using std::endl;
using std::function;

void cnode::print(cnode const *node, unsigned int nestlev)
{
    if (node == nullptr) return;

    // リストの場合、各子要素のインデントが同じレベルになるよう調整する
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

const char * cnode::name() const
{
    switch (op_)
    {
    case OP_NODE:     return "NODE";     break;
    case OP_STATS:    return "STATS";    break;
    case OP_ASSIGN:   return "ASSIGN";   break;
    case OP_REASSIGN: return "REASSIGN"; break;
    case OP_LSASSIGN: return "LSASSIGN"; break;
    case OP_INC:      return "INC";      break;
    case OP_DEC:      return "DEC";      break;
    case OP_CALL:     return "CALL";     break;
    case OP_IF:       return "IF";       break;
    case OP_ELIF:     return "ELIF";     break;
    case OP_ELSE:     return "ELSE";     break;
    case OP_FUN:      return "FUN";      break;
    case OP_RET:      return "RET";      break;
    case OP_LOOP:     return "LOOP";     break;
    case OP_CONT:     return "CONT";     break;
    case OP_BREAK:    return "BREAK";    break;
    case OP_EXPRS:    return "EXPRS";    break;
    case OP_ARGS:     return "ARGS";     break;
    case OP_NEG:      return "NEG";      break;
    case OP_PLUS:     return "PLUS";     break;
    case OP_MINUS:    return "MINUS";    break;
    case OP_TIMES:    return "TIMES";    break;
    case OP_DIVIDE:   return "DIVIDE";   break;
    case OP_MODULO:   return "MODULO";   break;
    case OP_EQ:       return "EQ";       break;
    case OP_NEQ:      return "NEQ";      break;
    case OP_LT:       return "LT";       break;
    case OP_LTEQ:     return "LTEQ";     break;
    case OP_GT:       return "GT";       break;
    case OP_GTEQ:     return "GTEQ";     break;
    case OP_AND:      return "AND";      break;
    case OP_OR:       return "OR";       break;
    case OP_CALLEXPR: return "CALLEXPR"; break;
    case OP_LCOMMENT: return "LCOMMENT"; break;
    case OP_MCOMMENT: return "MCOMMENT"; break;
    case OP_RCOMMENT: return "RCOMMENT"; break;
    case OP_ID:       return "ID";       break;
    case OP_VAR:      return "VAR";      break;
    case OP_INT:      return "INT";      break;
    case OP_STR:      return "STR";      break;
    case OP_ARRAY:    return "ARRAY";    break;
    case OP_ELEMENT:  return "ELEMENT";  break;
    case OP_SLICE:    return "SLICE";    break;
    case OP_ATTRS:    return "ATTRS";    break;
    case OP_EMPTY:    return "EMPTY";    break;
    default:          return "unknown";  break;
    }
}

bool cnode::each(function<bool(cnode &node)> const &on_enter)
{
    if (!on_enter(*this))
        return false;

    if (this->left() != nullptr) {
        bool ret = this->left()->each(on_enter);
        if (!ret) return false;
    }
    if (this->right() != nullptr) {
        bool ret = this->right()->each(on_enter);
        if (!ret) return false;
    }
    return true;
}

bool cnode::each(function<bool(cnode const &node)> const &on_enter) const
{
    if (!on_enter(*this))
        return false;

    if (this->left() != nullptr) {
        bool ret = this->left()->each(on_enter);
        if (!ret) return false;
    }
    if (this->right() != nullptr) {
        bool ret = this->right()->each(on_enter);
        if (!ret) return false;
    }
    return true;
}

bool cnode::each(function<bool(cnode::cctrl &ctrl, cnode &node)> const &on_enter, function<bool(cnode::cctrl &ctrl, cnode &node)> const &on_leave)
{
    cctrl ctrl;
    if (!on_enter(ctrl, *this))
        return false;

    if (ctrl.action_ == cctrl::eaction::action_do_break) return false;

    if (ctrl.action_ != cctrl::eaction::action_skip_children)
    {
        if (this->left() != nullptr) {
            bool ret = this->left()->each(on_enter, on_leave);
            if (!ret) return false;
        }
        if (this->right() != nullptr) {
            bool ret = this->right()->each(on_enter, on_leave);
            if (!ret) return false;
        }
    }

    cctrl ctrl2;
    if (!on_leave(ctrl2, *this))
        return false;

    return true;
}

bool cnode::each(function<bool(cnode::cctrl &ctrl, cnode const &node)> const &on_enter, function<bool(cnode::cctrl &ctrl, cnode const &node)> const &on_leave) const
{
    cctrl ctrl;
    if (!on_enter(ctrl, *this))
        return false;
    if (ctrl.action_ == cctrl::eaction::action_do_break) return false;
    if (ctrl.action_ != cctrl::eaction::action_skip_children) {
        if (this->left() != nullptr) {
            bool ret = this->left()->each( on_enter, on_leave);
            if (!ret) return false;
        }
        if (this->right() != nullptr) {
            bool ret = this->right()->each(on_enter, on_leave);
            if (!ret) return false;
        }
    }
    cctrl ctrl2;
    if (!on_leave(ctrl2, *this))
        return false;

    return true;
}

