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

#include "folly/Optional.h"

#include "hphp/runtime/vm/jit/code-gen-helpers-x64.h"
#include "hphp/runtime/vm/jit/back-end.h"
#include "hphp/runtime/vm/jit/back-end-x64.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/mc-generator-internal.h"
#include "hphp/runtime/vm/jit/service-requests-inline.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/util/asm-x64.h"
#include "hphp/util/ringbuffer.h"

namespace HPHP { namespace jit { namespace x64 {

using jit::reg::rip;

TRACE_SET_MOD(servicereq);

// An intentionally funny-looking-in-core-dumps constant for uninitialized
// instruction pointers.
constexpr uint64_t kUninitializedRIP = 0xba5eba11acc01ade;

TCA
emitServiceReqImpl(TCA stubStart, TCA start, TCA& end, int maxStubSpace,
                   SRFlags flags, ServiceRequest req,
                   const ServiceReqArgVec& argv);

namespace {

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
  Asm a { cb };
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

/*
 * NativeImpl is a special operation in the sense that it must be the
 * only opcode in a function body, and also functions as the return.
 */
void emitNativeImpl(CodeBlock& mainCode, const Func* func) {
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
  Asm a { mainCode };
  a.   movq  (rVmFp, argNumToRegName[0]);
  if (mcg->fixupMap().eagerRecord(func)) {
    emitEagerSyncPoint(a, reinterpret_cast<const Op*>(func->getEntry()));
  }
  emitCall(a, (TCA)builtinFuncPtr, argSet(1));

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
  Offset pcOffset = 0;  // NativeImpl is the only instruction in the func
  Offset stackOff = func->numLocals(); // Builtin stubs have no
                                       // non-arg locals
  mcg->recordSyncPoint(mainCode.frontier(), pcOffset, stackOff);

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
  a.   loadq  (rVmFp[AROFF(m_sfp)], rVmFp);

  emitRB(a, Trace::RBTypeFuncExit, func->fullName()->data());
  auto adjust = safe_cast<int>(sizeof(ActRec) + cellsToBytes(nLocalCells-1));
  if (adjust) {
    a.  addq(adjust, rVmSp);
  }
}

static int maxStubSpace() {
  /* max space for moving to align, saving VM regs plus emitting args */
  static constexpr int
    kVMRegSpace = 0x14,
    kMovSize = 0xa,
    kNumServiceRegs = sizeof(serviceReqArgRegs) / sizeof(PhysReg),
    kMaxStubSpace = kJmpTargetAlign - 1 + kVMRegSpace +
      kNumServiceRegs * kMovSize;
  return kMaxStubSpace;
}

void emitBindCallHelper(CodeBlock& mainCode, CodeBlock& frozenCode,
                        SrcKey srcKey,
                        const Func* funcd,
                        int numArgs) {
  // Whatever prologue we're branching to will check at runtime that we
  // went to the right Func*, correcting if necessary. We treat the first
  // Func we encounter as a decent prediction. Make space to burn in a
  // TCA.
  ReqBindCall* req = mcg->globalData().alloc<ReqBindCall>();

  // Use some space from the beginning of the service
  // request stub to emit BIND_CALL specific code.
  TCA start = mcg->getFreeStub(frozenCode, &mcg->cgFixups());

  Asm a { mainCode };
  mcg->backEnd().prepareForSmash(mainCode, kCallLen);
  TCA toSmash = mainCode.frontier();
  a.    call(start);

  TCA end;
  CodeBlock cb;
  auto stubSpace = maxStubSpace();
  cb.init(start, stubSpace, "stubTemp");
  Asm as { cb };

  as.    movq   (rStashedAR, serviceReqArgRegs[1]);
  emitPopRetIntoActRec(as);

  auto spaceLeft = stubSpace - (cb.frontier() - start);
  ServiceReqArgVec argv;
  packServiceReqArgs(argv, req);

  emitServiceReqImpl(start, cb.frontier(), end, spaceLeft,
                     SRFlags::None, jit::REQ_BIND_CALL, argv);

  if (start == frozenCode.frontier()) {
    frozenCode.skip(end - start);
  }

  TRACE(1, "will bind static call: tca %p, funcd %p, acold %p\n",
        toSmash, funcd, frozenCode.frontier());
  mcg->cgFixups().m_codePointers.insert(&req->m_toSmash);
  req->m_toSmash = toSmash;
  req->m_nArgs = numArgs;
  req->m_sourceInstr = srcKey;
  req->m_isImmutable = (bool)funcd;
}

} // anonymous namespace

//////////////////////////////////////////////////////////////////////

TCA
emitServiceReqImpl(TCA stubStart, TCA start, TCA& end, int maxStubSpace,
                   SRFlags flags, ServiceRequest req,
                   const ServiceReqArgVec& argv) {
  assert(start);
  const bool align   = flags & SRFlags::Align;
  const bool persist = flags & SRFlags::Persist;

  DEBUG_ONLY static constexpr int kMovSize = 0xa;

  CodeBlock cb;
  cb.init(start, maxStubSpace, "stubTemp");
  Asm as { cb };

  if (align) {
    moveToAlign(cb);
  }
  TCA retval = as.frontier();
  TRACE(3, "Emit Service Req @%p %s(", start, serviceReqName(req));
  /*
   * Move args into appropriate regs. Eager VMReg save may bash flags,
   * so set the CondCode arguments first.
   */
  for (int i = 0; i < argv.size(); ++i) {
    assert(i < kNumServiceReqArgRegs);
    auto reg = serviceReqArgRegs[i];
    const auto& argInfo = argv[i];
    switch(argv[i].m_kind) {
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
  emitEagerVMRegSave(as, RegSaveFlags::SaveFP);
  if (persist) {
    as.  emitImmReg(0, jit::x64::rAsm);
  } else {
    as.  lea(rip[(int64_t)stubStart], jit::x64::rAsm);
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

  if (!persist) {
    /*
     * Recycled stubs need to be uniformly sized. Make space for the
     * maximal possible service requests.
     */
    assert(as.frontier() - start <= maxStubSpace);
    // do not use nops, or the relocator will strip them out
    while (as.frontier() - start <= maxStubSpace - 2) as.ud2();
    if (as.frontier() - start < maxStubSpace) as.int3();
    assert(as.frontier() - start == maxStubSpace);
  }

  end = cb.frontier();
  return retval;
}

TCA
emitServiceReqWork(CodeBlock& cb, TCA start, SRFlags flags,
                   ServiceRequest req, const ServiceReqArgVec& argv) {
  TCA end;
  auto ret = emitServiceReqImpl(start, start, end, maxStubSpace(), flags,
                                req, argv);

  if (start == cb.frontier()) {
    cb.skip(end - start);
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

void emitBindCall(CodeBlock& mainCode, CodeBlock& coldCode,
                  CodeBlock& frozenCode, SrcKey srcKey,
                  const Func* funcd, int numArgs) {
  // If this is a call to a builtin and we don't need any argument
  // munging, we can skip the prologue system and do it inline.
  if (isNativeImplCall(funcd, numArgs)) {
    Asm a { mainCode };
    auto retAddr = (int64_t)mcg->tx().uniqueStubs.retHelper;
    if (deltaFits(retAddr, sz::dword)) {
      a.storeq(int32_t(retAddr),
               rVmSp[cellsToBytes(numArgs) + AROFF(m_savedRip)]);
    } else {
      a.lea(rip[retAddr], reg::rax);
      a.storeq(reg::rax, rVmSp[cellsToBytes(numArgs) + AROFF(m_savedRip)]);
    }
    assert(funcd->numLocals() == funcd->numParams());
    assert(funcd->numIterators() == 0);
    emitLea(a, rVmSp[cellsToBytes(numArgs)], rVmFp);
    emitCheckSurpriseFlagsEnter(mainCode, coldCode, Fixup(0, numArgs));
    // rVmSp is already correctly adjusted, because there's no locals
    // other than the arguments passed.
    return emitNativeImpl(mainCode, funcd);
  }

  Asm a { mainCode };
  if (debug && RuntimeOption::EvalHHIRGenerateAsserts) {
    auto off = cellsToBytes(numArgs) + AROFF(m_savedRip);
    emitImmStoreq(a, kUninitializedRIP, rVmSp[off]);
  }
  // Stash callee's rVmFp into rStashedAR for the callee's prologue
  emitLea(a, rVmSp[cellsToBytes(numArgs)], rStashedAR);
  emitBindCallHelper(mainCode, frozenCode, srcKey, funcd, numArgs);
}


}}}
