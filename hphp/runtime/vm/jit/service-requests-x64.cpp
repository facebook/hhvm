/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#include "hphp/runtime/vm/jit/jump-smash.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/translator-x64.h"
#include "hphp/runtime/vm/jit/translator-x64-internal.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/util/asm-x64.h"
#include "hphp/util/ringbuffer.h"

namespace HPHP { namespace JIT { namespace X64 {


TRACE_SET_MOD(servicereq);

// An intentionally funny-looking-in-core-dumps constant for uninitialized
// instruction pointers.
constexpr uint64_t kUninitializedRIP = 0xba5eba11acc01ade;

namespace {

void emitBindJ(CodeBlock& cb, CodeBlock& stubs,
               ConditionCode cc, SrcKey dest, ServiceRequest req) {
  prepareForSmash(cb, cc == JIT::CC_None ? kJmpLen : kJmpccLen);
  TCA toSmash = cb.frontier();
  if (cb.base() == stubs.base()) {
    Asm a { cb };
    emitJmpOrJcc(a, cc, toSmash);
  }

  tx64->setJmpTransID(toSmash);

  TCA sr = (req == JIT::REQ_BIND_JMP
            ? emitEphemeralServiceReq(tx64->stubsCode, tx64->getFreeStub(), req,
                                      toSmash, dest.offset())
            : emitServiceReq(tx64->stubsCode, req, toSmash, dest.offset()));

  Asm a { cb };
  if (cb.base() == stubs.base()) {
    CodeCursor cursor(cb, toSmash);
    emitJmpOrJcc(a, cc, sr);
  } else {
    emitJmpOrJcc(a, cc, sr);
  }
}

/*
 * NativeImpl is a special operation in the sense that it must be the
 * only opcode in a function body, and also functions as the return.
 *
 * if emitSavedRIPReturn is false, it returns the amount by which
 * rVmSp should be adjusted, otherwise, it emits code to perform
 * the adjustment (this allows us to combine updates to rVmSp)
 */
int32_t emitNativeImpl(CodeBlock& mainCode, const Func* func,
                       bool emitSavedRIPReturn) {
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
  a.   mov_reg64_reg64(rVmFp, argNumToRegName[0]);
  if (tx64->fixupMap().eagerRecord(func)) {
    emitEagerSyncPoint(a, func->getEntry(), 0);
  }
  emitCall(a, (TCA)builtinFuncPtr);

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
  assert(toOp(*func->getEntry()) == OpNativeImpl);
  assert(instrLen((Op*)func->getEntry()) == func->past() - func->base());
  Offset pcOffset = 0;  // NativeImpl is the only instruction in the func
  Offset stackOff = func->numLocals(); // Builtin stubs have no
                                       // non-arg locals
  tx64->fixupMap().recordSyncPoint(mainCode.frontier(), pcOffset, stackOff);

  if (emitSavedRIPReturn) {
    // push the return address to get ready to ret.
    a.   push  (rVmFp[AROFF(m_savedRip)]);
  }

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
  if (emitSavedRIPReturn) {
    a. add_imm64_reg64(sizeof(ActRec) + cellsToBytes(nLocalCells-1), rVmSp);
  }
  a.   load_reg64_disp_reg64(rVmFp, AROFF(m_savedRbp), rVmFp);

  emitRB(a, Trace::RBTypeFuncExit, func->fullName()->data());
  if (emitSavedRIPReturn) {
    a. ret();
    if (debug) {
      a.ud2();
    }
    return 0;
  }
  return sizeof(ActRec) + cellsToBytes(nLocalCells-1);
}

void emitBindCallHelper(CodeBlock& mainCode, CodeBlock& stubsCode,
                        SrcKey srcKey,
                        const Func* funcd,
                        int numArgs) {
  // Whatever prologue we're branching to will check at runtime that we
  // went to the right Func*, correcting if necessary. We treat the first
  // Func we encounter as a decent prediction. Make space to burn in a
  // TCA.
  ReqBindCall* req = tx64->globalData().alloc<ReqBindCall>();

  Asm a { mainCode };
  prepareForSmash(mainCode, kCallLen);
  TCA toSmash = mainCode.frontier();
  a.    call(stubsCode.frontier());

  Asm astubs { stubsCode };
  astubs.    mov_reg64_reg64(rStashedAR, serviceReqArgRegs[1]);
  emitPopRetIntoActRec(astubs);
  emitServiceReq(stubsCode, JIT::REQ_BIND_CALL, req);

  TRACE(1, "will bind static call: tca %p, funcd %p, astubs %p\n",
        toSmash, funcd, stubsCode.frontier());
  req->m_toSmash = toSmash;
  req->m_nArgs = numArgs;
  req->m_sourceInstr = srcKey;
  req->m_isImmutable = (bool)funcd;
}

bool isNativeImplCall(const Func* funcd, int numArgs) {
  return funcd && funcd->methInfo() && numArgs == funcd->numParams();
}

} // anonymous namespace

//////////////////////////////////////////////////////////////////////

TCA
emitServiceReqWork(CodeBlock& cb, TCA start, bool persist, SRFlags flags,
                   ServiceRequest req, const ServiceReqArgVec& argv) {
  assert(start);
  const bool align = flags & SRFlags::Align;
  Asm as { cb };

  /*
   * Remember previous state of the code cache.
   */
  folly::Optional<CodeCursor> maybeCc = folly::none;
  if (start != as.frontier()) {
    maybeCc.emplace(cb, start);
  }

  /* max space for moving to align, saving VM regs plus emitting args */
  static const int
    kVMRegSpace = 0x14,
    kMovSize = 0xa,
    kNumServiceRegs = sizeof(serviceReqArgRegs) / sizeof(PhysReg),
    kMaxStubSpace = kJmpTargetAlign - 1 + kVMRegSpace +
      kNumServiceRegs * kMovSize;
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
    as.  emitImmReg(0, JIT::reg::rAsm);
  } else {
    as.  emitImmReg((uint64_t)start, JIT::reg::rAsm);
  }
  TRACE(3, ")\n");
  as.    emitImmReg(req, JIT::reg::rdi);

