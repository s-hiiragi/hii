#pragma once

#include "cnode.h"
#include "clist.h"

class cleaf : public cnode
{
  public:
    cleaf()
        : cnode(OP_EMPTY) { group_ = NG_LEAF; }

    // op = ID or STR or LCMNT
    cleaf(int op, char const *str)
        : cnode(op) { sval_ = new std::string(str); group_ = NG_LEAF; }
    cleaf(int op, std::string *sval)
        : cnode(op), sval_(sval) { group_ = NG_LEAF; }

    // op = STR
    cleaf(int op, char c)
        : cnode(op), sval_(new std::string(1, c)) { group_ = NG_LEAF; }

    // op = INT
    cleaf(int op, int ival)
        : cnode(op), ival_(ival) { group_ = NG_LEAF; }

    cleaf(cleaf const & obj)
        : cnode(obj)
    {
        group_ = NG_LEAF;
        copy_members(obj);
    }

    ~cleaf()
    {
        delete sval_;
    }

    bool has_sval() const { return sval_ != nullptr; }
    std::string const & sval() const { return *sval_; }

    int ival() const { return ival_; }

    std::string to_string() const
    {
        return "cleaf(" + std::string(name()) + ")";
    }

    cleaf & operator=(cleaf const & obj)
    {
        if (&obj != this) {
            cnode::operator=(obj);
            copy_members(obj);
        }
        return *this;
    }

  private:
    void copy_members(cleaf const & obj)
    {
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

    // XXX 値を共用体で持つ
    int ival_ = 0;
    std::string * sval_ = nullptr;
};

