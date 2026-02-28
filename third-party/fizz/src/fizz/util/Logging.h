/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */
#pragma once

/**
 * fizz/util/Logging.h defines the FIZZ_LOG, FIZZ_VLOG, FIZZ_CHECK, and
 * FIZZ_DCHECK macros.
 */

// We hardcode this for now as we migrate call sites. In future diffs this
// will move to fizz-config.h and be configurable at build time.
#define FIZZ_LOGGING_GLOG 1

#if FIZZ_LOGGING_GLOG

#include <folly/GLog.h>
#define FIZZ_VLOG VLOG
#define FIZZ_LOG LOG
#define FIZZ_CHECK CHECK
#define FIZZ_CHECK_EQ CHECK_EQ
#define FIZZ_CHECK_NE CHECK_NE
#define FIZZ_CHECK_GE CHECK_GE
#define FIZZ_CHECK_GT CHECK_GT
#define FIZZ_CHECK_LE CHECK_LE
#define FIZZ_CHECK_LT CHECK_LT
#define FIZZ_PCHECK PCHECK
#define FIZZ_DCHECK DCHECK
#define FIZZ_DCHECK_EQ DCHECK_EQ
#define FIZZ_DCHECK_NE DCHECK_NE
#define FIZZ_DCHECK_GE DCHECK_GE
#define FIZZ_DCHECK_GT DCHECK_GT
#define FIZZ_DCHECK_LE DCHECK_LE
#define FIZZ_DCHECK_LT DCHECK_LT

#elif FIZZ_LOGGING_FOLLY_LOGGING

#include <folly/logging/xlog.h>
#define FIZZ_LOGGING_CONCAT_(a, b) a##b

// Mapping between glog level tokens -> the corresponding folly::xlog
// equivalent.
#define FIZZ_LOGLEVEL_INFO INFO
#define FIZZ_LOGLEVEL_WARNING WARN
#define FIZZ_LOGLEVEL_ERROR ERR
#define FIZZ_LOGLEVEL_FATAL FATAL
#define FIZZ_LOGLEVEL_DFATAL DFATAL

#define FIZZ_VLOG(n) XLOG(FIZZ_LOGGING_CONCAT_(DBG, n))
#define FIZZ_LOG(level) XLOG(FIZZ_LOGLEVEL_##level)

#define FIZZ_CHECK XCHECK
#define FIZZ_CHECK_EQ XCHECK_EQ
#define FIZZ_CHECK_NE XCHECK_NE
#define FIZZ_CHECK_GE XCHECK_GE
#define FIZZ_CHECK_GT XCHECK_GT
#define FIZZ_CHECK_LE XCHECK_LE
#define FIZZ_CHECK_LT XCHECK_LT
#define FIZZ_PCHECK XCHECK
#define FIZZ_DCHECK XDCHECK
#define FIZZ_DCHECK_EQ XDCHECK_EQ
#define FIZZ_DCHECK_NE XDCHECK_NE
#define FIZZ_DCHECK_GE XDCHECK_GE
#define FIZZ_DCHECK_GT XDCHECK_GT
#define FIZZ_DCHECK_LE XDCHECK_LE
#define FIZZ_DCHECK_LT XDCHECK_LT

#else

// Fizz logging disabled
//
// DCHECK()s are mapped to assert(...)
// CHECK()s are mapped to if (!(expr)) { abort(); }
//
// All log output is silently dropped

#include <cassert>
#include <cstdlib>

namespace fizz::logging::detail {
struct NoopStream {};

template <class T>
inline NoopStream operator<<(NoopStream stream, T&&) {
  return stream;
}

} // namespace fizz::logging::detail

// The value of this is not important. These definitions are here to make sure
// that the loglevel of FIZZ_LOG() is either {INFO, WARNING, ERROR, FATAL,
// DFATAL} tokens since anything else will cause the comma operator expansion
// to fail.
#define FIZZ_LOGLEVEL_INFO 0
#define FIZZ_LOGLEVEL_WARNING 0
#define FIZZ_LOGLEVEL_ERROR 0
#define FIZZ_LOGLEVEL_FATAL 0
#define FIZZ_LOGLEVEL_DFATAL 0

#define FIZZ_DCHECK_BINOP_(a, op, b)              \
  ([&] {                                          \
    assert((a)op(b));                             \
    return ::fizz::logging::detail::NoopStream{}; \
  }())
#define FIZZ_CHECK_BINOP_(a, op, b)               \
  ([&] {                                          \
    if (!((a)op(b))) {                            \
      std::abort();                               \
    }                                             \
    return ::fizz::logging::detail::NoopStream{}; \
  }())
#define FIZZ_VLOG(level) \
  ([](int) { return ::fizz::logging::detail::NoopStream{}; }((level)))
#define FIZZ_LOG(level) \
  ((void)FIZZ_LOGLEVEL_##level, ::fizz::logging::detail::NoopStream{})
#define FIZZ_CHECK(expr)                          \
  ([&] {                                          \
    if (!(expr)) {                                \
      std::abort();                               \
    }                                             \
    return ::fizz::logging::detail::NoopStream{}; \
  }())
#define FIZZ_CHECK_EQ(a, b) FIZZ_CHECK_BINOP_(a, ==, b)
#define FIZZ_CHECK_NE(a, b) FIZZ_CHECK_BINOP_(a, !=, b)
#define FIZZ_CHECK_GE(a, b) FIZZ_CHECK_BINOP_(a, >=, b)
#define FIZZ_CHECK_GT(a, b) FIZZ_CHECK_BINOP_(a, >, b)
#define FIZZ_CHECK_LE(a, b) FIZZ_CHECK_BINOP_(a, <=, b)
#define FIZZ_CHECK_LT(a, b) FIZZ_CHECK_BINOP_(a, <, b)
#define FIZZ_PCHECK FIZZ_CHECK
#define FIZZ_DCHECK(expr)                         \
  ([&] {                                          \
    assert((expr));                               \
    return ::fizz::logging::detail::NoopStream{}; \
  }())
#define FIZZ_DCHECK_EQ(a, b) FIZZ_DCHECK_BINOP_(a, ==, b)
#define FIZZ_DCHECK_NE(a, b) FIZZ_DCHECK_BINOP_(a, !=, b)
#define FIZZ_DCHECK_GE(a, b) FIZZ_DCHECK_BINOP_(a, >=, b)
#define FIZZ_DCHECK_GT(a, b) FIZZ_DCHECK_BINOP_(a, >, b)
#define FIZZ_DCHECK_LE(a, b) FIZZ_DCHECK_BINOP_(a, <=, b)
#define FIZZ_DCHECK_LT(a, b) FIZZ_DCHECK_BINOP_(a, <, b)

#endif
