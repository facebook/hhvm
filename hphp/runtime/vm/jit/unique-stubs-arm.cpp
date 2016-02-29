/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/unique-stubs-arm.h"

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/align-arm.h"
#include "hphp/runtime/vm/jit/unwind-arm.h"

#include "hphp/vixl/a64/constants-a64.h"
#include "hphp/vixl/a64/macro-assembler-a64.h"
#include "hphp/vixl/a64/decoder-a64.h"
#include "hphp/vixl/a64/disasm-a64.h"
#include "hphp/vixl/a64/instructions-a64.h"
#include "hphp/vixl/a64/simulator-a64.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/arg-group.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/code-gen-internal.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/unwind-itanium.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"

#include <folly/ScopeGuard.h>

#include <iostream>

namespace HPHP { namespace jit {

TRACE_SET_MOD(ustubs);

///////////////////////////////////////////////////////////////////////////////

namespace arm {

///////////////////////////////////////////////////////////////////////////////

static void alignJmpTarget(CodeBlock& cb) {
  align(cb, Alignment::JmpTarget, AlignContext::Dead);
}

///////////////////////////////////////////////////////////////////////////////

TCA emitFunctionEnterHelper(CodeBlock& cb, UniqueStubs& us) {
  // FIXME: This function is near identical to its X64 equivalent, except for
  // the pushing and popping of the link register, and using lea to manipulate
  // the stack pointer.

  alignJmpTarget(cb);

  auto const start = vwrap(cb, [&] (Vout& v) {
    auto const ar = v.makeReg();

    v << copy{rvmfp(), ar};

    // Set up the call frame for the stub.  We can't skip this like we do in
    // other stubs because we need the return IP for this frame in the %rbp
    // chain, in order to find the proper fixup for the VMRegAnchor in the
    // intercept handler.

    v << push{PhysReg{rLinkReg}};
    v << push{rvmfp()};
    v << copy{rsp(), rvmfp()};

    // When we call the event hook, it might tell us to skip the callee
    // (because of fb_intercept).  If that happens, we need to return to the
    // caller, but the handler will have already popped the callee's frame.
    // So, we need to save these values for later.
    v << pushm{ar[AROFF(m_savedRip)]};
    v << pushm{ar[AROFF(m_sfp)]};

    v << copy2{ar, v.cns(EventHook::NormalFunc), rarg(0), rarg(1)};

    bool (*hook)(const ActRec*, int) = &EventHook::onFunctionCall;
    v << call{TCA(hook), rarg(0)|rarg(1)};
  });

  us.functionEnterHelperReturn = vwrap2(cb, [&] (Vout& v, Vout& vcold) {
    auto const sf = v.makeReg();
    v << testb{rret(), rret(), sf};

    unlikelyIfThen(v, vcold, CC_Z, sf, [&] (Vout& v) {
      auto const saved_rip = v.makeReg();

      // The event hook has already cleaned up the stack and popped the
      // callee's frame, so we're ready to continue from the original call
      // site.  We just need to grab the fp/rip of the original frame that we
      // saved earlier, and sync rvmsp().
      v << pop{rvmfp()};
      v << pop{saved_rip};

      // Drop our call frame.
      v << lea{rsp()[16], rsp()};

      // Sync vmsp and return to the caller.  This unbalances the return stack
      // buffer, but if we're intercepting, we probably don't care.
      v << load{rvmtl()[rds::kVmspOff], rvmsp()};
      v << jmpr{saved_rip};
    });

    // Skip past the stuff we saved for the intercept case.
    v << lea{rsp()[16], rsp()};

    // Execute a leave, returning us to the callee's prologue.
    v << pop{rvmfp()};
    v << ret{}; // pop{lr};ret{lr}
  });

  return start;
}

TCA emitFreeLocalsHelpers(CodeBlock& cb, UniqueStubs& us) {
  auto const release = vwrap(cb, [&](Vout& v) {
    v << ud2{};
  });
  us.freeManyLocalsHelper = vwrap(cb, [&](Vout& v) {
    v << ud2{};
  });
  return release;
}

///////////////////////////////////////////////////////////////////////////////

extern "C" void enterTCExit();

TCA emitCallToExit(CodeBlock& cb) {
  vixl::MacroAssembler a { cb };
  vixl::Label data;
  auto const start = cb.frontier();

  // Emulating the return to enterTCExit. Pop off the x29,x30 pair
  a.Add(vixl::sp, vixl::sp, 16);

  // Jump to enterTCExit
  a.Ldr(rAsm, &data);
  a.Br(rAsm);
  a.bind(&data);
  a.dc64(reinterpret_cast<int64_t>(enterTCExit));
  return start;
}

TCA emitEndCatchHelper(CodeBlock& cb, UniqueStubs& us) {
  auto const start = vwrap(cb, [&] (Vout& v) {
    v << ret{};
  });
  return start;
}

///////////////////////////////////////////////////////////////////////////////

void enterTCImpl(TCA start, ActRec* stashedAR) {
  CALLEE_SAVED_BARRIER();
  auto& regs = vmRegsUnsafe();
  jit::enterTCHelper(regs.stack.top(), regs.fp, start,
                     vmFirstAR(), rds::tl_base, stashedAR);
  CALLEE_SAVED_BARRIER();
}

///////////////////////////////////////////////////////////////////////////////

}}}
