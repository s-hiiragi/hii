#include <stdexcept>
#include <sstream>
#include "cvalue.h"
#include "clog.h"

using std::stringstream;
using my::clog;

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

bool cvalue::to_bool() const
{
    switch (type_)
    {
    case VOID:
        throw std::logic_error("voidからboolへの変換は不可");
        break;
    case INTEGER:
        return value_.i != 0;
        break;
    case STRING:
        return (value_.s->size() != 0);
        break;
    case ARRAY:
        return (value_.a->size() != 0);
        break;
    case DICT:
        return (value_.d->size() != 0);
        break;
    }
    return false;
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
            std::string s;  // TODO stringstreamに置き換える
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
        break;
    case DICT:
        {
            std::stringstream ss;

            ss << "[";
            bool tail = false;
            for (auto const &e : *value_.d) {
                if (tail) {
                    ss << ", ";
                } else {
                    tail = true;
                }
                ss << e.first << ":" << e.second.to_string();
            }
            ss << "]";
            return ss.str();
        }
        break;
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
    case DICT:
        if (value_.d->size() != obj.d().size())
            return false;
        {
            auto i1 = value_.d->cbegin();
            auto i2 = obj.d().cbegin();
            for (; i1 != value_.d->cend(); ++i1, ++i2)
            {
                if (i1->first != i2->first)
                    return false;
                if (i1->second != i2->second)
                    return false;
            }
        }
        return true;
    default:
        throw std::logic_error("unknown type");
    }

    return true;
}

