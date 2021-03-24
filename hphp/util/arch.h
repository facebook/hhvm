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

#include "hphp/util/assertions.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

enum class Arch { X64, ARM, };

constexpr Arch arch() {
#if defined(__aarch64__)
  return Arch::ARM;
#else
  return Arch::X64;
#endif
}

constexpr bool arch_any() { return false; }

/*
 * Convenience helper for guarding on a set of architectures.
 */
template<class... Tail>
bool arch_any(Arch a, Tail&&... archs) {
  return arch() == a || arch_any(archs...);
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
#define ARCH_SWITCH_CALL(func, ...)                                   \
  ([&]() -> decltype(x64::func(__VA_ARGS__)) {                        \
    switch (arch()) {                                                 \
      case Arch::X64:                                                 \
        return x64::MSVC_GLUE(func, (__VA_ARGS__));                   \
      case Arch::ARM:                                                 \
        return arm::MSVC_GLUE(func, (__VA_ARGS__));                   \
    }                                                                 \
    not_reached();                                                    \
  }())

///////////////////////////////////////////////////////////////////////////////

}

