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

    cleaf(cleaf const &obj)
        : cnode(obj)
    {
        if (obj.has_sval()) sval_ = new std::string(obj.sval());
        ival_ = obj.ival();
    }

    ~cleaf() {
        delete sval_;
    }

    bool has_sval() const { return sval_ != nullptr; }
    const std::string &sval() const { return *sval_; }
    int ival() const { return ival_; }

    cleaf & operator =(cleaf const &obj)
    {
        return *this;
    }

private:
    std::string *sval_ = nullptr;
    int ival_ = 0;
};

