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
#include "hphp/util/ringbuffer.h"

#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-x64.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/mc-generator-internal.h"
#include "hphp/runtime/vm/jit/prof-data.h"
#include "hphp/runtime/vm/jit/translator-runtime.h"
#include "hphp/runtime/vm/jit/write-lease.h"
#include "hphp/runtime/vm/jit/relocation.h"

namespace HPHP { namespace jit { namespace x64 {

//////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(mcg);

//////////////////////////////////////////////////////////////////////

namespace {

// Generate an if-then block into a.  thenBlock is executed if cc is true.
template <class Then>
void ifThen(jit::X64Assembler& a, ConditionCode cc, Then thenBlock) {
  Label done;
  a.jcc8(ccNegate(cc), done);
  thenBlock(a);
  asm_label(a, done);
}

void emitStackCheck(X64Assembler& a, int funcDepth, Offset pc) {
  using reg::rax;
  assertx(kScratchCrossTraceRegs.contains(rax));

  funcDepth += kStackCheckPadding * sizeof(Cell);

  /*
   * rVmFp points to the bottom of the callee's ActRec.  We're checking for
   * `funcDepth' additional space below it, which includes everything
   * maxStackCells() includes for the callee (see comment in func.h).  If
   * nPassed > nparams, the stack might currently be deeper than that, but
   * we're going to be removing the excess nPassed (and the caller already
   * checked it was ok to go that deep in their maxStackCells) before we start
   * using our space.
   */
  auto const stackMask =
    int32_t{cellsToBytes(RuntimeOption::EvalVMStackElms) - 1};
  a.    movq   (rVmFp, rax);  // copy to destroy
  a.    andq   (stackMask, rax);
  a.    subq   (funcDepth + Stack::sSurprisePageSize, rax);
  a.    jl     (mcg->tx().uniqueStubs.stackOverflowHelper);
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
  assertx(kScratchCrossTraceRegs.contains(rax));

  auto const funcImm = Immed64(func);
  int nBytes, offset;

  // Ensure the immediate is safely smashable
  // the immediate must not cross a qword boundary,
  if (!funcImm.fits(sz::dword)) {
    nBytes = kFuncGuardLen;
    offset = kFuncMovImm;
  } else {
    nBytes = kFuncGuardShortLen;
    offset = kFuncCmpImm;
  }

  mcg->backEnd().prepareForSmash(a.code(), nBytes, offset);

  TCA aStart DEBUG_ONLY = a.frontier();
  if (!funcImm.fits(sz::dword)) {
    /*
      Although func doesnt fit in a signed 32-bit immediate, it may still
      fit in an unsigned one. Rather than deal with yet another case
      (which only happens when we disable jemalloc) just force it to
      be an 8-byte immediate, and patch it up afterwards.
    */
    a.  movq   (0xdeadbeeffeedface, rax);
    assertx(((uint64_t*)a.frontier())[-1] == 0xdeadbeeffeedface);
    ((uint64_t*)a.frontier())[-1] = uintptr_t(func);
    a.  cmpq   (rax, rVmFp[AROFF(m_func)]);
  } else {
    a.  cmpq   (funcImm.l(), rVmFp[AROFF(m_func)]);
  }
  a.    jnz    (mcg->tx().uniqueStubs.funcPrologueRedispatch);

  assertx(funcPrologueToGuard(a.frontier(), func) == aStart);
  assertx(funcPrologueHasGuard(a.frontier(), func));
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

SrcKey emitPrologueWork(TransID transID, Func* func, int nPassed) {
  using namespace reg;

  auto const numNonVariadicParams = func->numNonVariadicParams();
  auto const& paramInfo           = func->params();
  auto const entryOffset          = func->getEntryForNumArgs(nPassed);

  Asm a { mcg->code.main() };

  // "assert" nothing uses rVmSp.
  if (debug) {
    a.    movq   (0x0fffafffbeefbeef, rVmSp);
  }

  if (mcg->tx().mode() == TransKind::Proflogue) {
    assertx(shouldPGOFunc(*func));
    auto const counterAddr = mcg->tx().profData()->transCounterAddr(transID);
    a.    movq   (counterAddr, rAsm);
    a.    decq   (rAsm[0]);
    mcg->tx().profData()->setProfiling(func->getFuncId());
  }

  auto invNameAlreadyOK = false;

  if (nPassed > numNonVariadicParams) {
    // Too many args; a weird case, so call out to an appropriate helper.
    // Stash ar somewhere callee-saved.
    if (false) { // typecheck
      shuffleExtraArgsMayUseVV((ActRec*)nullptr);
      shuffleExtraArgsVariadicAndVV((ActRec*)nullptr);
      shuffleExtraArgsVariadic((ActRec*)nullptr);
      trimExtraArgs((ActRec*)nullptr);
    }
    a.    movq   (rVmFp, argNumToRegName[0]);

    if (LIKELY(func->discardExtraArgs())) {
      emitCall(a, TCA(trimExtraArgs), argSet(1));
    } else if (func->attrs() & AttrMayUseVV) {
      emitCall(a,
               func->hasVariadicCaptureParam()
                 ? TCA(shuffleExtraArgsVariadicAndVV)
                 : TCA(shuffleExtraArgsMayUseVV),
               argSet(1));
    } else {
      assertx(func->hasVariadicCaptureParam());
      emitCall(a, TCA(shuffleExtraArgsVariadic), argSet(1));
    }
    invNameAlreadyOK = true;
  } else if (nPassed < numNonVariadicParams) {
    if (numNonVariadicParams - nPassed <= kMaxParamsInitUnroll) {
      for (auto i = nPassed; i < numNonVariadicParams; i++) {
        auto const offset = cellsToBytes(-i - 1);
        emitStoreTVType(a, KindOfUninit, rVmFp[offset + TVOFF(m_type)]);
      }
      if (func->hasVariadicCaptureParam()) {
        auto const offset = cellsToBytes(-numNonVariadicParams - 1);
        emitStoreTVType(a, KindOfArray, rVmFp[offset + TVOFF(m_type)]);
        emitImmStoreq(a, staticEmptyArray(),
                      rVmFp[offset + TVOFF(m_data)]);
      }
    } else {
      Label loopTop;
      assertx(kScratchCrossTraceRegs.contains(rax));
      assertx(kScratchCrossTraceRegs.contains(rdx));
      a.    lea    (rVmFp[-cellsToBytes(nPassed + 1)], rdx);
      a.    lea    (rVmFp[-cellsToBytes(numNonVariadicParams + 1)], rax);
    asm_label(a, loopTop);
      emitStoreUninitNull(a, 0, rdx);
      a.    subq   (int32_t{sizeof(Cell)}, rdx);
      a.    cmpq   (rax, rdx);
      a.    jnz8   (loopTop);

      if (func->hasVariadicCaptureParam()) {
        emitStoreTVType(a, KindOfArray, rax[TVOFF(m_type)]);
        emitImmStoreq(a, staticEmptyArray(), rax[TVOFF(m_data)]);
      }
    }
  } else if (func->hasVariadicCaptureParam()) {
    assertx(nPassed == numNonVariadicParams);
    assertx(!func->isMagic());
    auto const offset = cellsToBytes(-nPassed - 1);
    emitStoreTVType(a, KindOfArray, rVmFp[offset + TVOFF(m_type)]);
    emitImmStoreq(a, staticEmptyArray(), rVmFp[offset + TVOFF(m_data)]);
  }

  if (!invNameAlreadyOK) {
    if (func->attrs() & AttrMayUseVV) {
      a.    storeq (0, rVmFp[AROFF(m_invName)]);
    }
  }

  int numLocals = func->numParams();
  if (func->isClosureBody()) {
    // Closure object properties are the use vars followed by the
    // static locals (which are per-instance).
    auto const numUseVars = func->baseCls()->numDeclProperties() -
                            func->numStaticLocals();

    assertx(kScratchCrossTraceRegs.contains(rcx));
    auto const rClosure = rcx;
    a.  loadq  (rVmFp[AROFF(m_this)], rClosure);

    // Swap in the $this or late bound class
    assertx(kScratchCrossTraceRegs.contains(rax));
    a.  loadq  (rClosure[c_Closure::ctxOffset()], rax);
    a.  storeq (rax, rVmFp[AROFF(m_this)]);

    if (!func->isStatic()) {
      a.shrq   (1, rax);
      ifThen(a, CC_NBE, [&] (Asm& a) {
        a.shlq (1, rax);
        emitIncRefCheckNonStatic(a, rax, KindOfObject);
      });
    }

    // Copy in all the use vars
    int baseUVOffset = sizeof(ObjectData) + func->baseCls()->builtinODTailSize();
    for (int i = 0; i < numUseVars + 1; i++) {
      int fpOffset = -cellsToBytes(i + 1 + numLocals);

      if (i == 0) {
        // The closure is the first local.
        // We don't incref because it used to be $this
        // and now it is a local, so they cancel out
        emitStoreTypedValue(a, KindOfObject, rClosure, fpOffset, rVmFp);
        continue;
      }

      int uvOffset = baseUVOffset + cellsToBytes(i-1);

      emitCopyTo(a, rClosure, uvOffset, rVmFp, fpOffset, rAsm);
      emitIncRefGenericRegSafe(a, rVmFp, fpOffset, rAsm);
    }

    numLocals += numUseVars + 1;
  }

  // We're in the callee frame; initialize locals. Unroll the loop all
  // the way if there are a modest number of locals to update;
  // otherwise, do it in a compact loop.
  int numUninitLocals = func->numLocals() - numLocals;
  assertx(numUninitLocals >= 0);
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
      if (func->numParams() < func->numLocals()) {
        a.xorl (eax, eax);
      }
      for (k = numLocals; k < func->numLocals(); ++k) {
        locToRegDisp(k, &base, &disp);
        emitStoreTVType(a, eax, base[disp + TVOFF(m_type)]);
      }
    }
  }