  /*
   * Weird hand-shaking with enterTC: reverse-call a service routine.
   *
   * In the case of some special stubs (m_callToExit, m_retHelper), we
   * have already unbalanced the return stack by doing a ret to
   * something other than enterTCHelper.  In that case
   * SRJmpInsteadOfRet indicates to fake the return.
   */
  if (flags & SRFlags::JmpInsteadOfRet) {
    as.  pop(JIT::reg::rax);
    as.  jmp(JIT::reg::rax);
  } else {
    as.  ret();
  }

  // TODO(2796856): we should record an OpServiceRequest pseudo-bytecode here.

  if (debug) {
    // not reached
    as.ud2();
  }

  if (!persist) {
    /*
     * Recycled stubs need to be uniformly sized. Make space for the
     * maximal possible service requests.
     */
    assert(as.frontier() - start <= kMaxStubSpace);
    as.emitNop(start + kMaxStubSpace - as.frontier());
    assert(as.frontier() - start == kMaxStubSpace);
  }
  return retval;
}

void emitBindSideExit(CodeBlock& cb, CodeBlock& stubs, JIT::ConditionCode cc,
                      SrcKey dest) {
  emitBindJ(cb, stubs, cc, dest, REQ_BIND_SIDE_EXIT);
}

void emitBindJcc(CodeBlock& cb, CodeBlock& stubs, JIT::ConditionCode cc,
                 SrcKey dest) {
  emitBindJ(cb, stubs, cc, dest, REQ_BIND_JCC);
}

void emitBindJmp(CodeBlock& cb, CodeBlock& stubs, SrcKey dest) {
  emitBindJ(cb, stubs, JIT::CC_None, dest, REQ_BIND_JMP);
}

int32_t emitBindCall(CodeBlock& mainCode, CodeBlock& stubsCode,
                     SrcKey srcKey, const Func* funcd, int numArgs) {
  // If this is a call to a builtin and we don't need any argument
  // munging, we can skip the prologue system and do it inline.
  if (isNativeImplCall(funcd, numArgs)) {
    StoreImmPatcher patchIP(mainCode, (uint64_t)mainCode.frontier(), reg::rax,
                            cellsToBytes(numArgs) + AROFF(m_savedRip),
                            rVmSp);
    assert(funcd->numLocals() == funcd->numParams());
    assert(funcd->numIterators() == 0);
    Asm a { mainCode };
    emitLea(a, rVmSp[cellsToBytes(numArgs)], rVmFp);
    emitCheckSurpriseFlagsEnter(mainCode, stubsCode, true, tx64->fixupMap(),
                                Fixup(0, numArgs));
    // rVmSp is already correctly adjusted, because there's no locals
    // other than the arguments passed.
    auto retval = emitNativeImpl(mainCode, funcd,
                                 false /* don't jump to return */);
    patchIP.patch(uint64_t(mainCode.frontier()));
    return retval;
  }

  Asm a { mainCode };
  if (debug) {
    a.    storeq (kUninitializedRIP,
                  rVmSp[cellsToBytes(numArgs) + AROFF(m_savedRip)]);
  }
  // Stash callee's rVmFp into rStashedAR for the callee's prologue
  emitLea(a, rVmSp[cellsToBytes(numArgs)], rStashedAR);
  emitBindCallHelper(mainCode, stubsCode, srcKey, funcd, numArgs);
  return 0;
}


}}}
