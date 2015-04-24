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
#include "hphp/runtime/vm/jit/service-requests.h"

#include <cstring>

#include "hphp/util/asm-x64.h"
#include "hphp/util/safe-cast.h"
#include "hphp/runtime/base/arch.h"
#include "hphp/runtime/vm/jit/back-end-x64.h"

namespace HPHP { namespace jit {

//////////////////////////////////////////////////////////////////////

namespace {

void only_x64() {
  switch (arch()) {
  case Arch::ARM: always_assert_flog(false, "not implemented");
  case Arch::X64: break;
  }
}

}

//////////////////////////////////////////////////////////////////////

FPInvOffset serviceReqSPOff(TCA addr) {
  only_x64();

  DecodedInstruction instr(addr);
  always_assert(instr.isLea());
  auto const offBytes = safe_cast<int32_t>(instr.offset());
  always_assert((offBytes % sizeof(Cell)) == 0);
  return FPInvOffset{-(offBytes / int32_t{sizeof(Cell)})};
}

void adjustBindJmpPatchableJmpAddress(TCA addr,
                                      bool targetIsResumed,
                                      TCA newJmpIp) {
  only_x64();

  // We rely on emitServiceReqWork putting an optional lea for the SP offset
  // first (depending on whether the target SrcKey is a resumed function0,
  // followed by an RIP relative lea of the jump address.
  if (!targetIsResumed) {
    DecodedInstruction instr(addr);
    always_assert(instr.isLea());
    addr += instr.size();
  }
  auto const leaIp = addr;
  always_assert((leaIp[0] & 0x48) == 0x48); // REX.W
  always_assert(leaIp[1] == 0x8d); // lea
  auto const afterLea = leaIp + x64::kRipLeaLen;
  auto const delta = safe_cast<int32_t>(newJmpIp - afterLea);
  std::memcpy(afterLea - sizeof(delta), &delta, sizeof(delta));
}

//////////////////////////////////////////////////////////////////////

}}
