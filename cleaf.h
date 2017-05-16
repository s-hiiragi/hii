#ifndef CLEAF_H_
#define CLEAF_H_

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
        : cnode(obj) {
        group_ = NG_LEAF;
        copy_members(obj);
    }

    ~cleaf() {
        delete sval_;
    }

    bool has_sval() const { return sval_ != nullptr; }
    std::string const & sval() const { return *sval_; }
    int ival() const { return ival_; }

    cleaf & operator=(cleaf const & obj) {
        if (&obj == this) {
            return *this;
        }
        cnode::operator=(obj);
        copy_members(obj);
        return *this;
    }

protected:
    void copy_members(cleaf const &obj) {
        // free memory of members
        if (has_sval()) {
            delete sval_;
        }
        // copy members
        if (obj.has_sval()) {
            sval_ = new std::string(obj.sval());
        } else {
            sval_ = nullptr;
        }
        ival_ = obj.ival();
    }

private:
    // XXX 値を共用体で持つ
    std::string *sval_ = nullptr;
    int ival_ = 0;
};

#endif //CLEAF_H_

