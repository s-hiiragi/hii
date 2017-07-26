#include <stdexcept>
#include "cvalue.h"

size_t cvalue::to_positive_index(int index, size_t size)
{
    if (index < 0) {
        if (-index > size) {
            return size;
        }
        return size + index;
    }
    return index;
}

std::string cvalue::to_string() const
{
    switch (type_)
    {
    case VOID:
        return "<void>";
    case INTEGER:
        return std::to_string(value_.i);
    case STRING:
        return *value_.s;
    case ARRAY:
        {
            std::string s;
            s += "[";
            size_t i = 0;
            for (auto &&e : *value_.a) {
                s += e.to_string();
                if (++i < value_.a->size()) {
                    s += ", ";
                }
            }
            s += "]";
            return s;
        }
    default:
        throw std::logic_error("unknown type");
    }
}

bool cvalue::operator == (cvalue const &obj) const
{
    if (type_ != obj.type()) {
        // TODO 静的にチェックしたい
        return false;
    }

    switch (type_)
    {
    case VOID:
        return false;
    case INTEGER:
        return (value_.i == obj.i());
    case STRING:
        return (*value_.s == obj.s());
    case ARRAY:
        if (value_.a->size() != obj.a().size())
            return false;
        for (size_t i = 0; i < value_.a->size(); i++) {
            if (value_.a->at(i) != obj.a().at(i)) {
                return false;
            }
        }
        return true;
    default:
        throw std::logic_error("unknown type");
    }

    return true;
}

bool cvalue::operator != (cvalue const &obj) const
{
    return !operator==(obj);
}

