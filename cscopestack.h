#pragma once

#include <vector>
#include <cstdio>
#include "cscope.h"

// XXX このクラスの必要性は？
class cscopestack
{
  public:
    void push()
    {
        scopes_.push_back(cscope());
    }

    void push(cscope const &scope)
    {
        scopes_.push_back(scope);
    }

    void pop()
    {
        scopes_.pop_back();
    }

    // 以下はcscopeに任せて、cscopestackはスタック管理だけに徹したい
//    void find_var();
//    void find_fun();
//    void add_var();
//    void add_fun();

    cscope const & back() const
    {
        if (scopes_.size() == 0) {
            throw std::logic_error("stack is empty");
        }
        return scopes_.back();
    }

    cscope & back()
    {
        return const_cast<cscope &>(const_cast<cscopestack const *>(this)->back());
    }

    size_t size() const { return scopes_.size(); }

    // for debug
    void print()
    {
        std::printf("put scopes (size=%zu)\n", scopes_.size());
        std::printf("  --\n");
        for (auto &&s : scopes_) {
            s.print();
        }
    }

  private:
    std::vector<cscope> scopes_;
};