  auto destPC = func->unit()->entry() + entryOffset;
  SrcKey funcBody(func, destPC, false);

  Fixup const fixup(funcBody.offset() - func->base(), func->numSlotsInFrame());

  // Emit warnings for any missing arguments
  if (!func->isCPPBuiltin()) {
    for (int i = nPassed; i < numNonVariadicParams; ++i) {
      if (paramInfo[i].funcletOff == InvalidAbsoluteOffset) {
        if (false) { // typecheck
          raiseMissingArgument((const Func*) nullptr, 0);
        }
        a.  emitImmReg((intptr_t)func, argNumToRegName[0]);
        a.  emitImmReg(nPassed, argNumToRegName[1]);
        emitCall(a, TCA(raiseMissingArgument), argSet(2));
        mcg->recordSyncPoint(a.frontier(), fixup);
        break;
      }
    }
  }

  // Check surprise flags in the same place as the interpreter: after
  // setting up the callee's frame but before executing any of its
  // code
  {
    Asm acold { mcg->code.cold() };
    a.    cmpq    (rVmFp, rVmTl[rds::kSurpriseFlagsOff]);
    a.    jnbe    (acold.frontier());

    emitCall(acold, mcg->tx().uniqueStubs.functionEnterHelper, argSet(0));
    mcg->recordSyncPoint(acold.frontier(), fixup);
    acold.    jmp   (a.frontier());
  }

