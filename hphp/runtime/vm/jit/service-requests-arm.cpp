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

//////////////////////////////////////////////////////////////////////

size_t reusableStubSize() {
  // There are 4 instructions after the argument-shuffling, and they're all
  // single instructions (i.e. not macros). There are up to 4 instructions per
  // argument (it may take up to 4 instructions to move a 64-bit immediate into
  // a register).
  return 4 * vixl::kInstructionSize +
    (4 * maxArgReg()) * vixl::kInstructionSize;
}

TCA emitServiceReqWork(CodeBlock& cb,
                       TCA start,
                       SRFlags flags,
                       folly::Optional<FPInvOffset> spOff,
                       ServiceRequest req,
                       const ServiceReqArgVec& argv) {
  MacroAssembler a { cb };

  const bool persist = flags & SRFlags::Persist;

  folly::Optional<CodeCursor> maybeCc = folly::none;
  if (start != cb.frontier()) {
    maybeCc.emplace(cb, start);
  }

  const auto kMaxStubSpace = reusableStubSize();

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
  a.     Ret   ();
  a.     Brk   (0);

  if (!persist) {
    assertx(cb.frontier() - start <= kMaxStubSpace);
    while (cb.frontier() - start < kMaxStubSpace) {
      a. Nop   ();
    }
  }

  return start;
}

void emitBindJ(CodeBlock& cb, CodeBlock& frozen, ConditionCode cc,
               SrcKey dest) {
  TCA toSmash = cb.frontier();
  if (cb.base() == frozen.base()) {
    // This is just to reserve space. We'll overwrite with the real dest later.
    mcg->backEnd().emitSmashableJump(cb, toSmash, cc);
  }

  mcg->setJmpTransID(toSmash);

  TCA sr = emitEphemeralServiceReq(
    frozen,
    mcg->getFreeStub(frozen, &mcg->cgFixups()),
    folly::none,
    REQ_BIND_JMP,
    toSmash,
    dest.toAtomicInt(),
    TransFlags{}.packed
  );

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

}}}
