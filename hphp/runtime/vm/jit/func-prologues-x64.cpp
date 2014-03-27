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
#include "hphp/runtime/vm/jit/func-prologues-x64.h"

#include "hphp/util/asm-x64.h"

#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-x64.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/mc-generator-internal.h"
#include "hphp/runtime/vm/jit/write-lease.h"
#include "hphp/runtime/vm/jit/translator-runtime.h"

namespace HPHP { namespace JIT { namespace X64 {

//////////////////////////////////////////////////////////////////////


TRACE_SET_MOD(mcg);

//////////////////////////////////////////////////////////////////////

namespace {

void emitStackCheck(X64Assembler& a, int funcDepth, Offset pc) {
  using namespace reg;
  funcDepth += kStackCheckPadding * sizeof(Cell);

  int stackMask = cellsToBytes(RuntimeOption::EvalVMStackElms) - 1;
  a.    movq   (rVmSp, rAsm);  // copy to destroy
  a.    andq   (stackMask, rAsm);
  a.    subq   (funcDepth + Stack::sSurprisePageSize, rAsm);
  a.    jl     (tx->uniqueStubs.stackOverflowHelper);
}

/*
 * This will omit overflow checks if it is a leaf function that can't
 * use more than kStackCheckLeafPadding cells.
 */
void maybeEmitStackCheck(X64Assembler& a, const Func* func) {
  auto const needStackCheck =
    !(func->attrs() & AttrPhpLeafFn) ||
    func->maxStackCells() >= kStackCheckLeafPadding;
  if (needStackCheck) {
    emitStackCheck(a, cellsToBytes(func->maxStackCells()), func->base());
  }
}

TCA emitFuncGuard(X64Assembler& a, const Func* func) {
  using namespace reg;
  assert(kScratchCrossTraceRegs.contains(rax));
  assert(kScratchCrossTraceRegs.contains(rdx));

  const int kAlign = kX64CacheLineSize;
  const int kAlignMask = kAlign - 1;
  auto funcImm = Immed64(func);
  int loBits = uintptr_t(a.frontier()) & kAlignMask;
  int delta, size;

  // Ensure the immediate is safely smashable
  // the immediate must not cross a qword boundary,
  if (!funcImm.fits(sz::dword)) {
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
  if (!funcImm.fits(sz::dword)) {
    a.  loadq  (rStashedAR[AROFF(m_func)], rax);
    /*
      Although func doesnt fit in a signed 32-bit immediate, it may still
      fit in an unsigned one. Rather than deal with yet another case
      (which only happens when we disable jemalloc) just force it to
      be an 8-byte immediate, and patch it up afterwards.
    */
    a.  movq   (0xdeadbeeffeedface, rdx);
    assert(((uint64_t*)a.frontier())[-1] == 0xdeadbeeffeedface);
    ((uint64_t*)a.frontier())[-1] = uintptr_t(func);
    a.  cmpq   (rax, rdx);
  } else {
    a.  cmpq   (funcImm.l(), rStashedAR[AROFF(m_func)]);
  }
  a.    jnz    (tx->uniqueStubs.funcPrologueRedispatch);

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
  using namespace reg;

  int numParams = func->numParams();
  const Func::ParamInfoVec& paramInfo = func->params();

  Offset entryOffset = func->getEntryForNumArgs(nPassed);

  Asm a { mcg->code.main() };

  if (tx->mode() == TransProflogue) {
    assert(func->shouldPGO());
    TransID transId  = tx->profData()->curTransID();
    auto counterAddr = tx->profData()->transCounterAddr(transId);
    a.movq(counterAddr, rAsm);
    a.decq(rAsm[0]);
  }

  // Note: you're not allowed to use rVmSp around here for anything in
  // the nPassed == numParams case, because it might be junk if we
  // came from emitMagicFuncPrologue.

  if (nPassed > numParams) {
    // Too many args; a weird case, so just callout. Stash ar
    // somewhere callee-saved.
    if (false) { // typecheck
      JIT::trimExtraArgs((ActRec*)nullptr);
    }
    a.    movq   (rStashedAR, argNumToRegName[0]);
    emitCall(a, TCA(JIT::trimExtraArgs));
    // We'll fix rVmSp below.
  } else if (nPassed < numParams) {
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
      a.    subq   (int(sizeof(Cell)), rVmSp);
      a.    incl   (eax);
      emitStoreUninitNull(a, 0, rVmSp);
      a.    cmpl   (numParams, eax);
      a.    jl8    (loopTop);
    }
  }

  // Entry point for numParams == nPassed is here.
  // Args are kosher. Frame linkage: set fp = ar.
  a.    movq   (rStashedAR, rVmFp);

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
  // otherwise, do it in a compact loop.
  int numUninitLocals = func->numLocals() - numLocals;
  assert(numUninitLocals >= 0);
  if (numUninitLocals > 0) {

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
      a.  addq   (int(sizeof(Cell)), loopReg);
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

  auto destPC = func->unit()->entry() + entryOffset;
  SrcKey funcBody(func, destPC);

  // Move rVmSp to the right place: just past all locals
  int frameCells = func->numSlotsInFrame();
  emitLea(a, rVmFp[-cellsToBytes(frameCells)], rVmSp);

  Fixup fixup(funcBody.offset() - func->base(), frameCells);

  // Emit warnings for any missing arguments
  if (!func->isCPPBuiltin()) {
    for (int i = nPassed; i < numParams; ++i) {
      if (paramInfo[i].funcletOff() == InvalidAbsoluteOffset) {
        a.  emitImmReg((intptr_t)func->name()->data(), argNumToRegName[0]);
        a.  emitImmReg(numParams, argNumToRegName[1]);
        a.  emitImmReg(i, argNumToRegName[2]);
        emitCall(a, (TCA)raiseMissingArgument);
        mcg->fixupMap().recordFixup(a.frontier(), fixup);
        break;
      }
    }
  }

  // Check surprise flags in the same place as the interpreter: after
  // setting up the callee's frame but before executing any of its
  // code
  emitCheckSurpriseFlagsEnter(mcg->code.main(), mcg->code.stubs(), false,
                              mcg->fixupMap(), fixup);

  if (func->isClosureBody() && func->cls()) {
    int entry = nPassed <= numParams ? nPassed : numParams + 1;
    // Relying on rStashedAR == rVmFp here
    a.    loadq   (rStashedAR[AROFF(m_func)], rax);
    a.    loadq   (rax[Func::prologueTableOff() + sizeof(TCA)*entry],
                   rax);
    a.    jmp     (rax);
  } else {
    emitBindJmp(mcg->code.main(), mcg->code.stubs(), funcBody);
  }
  return funcBody;
}

} // anonymous namespace

//////////////////////////////////////////////////////////////////////

TCA emitCallArrayPrologue(Func* func, DVFuncletsVec& dvs) {
  auto& mainCode = mcg->code.main();
  auto& stubsCode = mcg->code.stubs();
  Asm a { mainCode };
  TCA start = mainCode.frontier();
  if (dvs.size() == 1) {
    a.  cmpl  (dvs[0].first, rVmFp[AROFF(m_numArgsAndGenCtorFlags)]);
    emitBindJcc(mainCode, stubsCode, CC_LE, SrcKey(func, dvs[0].second));
    emitBindJmp(mainCode, stubsCode, SrcKey(func, func->base()));
  } else {
    a.    loadl  (rVmFp[AROFF(m_numArgsAndGenCtorFlags)], reg::eax);
    for (unsigned i = 0; i < dvs.size(); i++) {
      a.  cmpl   (dvs[i].first, reg::eax);
      emitBindJcc(mainCode, stubsCode, CC_LE, SrcKey(func, dvs[i].second));
    }
    emitBindJmp(mainCode, stubsCode, SrcKey(func, func->base()));
  }
  return start;
}

SrcKey emitFuncPrologue(Func* func, int nPassed, TCA& start) {
  assert(!func->isMagic());
  Asm a { mcg->code.main() };

  start = emitFuncGuard(a, func);
  if (RuntimeOption::EvalJitTransCounters) emitTransCounterInc(a);
  a.    pop    (rStashedAR[AROFF(m_savedRip)]);
  maybeEmitStackCheck(a, func);
  return emitPrologueWork(func, nPassed);
}

SrcKey emitMagicFuncPrologue(Func* func, uint32_t nPassed, TCA& start) {
  assert(func->isMagic());
  assert(func->numParams() == 2);
  using namespace reg;
  using MkPacked = HphpArray* (*)(uint32_t, const TypedValue*);

  Asm a { mcg->code.main() };
  Label not_magic_call;
  auto const rInvName = r13;
  assert(!kSpecialCrossTraceRegs.contains(r13));

  auto skFuncBody = SrcKey {};
  auto callFixup  = TCA { nullptr };

  /*
   * If nPassed is not 2, we need to generate a non-magic prologue
   * that can be used if there is no invName on the ActRec.
   * (I.e. someone called __call directly.)  In the case where nPassed
   * is 2, whether it's magic or not the prologue we generate at the
   * end will work.
   *
   * This is placed in a ahead of the actual prologue entry point, but
   * only because emitPrologueWork can't easily go to astubs right now.
   */
  if (nPassed != 2) {
    asm_label(a, not_magic_call);
    skFuncBody = emitPrologueWork(func, nPassed);
    // There is a REQ_BIND_JMP at the end of emitPrologueWork.
  }

  // Main prologue entry point is here.
  start = emitFuncGuard(a, func);
  if (RuntimeOption::EvalJitTransCounters) emitTransCounterInc(a);
  a.    pop    (rStashedAR[AROFF(m_savedRip)]);
  maybeEmitStackCheck(a, func);

  /*
   * Detect if this was actually a magic call (i.e. the ActRec has an
   * invName), and shuffle the magic call arguments into a packed
   * array.
   *
   * If it's not a magic call, we jump backward to a normal function
   * prologue (see above) for nPassed.  Except if nPassed is 2, we'll
   * be jumping over the magic call shuffle, to the prologue for 2
   * args below.
   */
  a.    loadq  (rStashedAR[AROFF(m_invName)], rInvName);
  a.    testb  (1, rbyte(rInvName));
  if (nPassed == 2) {
    a.  jz8    (not_magic_call);
  } else {
    not_magic_call.jccAuto(a, CC_Z);
  }
  a.    decq   (rInvName);
  a.    storeq (0, rStashedAR[AROFF(m_varEnv)]);
  if (nPassed != 0) { // for zero args, we use the empty array
    a.  movq   (rStashedAR, argNumToRegName[0]);
    a.  subq   (rVmSp, argNumToRegName[0]);
    a.  shrq   (0x4, argNumToRegName[0]);
    a.  movq   (rVmSp, argNumToRegName[1]);
    emitCall(a, reinterpret_cast<CodeAddress>(MkPacked{HphpArray::MakePacked}));
    callFixup = a.frontier();
  }
  if (nPassed != 2) {
    a.  storel (2, rStashedAR[AROFF(m_numArgsAndGenCtorFlags)]);
  }
  if (debug) { // "assertion": the emitPrologueWork path fixes up rVmSp.
    a.  movq   (0, rVmSp);
  }

  // Magic calls expect two arguments---first the name of the called
  // function, and then a packed array of the arguments to the
  // function.  These are where these two TV's will be.
  auto const strTV   = rStashedAR - cellsToBytes(1);
  auto const arrayTV = rStashedAR - cellsToBytes(2);

  // Store the two arguments for the magic call.
  emitStoreTVType(a, KindOfString, strTV[TVOFF(m_type)]);
  a.    storeq (rInvName, strTV[TVOFF(m_data)]);
  emitStoreTVType(a, KindOfArray, arrayTV[TVOFF(m_type)]);
  if (nPassed == 0) {
    emitImmStoreq(a, HphpArray::GetStaticEmptyArray(),
                  arrayTV[TVOFF(m_data)]);
  } else {
    a.  storeq (rax, arrayTV[TVOFF(m_data)]);
  }

  // Every magic call prologue has a case for nPassed == 2, because
  // this is how it works when the call is actually magic.
  if (nPassed == 2) asm_label(a, not_magic_call);
  auto const skFor2Args = emitPrologueWork(func, 2);
  if (nPassed == 2) skFuncBody = skFor2Args;

  if (RuntimeOption::HHProfServerEnabled && callFixup) {
    mcg->fixupMap().recordFixup(
      callFixup,
      Fixup { skFuncBody.offset() - func->base(), func->numSlotsInFrame() }
    );
  }

  return skFuncBody;
}

}}}
