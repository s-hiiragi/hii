#include <stdexcept>
#include "cvalue.h"

static size_t to_positive_index(int index, size_t size)
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

