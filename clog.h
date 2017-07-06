#ifndef LOG_H_
#define LOG_H_

#include <cstdio>
#include <string>

namespace my
{

class clog
{
  public:
    template<char Level, class... Args>
    static void fwriteln(FILE *fp, char const *format, Args const &... args)
    {
        std::fprintf(fp, "%c: ", Level);
        std::fprintf(fp, format, args...);
        std::fprintf(fp, "\n");
    }

    template<char Level>
    static void fwriteln(FILE *fp, char const *message)
    {
        std::fprintf(fp, "%c: %s\n", Level, message);
    }

    template<class... Args>
    static void e(char const *format, Args const &... args)
    {
        std::fprintf(stderr, "\e[01;31m");
        fwriteln<'E'>(stderr, format, args...);
        std::fprintf(stderr, "\e[00m");
    }

    template<class... Args>
    static void i(char const *format, Args const &... args)
    {
        std::fprintf(stdout, "\e[01;33m");
        fwriteln<'I'>(stdout, format, args...);
        std::fprintf(stdout, "\e[00m");
    }

    template<class... Args>
    static void d(char const *format, Args const &... args)
    {
        if (!debug_) return;
        fwriteln<'D'>(stdout, format, args...);
    }

    static void set_debug(bool debug) { debug_ = debug; }

    static bool is_debug() { return debug_; }

    /*
    template<class T, class U>
    static U const & to_c_str(T const &obj) { return obj; }
    */

  private:

    static bool debug_;
};

/*
template<>
char const * clog::to_c_str<std::string, char>(std::string const &s) { return s.c_str(); }
*/

}//namespace my

#endif //LOG_H_

