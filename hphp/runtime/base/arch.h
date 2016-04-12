/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_ARCH_H
#define incl_HPHP_ARCH_H

#include "hphp/runtime/base/runtime-option.h"

#include <boost/type_traits.hpp>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

enum class Arch { X64, ARM, PPC64, };

inline Arch arch() {
#if defined(__powerpc64__)
  return Arch::PPC64;
#else
  if (RuntimeOption::EvalSimulateARM) return Arch::ARM;
  return Arch::X64;
#endif
}

// MSVC's Preprocessor is completely idiotic, so we have to play by its
// rules and forcefully expand the variadic args so they aren't all
// interpreted as the first argument to func.
#define MSVC_GLUE(x, y) x y

/*
 * Macro for defining easy arch-dispatch wrappers.
 *
 * We need to specify the return type explicitly, or else we may drop refs.
 */
#define ARCH_SWITCH_CALL(func, ...)                  \
  ([&]() -> boost::function_traits<decltype(x64::func)>::result_type {  \
    switch (arch()) {                                \
      case Arch::X64:                                \
        return x64::MSVC_GLUE(func, (__VA_ARGS__));  \
      case Arch::ARM:                                \
        return arm::MSVC_GLUE(func, (__VA_ARGS__));  \
      case Arch::PPC64:                              \
        not_implemented();                           \
        break;                                       \
    }                                                \
    not_reached();                                   \
  }())

///////////////////////////////////////////////////////////////////////////////

}

#endif
