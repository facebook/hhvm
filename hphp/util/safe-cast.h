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
#ifndef incl_HPHP_SAFE_CAST_H_
#define incl_HPHP_SAFE_CAST_H_

#include <string>
#include <boost/numeric/conversion/cast.hpp>

#include <folly/Conv.h>

#include "hphp/util/compilation-flags.h"

namespace HPHP {

//////////////////////////////////////////////////////////////////////

void safe_cast_failure(const std::string&, const char*, const char*)
  __attribute__((__noreturn__));

//////////////////////////////////////////////////////////////////////

/*
 * Optionally DEBUG-only wrappers around boost::numeric_cast that convert any
 * thrown exceptions to a failed assertion.
 */

template<typename To, typename From>
To always_safe_cast(From val) {
  try {
    return boost::numeric_cast<To>(val);
  } catch (std::bad_cast& bce) {
    safe_cast_failure(
      folly::to<std::string>(val),
      // The function is always safe_cast, but will indicate which
      // types were involved.
      __PRETTY_FUNCTION__,
      bce.what()
    );
  }
}

template<typename To, typename From>
To safe_cast(From val) {
  return debug ? always_safe_cast<To>(val) : static_cast<To>(val);
}

//////////////////////////////////////////////////////////////////////

}

#endif
