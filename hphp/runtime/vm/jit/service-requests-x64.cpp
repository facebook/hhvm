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

#include "hphp/runtime/vm/jit/service-requests-x64.h"

#include <folly/Optional.h>

#include "hphp/runtime/vm/jit/code-gen-helpers-x64.h"
#include "hphp/runtime/vm/jit/back-end.h"
#include "hphp/runtime/vm/jit/back-end-x64.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/mc-generator-internal.h"
#include "hphp/runtime/vm/jit/service-requests-inline.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/util/asm-x64.h"
#include "hphp/util/ringbuffer.h"

namespace HPHP { namespace jit { namespace x64 {

using jit::reg::rip;

TRACE_SET_MOD(servicereq);

namespace {

constexpr int kMovSize = 0xa;
constexpr int kLeaVmSpSize = 0x7;

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

const int kExtraRegs = 2; // we also set rdi and r10
static constexpr int maxStubSpace() {
  /* max space for emitting args */
  return (kNumServiceReqArgRegs + kExtraRegs) * kMovSize + kLeaVmSpSize;
}

// fill remaining space in stub with ud2 or int3
void padStub(CodeBlock& stub) {
  Asm a{stub};
  // do not use nops, or the relocator will strip them out
  while (stub.available() >= 2) a.ud2();
  if (stub.available() > 0) a.int3();
  assertx(stub.available() == 0);
}

void emitServiceReqImpl(CodeBlock& stub,
                        SRFlags flags,
                        folly::Optional<FPInvOffset> spOff,
                        ServiceRequest req,
                        const ServiceReqArgVec& argv) {
  const bool persist = flags & SRFlags::Persist;
  Asm as{stub};
  FTRACE(2, "Emit Service Req @{} {}(", stub.base(), serviceReqName(req));

  /*
   * If we have an spOff, materialize rVmSp so that handleSRHelper can do a vm
   * reg sync.  When we don't have an spOff, the caller of the service request
   * was responsible for making sure rVmSp already contained the top of the
   * stack.
   */
  if (spOff) {
    as.    lea(rVmFp[-cellsToBytes(spOff->offset)], rVmSp);
  }

  /*
   * Move args into appropriate regs. Eager VMReg save may bash flags,
   * so set the CondCode arguments first.
   */
  assertx(argv.size() <= kNumServiceReqArgRegs);
  for (int i = 0; i < argv.size(); ++i) {
    auto reg = serviceReqArgRegs[i];
    const auto& argInfo = argv[i];
    switch (argInfo.m_kind) {
      case ServiceReqArgInfo::Immediate: {
        FTRACE(2, "{}, ", argInfo.m_imm);
        as.    emitImmReg(argInfo.m_imm, reg);
      } break;
      case ServiceReqArgInfo::RipRelative: {
        FTRACE(2, "{}(%rip), ", argInfo.m_imm);
        as.    lea(rip[argInfo.m_imm], reg);
      } break;
      case ServiceReqArgInfo::CondCode: {
        // Already set before VM reg save.
        DEBUG_ONLY TCA start = as.frontier();
        as.    setcc(argInfo.m_cc, rbyte(reg));
        assertx(start - as.frontier() <= kMovSize);
        FTRACE(2, "cc({}), ", cc_names[argInfo.m_cc]);
      } break;
    }
  }
  if (persist) {
    FTRACE(2, "no stub");
    as.  emitImmReg(0, rAsm);
  } else {
    FTRACE(2, "stub: {}", stub.base());
    as.  lea(rip[(int64_t)stub.base()], rAsm);
  }
  FTRACE(2, ")\n");
  as.    emitImmReg(req, reg::rdi);

  /*
   * Jump to the helper that will pack our args into a struct and call into
   * MCGenerator::handleServiceRequest().
   */
  as.    jmp(TCA(handleSRHelper));

  if (debug || !persist) {
    /*
     * not reached.
     * For re-usable stubs, used to mark the
     * end of the code, for the relocator's benefit.
     */
    as.ud2();
  }

  // Recycled stubs need to be uniformly sized. Make space for the
  // maximal possible service requests.
  if (!persist) {
    padStub(stub);
  }
}

} // anonymous namespace

//////////////////////////////////////////////////////////////////////

size_t reusableStubSize() {
  return maxStubSpace();
}

TCA emitServiceReqWork(CodeBlock& cb,
                       TCA start,
                       SRFlags flags,
                       folly::Optional<FPInvOffset> spOff,
                       ServiceRequest req,
                       const ServiceReqArgVec& argv) {
  auto const is_reused = start != cb.frontier();

  CodeBlock stub;
  stub.init(start, maxStubSpace(), "stubTemp");
  emitServiceReqImpl(stub, flags, spOff, req, argv);
  if (!is_reused) cb.skip(stub.used());

  return start;
}

void emitBindJ(CodeBlock& cb,
               CodeBlock& frozen,
               ConditionCode cc,
               SrcKey dest,
               FPInvOffset spOff,
               TransFlags trflags) {
  auto const smashInfo = emitBindJPre(cb, frozen, cc);
  auto optSPOff = folly::Optional<FPInvOffset>{};
  if (!dest.resumed()) optSPOff = spOff;
  auto const sr = emitEphemeralServiceReq(
    frozen,
    mcg->getFreeStub(frozen, &mcg->cgFixups()),
    optSPOff,
    REQ_BIND_JMP,
    RipRelative(smashInfo.primary),
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
  auto const sr = emitServiceReq(
    frozen,
    SRFlags::None,
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

  auto const sr = emitEphemeralServiceReq(
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

  auto const sr = emitEphemeralServiceReq(
    frozen,
    mcg->getFreeStub(frozen, &mcg->cgFixups()),
    optSPOff,
    REQ_BIND_JMPCC_FIRST,
    RipRelative(jccAddr),
    targetSk1.toAtomicInt(),
    targetSk0.toAtomicInt(),
    ccServiceReqArgInfo(cc)
  );
  CodeCursor cursor{cb, jccAddr};
  as.jcc(cc, sr);
  as.jmp(sr);
}

}}}
