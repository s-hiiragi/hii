#pragma once

#include <string>
#include <vector>
#include <stdexcept>

class cvalue
{
  public:
    enum type_t
    {
        VOID,
        INTEGER,
        STRING,
        ARRAY
        // BOOLEAN 欲しい
        // INTEGER -> NUMBERにしたい(実数も扱いたい)
    };

    static size_t to_positive_index(int index, size_t size);

    cvalue()
        : type_(VOID) {}

    cvalue(int i)
        : type_(INTEGER) { value_.i = i; }

    cvalue(std::string const &s)
        : type_(STRING) { value_.s = new std::string(s); }

    cvalue(std::vector<cvalue> const &a)
        : type_(ARRAY) { value_.a = new std::vector<cvalue>(a); }

    cvalue(std::vector<cvalue> &&a)
        : type_(ARRAY) { value_.a = new std::vector<cvalue>(std::move(a)); }

    cvalue(cvalue const &obj)
        : type_(VOID)
    {
        copy_members(obj);
    }

    cvalue(cvalue &&obj)
        : type_(VOID)
    {
        switch (obj.type_) {
        case VOID:
            break;
        case INTEGER:
            type_ = INTEGER;
            value_.i = obj.value_.i;
            break;
        case STRING:
            type_ = STRING;
            value_.s = obj.value_.s;
            obj.value_.s = nullptr;
            break;
        case ARRAY:
            type_ = ARRAY;
            value_.a = obj.value_.a;
            obj.value_.a = nullptr;
            break;
        default:
            throw std::logic_error("invalid type");
        }
        obj.type_ = VOID;
    }

    ~cvalue()
    {
        if (is_str()) delete value_.s;
        if (is_ary()) delete value_.a;
    }

    cvalue & operator=(cvalue const &obj)
    {
        if (&obj != this) {
            copy_members(obj);
        }
        return *this;
    }

    type_t type() const { return type_; }
    bool is_void() const { return type_ == VOID; }
    bool is_int() const { return type_ == INTEGER; }
    bool is_str() const { return type_ == STRING; }
    bool is_ary() const { return type_ == ARRAY; }

    std::string type_name() const
    {
        switch (type_) {
        case VOID:
            return "void";
        case INTEGER:
            return "int";
        case STRING:
            return "str";
        case ARRAY:
            return "ary";
        default:
            throw std::logic_error("invalid type");
        }
    }

    int i() const { return value_.i; }

    std::string & s() { return *value_.s; }
    std::string const & s() const { return *value_.s; }

    std::vector<cvalue> & a() { return *value_.a; }
    std::vector<cvalue> const & a() const { return *value_.a; }

    cvalue & a(size_t index)
    {
        return const_cast<cvalue &>(const_cast<cvalue const *>(this)->a(index));
    }

    cvalue const & a(size_t index) const
    {
        return value_.a->at(index);
    }

    std::string to_string() const;

    bool operator == (cvalue const &obj) const;
    bool operator != (cvalue const &obj) const;

  private:
    void copy_members(cvalue const &obj)
    {
        switch (type_)
        {
        case VOID:
            break;
        case INTEGER:
            break;
        case STRING:
            delete value_.s;
            value_.s = nullptr;
            break;
        case ARRAY:
            delete value_.a;
            value_.a = nullptr;
            break;
        }

        type_ = obj.type();
        switch (obj.type())
        {
        case VOID:
            break;
        case INTEGER:
            value_.i = obj.i();
            break;
        case STRING:
            value_.s = new std::string(obj.s());
            break;
        case ARRAY:
            value_.a = new std::vector<cvalue>(obj.a());
            break;
        }
    }

    type_t type_;
    union {
        int i;
        std::string *s;
        std::vector<cvalue> *a;
    } value_;
};

