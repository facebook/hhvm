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
#include "hphp/runtime/vm/jit/func-prologues-arm.h"

#include "hphp/vixl/a64/macro-assembler-a64.h"

#include "hphp/runtime/ext/ext_closure.h"
#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-arm.h"
#include "hphp/runtime/vm/jit/jump-smash.h"
#include "hphp/runtime/vm/jit/service-requests-arm.h"
#include "hphp/runtime/vm/jit/mc-generator.h"

namespace HPHP { namespace JIT { namespace ARM {


//////////////////////////////////////////////////////////////////////

namespace {

void emitStackCheck(int funcDepth, Offset pc) {
  vixl::MacroAssembler a { mcg->code.main() };
  funcDepth += cellsToBytes(kStackCheckPadding);

  uint64_t stackMask = cellsToBytes(RuntimeOption::EvalVMStackElms) - 1;
  a.   And  (rAsm, rVmSp, stackMask);
  a.   Sub  (rAsm, rAsm, funcDepth + Stack::sSurprisePageSize, vixl::SetFlags);
  // This doesn't need to be smashable, but it is a long jump from mainCode to
  // stubs, so it can't be direct.
  emitSmashableJump(mcg->code.main(), tx->uniqueStubs.stackOverflowHelper,
                    CC_L);
}

TCA emitFuncGuard(vixl::MacroAssembler& a, Func* func) {
  vixl::Label success;
  vixl::Label redispatchStubAddr;
  vixl::Label funcAddr;
  DEBUG_ONLY TCA start = a.frontier();

  a.   Ldr   (rAsm, rStashedAR[AROFF(m_func)]);
  a.   Ldr   (rAsm2, &funcAddr);
  a.   Cmp   (rAsm, rAsm2);
  a.   B     (&success, vixl::eq);
  // Load the address of the redispatch stub and jump to it.
  a.   Ldr   (rAsm, &redispatchStubAddr);
  a.   Br    (rAsm);

  if (!a.isFrontierAligned(8)) {
    a. Nop   ();
    assert(a.isFrontierAligned(8));
  }
  a.   bind  (&redispatchStubAddr);
  a.   dc64  (tx->uniqueStubs.funcPrologueRedispatch);
  // The guarded Func* comes right before the end so that
  // funcPrologueToGuardImmPtr() is simple.
  a.   bind  (&funcAddr);
  a.   dc64  (func);
  a.   bind  (&success);

  assert(funcPrologueToGuard(a.frontier(), func) == start);
  assert(funcPrologueHasGuard(a.frontier(), func));

  return a.frontier();
}

constexpr auto kLocalsToInitializeInline = 9;

SrcKey emitPrologueWork(Func* func, int nPassed) {
  vixl::MacroAssembler a { mcg->code.main() };

  if (tx->mode() == TransProflogue) {
    not_implemented();
  }

  auto dvInitializer = InvalidAbsoluteOffset;
  auto const numParams = func->numParams();
  auto const& paramInfo = func->params();

  // Resolve cases where the wrong number of args was passed.
  if (nPassed > numParams) {
    void (*helper)(ActRec*) = JIT::trimExtraArgs;
    a.  Mov    (argReg(0), rStashedAR);
    emitCall(a, CppCall(helper));
    // We'll fix rVmSp below.
  } else if (nPassed < numParams) {
    for (auto i = nPassed; i < numParams; ++i) {
      auto const& pi = paramInfo[i];
      if (pi.hasDefaultValue()) {
        dvInitializer = pi.funcletOff();
        break;
      }
    }

    a.  Mov    (rAsm, nPassed);

    // do { *(--rVmSp) = NULL; nPassed++; } while (nPassed < numParams);
    vixl::Label loopTop;
    a.  bind   (&loopTop);
    a.  Sub    (rVmSp, rVmSp, sizeof(Cell));
    a.  Add    (rAsm, rAsm, 1);
    static_assert(KindOfUninit == 0, "need this for zero-register hack");
    a.  Strb   (vixl::xzr, rVmSp[TVOFF(m_type)]);
    a.  Cmp    (rAsm, numParams);
    a.  B      (&loopTop, vixl::lt);
  }

  // Frame linkage.
  a.    Mov    (rVmFp, rStashedAR);

  auto numLocals = numParams;

  if (func->isClosureBody()) {
    int numUseVars = func->cls()->numDeclProperties() -
                     func->numStaticLocals();

    emitRegGetsRegPlusImm(a, rVmSp, rVmFp, -cellsToBytes(numParams));

    // This register needs to live a long time, across calls to helpers that may
    // use both rAsm and rAsm2. So it can't be one of them. Fortunately, we're
    // between blocks here, so no normal registers are live; just pick any.
    auto const& rClosure = vixl::x0;
    a.    Ldr    (rClosure, rVmFp[AROFF(m_this)]);

    // Swap in the $this or late bound class
    a.    Ldr    (rAsm, rClosure[c_Closure::ctxOffset()]);
    a.    Str    (rAsm, rVmFp[AROFF(m_this)]);

    if (!(func->attrs() & AttrStatic)) {
      // Only do the incref if rAsm is not zero AND its LSB is zero.
      vixl::Label notRealThis;
      // Jump if rAsm is zero.
      a.  Cbz    (rAsm, &notRealThis);
      // Tbnz = test and branch if not zero. It tests a single bit, given by a
      // position (in this case, 0, the second argument).
      a.  Tbnz   (rAsm, 0, &notRealThis);

      auto wCount = rAsm2.W();
      a.  Ldr    (wCount, rAsm[FAST_REFCOUNT_OFFSET]);
      a.  Add    (wCount, wCount, 1);
      a.  Str    (wCount, rAsm[FAST_REFCOUNT_OFFSET]);
      a.  bind   (&notRealThis);
    }

    // Put in the correct context
    a.    Ldr    (rAsm, rClosure[c_Closure::funcOffset()]);
    a.    Str    (rAsm, rVmFp[AROFF(m_func)]);

    // Copy in all the use vars
    int baseUVOffset = sizeof(ObjectData) + func->cls()->builtinODTailSize();
    for (auto i = 0; i < numUseVars + 1; i++) {
      auto spOffset = -cellsToBytes(i + 1);
      if (i == 0) {
        // The closure is the first local.
        // We don't incref because it used to be $this
        // and now it is a local, so they cancel out
        a.Mov    (rAsm, KindOfObject);
        a.Strb   (rAsm.W(), rVmSp[spOffset + TVOFF(m_type)]);
        a.Str    (rClosure, rVmSp[spOffset + TVOFF(m_data)]);
        continue;
      }

      auto uvOffset = baseUVOffset + cellsToBytes(i - 1);

      a.  Ldr    (rAsm, rClosure[uvOffset + TVOFF(m_data)]);
      a.  Str    (rAsm, rVmSp[spOffset + TVOFF(m_data)]);
      a.  Ldrb   (rAsm.W(), rClosure[uvOffset + TVOFF(m_type)]);
      a.  Strb   (rAsm.W(), rVmSp[spOffset + TVOFF(m_type)]);
      emitIncRefGeneric(a, rVmSp, spOffset);
    }

    numLocals += numUseVars + 1;
  }

  auto numUninitLocals = func->numLocals() - numLocals;
  assert(numUninitLocals >= 0);
  if (numUninitLocals > 0) {
    if (numUninitLocals > kLocalsToInitializeInline) {
      auto const& loopReg = rAsm2;

      auto loopStart = cellsToBytes(-func->numLocals()) + TVOFF(m_type);
      auto loopEnd = cellsToBytes(-numLocals) + TVOFF(m_type);

      a.  Mov  (loopReg, loopStart);

      vixl::Label loopTop;
      a.  bind (&loopTop);
      // do {
      //   rVmFp[loopReg].m_type = KindOfUninit;
      // } while(++loopReg != loopEnd);

      static_assert(KindOfUninit == 0, "need this for zero-register hack");
      a.  Strb  (vixl::xzr, rVmFp[loopReg]);
      a.  Add   (loopReg, loopReg, sizeof(Cell));
      a.  Cmp   (loopReg, loopEnd);
      a.  B     (&loopTop, vixl::ne);
    } else {
      for (auto k = numLocals; k < func->numLocals(); ++k) {
        int disp =
          cellsToBytes(locPhysicalOffset(Location(Location::Local, k), func));
        a.Strb  (vixl::xzr, rVmFp[disp + TVOFF(m_type)]);
      }
    }
  }

  auto const* destPC = func->unit()->entry() + func->base();
  if (dvInitializer != InvalidAbsoluteOffset) {
    destPC = func->unit()->entry() + dvInitializer;
  }
  SrcKey funcBody(func, destPC);

  // Set stack pointer just past all locals
  int frameCells = func->numSlotsInFrame();
  emitRegGetsRegPlusImm(a, rVmSp, rVmFp, -cellsToBytes(frameCells));

  Fixup fixup(funcBody.offset() - func->base(), frameCells);

  // Emit warnings for missing arguments
  if (!func->isCPPBuiltin()) {
    for (auto i = nPassed; i < numParams; ++i) {
      if (paramInfo[i].funcletOff() == InvalidAbsoluteOffset) {
        a.  Mov  (argReg(0), func->name()->data());
        a.  Mov  (argReg(1), numParams);
        a.  Mov  (argReg(2), i);
        auto fixupAddr = emitCall(a, CppCall(JIT::raiseMissingArgument));
        mcg->fixupMap().recordFixup(fixupAddr, fixup);
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
    a.   Ldr   (rAsm, rStashedAR[AROFF(m_func)]);
    a.   Ldr   (rAsm, rAsm[Func::prologueTableOff() + sizeof(TCA)*entry]);
    a.   Br    (rAsm);
  } else {
    emitBindJmp(mcg->code.main(), mcg->code.stubs(), funcBody);
  }
  return funcBody;
}

//////////////////////////////////////////////////////////////////////
// ARM-only prologue runtime helpers

void setArgInActRec(ActRec* ar, int argNum, uint64_t datum, DataType t) {
  TypedValue* tv =
    (TypedValue*)(uintptr_t(ar) - (argNum+1) * sizeof(TypedValue));
  tv->m_data.num = datum;
  tv->m_type = t;
}

const StaticString s_call("__call");
const StaticString s_callStatic("__callStatic");

int shuffleArgsForMagicCall(ActRec* ar) {
  if (!ar->hasInvName()) {
    return 0;
  }
  const Func* f UNUSED = ar->m_func;
  f->validate();
  assert(f->name()->isame(s_call.get())
         || f->name()->isame(s_callStatic.get()));
  assert(f->numParams() == 2);
  assert(ar->hasInvName());
  StringData* invName = ar->getInvName();
  assert(invName);
  ar->setVarEnv(nullptr);
  int nargs = ar->numArgs();

  // We need to make an array containing all the arguments passed by the
  // caller and put it where the second argument is
  PackedArrayInit aInit(nargs);
  for (int i = 0; i < nargs; ++i) {
    auto const tv = reinterpret_cast<TypedValue*>(
      uintptr_t(ar) - (i+1) * sizeof(TypedValue)
    );
    aInit.append(tvAsCVarRef(tv));
    tvRefcountedDecRef(tv);
  }

  // Put invName in the slot for first argument
  setArgInActRec(ar, 0, uint64_t(invName), KindOfString);
  // Put argArray in the slot for second argument
  auto const argArray = aInit.toArray().detach();
  setArgInActRec(ar, 1, uint64_t(argArray), KindOfArray);
  // Fix up ActRec's numArgs
  ar->initNumArgs(2);
  return 1;
}

//////////////////////////////////////////////////////////////////////

} // anonymous namespace

//////////////////////////////////////////////////////////////////////

TCA emitCallArrayPrologue(Func* func, DVFuncletsVec& dvs) {
  auto& mainCode = mcg->code.main();
  auto& stubsCode = mcg->code.stubs();
  vixl::MacroAssembler a { mainCode };
  vixl::MacroAssembler astubs { stubsCode };
  TCA start = mainCode.frontier();
  a.   Ldr   (rAsm.W(), rVmFp[AROFF(m_numArgsAndGenCtorFlags)]);
  for (auto i = 0; i < dvs.size(); ++i) {
    a. Cmp   (rAsm.W(), dvs[i].first);
    emitBindJcc(mainCode, stubsCode, CC_LE, SrcKey(func, dvs[i].second));
  }
  emitBindJmp(mainCode, stubsCode, SrcKey(func, func->base()));
  return start;
}

SrcKey emitFuncPrologue(CodeBlock& mainCode, CodeBlock& stubsCode,
                        Func* func, bool funcIsMagic, int nPassed,
                        TCA& start, TCA& aStart) {
  vixl::MacroAssembler a { mainCode };
  vixl::Label veryStart;
  a.bind(&veryStart);

  if (!func->isMagic()) {
    start = aStart = emitFuncGuard(a, func);
  }

  if (RuntimeOption::EvalJitTransCounters) {
    emitTransCounterInc(a);
  }

  if (!func->isMagic()) {
    emitStoreRetIntoActRec(a);

    auto const needStackCheck =
      !(func->attrs() & AttrPhpLeafFn) ||
      func->maxStackCells() >= kStackCheckLeafPadding;
    if (needStackCheck) {
      emitStackCheck(cellsToBytes(func->maxStackCells()), func->base());
    }
  }

  SrcKey skFuncBody = emitPrologueWork(func, nPassed);

  if (func->isMagic()) {
    TCA magicStart = emitFuncGuard(a, func);
    emitStoreRetIntoActRec(a);
    // emit rb

    emitStackCheck(cellsToBytes(func->maxStackCells()), func->base());
    assert(func->numParams() == 2);
    // Special __call prologue
    a.   Mov   (argReg(0), rStashedAR);
    auto fixupAddr = emitCall(a, CppCall(shuffleArgsForMagicCall));
    if (RuntimeOption::HHProfServerEnabled) {
      mcg->fixupMap().recordFixup(
        fixupAddr,
        Fixup(skFuncBody.offset() - func->base(), func->numSlotsInFrame())
      );
    }

    if (nPassed == 2) {
      a.  B    (&veryStart);
    } else {
      // "compare and branch if zero"
      a.  Cbz  (rReturnReg, &veryStart);
      nPassed = 2;
      emitRegGetsRegPlusImm(a, rVmSp, rStashedAR, -cellsToBytes(nPassed));
      emitPrologueWork(func, nPassed);
    }
    start = magicStart;
  }

  return skFuncBody;
}

}}}
