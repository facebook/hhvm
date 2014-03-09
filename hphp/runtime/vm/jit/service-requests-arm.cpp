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

#include "hphp/vixl/a64/macro-assembler-a64.h"

#include "folly/Optional.h"

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-arm.h"
#include "hphp/runtime/vm/jit/jump-smash.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/mc-generator.h"

namespace HPHP { namespace JIT { namespace ARM {

using namespace vixl;

namespace {

void emitBindJ(CodeBlock& cb, CodeBlock& stubs, SrcKey dest,
               JIT::ConditionCode cc, ServiceRequest req) {

  TCA toSmash = cb.frontier();
  if (cb.base() == stubs.base()) {
    // This is just to reserve space. We'll overwrite with the real dest later.
    emitSmashableJump(cb, toSmash, cc);
  }

  mcg->setJmpTransID(toSmash);

  TCA sr = (req == JIT::REQ_BIND_JMP
            ? emitEphemeralServiceReq(mcg->code.stubs(), mcg->getFreeStub(),
                                      req, toSmash, dest.offset())
            : emitServiceReq(mcg->code.stubs(), req, toSmash, dest.offset()));

  MacroAssembler a { cb };
  if (cb.base() == stubs.base()) {
    UndoMarker um {cb};
    cb.setFrontier(toSmash);
    emitSmashableJump(cb, sr, cc);
    um.undo();
  } else {
    emitSmashableJump(cb, sr, cc);
  }
}

} // anonymous namespace

//////////////////////////////////////////////////////////////////////

TCA emitServiceReqWork(CodeBlock& cb, TCA start, bool persist, SRFlags flags,
                       ServiceRequest req, const ServiceReqArgVec& argv) {
  MacroAssembler a { cb };

  folly::Optional<CodeCursor> maybeCc = folly::none;
  if (start != cb.frontier()) {
    maybeCc.emplace(cb, start);
  }

  // There are 6 instructions after the argument-shuffling, and they're all
  // single instructions (i.e. not macros). There are up to 4 instructions per
  // argument (it may take up to 4 instructions to move a 64-bit immediate into
  // a register).
  constexpr auto kMaxStubSpace = 6 * vixl::kInstructionSize +
    (4 * maxArgReg()) * vixl::kInstructionSize;

  for (auto i = 0; i < argv.size(); ++i) {
    auto reg = serviceReqArgReg(i);
    auto const& arg = argv[i];
    switch (arg.m_kind) {
      case ServiceReqArgInfo::Immediate:
        a.   Mov  (reg, arg.m_imm);
        break;
      case ServiceReqArgInfo::CondCode:
        not_implemented();
        break;
      default: not_reached();
    }
  }

  // Save VM regs
  a.     Str   (rVmFp, rGContextReg[offsetof(ExecutionContext, m_fp)]);
  a.     Str   (rVmSp, rGContextReg[offsetof(ExecutionContext, m_stack) +
                                    Stack::topOfStackOffset()]);

  if (persist) {
    a.   Mov   (rAsm, 0);
  } else {
    a.   Mov   (rAsm, reinterpret_cast<intptr_t>(start));
  }
  a.     Mov   (argReg(0), req);

  a.     Ldr   (rLinkReg, MemOperand(sp, 16, PostIndex));
  if (flags & SRFlags::JmpInsteadOfRet) {
    a.   Br    (rLinkReg);
  } else {
    a.   Ret   ();
  }
  a.     Brk   (0);

  if (!persist) {
    assert(cb.frontier() - start <= kMaxStubSpace);
    while (cb.frontier() - start < kMaxStubSpace) {
      a. Nop   ();
    }
  }

  return start;
}

void emitBindJmp(CodeBlock& cb, CodeBlock& stubs, SrcKey dest) {
  emitBindJ(cb, stubs, dest, JIT::CC_None, REQ_BIND_JMP);
}

void emitBindJcc(CodeBlock& cb, CodeBlock& stubs, JIT::ConditionCode cc,
                 SrcKey dest) {
  emitBindJ(cb, stubs, dest, cc, REQ_BIND_JCC);
}

void emitBindSideExit(CodeBlock& cb, CodeBlock& stubs, SrcKey dest,
                      JIT::ConditionCode cc) {
  emitBindJ(cb, stubs, dest, cc, REQ_BIND_SIDE_EXIT);
}

//////////////////////////////////////////////////////////////////////

int32_t emitNativeImpl(CodeBlock& cb, const Func* func) {

  BuiltinFunction builtinFuncPtr = func->builtinFuncPtr();

  MacroAssembler a { cb };
  a.  Mov  (argReg(0), rVmFp);
  if (mcg->fixupMap().eagerRecord(func)) {
    a.Mov  (rAsm, func->getEntry());
    a.Str  (rAsm, rGContextReg[offsetof(ExecutionContext, m_pc)]);
    a.Str  (rVmFp, rGContextReg[offsetof(ExecutionContext, m_fp)]);
    a.Str  (rVmSp, rGContextReg[offsetof(ExecutionContext, m_stack) +
                                Stack::topOfStackOffset()]);
  }
  auto syncPoint = emitCall(a, CppCall(builtinFuncPtr));

  Offset pcOffset = 0;
  Offset stackOff = func->numLocals();
  mcg->fixupMap().recordSyncPoint(syncPoint, pcOffset, stackOff);

  int nLocalCells = func->numSlotsInFrame();
  a.  Ldr  (rVmFp, rVmFp[AROFF(m_savedRbp)]);

  return sizeof(ActRec) + cellsToBytes(nLocalCells - 1);
}

int32_t emitBindCall(CodeBlock& mainCode, CodeBlock& stubsCode,
                  SrcKey srcKey, const Func* funcd, int numArgs) {
  if (isNativeImplCall(funcd, numArgs)) {
    MacroAssembler a { mainCode };

    // We need to store the return address into the AR, but we don't know it
    // yet. Write out a mov instruction (two instructions, under the hood) with
    // a placeholder address, and we'll overwrite it later.
    auto toOverwrite = mainCode.frontier();
    a.    Mov  (rAsm, toOverwrite);
    a.    Str  (rAsm, rVmSp[cellsToBytes(numArgs) + AROFF(m_savedRip)]);

    emitRegGetsRegPlusImm(a, rVmFp, rVmSp, cellsToBytes(numArgs));
    emitCheckSurpriseFlagsEnter(mainCode, stubsCode, true, mcg->fixupMap(),
                                Fixup(0, numArgs));
    // rVmSp is already correctly adjusted, because there's no locals other than
    // the arguments passed.

    auto retval = emitNativeImpl(mainCode, funcd);
    {
      auto realRetAddr = mainCode.frontier();
      // Go back and overwrite with the proper return address.
      CodeCursor cc(mainCode, toOverwrite);
      a.  Mov  (rAsm, realRetAddr);
    }

    return retval;
  }

  MacroAssembler a { mainCode };
  emitRegGetsRegPlusImm(a, rStashedAR, rVmSp, cellsToBytes(numArgs));

  ReqBindCall* req = mcg->globalData().alloc<ReqBindCall>();

  auto toSmash = mainCode.frontier();
  emitSmashableCall(mainCode, stubsCode.frontier());

  MacroAssembler astubs { stubsCode };
  astubs.  Mov  (serviceReqArgReg(1), rStashedAR);
  // Put return address into pre-live ActRec, and restore the saved one.
  emitStoreRetIntoActRec(astubs);

  emitServiceReq(stubsCode, REQ_BIND_CALL, req);

  req->m_toSmash = toSmash;
  req->m_nArgs = numArgs;
  req->m_sourceInstr = srcKey;
  req->m_isImmutable = (bool)funcd;

  return 0;
}


}}}
