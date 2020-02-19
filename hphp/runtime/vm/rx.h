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

#ifndef incl_HPHP_VM_RX_H_
#define incl_HPHP_VM_RX_H_

#include "hphp/runtime/base/attr.h"

namespace HPHP {
///////////////////////////////////////////////////////////////////////////////

enum class RxLevel : uint8_t {
  None               = 0,
  Local              = 1,
  Shallow            = 2,
  Rx                 = 3,
  Pure               = 4,
};

constexpr int kRxAttrShift = 14;
#define ASSERT_LEVEL(attr, rl) \
  static_assert(static_cast<RxLevel>(attr >> kRxAttrShift) == RxLevel::rl, "")
ASSERT_LEVEL(AttrRxLevel0, Local);
ASSERT_LEVEL(AttrRxLevel1, Shallow);
ASSERT_LEVEL((AttrRxLevel0 | AttrRxLevel1), Rx);
ASSERT_LEVEL(AttrRxLevel2, Pure);
#undef ASSERT_LEVEL

constexpr uint32_t kRxAttrMask = AttrRxLevel0 | AttrRxLevel1 | AttrRxLevel2;
constexpr uint32_t kRxLevelMask = 7u;
static_assert(kRxAttrMask >> kRxAttrShift == kRxLevelMask, "");
static_assert(AttrRxNonConditional == (8u << kRxAttrShift), "");


constexpr RxLevel rxLevelFromAttr(Attr attrs) {
  return static_cast<RxLevel>(
    (static_cast<uint32_t>(attrs) >> kRxAttrShift) & kRxLevelMask
  );
}

constexpr bool rxConditionalFromAttr(Attr attrs) {
  return !(attrs & AttrRxNonConditional);
}

constexpr Attr rxMakeAttr(RxLevel level, bool conditional) {
  return static_cast<Attr>(static_cast<uint32_t>(level) << kRxAttrShift)
    | (conditional ? AttrNone : AttrRxNonConditional);
}

Attr rxAttrsFromAttrString(const std::string& a);
const char* rxAttrsToAttrString(Attr a);

const char* rxLevelToString(RxLevel r);

constexpr bool funcAttrIsAnyRx(Attr a) {
  return static_cast<uint32_t>(a) & kRxAttrMask;
}

bool rxEnforceCallsInLevel(RxLevel level);
RxLevel rxRequiredCalleeLevel(RxLevel level);

///////////////////////////////////////////////////////////////////////////////
}

#define incl_HPHP_VM_RX_INL_H_
#include "hphp/runtime/vm/rx-inl.h"
#undef incl_HPHP_VM_RX_INL_H_

#endif // incl_HPHP_VM_RX_H_
