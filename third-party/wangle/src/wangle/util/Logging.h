/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#pragma once

#include <wangle/wangle-config.h>

/**
 * wangle/util/Logging.h defines the WANGLE_LOG, WANGLE_VLOG, WANGLE_CHECK,
 * WANGLE_DCHECK, and WANGLE_PCHECK macros.
 *
 * The backend is selected by exactly one of WANGLE_LOGGING_GLOG,
 * WANGLE_LOGGING_XLOG, or WANGLE_LOGGING_DISABLED being defined to 1
 * in <wangle/wangle-config.h>. CMake derives that from -DWANGLE_LOGGING_BACKEND
 * ({GLOG,XLOG,DISABLED}); Buck consumers pick a hand-written variant via the
 * //wangle:config target.
 *
 * Macro authoring constraints:
 *   - WANGLE_LOG(level): `level` must be one of the unqualified glog tokens
 *     INFO, WARNING, ERROR, FATAL, DFATAL.
 *   - WANGLE_VLOG(n): `n` must be an integer literal 0..9. Values >9 are
 *     accepted (saturated to DBG9 in folly-logging mode) but for granularity
 *     finer than 0..9, use the appropriate WANGLE_LOG level instead.
 *   - WANGLE_PCHECK loses the `: <strerror(errno)>` suffix in folly-logging
 *     and disabled modes. TODO: fix once folly exposes XPCHECK.
 */

#if WANGLE_LOGGING_GLOG

#include <folly/GLog.h>
#define WANGLE_VLOG VLOG
#define WANGLE_VLOG_IF VLOG_IF
#define WANGLE_LOG LOG
#define WANGLE_LOG_IF LOG_IF
#define WANGLE_LOG_EVERY_MS FB_LOG_EVERY_MS
#define WANGLE_CHECK CHECK
#define WANGLE_CHECK_EQ CHECK_EQ
#define WANGLE_CHECK_NE CHECK_NE
#define WANGLE_CHECK_GE CHECK_GE
#define WANGLE_CHECK_GT CHECK_GT
#define WANGLE_CHECK_LE CHECK_LE
#define WANGLE_CHECK_LT CHECK_LT
#define WANGLE_CHECK_NOTNULL CHECK_NOTNULL
#define WANGLE_PCHECK PCHECK
#define WANGLE_DCHECK DCHECK
#define WANGLE_DCHECK_EQ DCHECK_EQ
#define WANGLE_DCHECK_NE DCHECK_NE
#define WANGLE_DCHECK_GE DCHECK_GE
#define WANGLE_DCHECK_GT DCHECK_GT
#define WANGLE_DCHECK_LE DCHECK_LE
#define WANGLE_DCHECK_LT DCHECK_LT

#elif WANGLE_LOGGING_XLOG

#include <folly/logging/xlog.h>

// Mapping from glog level tokens to the corresponding folly::xlog values.
#define WANGLE_LOGLEVEL_INFO INFO
#define WANGLE_LOGLEVEL_WARNING WARN
#define WANGLE_LOGLEVEL_ERROR ERR
#define WANGLE_LOGLEVEL_FATAL FATAL
#define WANGLE_LOGLEVEL_DFATAL DFATAL

// glog's VLOG(n) accepts arbitrary integer verbosity; folly's xlog only
// defines DBG0..DBG9. Map 0..9 directly; saturate higher values to DBG9 so
// VLOG(10+) call sites still compile in folly-logging mode. The cap is
// documented at the top of this header.
#define WANGLE_LOGGING_DBG_0 DBG0
#define WANGLE_LOGGING_DBG_1 DBG1
#define WANGLE_LOGGING_DBG_2 DBG2
#define WANGLE_LOGGING_DBG_3 DBG3
#define WANGLE_LOGGING_DBG_4 DBG4
#define WANGLE_LOGGING_DBG_5 DBG5
#define WANGLE_LOGGING_DBG_6 DBG6
#define WANGLE_LOGGING_DBG_7 DBG7
#define WANGLE_LOGGING_DBG_8 DBG8
#define WANGLE_LOGGING_DBG_9 DBG9
#define WANGLE_LOGGING_DBG_10 DBG9
#define WANGLE_LOGGING_DBG_11 DBG9
#define WANGLE_LOGGING_DBG_12 DBG9
#define WANGLE_LOGGING_DBG_13 DBG9
#define WANGLE_LOGGING_DBG_14 DBG9
#define WANGLE_LOGGING_DBG_15 DBG9

