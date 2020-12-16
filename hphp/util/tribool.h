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

#include <cstdint>

namespace HPHP {

//////////////////////////////////////////////////////////////////////

/*
 * Represents definitely yes, definitely no, or maybe.
 */

enum class TriBool : uint8_t {
  Yes   = (1 << 0),
  No    = (1 << 1),
  Maybe = Yes | No
};

// Union represents "either", so values become Maybe unless both
// agree. The bit values of yes and no have been chosen to make this
// efficient.
inline TriBool operator|(TriBool a, TriBool b) {
  return static_cast<TriBool>(
    static_cast<std::underlying_type<TriBool>::type>(a) |
    static_cast<std::underlying_type<TriBool>::type>(b)
  );
}

inline TriBool& operator|=(TriBool& a, TriBool b) {
  a = a | b;
  return a;
}

inline TriBool yesOrNo(bool b) { return b ? TriBool::Yes : TriBool::No; }

inline const char* show(TriBool b) {
  if (b == TriBool::Yes) return "yes";
  if (b == TriBool::No) return "no";
  return "maybe";
}

//////////////////////////////////////////////////////////////////////

}
