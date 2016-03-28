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
#include "hphp/runtime/vm/jit/code-gen-tls.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/unwind-itanium.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"

#include <folly/ScopeGuard.h>

#include <iostream>

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(ustubs);

extern "C" void enterTCHelper(Cell* vm_sp,
                              ActRec* vm_fp,
                              TCA start,
                              ActRec* firstAR,
                              void* targetCacheBase,
                              ActRec* stashedAR);

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

///////////////////////////////////////////////////////////////////////////////

/*
 * Helper for the freeLocalsHelpers which does the actual work of decrementing
 * a value's refcount or releasing it.
 *
 * This helper is reached via call from the various freeLocalHelpers.  It
 * expects `tv' to be the address of a TypedValue with refcounted type `type'
 * (though it may be static, and we will do nothing in that case).
 *
 * The `saved' register should be a callee-saved GP register that the helper
 * can use to preserve `tv' across native calls.
 */
static TCA emitDecRefHelper(CodeBlock& cb, PhysReg tv, PhysReg type,
                            RegSet live) {
  return vwrap(cb, [&] (Vout& v) {
    v << stublogue{};

    // We use the first argument register for the TV data because we may pass
    // it to the release routine.  It's not live when we enter the helper.
    auto const data = rarg(0);
    v << load{tv[TVOFF(m_data)], data};

    auto const sf = v.makeReg();
    v << cmplim{1, data[FAST_REFCOUNT_OFFSET], sf};

    ifThen(v, CC_NL, sf, [&] (Vout& v) {
      // The refcount is positive, so the value is refcounted.  We need to
      // either decref or release.
      ifThen(v, CC_NE, sf, [&] (Vout& v) {
        // The refcount is greater than 1; decref it.
        v << declm{data[FAST_REFCOUNT_OFFSET], v.makeReg()};
        v << stubret{};
      });

      // Note that the stack is aligned since we called to this helper from an
      // stack-unaligned stub.
      PhysRegSaver prs{v, live};

      // The refcount is exactly 1; release the value.
      // Avoid 'this' pointer overwriting by reserving it as an argument.
      v << callm{lookupDestructor(v, type), arg_regs(1)};

      // Between where %rsp is now and the saved RIP of the call into the
      // freeLocalsHelpers stub, we have all the live regs we pushed, plus the
      // saved RIP of the call from the stub to this helper.
      v << syncpoint{makeIndirectFixup(prs.dwordsPushed() + 1)};
      // fallthru
    });

    // Either we did a decref, or the value was static.
    v << stubret{};
  });
}

TCA emitFreeLocalsHelpers(CodeBlock& cb, UniqueStubs& us) {
  // The address of the first local is passed in the second argument register.
  // We use the third and fourth as scratch registers.
  auto const local = rarg(1);
  auto const last = rarg(2);
  auto const type = rarg(3);

  // This stub is very hot; keep it cache-aligned.
  align(cb, Alignment::CacheLine, AlignContext::Dead);
  auto const release = emitDecRefHelper(cb, local, type, local | last);

  auto const decref_local = [&] (Vout& v) {
    auto const sf = v.makeReg();

    // We can't use emitLoadTVType() here because it does a byte load, and we
    // need to sign-extend since we use `type' as a 32-bit array index to the
    // destructor table.
    v << loadzbl{local[TVOFF(m_type)], type};
    emitCmpTVType(v, sf, KindOfRefCountThreshold, type);

    ifThen(v, CC_G, sf, [&] (Vout& v) {
      v << call{release, arg_regs(3)};
    });
  };

  auto const next_local = [&] (Vout& v) {
    v << addqi{static_cast<int>(sizeof(TypedValue)),
               local, local, v.makeReg()};
  };

  alignJmpTarget(cb);

  us.freeManyLocalsHelper = vwrap(cb, [&] (Vout& v) {
    v << stublogue{true};
    // We always unroll the final `kNumFreeLocalsHelpers' decrefs, so only loop
    // until we hit that point.
    v << lea{rvmfp()[localOffset(kNumFreeLocalsHelpers - 1)], last};

    doWhile(v, CC_NZ, {},
      [&] (const VregList& in, const VregList& out) {
        auto const sf = v.makeReg();

        decref_local(v);
        next_local(v);
        v << cmpq{local, last, sf};
        return sf;
      }
    );
    v << stubret{RegSet(), true};
  });

  for (auto i = kNumFreeLocalsHelpers - 1; i >= 0; --i) {
    us.freeLocalsHelpers[i] = vwrap(cb, [&] (Vout& v) {
      v << stublogue{true};
      decref_local(v);
      if (i != 0) next_local(v);
      v << stubret{RegSet(), true};
    });
  }

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
  auto const udrspo = rvmtl()[unwinderDebuggerReturnSPOff()];

  auto const debuggerReturn = vwrap(cb, [&] (Vout& v) {
    v << load{udrspo, rvmsp()};
    v << storeqi{0, udrspo};
  });
  svcreq::emit_persistent(cb, folly::none, REQ_POST_DEBUGGER_RET);

  auto const resumeCPPUnwind = vwrap(cb, [] (Vout& v) {
    static_assert(sizeof(tl_regState) == 1,
                  "The following store must match the size of tl_regState.");
    auto const regstate = emitTLSAddr(v, tls_datum(tl_regState));
    v << storebi{static_cast<int32_t>(VMRegState::CLEAN), regstate};

    v << load{rvmtl()[unwinderExnOff()], rarg(0)};
    v << call{TCA(_Unwind_Resume), arg_regs(1)};
  });
  us.endCatchHelperPast = cb.frontier();
  vwrap(cb, [] (Vout& v) { v << ud2{}; });

  alignJmpTarget(cb);

  return vwrap(cb, [&] (Vout& v) {
    auto const done1 = v.makeBlock();
    auto const sf1 = v.makeReg();

    v << cmpqim{0, udrspo, sf1};
    v << jcci{CC_NE, sf1, done1, debuggerReturn};
    v = done1;

    // Normal end catch situation: call back to tc_unwind_resume, which returns
    // the catch trace (or null) in x0, and the new vmfp in x1.
    v << copy{rvmfp(), rarg(0)};
    v << call{TCA(tc_unwind_resume)};
    v << copy{rarg(1), rvmfp()};

    auto const done2 = v.makeBlock();
    auto const sf2 = v.makeReg();

    v << testq{rarg(0), rarg(0), sf2};
    v << jcci{CC_Z, sf2, done2, resumeCPPUnwind};
    v = done2;

    // We need to do a syncForLLVMCatch(), but vmfp is already in rdx.
    v << jmpr{rarg(0)};
  });
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