#define WANGLE_VLOG(n) XLOG(WANGLE_LOGGING_DBG_##n)
#define WANGLE_VLOG_IF(n, cond) XLOG_IF(WANGLE_LOGGING_DBG_##n, (cond))
#define WANGLE_LOG(level) XLOG(WANGLE_LOGLEVEL_##level)
#define WANGLE_LOG_IF(level, cond) XLOG_IF(WANGLE_LOGLEVEL_##level, (cond))
#define WANGLE_LOG_EVERY_MS(level, ms) \
  XLOG_EVERY_MS(WANGLE_LOGLEVEL_##level, ms)

#define WANGLE_CHECK XCHECK
#define WANGLE_CHECK_EQ XCHECK_EQ
#define WANGLE_CHECK_NE XCHECK_NE
#define WANGLE_CHECK_GE XCHECK_GE
#define WANGLE_CHECK_GT XCHECK_GT
#define WANGLE_CHECK_LE XCHECK_LE
#define WANGLE_CHECK_LT XCHECK_LT
// XCHECK_NOTNULL is not provided by folly; emulate via a generic lambda that
// asserts non-null and returns the original (possibly-rvalue) pointer so call
// sites like `auto p = WANGLE_CHECK_NOTNULL(expr);` keep working.
#define WANGLE_CHECK_NOTNULL(p)                          \
  ([](auto&& _wangle_p) -> decltype(auto) {              \
    XCHECK(_wangle_p != nullptr);                        \
    return std::forward<decltype(_wangle_p)>(_wangle_p); \
  }(p))
// XCHECK does not append : <strerror(errno)> the way glog's PCHECK does.
// TODO: replace with XPCHECK once folly provides one.
#define WANGLE_PCHECK XCHECK
#define WANGLE_DCHECK XDCHECK
#define WANGLE_DCHECK_EQ XDCHECK_EQ
#define WANGLE_DCHECK_NE XDCHECK_NE
#define WANGLE_DCHECK_GE XDCHECK_GE
#define WANGLE_DCHECK_GT XDCHECK_GT
#define WANGLE_DCHECK_LE XDCHECK_LE
#define WANGLE_DCHECK_LT XDCHECK_LT

#else // WANGLE_LOGGING_DISABLED

// Wangle logging disabled
//
// WANGLE_DCHECK()s are mapped to assert(...)
// WANGLE_CHECK()s and WANGLE_PCHECK()s are mapped to if (!(expr)) { abort(); }
//   (PCHECK loses the strerror(errno) suffix glog provides; see TODO above.)
//
// All log output is silently dropped.

#include <cassert>
#include <cstdlib>
#include <utility>

namespace wangle::logging::detail {
struct NoopStream {};

template <class T>
inline NoopStream operator<<(NoopStream stream, T&&) {
  return stream;
}

} // namespace wangle::logging::detail

// The value of this is not important. These definitions are here to make sure
// that the loglevel of WANGLE_LOG() is either {INFO, WARNING, ERROR, FATAL,
// DFATAL} tokens since anything else will cause the comma operator expansion
// to fail.
#define WANGLE_LOGLEVEL_INFO 0
#define WANGLE_LOGLEVEL_WARNING 0
#define WANGLE_LOGLEVEL_ERROR 0
#define WANGLE_LOGLEVEL_FATAL 0
#define WANGLE_LOGLEVEL_DFATAL 0

#define WANGLE_DCHECK_BINOP_(a, op, b)              \
  ([&] {                                            \
    assert((a)op(b));                               \
    return ::wangle::logging::detail::NoopStream{}; \
  }())
#define WANGLE_CHECK_BINOP_(a, op, b)               \
  ([&] {                                            \
    if (!((a)op(b))) {                              \
      std::abort();                                 \
    }                                               \
    return ::wangle::logging::detail::NoopStream{}; \
  }())
#define WANGLE_VLOG(level) \
  ([](int) { return ::wangle::logging::detail::NoopStream{}; }((level)))
#define WANGLE_VLOG_IF(level, cond)                                   \
  ([](int, bool) { return ::wangle::logging::detail::NoopStream{}; }( \
       (level), static_cast<bool>(cond)))
#define WANGLE_LOG(level) \
  ((void)WANGLE_LOGLEVEL_##level, ::wangle::logging::detail::NoopStream{})
#define WANGLE_LOG_IF(level, cond) \
  ((void)WANGLE_LOGLEVEL_##level,  \
   (void)static_cast<bool>(cond),  \
   ::wangle::logging::detail::NoopStream{})
#define WANGLE_LOG_EVERY_MS(level, ms) \
  ((void)WANGLE_LOGLEVEL_##level,      \
   (void)(ms),                         \
   ::wangle::logging::detail::NoopStream{})
#define WANGLE_CHECK(expr)                          \
  ([&] {                                            \
    if (!(expr)) {                                  \
      std::abort();                                 \
    }                                               \
    return ::wangle::logging::detail::NoopStream{}; \
  }())
#define WANGLE_CHECK_EQ(a, b) WANGLE_CHECK_BINOP_(a, ==, b)
#define WANGLE_CHECK_NE(a, b) WANGLE_CHECK_BINOP_(a, !=, b)
#define WANGLE_CHECK_GE(a, b) WANGLE_CHECK_BINOP_(a, >=, b)
#define WANGLE_CHECK_GT(a, b) WANGLE_CHECK_BINOP_(a, >, b)
#define WANGLE_CHECK_LE(a, b) WANGLE_CHECK_BINOP_(a, <=, b)
#define WANGLE_CHECK_LT(a, b) WANGLE_CHECK_BINOP_(a, <, b)
#define WANGLE_CHECK_NOTNULL(p)                          \
  ([](auto&& _wangle_p) -> decltype(auto) {              \
    if (_wangle_p == nullptr) {                          \
      std::abort();                                      \
    }                                                    \
    return std::forward<decltype(_wangle_p)>(_wangle_p); \
  }(p))
// PCHECK loses the strerror(errno) suffix glog provides; see TODO above.
#define WANGLE_PCHECK WANGLE_CHECK
#define WANGLE_DCHECK(expr)                         \
  ([&] {                                            \
    assert((expr));                                 \
    return ::wangle::logging::detail::NoopStream{}; \
  }())
#define WANGLE_DCHECK_EQ(a, b) WANGLE_DCHECK_BINOP_(a, ==, b)
#define WANGLE_DCHECK_NE(a, b) WANGLE_DCHECK_BINOP_(a, !=, b)
#define WANGLE_DCHECK_GE(a, b) WANGLE_DCHECK_BINOP_(a, >=, b)
#define WANGLE_DCHECK_GT(a, b) WANGLE_DCHECK_BINOP_(a, >, b)
#define WANGLE_DCHECK_LE(a, b) WANGLE_DCHECK_BINOP_(a, <=, b)
#define WANGLE_DCHECK_LT(a, b) WANGLE_DCHECK_BINOP_(a, <, b)

#endif
