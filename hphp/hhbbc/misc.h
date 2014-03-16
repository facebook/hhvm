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
#ifndef incl_HHBBC_MISC_H_
#define incl_HHBBC_MISC_H_

#include <chrono>
#include <cassert>

#include <boost/variant.hpp>
#include <memory>

#include "hphp/util/trace.h"
#include "hphp/util/match.h"

namespace HPHP {
struct StringData;
struct ArrayData;
}

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

/*
 * Self-documenting type alias for pointers that aren't owned.
 *
 * This type is intended to imply that someone else has an owning
 * pointer on this value which is guaranteed to live longer than this
 * pointer.
 */
template<class T> using borrowed_ptr = T*;

template<class T>
borrowed_ptr<T> borrow(const std::unique_ptr<T>& p) {
  return p.get();
}

/*
 * String that must be a static string, and and Array that must be a
 * static array.
 */
using SString = const StringData*;
using SArray  = const ArrayData*;

/*
 * HHBC evaluation stack flavors.
 */
enum class Flavor { C, V, A, R, F, U };

/*
 * Types of parameter preparation (or unknown).
 */
enum class PrepKind { Ref, Val, Unknown };

//////////////////////////////////////////////////////////////////////

/*
 * Many places in the code want to bump tracing levels by some amount
 * for systemlib-related processing.  This is the amount they all bump
 * by.
 */
constexpr int kSystemLibBump = 10;

//////////////////////////////////////////////////////////////////////

struct trace_time {
  using clock      = std::chrono::high_resolution_clock;
  using time_point = clock::time_point;

  explicit trace_time(const char* what,
                      const std::string& extra = std::string{})
    : what(what)
    , start(clock::now())
  {
    if (Trace::moduleEnabledRelease(Trace::hhbbc_time, 1)) {
      Trace::traceRelease(
        "%s",
        folly::format("{}: start{}\n",
          what,
          !extra.empty() ? folly::format(" ({})", extra).str() : extra
        ).str().c_str()
      );
    }
  }

  ~trace_time() {
    namespace C = std::chrono;
    DEBUG_ONLY auto const elapsed = C::duration_cast<C::milliseconds>(
      clock::now() - start
    );
    if (Trace::moduleEnabledRelease(Trace::hhbbc_time, 1)) {
      Trace::traceRelease(
        "%s",
        folly::format("{}: {}ms elapsed\n", what, elapsed.count())
          .str().c_str()
      );
    }
  }

  trace_time(const trace_time&) = delete;
  trace_time& operator=(const trace_time&) = delete;

private:
  const char* what;
  time_point start;
};

//////////////////////////////////////////////////////////////////////

/*
 * A smart pointer that does deep copies when you copy construct it.
 */
template<class T>
struct copy_ptr {
  copy_ptr() noexcept : m_p(nullptr) {}
  explicit copy_ptr(T* t) noexcept : m_p(t) {}

  copy_ptr(const copy_ptr& o) : m_p(o.m_p ? new T(*o.m_p) : nullptr) {}

  copy_ptr(copy_ptr&& o) noexcept
    : m_p(o.m_p)
  {
    o.m_p = nullptr;
  }

  copy_ptr& operator=(const copy_ptr& o) {
    if (!m_p) {
      m_p = o.m_p ? new T(*o.m_p) : nullptr;
      return *this;
    }

    if (o.m_p) {
      *m_p = *o.m_p;
      return *this;
    }

    delete m_p;
    m_p = nullptr;
    return *this;
  }

  copy_ptr& operator=(copy_ptr&& o) noexcept {
    delete m_p;
    m_p = o.m_p;
    o.m_p = nullptr;
    return *this;
  }

  ~copy_ptr() { delete m_p; assert((m_p = nullptr, true)); }

  explicit operator bool() const { return !!m_p; }

  T& operator*() const { return *m_p; }
  T* operator->() const { return m_p; }
  T* get() const { return m_p; }

private:
  T* m_p;
};

template<class T, class... Args>
copy_ptr<T> make_copy_ptr(Args&&... t) {
  return copy_ptr<T>(new T(std::forward<Args>(t)...));
}

//////////////////////////////////////////////////////////////////////

}}

#endif
