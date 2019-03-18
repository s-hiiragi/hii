#ifndef HII_CTYPE_H
#define HII_CTYPE_H

#include <string>
#include "ctypefunc.h"
#include "ctypeclass.h"

class ctype
{
public:
    enum kind
    {
        PRIMITIVE,

        // delivertive
        FUNC,
        CLASS
    };

    ctype(std::string const & name)
        : kind_(PRIMITIVE), name_(name) {}

    ctype(std::vector<ctype> const & argtypes, ctype const & rettype)
        : kind_(FUNC), name_("FUNC")
    {
        value_.f.argtypes = argtypes;
        value_.f.rettype = rettype;
    }

    ctype(std::vector<ctype> const & membertypes)
        : kind_(CLASS), name_("CLASS")
    {
        value_.c.membertypes = membertypes;
    }

private:
    struct ctypefunc
    {
        std::vector<ctype> argtypes;
        ctype rettype;
    };

    struct ctypeclass
    {
        std::vector<ctype> membertypes;
    };

    kind kind_;
    std::string name_;

    union
    {
        int dummy;
        ctypefunc f;
        ctypeclass c;
    } value_;
};

#endif //HII_CTYPE_H

