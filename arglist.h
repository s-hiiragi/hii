#ifndef ARGLIST_H_
#define ARGLIST_H_

#include <string>
#include "node.h"

class arglist : public cnode
{
  public:
    arglist()
      : cnode(OP_EMPTY) {}

    arglist(std::string *name)
      : cnode(OP_NODE, new cnode(OP_ID, name), new cnode(OP_EMPTY)) {}
    
    arglist & concat(std::string *name);
};

#endif //ARGLIST_H_

