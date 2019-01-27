#pragma once

#include <stdexcept>
#include <string>
#include <utility>
#include <cstdio>

namespace my
{

/**
 * 書式付き文字列をstd::stringに変換するクラス
 * 静的メンバ変数を使っているのでスレッドセーフではない
 *
 * 最大文字数255 (超えた分は...)
 */
class error_format
{
public:
    template<class... Args>
    static std::string const & str(char const *message, Args const &... args)
    {
        std::snprintf(tmp, sizeof(tmp), message, args...);
        s = tmp;
        return s;
    }

private:
    static char tmp[256];
    static std::string s;
};

/**
 * エラーメッセージを保持するクラス
 */
class error_info
{
public:
    error_info() {}

    error_info(char const *message)
        : message_(message) {}

    template<class... Args>
    error_info(char const *message, Args const &... args)
        : message_(error_format::str(message, args...)) {}

    error_info(std::string const &message)
        : message_(message) {}

    error_info(std::string &&message)
        : message_(std::move(message)) {}

    error_info(error_info const &obj)
        : message_(obj.message()) {}

    error_info(error_info &&obj)
        : message_(std::move(obj.message_)) {}

    error_info & operator=(error_info const &obj)
    {
        message_ = obj.message_;
        return *this;
    }

    error_info & operator=(error_info &&obj)
    {
        message_ = std::move(obj.message_);
        return *this;
    }

    std::string const & message() const { return message_; }

private:
    std::string message_;
};

/**
 * expectedクラスで使用するダミー価用のクラス
 * 成否のみ必要で成功値が不要な場合に使用する(?)
 */
class dummy_value {};

template<class T = dummy_value, class E = error_info>
class expected
{
public:
    expected(T const &value)
        : success_(true)
    {
        new(&value_) T(value);
    }

    expected(T &&value)
        : success_(true)
    {
        new(&value_) T(std::move(value));
    }
    
    expected(E const &error)
        : success_(false)
    {
        new(&error_) E(error);
    }
    
    expected(E &&error)
        : success_(false)
    {
        new (&error_) E(std::move(error));
    }

    expected(expected const &obj)
        : success_(obj.success_)
    {
        if (success_) {
            new (&value_) T(obj.value_);
        } else {
            new (&error_) T(obj.error_);
        }
    }

    expected(expected &&obj)
        : success_(obj.success_)
    {
        if (success_) {
            new (&value_) T(std::move(obj.value_));
        } else {
            new (&error_) E(std::move(obj.error_));
        }
    }

    ~expected()
    {
        if (success_) {
            value_.~T();
        } else {
            error_.~E();
        }
    }

    expected & operator=(expected const &obj)
    {
        if (success_) {
            value_.~T();
        } else {
            error_.~E();
        }
        success_ = obj.success_;
        if (obj.success_) {
            new(&value_) T(obj.value_);
        } else {
            new(&error_) E(obj.error_);
        }
        return *this;
    }

    expected & operator=(expected &&obj)
    {
        if (success_) {
            value_.~T();
        } else {
            error_.~E();
        }
        success_ = obj.success_;
        if (obj.success_) {
            new(&value_) T(std::move(obj.value_));
        } else {
            new(&error_) E(std::move(obj.error_));
        }
        return *this;
    }

    operator bool() const { return success_; }

    T & value()
    {
        if (!success_) {
            throw std::logic_error("value is not contained");
        }
        return value_;
    }

    T const & value() const
    {
        if (!success_) {
            throw std::logic_error("value is not contained");
        }
        return value_;
    }

    E & error()
    {
        if (success_) {
            throw std::logic_error("error is not contained");
        }
        return error_;
    }

    E const & error() const
    {
        if (success_) {
            throw std::logic_error("error is not contained");
        }
        return error_;
    }

private:
    bool success_;
    union {
        T value_;
        E error_;
    };
};

/*
 * XXX 作りかけになっている
 * このクラスは多分例外クラスのコンストラクタに指定するメッセージをprintfライクにしたかったんだと思う
 */
class cerror : public std::runtime_error
{
public:
    explicit cerror(std::string const &message)
        : std::runtime_error(message) {}

    explicit cerror(char const *message)
        : std::runtime_error(message) {}

    // printfスタイルで使いたい
    template<class... Args>
    explicit cerror(char const *message, Args const &... args)
        : std::runtime_error(error_format::str(message, args...)) {}
};

}//namespace hii

