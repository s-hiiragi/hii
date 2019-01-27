#ifndef LOG_H_
#define LOG_H_

#include <cstdio>
#include <string>

/* clog API
 *
 * clog::e(format, ...args) -- write error log
 * clog::i(format, ...args) -- write information log
 * clog::d(format, ...args) -- write debug log (if is_debug() == true)
 *
 * clog::set_debug(debug)
 * clog::is_debug() -> bool
 *
 * CLOG_TRACE(format, ...args) -- write trace log (with file name and line number)
 */

#define CLOG_TRACE(format, args...) \
    clog::t(__FILE__, __LINE__, (format), args)

namespace my
{

class clog
{
  public:
    template<char level, class... Args>
    static void fwrite(FILE *fp, char const *format, Args const &... args)
    {
        std::fprintf(fp, "%c: ", level);
        std::fprintf(fp, format, args...);
    }

    template<char level>
    static void fwrite(FILE *fp, char const *message)
    {
        std::fprintf(fp, "%c: %s", level, message);
    }

    template<char level, class... Args>
    static void fwriteln(FILE *fp, char const *format, Args const &... args)
    {
        std::fprintf(fp, "%c: ", level);
        std::fprintf(fp, format, args...);
        std::fprintf(fp, "\n");
    }

    template<char level>
    static void fwriteln(FILE *fp, char const *message)
    {
        std::fprintf(fp, "%c: %s\n", level, message);
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
    static void t(const char *filename, int linenumber, char const *format, Args const &... args)
    {
        std::fprintf(stdout, "\e[01;33m");
        fwriteln<'T'>(stdout, "%s(%d): ", filename, linenumber);
        std::fprintf(stdout, format, args...);
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

