#pragma once

#include <stdexcept>

class ceval_error : public std::runtime_error
{
  public:
    ceval_error() {}
};