  emitBindJ(
    mcg->code.main(),
    mcg->code.frozen(),
    CC_None,
    funcBody,
    FPInvOffset{func->numSlotsInFrame()},
    TransFlags{}
  );

  return funcBody;
}

} // anonymous namespace

//////////////////////////////////////////////////////////////////////

TCA emitCallArrayPrologue(Func* func, DVFuncletsVec& dvs) {
  auto& mainCode = mcg->code.main();
  auto& frozenCode = mcg->code.frozen();
  Asm a { mainCode };
  auto const start = mainCode.frontier();
  assertx(mcg->cgFixups().empty());
  auto const spOff = FPInvOffset{func->numSlotsInFrame()};

  if (dvs.size() == 1) {
    a.  cmpl  (dvs[0].first, rVmFp[AROFF(m_numArgsAndFlags)]);
    emitBindJ(mainCode, frozenCode, CC_LE, SrcKey(func, dvs[0].second, false),
      spOff, TransFlags{});
    emitBindJ(mainCode, frozenCode, CC_None, SrcKey(func, func->base(), false),
      spOff, TransFlags{});
  } else {
    a.    loadl  (rVmFp[AROFF(m_numArgsAndFlags)], reg::eax);
    for (unsigned i = 0; i < dvs.size(); i++) {
      a.  cmpl   (dvs[i].first, reg::eax);
      emitBindJ(mainCode, frozenCode, CC_LE,
                SrcKey(func, dvs[i].second, false), spOff, TransFlags{});
    }
    emitBindJ(mainCode, frozenCode, CC_None, SrcKey(func, func->base(), false),
      spOff, TransFlags{});
  }

  if (RuntimeOption::EvalPerfRelocate) {
    GrowableVector<IncomingBranch> incomingBranches;
    SrcKey sk = SrcKey(func, dvs[0].second, false);
    recordPerfRelocMap(start, mainCode.frontier(),
                       frozenCode.frontier(), frozenCode.frontier(),
                       sk, 0,
                       incomingBranches,
                       mcg->cgFixups());
  }

  mcg->cgFixups().process(nullptr);
  return start;
}

SrcKey emitFuncPrologue(TransID transID, Func* func, int nPassed, TCA& start) {
  assertx(!func->isMagic());
  Asm a { mcg->code.main() };

  start = emitFuncGuard(a, func);
  if (Trace::moduleEnabled(Trace::ringbuffer)) {
    auto arg = 0;
    auto name = func->fullName()->data();
    a.  movq   (reinterpret_cast<uintptr_t>(name), argNumToRegName[arg++]);
    a.  movq   (strlen(name), argNumToRegName[arg++]);
    a.  movq   (Trace::RBTypeFuncPrologue, argNumToRegName[arg++]);
    a.  call   (TCA(Trace::ringbufferMsg));
  }

  if (RuntimeOption::EvalJitTransCounters) emitTransCounterInc(a);
  a.    pop    (rVmFp[AROFF(m_savedRip)]);
  maybeEmitStackCheck(a, func);
  return emitPrologueWork(transID, func, nPassed);
}

SrcKey emitMagicFuncPrologue(TransID transID, Func* func, int nPassed,
                             TCA& start) {
  assertx(func->isMagic());
  assertx(func->numParams() == 2);
  assertx(!func->hasVariadicCaptureParam());
  using namespace reg;
  using MkPacked = ArrayData* (*)(uint32_t, const TypedValue*);

  Asm a { mcg->code.main() };
  Label not_magic_call;
  auto const rInvName = r13;
  assertx(!kCrossCallRegs.contains(r13));

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
   * only because emitPrologueWork can't easily go to acold right now.
   */
  if (nPassed != 2) {
    asm_label(a, not_magic_call);
    skFuncBody = emitPrologueWork(transID, func, nPassed);
    // There is a REQ_BIND_JMP at the end of emitPrologueWork.
  }

  // Main prologue entry point is here.
  start = emitFuncGuard(a, func);
  if (RuntimeOption::EvalJitTransCounters) emitTransCounterInc(a);
  a.    pop    (rVmFp[AROFF(m_savedRip)]);
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
  a.    loadl  (rVmFp[AROFF(m_numArgsAndFlags)], eax);
  a.    andl   (static_cast<int32_t>(ActRec::Flags::MagicDispatch), eax);
  a.    cmpl   (static_cast<int32_t>(ActRec::Flags::MagicDispatch), eax);
  if (nPassed == 2) {
    a.  jnz8   (not_magic_call);
  } else {
    not_magic_call.jccAuto(a, CC_NZ);
  }
  a.    loadq  (rVmFp[AROFF(m_invName)], rInvName);
  a.    storeq (0, rVmFp[AROFF(m_varEnv)]);
  if (nPassed != 0) { // for zero args, we use the empty array
    static_assert(sizeof(Cell) == 16, "");
    a.  loadl  (rVmFp[AROFF(m_numArgsAndFlags)], eax);
    a.  movq   (rVmFp, argNumToRegName[1]);
    a.  andl   (ActRec::kNumArgsMask, eax);
    a.  movl   (eax, r32(argNumToRegName[0]));
    a.  shll   (0x4, eax);
    a.  subq   (rax, argNumToRegName[1]);
    emitCall(a, reinterpret_cast<CodeAddress>(
      MkPacked{MixedArray::MakePacked}), argSet(2));
    callFixup = a.frontier();
  }

  // We store 2 here even if nPassed == 2 already, because we're going to use
  // it to clear the ActRec::MagicDispatch flags.
  a.    storel (2, rVmFp[AROFF(m_numArgsAndFlags)]);

  // Magic calls expect two arguments---first the name of the called
  // function, and then a packed array of the arguments to the
  // function.  These are where these two TV's will be.
  auto const strTV   = rVmFp - cellsToBytes(1);
  auto const arrayTV = rVmFp - cellsToBytes(2);

  // Store the two arguments for the magic call.
  emitStoreTVType(a, KindOfString, strTV[TVOFF(m_type)]);
  a.    storeq (rInvName, strTV[TVOFF(m_data)]);
  emitStoreTVType(a, KindOfArray, arrayTV[TVOFF(m_type)]);
  if (nPassed == 0) {
    emitImmStoreq(a, staticEmptyArray(), arrayTV[TVOFF(m_data)]);
  } else {
    a.  storeq (rax, arrayTV[TVOFF(m_data)]);
  }

  // Every magic call prologue has a case for nPassed == 2, because
  // this is how it works when the call is actually magic.
  if (nPassed == 2) asm_label(a, not_magic_call);
  auto const skFor2Args = emitPrologueWork(transID, func, 2);
  if (nPassed == 2) skFuncBody = skFor2Args;

  if (RuntimeOption::HHProfServerEnabled && callFixup) {
    mcg->recordSyncPoint(callFixup,
                         Fixup{skFuncBody.offset() - func->base(),
                               func->numSlotsInFrame()});
  }

  return skFuncBody;
}

}}}
