#ifndef CFN_H_
#define CFN_H_

#include <string>
#include <vector>
#include <map>

class cfn
{
  public:
    cfn()
        : name_("__empty") {}
    cfn(const std::string & name, const std::vector<std::string> & args)
        : name_(name), args_(args) {}

    const std::string & name() const { return name_; }
    const std::vector<std::string> & args() const { return args_; }

    void add_var(const std::string & name, int value)
    {
        // TODO 二重定義のチェックをする
        vars_[name] = value;
    }

    int get_var(const std::string & name)
    {
        // 未定義のチェックをする
        return vars_[name];
    }

    void add_stat(const cnode & node)
    {
        stats_.push_back(node);
    }

  private:
    std::string name_;
    std::vector<std::string> args_;
    std::map<std::string, int> vars_;
    std::vector<cnode> stats_;
};

#endif //CFN_H_

