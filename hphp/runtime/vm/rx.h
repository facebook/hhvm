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

enum class RxLevel : uint32_t {
  None               = 0,
  ConditionalLocal   = 1,
  ConditionalShallow = 2,
  ConditionalRx      = 3,
  Local              = 4,
  Shallow            = 5,
  Rx                 = 6,
};

static_assert(AttrRxBit0 == (1u << 14), "");
static_assert(AttrRxBit1 == (1u << 15), "");
static_assert(AttrRxBit2 == (1u << 16), "");

constexpr RxLevel rxLevelFromAttr(Attr a) {
  return static_cast<RxLevel>((static_cast<uint32_t>(a) >> 14) & 7u);
}

constexpr Attr rxLevelToAttr(RxLevel r) {
  return static_cast<Attr>(static_cast<uint32_t>(r) << 14);
}

RxLevel rxLevelFromAttrString(const std::string& a);

const char* rxLevelToAttrString(RxLevel r);

///////////////////////////////////////////////////////////////////////////////
}

#define incl_HPHP_VM_RX_INL_H_
#include "hphp/runtime/vm/rx-inl.h"
#undef incl_HPHP_VM_RX_INL_H_

#endif // incl_HPHP_VM_RX_H_
