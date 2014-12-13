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

TCA
emitServiceReqImpl(CodeBlock& cb, SRFlags flags, ServiceRequest req,
                   const ServiceReqArgVec& argv);

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
  /* max space for moving to align plus emitting args */
  return
    kJmpTargetAlign - 1 +
    (kNumServiceReqArgRegs + kExtraRegs) * kMovSize;
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

TCA
emitServiceReqImpl(CodeBlock& stub, SRFlags flags, ServiceRequest req,
                   const ServiceReqArgVec& argv) {
  const bool align   = flags & SRFlags::Align;
  const bool persist = flags & SRFlags::Persist;
  Asm as{stub};
  if (align) moveToAlign(stub);
  TCA aligned_start = as.frontier();
  TRACE(3, "Emit Service Req @%p %s(", stub.base(), serviceReqName(req));
  /*
   * Move args into appropriate regs. Eager VMReg save may bash flags,
   * so set the CondCode arguments first.
   */
  for (int i = 0; i < argv.size(); ++i) {
    assert(i < kNumServiceReqArgRegs);
    auto reg = serviceReqArgRegs[i];
    const auto& argInfo = argv[i];
    switch (argInfo.m_kind) {
      case ServiceReqArgInfo::Immediate: {
        TRACE(3, "%" PRIx64 ", ", argInfo.m_imm);
        as.    emitImmReg(argInfo.m_imm, reg);
      } break;
      case ServiceReqArgInfo::RipRelative: {
        TRACE(3, "$rip(%" PRIx64 "), ", argInfo.m_imm);
        as.    lea(rip[argInfo.m_imm], reg);
      } break;
      case ServiceReqArgInfo::CondCode: {
        // Already set before VM reg save.
        DEBUG_ONLY TCA start = as.frontier();
        as.    setcc(argInfo.m_cc, rbyte(reg));
        assert(start - as.frontier() <= kMovSize);
        TRACE(3, "cc(%x), ", argInfo.m_cc);
      } break;
      default: not_reached();
    }
  }
  if (persist) {
    as.  emitImmReg(0, jit::x64::rAsm);
  } else {
    as.  lea(rip[(int64_t)stub.base()], jit::x64::rAsm);
  }
  TRACE(3, ")\n");
  as.    emitImmReg(req, jit::reg::rdi);

  /*
   * Weird hand-shaking with enterTC: reverse-call a service routine.
   *
   * In the case of some special stubs (m_callToExit, m_retHelper), we
   * have already unbalanced the return stack by doing a ret to
   * something other than enterTCHelper.  In that case
   * SRJmpInsteadOfRet indicates to fake the return.
   */
  if (flags & SRFlags::JmpInsteadOfRet) {
    as.  pop(jit::reg::rax);
    as.  jmp(jit::reg::rax);
  } else {
    as.  ret();
  }

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
  return aligned_start;
}

TCA
emitServiceReqWork(CodeBlock& cb, TCA start, SRFlags flags,
                   ServiceRequest req, const ServiceReqArgVec& argv) {
  CodeBlock stub;
  stub.init(start, maxStubSpace(), "stubTemp");
  auto ret = emitServiceReqImpl(stub, flags, req, argv);
  if (stub.base() == cb.frontier()) {
    cb.skip(stub.used());
  }
  return ret;
}

void emitBindSideExit(CodeBlock& cb, CodeBlock& frozen, jit::ConditionCode cc,
                      SrcKey dest, TransFlags trflags) {
  emitBindJ(cb, frozen, cc, dest, REQ_BIND_SIDE_EXIT, trflags);
}

void emitBindJcc(CodeBlock& cb, CodeBlock& frozen, jit::ConditionCode cc,
                 SrcKey dest) {
  emitBindJ(cb, frozen, cc, dest, REQ_BIND_JCC, TransFlags{});
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

void emitCallNativeImpl(Vout& v, Vout& vc, SrcKey srcKey, const Func* func,
                        int numArgs, Vreg inSp, Vreg outSp) {
  assert(isNativeImplCall(func, numArgs));
  auto retAddr = (int64_t)mcg->tx().uniqueStubs.retHelper;
  v << store{v.cns(retAddr), inSp[cellsToBytes(numArgs) + AROFF(m_savedRip)]};
  assert(numArgs == func->numLocals());
  assert(func->numIterators() == 0);
  v << lea{inSp[cellsToBytes(numArgs)], rVmFp};
  emitCheckSurpriseFlagsEnter(v, vc, Fixup{0, numArgs});
  BuiltinFunction builtinFuncPtr = func->builtinFuncPtr();
  if (false) { // typecheck
    ActRec* ar = nullptr;
    builtinFuncPtr(ar);
  }

  TRACE(2, "calling builtin preClass %p func %p\n", func->preClass(),
        builtinFuncPtr);
  /*
   * Call the native implementation. This will free the locals for us in the
   * normal case. In the case where an exception is thrown, the VM unwinder
   * will handle it for us.
   */
  if (mcg->fixupMap().eagerRecord(func)) {
    emitEagerSyncPoint(v, reinterpret_cast<const Op*>(func->getEntry()),
                       rVmFp, inSp);
  }
  v << vcall{CppCall::direct(builtinFuncPtr), v.makeVcallArgs({{rVmFp}}),
             v.makeTuple({}), Fixup{0, numArgs}};

  /*
   * We're sometimes calling this while curFunc() isn't really the
   * builtin---make sure to properly record the sync point as if we
   * are inside the builtin.
   *
   * The assumption here is that for builtins, the generated func
   * contains only a single opcode (NativeImpl), and there are no
   * non-argument locals.
   */
  assert(func->numIterators() == 0 && func->methInfo());
  assert(func->numLocals() == func->numParams());
  assert(*reinterpret_cast<const Op*>(func->getEntry()) == Op::NativeImpl);
  assert(instrLen((Op*)func->getEntry()) == func->past() - func->base());

  /*
   * The native implementation already put the return value on the
   * stack for us, and handled cleaning up the arguments.  We have to
   * update the frame pointer and the stack pointer, and load the
   * return value into the return register so the trace we are
   * returning to has it where it expects.
   *
   * TODO(#1273094): we should probably modify the actual builtins to
   * return values via registers (rax:edx) using the C ABI and do a
   * reg-to-reg move.
   */
  int nLocalCells = func->numSlotsInFrame();
  v << load{rVmFp[AROFF(m_sfp)], rVmFp};

  emitRB(v, Trace::RBTypeFuncExit, func->fullName()->data());
  auto adjust = safe_cast<int>(sizeof(ActRec) + cellsToBytes(nLocalCells-1));
  v << lea{inSp[adjust], outSp};
}

/*
 * Emit a smashable call into main that initially calls a recyclable
 * service request stub. the stub, and the eventual targets, take
 * rStashedAR as an argument.
 */
void emitBindCall(Vout& v, CodeBlock& frozen,
                  const Func* func, int numArgs) {
  assert(!isNativeImplCall(func, numArgs));

  auto& us = mcg->tx().uniqueStubs;
  auto addr = func ? us.immutableBindCallStub : us.bindCallStub;

  // emit the mainline code
  if (debug && RuntimeOption::EvalHHIRGenerateAsserts) {
    auto off = cellsToBytes(numArgs) + AROFF(m_savedRip);
    emitImmStoreq(v, kUninitializedRIP, rVmSp[off]);
  }
  v << lea{rVmSp[cellsToBytes(numArgs)], rStashedAR};
  v << bindcall{addr, kCrossCallRegs};
}

}}}
