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

#include "hphp/runtime/vm/jit/unique-stubs-x64.h"

#include "hphp/runtime/base/header-kind.h"
#include "hphp/runtime/base/rds-header.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/event-hook.h"
#include "hphp/runtime/vm/vm-regs.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/align-x64.h"
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

#include "hphp/util/asm-x64.h"
#include "hphp/util/data-block.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(ustubs);

///////////////////////////////////////////////////////////////////////////////

namespace x64 {

///////////////////////////////////////////////////////////////////////////////

static void alignJmpTarget(CodeBlock& cb) {
  align(cb, nullptr, Alignment::JmpTarget, AlignContext::Dead);
}

///////////////////////////////////////////////////////////////////////////////

TCA emitFunctionEnterHelper(CodeBlock& cb, DataBlock& data, UniqueStubs& us) {
  alignJmpTarget(cb);

  auto const start = vwrap2(cb, data, [&] (Vout& v, Vout& vcold) {
    auto const ar = v.makeReg();

    v << copy{rvmfp(), ar};

    // Fully set up the call frame for the stub.  We can't skip this like we do
    // in other stubs because we need the return IP for this frame in the %rbp
    // chain, in order to find the proper fixup for the VMRegAnchor in the
    // intercept handler.
    v << stublogue{true};
    v << copy{rsp(), rvmfp()};

    // When we call the event hook, it might tell us to skip the callee
    // (because of fb_intercept).  If that happens, we need to return to the
    // caller, but the handler will have already popped the callee's frame.
    // So, we need to save these values for later.
    v << pushm{ar[AROFF(m_savedRip)]};
    v << pushm{ar[AROFF(m_sfp)]};

    v << copy2{ar, v.cns(EventHook::NormalFunc), rarg(0), rarg(1)};

    bool (*hook)(const ActRec*, int) = &EventHook::onFunctionCall;
    v << call{TCA(hook), arg_regs(0), &us.functionEnterHelperReturn};

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

      // Drop our call frame; the stublogue{} instruction guarantees that this
      // is exactly 16 bytes.
      v << lea{rsp()[16], rsp()};

      // Sync vmsp and the return regs.
      v << load{rvmtl()[rds::kVmspOff], rvmsp()};
      v << load{rvmsp()[TVOFF(m_data)], rret_data()};
      v << load{rvmsp()[TVOFF(m_type)], rret_type()};

      // Return to the caller.  This unbalances the return stack buffer, but if
      // we're intercepting, we probably don't care.
      v << jmpr{saved_rip};
    });

    // Skip past the stuff we saved for the intercept case.
    v << lea{rsp()[16], rsp()};

    // Restore rvmfp() and return to the callee's func prologue.
    v << stubret{RegSet(), true};
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
 * The `live' registers must be preserved across any native calls (and
 * generally left untouched).
 */
static TCA emitDecRefHelper(CodeBlock& cb, DataBlock& data, CGMeta& fixups,
                            PhysReg tv, PhysReg type, RegSet live) {
  return vwrap(cb, data, fixups, [&] (Vout& v) {
    // We use the first argument register for the TV data because we might pass
    // it to the native release call.  It's not live when we enter the helper.
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
        v << ret{live};
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
    v << ret{live};
  });
}

TCA emitFreeLocalsHelpers(CodeBlock& cb, DataBlock& data, UniqueStubs& us) {
  // The address of the first local is passed in the second argument register.
  // We use the third and fourth as scratch registers.
  auto const local = rarg(1);
  auto const last = rarg(2);
  auto const type = rarg(3);
  CGMeta fixups;

  // This stub is very hot; keep it cache-aligned.
  align(cb, &fixups, Alignment::CacheLine, AlignContext::Dead);
  auto const release =
    emitDecRefHelper(cb, data, fixups, local, type, local | last);

  auto const decref_local = [&] (Vout& v) {
    auto const sf = v.makeReg();

    // We can't do a byte load here---we have to sign-extend since we use
    // `type' as a 32-bit array index to the destructor table.
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

  us.freeManyLocalsHelper = vwrap(cb, data, fixups, [&] (Vout& v) {
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
  });

  for (auto i = kNumFreeLocalsHelpers - 1; i >= 0; --i) {
    us.freeLocalsHelpers[i] = vwrap(cb, data, [&] (Vout& v) {
      decref_local(v);
      if (i != 0) next_local(v);
    });
  }

  // All the stub entrypoints share the same ret.
  vwrap(cb, data, fixups, [] (Vout& v) { v << ret{}; });

  // This stub is hot, so make sure to keep it small.
  // Alas, we have more work to do in this under Windows,
  // so we can't be this small :(
#ifndef _WIN32
  always_assert(Stats::enabled() ||
                (cb.frontier() - release <= 4 * x64::cache_line_size()));
#endif

  fixups.process(nullptr);
  return release;
}

///////////////////////////////////////////////////////////////////////////////

void assert_tc_saved_rip(void* sp) {
  auto const saved_rip = *reinterpret_cast<uint8_t**>(sp);
  auto const exittc = mcg->ustubs().enterTCExit;

  DecodedInstruction di(saved_rip);
  auto const jmp_target = [&] { return saved_rip + di.size() + di.offset(); };

  // We should either be returning to enterTCExit, or to a jmp to enterTCExit.
  always_assert(saved_rip == exittc || (di.isJmp() && jmp_target() == exittc));
}

TCA emitCallToExit(CodeBlock& cb, DataBlock& data, const UniqueStubs& us) {
  X64Assembler a { cb };

  // Emit a byte of padding. This is a kind of hacky way to avoid
  // hitting an assert in recordGdbStub when we call it with stub - 1
  // as the start address.
  a.emitNop(1);

  auto const start = a.frontier();
  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    always_assert(rarg(0) != rret(0) &&
                  rarg(0) != rret(1));
    a.movq(rsp(), rarg(0));

    // We need to spill the return registers around the assert call.
    a.push(rret(0));
    a.push(rret(1));
    a.call(TCA(assert_tc_saved_rip));
    a.pop(rret(1));
    a.pop(rret(0));
  }

