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

#ifndef incl_HPHP_VM_MEMBER_KEY_H_
#define incl_HPHP_VM_MEMBER_KEY_H_

#include "hphp/util/hash.h"

#include <folly/Optional.h>

#include <cstdint>
#include <string>

namespace HPHP {

struct StringData;

/**
 * E - an element, $x['y']
 * P - a property, $x->y
 * Q - a NullSafe version of P, $x?->y
 */
enum MemberCode : uint8_t {
  // Element and property, consuming a cell from the stack.
  MEC,
  MPC,

  // Element and property, using an immediate local id.
  MEL,
  MPL,

  // Element and property, using a string immediate
  MET,
  MPT,
  MQT,

  // Element, using an int64 immediate
  MEI,

  // New element operation: no key. If this is ever not the last MemberCode,
  // NumMemberCodes must be adjusted.
  MW,
};

constexpr size_t NumMemberCodes = MW + 1;

/*
 * Returns string representation of `mc'. Pointer to internal static data, does
 * not need to be freed.
 */
const char* memberCodeString(MemberCode mc);

/*
 * Try to parse the input as a MemberCode, returning folly::none on failure.
 */
folly::Optional<MemberCode> parseMemberCode(const char*);

constexpr bool mcodeIsProp(MemberCode mcode) {
  return mcode == MPC || mcode == MPL || mcode == MPT || mcode == MQT;
}

constexpr bool mcodeIsElem(MemberCode mcode) {
  return mcode == MEC || mcode == MEL || mcode == MET || mcode == MEI;
}

/*
 * Lookup key for a member operation: combination of a MemberCode and local id,
 * stack offset, int64, or literal string.
 */
struct MemberKey {
  MemberKey()
    : mcode{MW}
    , iva{0}
  {}

  MemberKey(MemberCode mcode, int32_t iva)
    : mcode{mcode}
    , iva{iva}
  {}

  MemberKey(MemberCode mcode, int64_t int64)
    : mcode{mcode}
    , int64{int64}
  {}

  MemberKey(MemberCode mcode, const StringData* litstr)
    : mcode{mcode}
    , litstr{litstr}
  {}

  MemberCode mcode;
  union {
    int64_t iva;
    int64_t int64;
    const StringData* litstr;
  };
};

inline bool operator==(MemberKey a, MemberKey b) {
  return a.mcode == b.mcode && a.int64 == b.int64;
}
inline bool operator!=(MemberKey a, MemberKey b) {
  return !(a == b);
}

std::string show(MemberKey);

}

#endif
