#ifndef LOG_H_
#define LOG_H_

#include <cstdio>

namespace my
{

class clog
{
  public:
    template<class... Args>
    static void d(char const *format, Args const &... args)
    {
        if (!debug_) return;
        std::printf("D: ");
        std::printf(format, args...);
        std::printf("\n");
    }

    static void d(char const *message)
    {
        if (!debug_) return;
        std::printf("D: %s\n", message);
    }

    template<class... Args>
    static void e(char const *format, Args const &... args)
    {
        std::fprintf(stderr, "E: ");
        std::fprintf(stderr, format, args...);
        std::fprintf(stderr, "\n");
    }

    static void set_debug(bool debug) { debug_ = debug; }

    static bool is_debug() { return debug_; }

    static void e(char const *message)
    {
        std::printf("E: %s\n", message);
    }
  private:
    static bool debug_;
};

}//namespace my

#endif //LOG_H_