  // Emulate a ret to enterTCExit without actually doing one to avoid
  // unbalancing the return stack buffer. The call from enterTCHelper() that
  // got us into the TC was popped off the RSB by the ret that got us to this
  // stub.
  a.addq(8, rsp());
  if (a.jmpDeltaFits(us.enterTCExit)) {
    a.jmp(us.enterTCExit);
  } else {
    // can't do a near jmp and a rip-relative load/jmp would require threading
    // through extra state to allocate a literal. use an indirect jump through
    // a register
    a.emitImmReg(us.enterTCExit, reg::rax);
    a.jmp(reg::rax);
  }

  // On a backtrace, gdb tries to locate the calling frame at address
  // returnRIP-1. However, for the first VM frame, there is no code at
  // returnRIP-1, since the AR was set up manually. For this frame,
  // record the tracelet address as starting from this callToExit-1,
  // so gdb does not barf.
  return start;
}

TCA emitEndCatchHelper(CodeBlock& cb, DataBlock& data, UniqueStubs& us) {
  auto const udrspo = rvmtl()[unwinderDebuggerReturnSPOff()];

  auto const debuggerReturn = vwrap(cb, data, [&] (Vout& v) {
    v << load{udrspo, rvmsp()};
    v << storeqi{0, udrspo};
  });
  svcreq::emit_persistent(cb, data, folly::none, REQ_POST_DEBUGGER_RET);

  auto const resumeCPPUnwind = vwrap(cb, data, [&] (Vout& v) {
    static_assert(sizeof(tl_regState) == 1,
                  "The following store must match the size of tl_regState.");
    auto const regstate = emitTLSAddr(v, tls_datum(tl_regState));
    v << storebi{static_cast<int32_t>(VMRegState::CLEAN), regstate};

    v << load{rvmtl()[unwinderExnOff()], rarg(0)};
    v << call{TCA(_Unwind_Resume), arg_regs(1), &us.endCatchHelperPast};
    v << ud2{};
  });

  alignJmpTarget(cb);

  return vwrap(cb, data, [&] (Vout& v) {
    auto const done1 = v.makeBlock();
    auto const sf1 = v.makeReg();

    v << cmpqim{0, udrspo, sf1};
    v << jcci{CC_NE, sf1, done1, debuggerReturn};
    v = done1;

    // Normal end catch situation: call back to tc_unwind_resume, which returns
    // the catch trace (or null) in %rax, and the new vmfp in %rdx.
    v << copy{rvmfp(), rarg(0)};
    v << call{TCA(tc_unwind_resume)};
    v << copy{reg::rdx, rvmfp()};

    auto const done2 = v.makeBlock();
    auto const sf2 = v.makeReg();

    v << testq{reg::rax, reg::rax, sf2};
    v << jcci{CC_Z, sf2, done2, resumeCPPUnwind};
    v = done2;

    v << jmpr{reg::rax};
  });
}

///////////////////////////////////////////////////////////////////////////////

void enterTCImpl(TCA start, ActRec* stashedAR) {
  static_assert(rvmfp() == reg::rbp &&
                rvmsp() == reg::rbx &&
                rvmtl() == reg::r12 &&
                rret_data() == reg::rax &&
                rret_type() == reg::rdx,
                "enterTCHelper needs to be modified to use the correct ABI");

  // We have to force C++ to spill anything that might be in a callee-saved
  // register (aside from %rbp), since enterTCHelper does not save them.
  CALLEE_SAVED_BARRIER();
  auto& regs = vmRegsUnsafe();
  mcg->ustubs().enterTCHelper(regs.stack.top(), regs.fp, start,
                              vmFirstAR(), rds::tl_base, stashedAR);
  CALLEE_SAVED_BARRIER();
}

///////////////////////////////////////////////////////////////////////////////

}}}
