/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/service-requests-x64.h"

#include "hphp/runtime/base/arch.h"
#include "hphp/runtime/base/typed-value.h"
#include "hphp/runtime/vm/srckey.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/back-end-x64.h"
#include "hphp/runtime/vm/jit/back-end.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-x64.h"
#include "hphp/runtime/vm/jit/mc-generator-internal.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/vasm-print.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/ringbuffer.h"
#include "hphp/util/safe-cast.h"

#include <folly/Optional.h>

#include <cstring>

namespace HPHP { namespace jit { namespace x64 {

using jit::reg::rip;

TRACE_SET_MOD(servicereq);

namespace {

/*
 * Work to be done for jmp-smashing service requests before the service request
 * is emitted.
 *
 * Most notably, we must check if the CodeBlock for the jmp and for the stub
 * are aliased.  If so, we reserve space for the jmp(s) which we'll emit
 * properly after the service request stub is emitted.  If the service request
 * is being jumped to conditionally, we'll also need an unconditional jump that
 * jumps over it (`secondary' in SmashInfo---nullptr if this isn't needed).
 */
struct SmashInfo { TCA primary; TCA secondary; };
ALWAYS_INLINE
SmashInfo emitBindJPre(CodeBlock& cb, CodeBlock& frozen, ConditionCode cc) {
  mcg->backEnd().prepareForSmash(cb, cc == CC_None ? kJmpLen : kJmpccLen);

  TCA toSmash = cb.frontier();
  TCA jmpSmash = nullptr;
  if (cb.base() == frozen.base()) {
    mcg->backEnd().emitSmashableJump(cb, toSmash, cc);

    /*
     * If we're emitting a conditional jump to the service request, the
     * fallthrough (jcc is not taken) will need a jump over the service request
     * code.  We could try to invert the jcc to generate better code, but this
     * only happens if we're emitting code in frozen anyway, so hopefully it
     * isn't hot.  To do that would require inverting the information we pass
     * everywhere about how to smash the jump, also.
     */
    if (cc != CC_None) {
      jmpSmash = cb.frontier();
      Asm a { cb };
      a.jmp(jmpSmash);
    }
  }

  mcg->setJmpTransID(toSmash);

  return { toSmash, jmpSmash };
}

/*
 * Work to be done for jmp-smashing service requests after the service request
 * stub is emitted.
 */
ALWAYS_INLINE
void emitBindJPost(CodeBlock& cb,
                   CodeBlock& frozen,
                   ConditionCode cc,
                   SmashInfo smashInfo,
                   TCA sr) {
  if (cb.base() == frozen.base()) {
    if (smashInfo.secondary) {
      auto const fallthrough = cb.frontier();
      CodeCursor cursor(cb, smashInfo.secondary);
      Asm a { cb };
      a.jmp(fallthrough);
    }
    CodeCursor cursor(cb, smashInfo.primary);
    mcg->backEnd().emitSmashableJump(cb, sr, cc);
    return;
  }

  mcg->backEnd().emitSmashableJump(cb, sr, cc);
}

} // anonymous namespace

//////////////////////////////////////////////////////////////////////

void emitBindJ(CodeBlock& cb,
               CodeBlock& frozen,
               ConditionCode cc,
               SrcKey dest,
               FPInvOffset spOff,
               TransFlags trflags) {
  auto const smashInfo = emitBindJPre(cb, frozen, cc);
  auto optSPOff = folly::Optional<FPInvOffset>{};
  if (!dest.resumed()) optSPOff = spOff;
  auto const sr = svcreq::emit_ephemeral(
    frozen,
    mcg->getFreeStub(frozen, &mcg->cgFixups()),
    optSPOff,
    REQ_BIND_JMP,
    smashInfo.primary,
    dest.toAtomicInt(),
    trflags.packed
  );
  emitBindJPost(cb, frozen, cc, smashInfo, sr);
}

TCA emitRetranslate(CodeBlock& cb,
                    CodeBlock& frozen,
                    jit::ConditionCode cc,
                    SrcKey dest,
                    folly::Optional<FPInvOffset> spOff,
                    TransFlags trflags) {
  auto const toSmash = emitBindJPre(cb, frozen, cc);
  auto const sr = svcreq::emit_persistent(
    frozen,
    spOff,
    REQ_RETRANSLATE,
    dest.offset(),
    trflags.packed
  );
  emitBindJPost(cb, frozen, cc, toSmash, sr);

  return toSmash.primary;
}

TCA emitBindAddr(CodeBlock& cb,
                 CodeBlock& frozen,
                 TCA* addr,
                 SrcKey sk,
                 FPInvOffset spOff) {
  const bool needsJump = cb.base() == frozen.base();
  TCA jumpAddr = nullptr;

  if (needsJump) {
    jumpAddr = cb.frontier();
    Asm as{cb};
    as.jmp(jumpAddr);
  }

  auto optSPOff = folly::Optional<FPInvOffset>{};
  if (!sk.resumed()) optSPOff = spOff;

  auto const sr = svcreq::emit_ephemeral(
    frozen,
    mcg->getFreeStub(frozen, &mcg->cgFixups()),
    optSPOff,
    REQ_BIND_ADDR,
    addr,
    sk.toAtomicInt(),
    TransFlags{}.packed
  );

  mcg->cgFixups().m_codePointers.insert(addr);

  if (needsJump) {
    assert(jumpAddr);
    TCA target = cb.frontier();
    CodeCursor cursor(cb, jumpAddr);
    Asm as{cb};
    as.jmp(target);
  }

  return sr;
}

void emitBindJmpccFirst(CodeBlock& cb,
                        CodeBlock& frozen,
                        ConditionCode cc,
                        SrcKey targetSk0,
                        SrcKey targetSk1,
                        FPInvOffset spOff) {
  mcg->backEnd().prepareForTestAndSmash(cb, 0,
                                        TestAndSmashFlags::kAlignJccAndJmp);
  Asm as{cb};
  auto const jccAddr = cb.frontier();
  mcg->setJmpTransID(jccAddr);
  as.jcc(cc, jccAddr);
  mcg->setJmpTransID(cb.frontier());
  as.jmp(cb.frontier());

  always_assert_flog(targetSk0.resumed() == targetSk1.resumed(),
                     "jmpcc service request was confused about resumables");
  auto optSPOff = folly::Optional<FPInvOffset>{};
  if (!targetSk0.resumed()) optSPOff = spOff;

  auto const sr = svcreq::emit_ephemeral(
    frozen,
    mcg->getFreeStub(frozen, &mcg->cgFixups()),
    optSPOff,
    REQ_BIND_JCC_FIRST,
    jccAddr,
    targetSk1.toAtomicInt(),
    targetSk0.toAtomicInt(),
    cc
  );
  CodeCursor cursor{cb, jccAddr};
  as.jcc(cc, sr);
  as.jmp(sr);
}

}

///////////////////////////////////////////////////////////////////////////////

void adjustBindJmpPatchableJmpAddress(TCA addr,
                                      bool targetIsResumed,
                                      TCA newJmpIp) {
  assert_not_implemented(arch() == Arch::X64);

  // We rely on emitServiceReqWork putting an optional lea for the SP offset
  // first (depending on whether the target SrcKey is a resumed function),
  // followed by an RIP relative lea of the jump address.
  if (!targetIsResumed) {
    DecodedInstruction instr(addr);
    addr += instr.size();
  }
  auto const leaIp = addr;
  always_assert((leaIp[0] & 0x48) == 0x48); // REX.W
  always_assert(leaIp[1] == 0x8d); // lea
  auto const afterLea = leaIp + x64::kRipLeaLen;
  auto const delta = safe_cast<int32_t>(newJmpIp - afterLea);
  std::memcpy(afterLea - sizeof(delta), &delta, sizeof(delta));
}

///////////////////////////////////////////////////////////////////////////////

}}
