#ifndef ARGLIST_H_
#define ARGLIST_H_

#include <string>
#include "cnode.h"
#include "cleaf.h"

class arglist : public cnode
{
  public:
    arglist()
      : cnode(OP_EMPTY) {}

    // 現状、arglistは引数名のリストになっている
    arglist(std::string *name)
      : cnode(OP_NODE, new cleaf(OP_ID, name), new cnode(OP_EMPTY)) {}
    
    arglist & concat(std::string *name);
};

#endif //ARGLIST_H_

