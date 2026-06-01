/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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

#pragma once

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

namespace arch {

struct X64 {};
struct ARM {};

template <typename T>
constexpr bool any() {
  return false;
}

template <>
constexpr bool any<X64>() {
#if defined(__aarch64__)
  return false;
#else
  return true;
#endif
}

template <>
constexpr bool any<ARM>() {
#if defined(__aarch64__)
  return true;
#else
  return false;
#endif
}

template <typename T, typename T2, typename... Tail>
constexpr bool any() {
  if constexpr (any<T>()) return true;
  return any<T2, Tail...>();
}

}

/*
 * MSVC's Preprocessor is completely idiotic, so we have to play by its rules
 * and forcefully expand the variadic args so they aren't all interpreted as the
 * first argument to func.
 */
#define MSVC_GLUE(x, y) x y

/*
 * Macro for defining easy arch-dispatch wrappers.
 *
 * We need to specify the return type explicitly, or else we may drop refs.
 */
#ifdef __aarch64__
#define ARCH_SWITCH_CALL(func, ...)             \
  ([&]() -> decltype(auto) {                    \
    return arm::MSVC_GLUE(func, (__VA_ARGS__)); \
  }())
#else
#define ARCH_SWITCH_CALL(func, ...)             \
  ([&]() -> decltype(auto) {                    \
    return x64::MSVC_GLUE(func, (__VA_ARGS__)); \
  }())
#endif

#ifdef __aarch64__
#define ARCH_MATCH(x64_lambda, arm_lambda)      \
  ([&]() -> decltype(auto) {                    \
    return (arm_lambda)(arch::ARM{});           \
  }())
#else
#define ARCH_MATCH(x64_lambda, arm_lambda)      \
  ([&]() -> decltype(auto) {                    \
    return (x64_lambda)(arch::X64{});           \
  }())
#endif

///////////////////////////////////////////////////////////////////////////////

}
