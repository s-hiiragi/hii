#ifndef EXPRLIST_H_
#define EXPRLIST_H_

#include <string>
#include "node.h"

class exprlist : public cnode
{
  public:
    exprlist()
      : cnode(OP_EMPTY) {}

    exprlist(cnode *expr)
      : cnode(OP_NODE, new cnode(OP_EMPTY), expr) {}
    
    exprlist & concat(cnode *expr);
};

#endif //EXPRLIST_H_

