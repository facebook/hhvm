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
#include <type_traits>
#include <utility>

#include <folly/Format.h>
#include <folly/Preprocessor.h>

///////////////////////////////////////////////////////////////////////////////

#define IMPLIES(a, b) (!(a) || (b))

#if defined(__INTEL_COMPILER) || defined(_MSC_VER)
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

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

/*
 * Assertion abort and log functions.
 *
 * These are intended for use primarily by the assert macros below.
 */
void assert_fail(const char* e,
                 const char* file,
                 unsigned int line,
                 const char* func,
                 const std::string& msg) __attribute__((__noreturn__));

void assert_fail_no_log(const char* e,
                        const char* file,
                        unsigned int line,
                        const char* func,
                        const std::string& msg) __attribute__((__noreturn__));

void assert_log_failure(const char* title, const std::string& msg);

/*
 * Register a function for auxiliary assert logging.
 */
using AssertFailLogger = std::function<void(const char*, const std::string&)>;

void register_assert_fail_logger(AssertFailLogger);

///////////////////////////////////////////////////////////////////////////////

/*
 * Stack-allocated detailed assertion logger.
 */
struct AssertDetailImpl {
  /*
   * Prints the results of all registered detailers to stderr.  Returns true if
   * we had any registered detailers.
   */
  static bool log();

protected:
  explicit AssertDetailImpl(const char* name)
    : m_name(name)
    , m_next(s_head)
  {
#ifndef NDEBUG
    if (m_name == nullptr) std::abort();
#endif
    s_head = this;
  }
  ~AssertDetailImpl() { if (m_name) s_head = m_next; }

  AssertDetailImpl(AssertDetailImpl&& other) noexcept {
#ifndef NDEBUG
    if (s_head != &other) std::abort();
#endif
    m_name = other.m_name;
    m_next = other.m_next;
    s_head = this;
    other.m_name = nullptr; // prevents ~other from messing it up
  }

  AssertDetailImpl(const AssertDetailImpl&) = delete;
  AssertDetailImpl& operator=(const AssertDetailImpl&) = delete;

private:
  static bool log_impl(const AssertDetailImpl*);
  virtual std::string run() const = 0;

private:
  static __thread AssertDetailImpl* s_head;

  const char* m_name;
  AssertDetailImpl* m_next{nullptr};
};

template<class F>
struct AssertDetailT final : AssertDetailImpl {
  AssertDetailT(const char* name, F&& f)
    : AssertDetailImpl(name)
    , m_f(std::move(f))
  {}
  AssertDetailT(AssertDetailT&&) = default;

private:
  std::string run() const override { return m_f(); }

private:
  F m_f;
};

namespace detail {

struct AssertDetailScopeMaker {
  explicit AssertDetailScopeMaker(const char* name) : m_name(name) {}

  template <typename F>
  AssertDetailT<typename std::decay<F>::type> operator+(F&& f) {
    return AssertDetailT<typename std::decay<F>::type>(
        m_name, std::forward<F>(f));
  }

private:
  const char* m_name;
};

}

#define SCOPE_ASSERT_DETAIL(name)           \
  auto FB_ANONYMOUS_VARIABLE(SCOPE_ASSERT)  \
  = ::HPHP::detail::AssertDetailScopeMaker(name) + [&]()

///////////////////////////////////////////////////////////////////////////////

/*
 * Exception class for throwing assertions.
 */
struct FailedAssertion : std::exception {
  FailedAssertion(const char* msg, const char* file, unsigned line,
                  const char* func)
      : msg(msg)
      , file(file)
      , line(line)
      , func(func)
      , summary(makeSummary()) {}

  const char* what() const noexcept override {
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

///////////////////////////////////////////////////////////////////////////////

#define assert_impl(cond, fail) \
  ((cond) ? static_cast<void>(0) : ((fail), static_cast<void>(0)))

#define assert_fail_impl(e, msg) \
  ::HPHP::assert_fail(#e, __FILE__, __LINE__, __PRETTY_FUNCTION__, msg)

#define assert_fail_impl_no_log(e, msg) \
  ::HPHP::assert_fail_no_log(#e, __FILE__, __LINE__, __PRETTY_FUNCTION__, msg)

#define assert_throw_fail_impl(e) \
  throw ::HPHP::FailedAssertion(#e, __FILE__, __LINE__, __PRETTY_FUNCTION__)

#define always_assert(e)            assert_impl(e, assert_fail_impl(e, ""))
#define always_assert_no_log(e)    assert_impl(e, \
                                        assert_fail_impl_no_log(e, ""))
#define always_assert_log(e, l)     assert_impl(e, assert_fail_impl(e, l()))
#define always_assert_flog(e, ...)  assert_impl(e, assert_fail_impl(e,        \
                                        ::folly::format(__VA_ARGS__).str()))
#define always_assert_throw(e)      assert_impl(e, assert_throw_fail_impl(e))
#define always_assert_throw_log(e, l)                   \
  assert_impl(e, (::HPHP::assert_log_failure(#e, l()),  \
                  assert_throw_fail_impl(e)))

#undef assert

#ifndef NDEBUG
#define assert(e) always_assert(e)
#define assertx(e) always_assert(e)
#define assert_no_log(e) always_assert_no_log(e)
#define assert_log(e, l) always_assert_log(e, l)
#define assert_flog(e, ...) always_assert_flog(e, __VA_ARGS__)
#define assert_throw(e) always_assert_throw(e)
#define assert_throw_log(e, l) always_assert_throw_log(e, l)
#else
#define assert(e) static_cast<void>(0)
#define assertx(e) static_cast<void>(0)
#define assert_no_log(e) static_cast<void>(0)
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

///////////////////////////////////////////////////////////////////////////////

}

#endif
