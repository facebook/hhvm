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
#include "hphp/runtime/vm/jit/func-prologues-x64.h"

#include "hphp/util/asm-x64.h"

#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-x64.h"
#include "hphp/runtime/vm/jit/translator-x64.h"
#include "hphp/runtime/vm/jit/translator-x64-internal.h"
#include "hphp/runtime/vm/jit/write-lease.h"

namespace HPHP { namespace JIT { namespace X64 {

//////////////////////////////////////////////////////////////////////

using Transl::TCA;

TRACE_SET_MOD(tx64);

constexpr auto kJcc8Len = 3;

//////////////////////////////////////////////////////////////////////

namespace {

void emitStackCheck(int funcDepth, Offset pc) {
  using namespace reg;
  Asm a { tx64->mainCode };
  funcDepth += kStackCheckPadding * sizeof(Cell);

  uint64_t stackMask = cellsToBytes(RuntimeOption::EvalVMStackElms) - 1;
  a.    mov_reg64_reg64(rVmSp, rAsm); // copy to destroy
  a.    and_imm64_reg64(stackMask, rAsm);
  a.    sub_imm64_reg64(funcDepth + Stack::sSurprisePageSize, rAsm);
  // Unlikely branch to failure.
  a.    jl(tx64->uniqueStubs.stackOverflowHelper);
  // Success.
}

TCA emitFuncGuard(X64Assembler& a, const Func* func) {
  using namespace reg;
  assert(kScratchCrossTraceRegs.contains(rax));
  assert(kScratchCrossTraceRegs.contains(rdx));

  const int kAlign = kX64CacheLineSize;
  const int kAlignMask = kAlign - 1;
  int loBits = uintptr_t(a.frontier()) & kAlignMask;
  int delta, size;

  // Ensure the immediate is safely smashable
  // the immediate must not cross a qword boundary,
  if (!deltaFits((intptr_t)func, sz::dword)) {
    size = 8;
    delta = loBits + kFuncMovImm;
  } else {
    size = 4;
    delta = loBits + kFuncCmpImm;
  }

  delta = (delta + size - 1) & kAlignMask;
  if (delta < size - 1) {
    a.emitNop(size - 1 - delta);
  }

  TCA aStart DEBUG_ONLY = a.frontier();
  if (!deltaFits((intptr_t)func, sz::dword)) {
    a.    load_reg64_disp_reg64(rStashedAR, AROFF(m_func), rax);
    /*
      Although func doesnt fit in a signed 32-bit immediate, it may still
      fit in an unsigned one. Rather than deal with yet another case
      (which only happens when we disable jemalloc) just force it to
      be an 8-byte immediate, and patch it up afterwards.
    */
    a.    mov_imm64_reg(0xdeadbeeffeedface, rdx);
    assert(((uint64_t*)a.frontier())[-1] == 0xdeadbeeffeedface);
    ((uint64_t*)a.frontier())[-1] = uintptr_t(func);
    a.    cmp_reg64_reg64(rax, rdx);
  } else {
    a.    cmp_imm32_disp_reg32(uint64_t(func), AROFF(m_func), rStashedAR);
  }

  assert(tx64->uniqueStubs.funcPrologueRedispatch);

  a.    jnz(tx64->uniqueStubs.funcPrologueRedispatch);

  assert(funcPrologueToGuard(a.frontier(), func) == aStart);
  assert(funcPrologueHasGuard(a.frontier(), func));
  return a.frontier();
}

// Initialize at most this many locals inline in function body prologue; more
// than this, and emitting a loop is more compact. To be precise, the actual
// crossover point in terms of code size is 6; 9 was determined by experiment to
// be the optimal point in certain benchmarks. #microoptimization
constexpr auto kLocalsToInitializeInline = 9;

// Maximum number of default-value parameter initializations to
// unroll. Beyond this, a loop is generated.
constexpr auto kMaxParamsInitUnroll = 5;

SrcKey emitPrologueWork(Func* func, int nPassed) {
  using Transl::Location;
  using namespace reg;

  int numParams = func->numParams();
  const Func::ParamInfoVec& paramInfo = func->params();

  Offset dvInitializer = InvalidAbsoluteOffset;

  assert(IMPLIES(func->isGenerator(), nPassed == numParams));

  Asm a { tx64->mainCode };

  if (tx64->mode() == TransProflogue) {
    assert(func->shouldPGO());
    TransID transId  = tx64->profData()->curTransID();
    auto counterAddr = tx64->profData()->transCounterAddr(transId);
    a.movq(counterAddr, rAsm);
    a.decq(rAsm[0]);
  }

  if (nPassed > numParams) {
    // Too many args; a weird case, so just callout. Stash ar
    // somewhere callee-saved.
    if (false) { // typecheck
      Transl::trimExtraArgs((ActRec*)nullptr);
    }
    a.  mov_reg64_reg64(rStashedAR, argNumToRegName[0]);
    emitCall(a, TCA(Transl::trimExtraArgs));
    // We'll fix rVmSp below.
  } else if (nPassed < numParams) {
    // Figure out which, if any, default value initializer to go to
    for (int i = nPassed; i < numParams; ++i) {
      const Func::ParamInfo& pi = paramInfo[i];
      if (pi.hasDefaultValue()) {
        dvInitializer = pi.funcletOff();
        break;
      }
    }
    TRACE(1, "Only have %d of %d args; getting dvFunclet\n",
          nPassed, numParams);
    if (numParams - nPassed <= kMaxParamsInitUnroll) {
      for (int i = nPassed; i < numParams; i++) {
        int offset = (nPassed - i - 1) * sizeof(Cell);
        emitStoreTVType(a, KindOfUninit, rVmSp[offset + TVOFF(m_type)]);
      }
    } else {
      a.  emitImmReg(nPassed, rax);
      // do { *(--rVmSp) = NULL; nPassed++; } while (nPassed < numParams);
      // This should be an unusual case, so optimize for code density
      // rather than execution speed; i.e., don't unroll the loop.
      TCA loopTop = a.frontier();
      a.  sub_imm32_reg64(sizeof(Cell), rVmSp);
      a.  incl(eax);
      emitStoreUninitNull(a, 0, rVmSp);
      a.  cmp_imm32_reg32(numParams, rax);
      a.  jcc8(CC_L, loopTop);
    }
  }

  // Entry point for numParams == nPassed is here.
  // Args are kosher. Frame linkage: set fp = ar.
  a.    mov_reg64_reg64(rStashedAR, rVmFp);

  int numLocals = numParams;
  if (func->isClosureBody()) {
    // Closure object properties are the use vars followed by the
    // static locals (which are per-instance).
    int numUseVars = func->cls()->numDeclProperties() -
                     func->numStaticLocals();

    emitLea(a, rVmFp[-cellsToBytes(numParams)], rVmSp);

    PhysReg rClosure = rcx;
    a.  loadq(rVmFp[AROFF(m_this)], rClosure);

    // Swap in the $this or late bound class
    a.  loadq(rClosure[c_Closure::ctxOffset()], rAsm);
    a.  storeq(rAsm, rVmFp[AROFF(m_this)]);

    if (!(func->attrs() & AttrStatic)) {
      a.shrq(1, rAsm);
      JccBlock<CC_BE> ifRealThis(a);
      a.shlq(1, rAsm);
      // XXX can objects be static?
      emitIncRefCheckNonStatic(a, rAsm, KindOfObject);
    }

    // Put in the correct context
    a.  loadq(rClosure[c_Closure::funcOffset()], rAsm);
    a.  storeq(rAsm, rVmFp[AROFF(m_func)]);

    // Copy in all the use vars
    int baseUVOffset = sizeof(ObjectData) + func->cls()->builtinODTailSize();
    for (int i = 0; i < numUseVars + 1; i++) {
      int spOffset = -cellsToBytes(i+1);

      if (i == 0) {
        // The closure is the first local.
        // We don't incref because it used to be $this
        // and now it is a local, so they cancel out
        emitStoreTypedValue(a, KindOfObject, rClosure, spOffset, rVmSp);
        continue;
      }

      int uvOffset = baseUVOffset + cellsToBytes(i-1);

      emitCopyTo(a, rClosure, uvOffset, rVmSp, spOffset, rAsm);
      emitIncRefGenericRegSafe(a, rVmSp, spOffset, rAsm);
    }

    numLocals += numUseVars + 1;
  }

  // We're in the callee frame; initialize locals. Unroll the loop all
  // the way if there are a modest number of locals to update;
  // otherwise, do it in a compact loop. If we're in a generator body,
  // named locals will be initialized by UnpackCont so we can leave
  // them alone here.
  int numUninitLocals = func->numLocals() - numLocals;
  assert(numUninitLocals >= 0);
  if (numUninitLocals > 0 && !func->isGenerator()) {

    // If there are too many locals, then emitting a loop to initialize locals
    // is more compact, rather than emitting a slew of movs inline.
    if (numUninitLocals > kLocalsToInitializeInline) {
      PhysReg loopReg = rcx;

      // rVmFp + rcx points to the count/type fields of the TypedValue we're
      // about to write to.
      int loopStart = -func->numLocals() * sizeof(TypedValue) + TVOFF(m_type);
      int loopEnd = -numLocals * sizeof(TypedValue) + TVOFF(m_type);

      a.  emitImmReg(loopStart, loopReg);
      a.  emitImmReg(KindOfUninit, rdx);

      TCA topOfLoop = a.frontier();
      // do {
      //   rVmFp[loopReg].m_type = KindOfUninit;
      // } while(++loopReg != loopEnd);

      emitStoreTVType(a, edx, rVmFp[loopReg]);
      a.  addq   (sizeof(Cell), loopReg);
      a.  cmpq   (loopEnd, loopReg);
      a.  jcc8   (CC_NE, topOfLoop);
    } else {
      PhysReg base;
      int disp, k;
      static_assert(KindOfUninit == 0, "");
      if (numParams < func->numLocals()) {
        a.xorl (eax, eax);
      }
      for (k = numLocals; k < func->numLocals(); ++k) {
        locToRegDisp(Location(Location::Local, k), &base, &disp, func);
        emitStoreTVType(a, eax, base[disp + TVOFF(m_type)]);
      }
    }
  }

  const HPHP::Opcode* destPC = func->unit()->entry() + func->base();
  if (dvInitializer != InvalidAbsoluteOffset) {
    // dispatch to funclet.
    destPC = func->unit()->entry() + dvInitializer;
  }
  SrcKey funcBody(func, destPC);

  // Move rVmSp to the right place: just past all locals
  int frameCells = func->numSlotsInFrame();
  if (func->isGenerator()) {
    frameCells = 1;
  } else {
    emitLea(a, rVmFp[-cellsToBytes(frameCells)], rVmSp);
  }

  Fixup fixup(funcBody.offset() - func->base(), frameCells);

  // Emit warnings for any missing arguments
  if (!func->isCPPBuiltin()) {
    for (int i = nPassed; i < numParams; ++i) {
      if (paramInfo[i].funcletOff() == InvalidAbsoluteOffset) {
        a.  emitImmReg((intptr_t)func->name()->data(), argNumToRegName[0]);
        a.  emitImmReg(numParams, argNumToRegName[1]);
        a.  emitImmReg(i, argNumToRegName[2]);
        emitCall(a, (TCA)raiseMissingArgument);
        tx64->fixupMap().recordFixup(a.frontier(), fixup);
        break;
      }
    }
  }

  // Check surprise flags in the same place as the interpreter: after
  // setting up the callee's frame but before executing any of its
  // code
  emitCheckSurpriseFlagsEnter(tx64->mainCode, tx64->stubsCode, false,
                              tx64->fixupMap(), fixup);

  if (func->isClosureBody() && func->cls()) {
    int entry = nPassed <= numParams ? nPassed : numParams + 1;
    // Relying on rStashedAR == rVmFp here
    a.    loadq   (rStashedAR[AROFF(m_func)], rax);
    a.    loadq   (rax[Func::prologueTableOff() + sizeof(TCA)*entry],
                   rax);
    a.    jmp     (rax);
  } else {
    emitBindJmp(tx64->mainCode, tx64->stubsCode, funcBody);
  }
  return funcBody;
}

} // anonymous namespace

//////////////////////////////////////////////////////////////////////

TCA emitCallArrayPrologue(Func* func, DVFuncletsVec& dvs) {
  auto& mainCode = tx64->mainCode;
  auto& stubsCode = tx64->stubsCode;
  Asm a { mainCode };
  TCA start = mainCode.frontier();
  if (dvs.size() == 1) {
    a.   cmp_imm32_disp_reg32(dvs[0].first,
                              AROFF(m_numArgsAndCtorFlag), rVmFp);
    emitBindJcc(mainCode, stubsCode, CC_LE, SrcKey(func, dvs[0].second));
    emitBindJmp(mainCode, stubsCode, SrcKey(func, func->base()));
  } else {
    a.   load_reg64_disp_reg32(rVmFp, AROFF(m_numArgsAndCtorFlag), reg::rax);
    for (unsigned i = 0; i < dvs.size(); i++) {
      a.   cmp_imm32_reg32(dvs[i].first, reg::rax);
      emitBindJcc(mainCode, stubsCode, CC_LE, SrcKey(func, dvs[i].second));
    }
    emitBindJmp(mainCode, stubsCode, SrcKey(func, func->base()));
  }
  return start;
}

SrcKey emitFuncPrologue(CodeBlock& mainCode, CodeBlock& stubsCode,
                        Func* func, bool funcIsMagic, int nPassed,
                        TCA& start, TCA& aStart) {
  Asm a { mainCode };

  // Guard: we're in the right callee. This happens in magicStart for
  // magic callees.
  if (!funcIsMagic) {
    start = aStart = emitFuncGuard(a, func);
  }

  emitRB(a, Trace::RBTypeFuncPrologueTry, func->fullName()->data());

  // NB: We have most of the register file to play with, since we know
  // we're between BB's. So, we hardcode some registers here rather
  // than using the scratch allocator.
  TRACE(2, "funcPrologue: user function: %s\n", func->name()->data());

  // Add a counter for the translation if requested
  if (RuntimeOption::EvalJitTransCounters) {
    emitTransCounterInc(a);
  }

  if (!funcIsMagic) {
    emitPopRetIntoActRec(a);
    // entry point for magic methods comes later
    emitRB(a, Trace::RBTypeFuncEntry, func->fullName()->data());

    /*
     * Guard: we have stack enough stack space to complete this
     * function.  We omit overflow checks if it is a leaf function
     * that can't use more than kStackCheckLeafPadding cells.
     */
    auto const needStackCheck =
      !(func->attrs() & AttrPhpLeafFn) ||
      func->maxStackCells() >= kStackCheckLeafPadding;
    if (needStackCheck) {
      emitStackCheck(cellsToBytes(func->maxStackCells()), func->base());
    }
  }

  SrcKey skFuncBody = emitPrologueWork(func, nPassed);

  if (funcIsMagic) {
    // entry points for magic methods is here
    TCA magicStart = emitFuncGuard(a, func);
    emitPopRetIntoActRec(a);
    emitRB(a, Trace::RBTypeFuncEntry, func->fullName()->data());
    // Guard: we have stack enough stack space to complete this function.
    emitStackCheck(cellsToBytes(func->maxStackCells()), func->base());
    assert(func->numParams() == 2);
    // Special __call prologue
    a.  mov_reg64_reg64(rStashedAR, argNumToRegName[0]);
    emitCall(a, TCA(Transl::shuffleArgsForMagicCall));
    if (memory_profiling) {
      tx64->fixupMap().recordFixup(
        a.frontier(),
        Fixup(skFuncBody.offset() - func->base(), func->numSlotsInFrame())
      );
    }
    // if shuffleArgs returns 0, that means this was not a magic call
    // and we should proceed to a prologue specialized for nPassed;
    // otherwise, proceed to a prologue specialized for nPassed==numParams (2).
    if (nPassed == 2) {
      a.jmp(start);
    } else {
      a.test_reg64_reg64(reg::rax, reg::rax);
      // z ==> not a magic call, go to prologue for nPassed
      if (deltaFits(start - (a.frontier() + kJcc8Len), sz::byte)) {
        a.jcc8(CC_Z, start);
      } else {
        a.jcc(CC_Z, start);
      }
      // this was a magic call
      // nPassed == 2
      // Fix up hardware stack pointer
      nPassed = 2;
      emitLea(a, rStashedAR[-cellsToBytes(nPassed)], rVmSp);
      // Optimization TODO: Reuse the prologue for args == 2
      emitPrologueWork(func, nPassed);
    }
    start = magicStart;
  }

  return skFuncBody;
}

}}}
