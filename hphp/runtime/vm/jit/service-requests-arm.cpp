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

#include <folly/Optional.h>

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/code-gen-helpers-arm.h"
#include "hphp/runtime/vm/jit/back-end.h"
#include "hphp/runtime/vm/jit/service-requests-inline.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/vasm-print.h"

namespace HPHP { namespace jit { namespace arm {

using namespace vixl;

namespace {

void emitBindJ(CodeBlock& cb, CodeBlock& frozen, SrcKey dest,
               ConditionCode cc, ServiceRequest req, TransFlags trflags) {

  TCA toSmash = cb.frontier();
  if (cb.base() == frozen.base()) {
    // This is just to reserve space. We'll overwrite with the real dest later.
    mcg->backEnd().emitSmashableJump(cb, toSmash, cc);
  }

  mcg->setJmpTransID(toSmash);

  TCA sr = emitEphemeralServiceReq(frozen,
                                   mcg->getFreeStub(frozen,
                                                    &mcg->cgFixups()),
                                   req, toSmash,
                                   dest.toAtomicInt(),
                                   trflags.packed);

  MacroAssembler a { cb };
  if (cb.base() == frozen.base()) {
    UndoMarker um {cb};
    cb.setFrontier(toSmash);
    mcg->backEnd().emitSmashableJump(cb, sr, cc);
    um.undo();
  } else {
    mcg->backEnd().emitSmashableJump(cb, sr, cc);
  }
}

} // anonymous namespace

//////////////////////////////////////////////////////////////////////

TCA emitServiceReqWork(CodeBlock& cb, TCA start, SRFlags flags,
                       ServiceRequest req, const ServiceReqArgVec& argv) {
  MacroAssembler a { cb };

  const bool persist = flags & SRFlags::Persist;

  folly::Optional<CodeCursor> maybeCc = folly::none;
  if (start != cb.frontier()) {
    maybeCc.emplace(cb, start);
  }

  // There are 4 instructions after the argument-shuffling, and they're all
  // single instructions (i.e. not macros). There are up to 4 instructions per
  // argument (it may take up to 4 instructions to move a 64-bit immediate into
  // a register).
  constexpr auto kMaxStubSpace = 4 * vixl::kInstructionSize +
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

void emitBindJmp(CodeBlock& cb, CodeBlock& frozen, SrcKey dest) {
  emitBindJ(cb, frozen, dest, jit::CC_None, REQ_BIND_JMP, TransFlags{});
}

void emitBindJcc(CodeBlock& cb, CodeBlock& frozen, jit::ConditionCode cc,
                 SrcKey dest) {
  emitBindJ(cb, frozen, dest, cc, REQ_BIND_JCC, TransFlags{});
}

void emitBindSideExit(CodeBlock& cb, CodeBlock& frozen, SrcKey dest,
                      jit::ConditionCode cc) {
  emitBindJ(cb, frozen, dest, cc, REQ_BIND_SIDE_EXIT, TransFlags{});
}

//////////////////////////////////////////////////////////////////////

void emitCallNativeImpl(Vout& v, Vout& vc, SrcKey srcKey,
                        const Func* func, int numArgs) {
  assert(isNativeImplCall(func, numArgs));

  // We need to store the return address into the AR, but we don't know it
  // yet. Use ldpoint, and point{} below, to get the address.
  PhysReg sp{rVmSp}, fp{rVmFp}, rds{rVmTl};
  auto ret_point = v.makePoint();
  auto ret_addr = v.makeReg();
  v << ldpoint{ret_point, ret_addr};
  v << store{ret_addr, sp[cellsToBytes(numArgs) + AROFF(m_savedRip)]};

  v << lea{sp[cellsToBytes(numArgs)], fp};
  emitCheckSurpriseFlagsEnter(v, vc, Fixup(0, numArgs));
  // rVmSp is already correctly adjusted, because there's no locals other than
  // the arguments passed.

  BuiltinFunction builtinFuncPtr = func->builtinFuncPtr();
  v << copy{fp, PhysReg{argReg(0)}};
  if (mcg->fixupMap().eagerRecord(func)) {
    v << store{v.cns(func->getEntry()), rds[RDS::kVmpcOff]};
    v << store{fp, rds[RDS::kVmfpOff]};
    v << store{sp, rds[RDS::kVmspOff]};
  }
  auto syncPoint = emitCall(v, CppCall::direct(builtinFuncPtr), argSet(1));

  Offset pcOffset = 0;
  Offset stackOff = func->numLocals();
  v << hcsync{Fixup{pcOffset, stackOff}, syncPoint};

  int nLocalCells = func->numSlotsInFrame();
  v << load{fp[AROFF(m_sfp)], fp};
  v << point{ret_point};

  int adjust = sizeof(ActRec) + cellsToBytes(nLocalCells - 1);
  if (adjust != 0) {
    v << addqi{adjust, sp, sp, v.makeReg()};
  }
}

void emitBindCall(Vout& v, CodeBlock& frozen, const Func* func, int numArgs) {
  assert(!isNativeImplCall(func, numArgs));

  auto& us = mcg->tx().uniqueStubs;
  auto addr = func ? us.immutableBindCallStub : us.bindCallStub;

  // emit the mainline code
  PhysReg new_fp{rStashedAR}, vmsp{arm::rVmSp};
  v << lea{vmsp[cellsToBytes(numArgs)], new_fp};
  v << bindcall{addr, RegSet(new_fp)};
}

}}}
