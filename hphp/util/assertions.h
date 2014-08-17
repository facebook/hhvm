/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/
#ifndef incl_HPHP_ASSERTIONS_H_
#define incl_HPHP_ASSERTIONS_H_

#include <cassert>
#include <cstdio>
#include <cstdlib>
#include <exception>
#include <functional>
#include <string>

//////////////////////////////////////////////////////////////////////

#define IMPLIES(a, b) (!(a) || (b))

#ifdef __INTEL_COMPILER
#define not_reached()                                                \
  do {                                                               \
    assert(false);                                                   \
  } while (true)
#else
#define not_reached() /* gcc-4.5 supports __builtin_unreachable() */  \
  do {                                                                \
    assert(false);                                                    \
    __builtin_unreachable();                                          \
  } while (true)
#endif

template<typename T>
T bad_value() {
  not_reached();
}

#define not_implemented() do {                   \
  fprintf(stderr, "not implemented: %s:%d %s\n", \
          __FILE__, __LINE__, __FUNCTION__);     \
  always_assert(0);                              \
} while (0)

#define assert_not_implemented(pred) do {        \
  if (! (pred) ) {                               \
    not_implemented();                           \
  }                                              \
} while(0)

#define ASSERT_NOT_IMPLEMENTED assert_not_implemented
#define NOT_IMPLEMENTED        not_implemented

namespace HPHP {

void assert_fail(const char* exp,
                 const char* file,
                 unsigned int line,
                 const char* func) __attribute__((__noreturn__));

void assert_fail_log(const char* title, const std::string& msg);
typedef std::function<void(const char*, const std::string&)> AssertFailLogger;
void register_assert_fail_logger(AssertFailLogger);

class FailedAssertion : public std::exception {
 public:
  FailedAssertion(const char* msg, const char* file, unsigned line,
                  const char* func)
      : msg(msg)
      , file(file)
      , line(line)
      , func(func)
      , summary(makeSummary()) {}

  ~FailedAssertion() throw() {}

  const char* what() const throw() {
    return summary.c_str();
  }

  void print() const {
    fputs(summary.c_str(), stderr);
    fputc('\n', stderr);
  }

  const char* const msg;
  const char* const file;
  unsigned const line;
  const char* const func;
  const std::string summary;

 private:
  std::string makeSummary() const {
    char buf[4096];
    if (snprintf(buf, sizeof(buf), "Failed assertion '%s' in %s at %s:%u",
                 msg, func, file, line) >= sizeof(buf)) {
      buf[sizeof(buf)-1] = '\0';
    }
    return std::string(buf);
  }
};

}

#define assert_impl(cond, fail)                   \
  ((cond) ? static_cast<void>(0) : ((fail), static_cast<void>(0)))

#define assert_log_impl(cond, fail, str) assert_impl(   \
    cond,                                               \
    (::HPHP::assert_fail_log(#cond, str), (fail)))      \

#define assert_fail_impl(e)                                             \
  ::HPHP::assert_fail(#e, __FILE__, __LINE__, __PRETTY_FUNCTION__)
#define always_assert(e) assert_impl(e, assert_fail_impl(e))
#define always_assert_log(e, l) assert_log_impl(e, assert_fail_impl(e), l())
#define always_assert_flog(e, ...)                                      \
  assert_log_impl(e, assert_fail_impl(e),                               \
                  ::folly::format(__VA_ARGS__).str())

#define assert_throw_fail_impl(e)                                       \
  throw ::HPHP::FailedAssertion(#e, __FILE__, __LINE__, __PRETTY_FUNCTION__)
#define always_assert_throw(e)                  \
  assert_impl(e, assert_throw_fail_impl(e))
#define always_assert_throw_log(e, l)           \
  assert_log_impl(e, assert_throw_fail_impl(e), l)

#undef assert
#ifndef NDEBUG
#define assert(e) always_assert(e)
#define assert_log(e, l) always_assert_log(e, l)
#define assert_flog(e, ...) always_assert_flog(e, __VA_ARGS__)
#define assert_throw(e) always_assert_throw(e)
#define assert_throw_log(e, l) always_assert_throw_log(e, l)
#else
#define assert(e) static_cast<void>(0)
#define assert_log(e, l) static_cast<void>(0)
#define assert_flog(e, ...) static_cast<void>(0)
#define assert_throw(e) static_cast<void>(0)
#define assert_throw_log(e, l) static_cast<void>(0)
#endif

const bool do_assert =
#ifdef NDEBUG
  false
#else
  true
#endif
  ;

//////////////////////////////////////////////////////////////////////

#endif
