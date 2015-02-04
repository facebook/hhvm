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

static constexpr int kMovSize = 0xa;

/*
 * Work to be done for jmp-smashing service requests before the service request
 * is emitted.
 *
 * Most notably, we must check if the CodeBlock for the jmp and for the stub
 * are aliased.  If so, we reserve space for the jmp which we'll emit properly
 * after the service request stub is emitted.
 */
ALWAYS_INLINE
TCA emitBindJPre(CodeBlock& cb, CodeBlock& frozen, ConditionCode cc) {
  mcg->backEnd().prepareForSmash(cb, cc == jit::CC_None ? kJmpLen : kJmpccLen);

  TCA toSmash = cb.frontier();
  if (cb.base() == frozen.base()) {
    mcg->backEnd().emitSmashableJump(cb, toSmash, cc);
  }

  mcg->setJmpTransID(toSmash);

  return toSmash;
}

/*
 * Work to be done for jmp-smashing service requests after the service request
 * stub is emitted.
 */
ALWAYS_INLINE
void emitBindJPost(CodeBlock& cb, CodeBlock& frozen,
                   ConditionCode cc, TCA toSmash, TCA sr) {
  if (cb.base() == frozen.base()) {
    CodeCursor cursor(cb, toSmash);
    mcg->backEnd().emitSmashableJump(cb, sr, cc);
  } else {
    mcg->backEnd().emitSmashableJump(cb, sr, cc);
  }
}

void emitBindJ(CodeBlock& cb, CodeBlock& frozen, ConditionCode cc,
               SrcKey dest, ServiceRequest req, TransFlags trflags) {
  auto toSmash = emitBindJPre(cb, frozen, cc);
  TCA sr = emitEphemeralServiceReq(frozen,
                                   mcg->getFreeStub(frozen,
                                                    &mcg->cgFixups()),
                                   req, RipRelative(toSmash),
                                   dest.toAtomicInt(),
                                   trflags.packed);
  emitBindJPost(cb, frozen, cc, toSmash, sr);
}

const int kExtraRegs = 2; // we also set rdi and r10
static constexpr int maxStubSpace() {
  /* max space for emitting args */
  return (kNumServiceReqArgRegs + kExtraRegs) * kMovSize;
}

// fill remaining space in stub with ud2 or int3
void padStub(CodeBlock& stub) {
  Asm a{stub};
  // do not use nops, or the relocator will strip them out
  while (stub.available() >= 2) a.ud2();
  if (stub.available() > 0) a.int3();
  assert(stub.available() == 0);
}

} // anonymous namespace

//////////////////////////////////////////////////////////////////////

void
emitServiceReqImpl(CodeBlock& stub, SRFlags flags, ServiceRequest req,
                   const ServiceReqArgVec& argv) {
  const bool persist = flags & SRFlags::Persist;
  Asm as{stub};
  FTRACE(2, "Emit Service Req @{} {}(", stub.base(), serviceReqName(req));
  /*
   * Move args into appropriate regs. Eager VMReg save may bash flags,
   * so set the CondCode arguments first.
   */
  assert(argv.size() <= kNumServiceReqArgRegs);
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
        assert(start - as.frontier() <= kMovSize);
        FTRACE(2, "cc({}), ", cc_names[argInfo.m_cc]);
      } break;
      default: not_reached();
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

TCA
emitServiceReqWork(CodeBlock& cb, TCA start, SRFlags flags,
                   ServiceRequest req, const ServiceReqArgVec& argv) {
  auto const is_reused = start != cb.frontier();

  CodeBlock stub;
  stub.init(start, maxStubSpace(), "stubTemp");
  emitServiceReqImpl(stub, flags, req, argv);
  if (!is_reused) cb.skip(stub.used());

  return start;
}

void emitBindJcc(CodeBlock& cb, CodeBlock& frozen, jit::ConditionCode cc,
                 SrcKey dest, TransFlags trflags) {
  emitBindJ(cb, frozen, cc, dest, REQ_BIND_JCC, trflags);
}

void emitBindJmp(CodeBlock& cb, CodeBlock& frozen,
                 SrcKey dest, TransFlags trflags) {
  emitBindJ(cb, frozen, CC_None, dest, REQ_BIND_JMP, trflags);
}

TCA emitRetranslate(CodeBlock& cb, CodeBlock& frozen, jit::ConditionCode cc,
                    SrcKey dest, TransFlags trflags) {
  auto toSmash = emitBindJPre(cb, frozen, cc);
  TCA sr = emitServiceReq(frozen, REQ_RETRANSLATE,
                          dest.offset(), trflags.packed);
  emitBindJPost(cb, frozen, cc, toSmash, sr);

  return toSmash;
}

}}}
