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

#include "hphp/vixl/a64/macro-assembler-a64.h"

#include "folly/Optional.h"

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/jump-smash.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/translator-x64.h"

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

  tx64->setJmpTransID(toSmash);

  TCA sr = (req == JIT::REQ_BIND_JMP
            ? emitEphemeralServiceReq(tx64->stubsCode, tx64->getFreeStub(), req,
                                      toSmash, dest.offset())
            : emitServiceReq(tx64->stubsCode, req, toSmash, dest.offset()));

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
  a.     Str   (rVmFp, rGContextReg[offsetof(VMExecutionContext, m_fp)]);
  a.     Str   (rVmSp, rGContextReg[offsetof(VMExecutionContext, m_stack) +
                                    Stack::topOfStackOffset()]);

  if (persist) {
    a.   Mov   (rAsm, 0);
  } else {
    a.   Mov   (rAsm, reinterpret_cast<intptr_t>(start));
  }
  a.     Mov   (argReg(0), req);

  // The x64 equivalent loads to rax. I knew this was a trap.
  if (flags & SRFlags::JmpInsteadOfRet) {
    a.   Ldr   (rAsm, MemOperand(sp, 8, PostIndex));
    a.   Br    (rAsm);
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

}}}
