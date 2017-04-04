#include "cnode.h"

class cleaf : public cnode {

public:
    cleaf()
        : cnode(OP_EMPTY) { group_ = NG_LEAF; }

    // opはID, LCMNTの場合があるため必須
    cleaf(int op, std::string *sval)
        : cnode(op), sval_(sval) { group_ = NG_LEAF; }
   
    cleaf(int op, int ival)
        : cnode(op), ival_(ival) { group_ = NG_LEAF; }

    ~cleaf() {
        delete sval_;
    }

    const std::string &sval() const { return *sval_; }
    int ival() const { return ival_; }

//    void set_sval(std::string *sval) { delete sval_; sval_ = sval; }
//    void set_ival(int ival) { ival_ = ival; }

private:
    std::string *sval_ = nullptr;
    int ival_ = 0;
};

