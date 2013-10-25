#include "hphp/runtime/vm/jit/func-prologues-arm.h"

#include "hphp/vixl/a64/macro-assembler-a64.h"

#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-arm.h"
#include "hphp/runtime/vm/jit/jump-smash.h"
#include "hphp/runtime/vm/jit/service-requests-arm.h"
#include "hphp/runtime/vm/jit/translator-x64.h"

namespace HPHP { namespace JIT { namespace ARM {

using Transl::TCA;

//////////////////////////////////////////////////////////////////////

namespace {

void emitStackCheck(int funcDepth, Offset pc) {
  vixl::MacroAssembler a { tx64->mainCode };
  funcDepth += cellsToBytes(kStackCheckPadding);

  uint64_t stackMask = cellsToBytes(RuntimeOption::EvalVMStackElms) - 1;
  a.   And  (rAsm, rVmSp, stackMask);
  a.   Sub  (rAsm, rAsm, funcDepth + Stack::sSurprisePageSize);
  // This doesn't need to be smashable, but it is a long jump from mainCode to
  // stubs, so it can't be direct.
  emitSmashableJump(tx64->mainCode, tx64->uniqueStubs.stackOverflowHelper,
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
  a.   dc64  (tx64->uniqueStubs.funcPrologueRedispatch);
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
  vixl::MacroAssembler a { tx64->mainCode };

  if (tx64->mode() == TransProflogue) {
    not_implemented();
  }

  auto dvInitializer = InvalidAbsoluteOffset;
  auto const numParams = func->numParams();
  auto const& paramInfo = func->params();

  // Resolve cases where the wrong number of args was passed.
  if (nPassed > numParams) {
    void (*helper)(ActRec*) = Transl::trimExtraArgs;
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
    not_implemented();
  }

  auto numUninitLocals = func->numLocals() - numLocals;
  assert(numUninitLocals >= 0);
  if (numUninitLocals > 0 && !func->isGenerator()) {
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
        using Transl::Location;
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
  if (func->isGenerator()) {
    frameCells = 1;
  } else {
    emitRegGetsRegPlusImm(a, rVmSp, rVmFp, -cellsToBytes(frameCells));
  }

  Fixup fixup(funcBody.offset() - func->base(), frameCells);

  // Emit warnings for missing arguments
  if (!func->info() && !(func->attrs() & AttrNative)) {
    for (auto i = nPassed; i < numParams; ++i) {
      if (paramInfo[i].funcletOff() == InvalidAbsoluteOffset) {
        a.  Mov  (argReg(0), func->name()->data());
        a.  Mov  (argReg(1), numParams);
        a.  Mov  (argReg(2), i);
        emitCall(a, CppCall(Transl::raiseMissingArgument));
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
    a.   Ldr   (rAsm, rStashedAR[AROFF(m_func)]);
    a.   Ldr   (rAsm, rAsm[Func::prologueTableOff() + sizeof(TCA)*entry]);
    a.   Br    (rAsm);
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
  vixl::MacroAssembler a { mainCode };
  vixl::MacroAssembler astubs { stubsCode };
  TCA start = mainCode.frontier();
  a.   Ldr   (rAsm.W(), rVmFp[AROFF(m_numArgsAndCtorFlag)]);
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
    emitCall(a, CppCall(Transl::shuffleArgsForMagicCall));
    if (memory_profiling) {
      tx64->fixupMap().recordFixup(
        a.frontier(),
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
