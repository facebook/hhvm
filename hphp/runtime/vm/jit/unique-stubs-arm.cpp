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

#include "hphp/runtime/vm/jit/unique-stubs-arm.h"

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/align-arm.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/code-gen-tls.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/unwind-arm.h"
#include "hphp/runtime/vm/jit/unwind-itanium.h"

#include "hphp/vixl/a64/macro-assembler-a64.h"
#include "hphp/vixl/a64/simulator-a64.h"

#include <folly/ScopeGuard.h>

#include <iostream>

namespace HPHP { namespace jit {

TRACE_SET_MOD(ustubs);

///////////////////////////////////////////////////////////////////////////////

namespace arm {

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

    // Set up the call frame for the stub.  We can't skip this like we do in
    // other stubs because we need the return IP for this frame in the %rbp
    // chain, in order to find the proper fixup for the VMRegAnchor in the
    // intercept handler.

    v << push{rlink()};
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

      // Drop our call frame; Pop off rlink() and rvmfp() pushed above
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
    v << pop{rvmfp()};
    v << pop{rlink()};
    v << ret{};
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
    // Save FP/LR
    v << pushp{rfp(), rlink()};

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
        // Pop FP/LR and return
        v << popp{rfp(), rlink()};
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
    // Pop FP/LR and return
    v << popp{rfp(), rlink()};
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

    // We can't use emitLoadTVType() here because it does a byte load, and we
    // need to sign-extend since we use `type' as a 32-bit array index to the
    // destructor table.
    v << loadzbl{local[TVOFF(m_type)], type};
    emitCmpTVType(v, sf, KindOfRefCountThreshold, type);

    ifThen(v, CC_G, sf, [&] (Vout& v) {
      // Save and restore caller's FP/LR
      v << pushp{rfp(), rlink()};
      v << call{release, arg_regs(3)};
      v << popp{rfp(), rlink()};
    });
  };

  auto const next_local = [&] (Vout& v) {
    v << addqi{static_cast<int>(sizeof(TypedValue)),
               local, local, v.makeReg()};
  };

  alignJmpTarget(cb);

  us.freeManyLocalsHelper = vwrap(cb, data, [&] (Vout& v) {
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

  // FIXME: This stub is hot, so make sure to keep it small.
#if 0
  always_assert(Stats::enabled() ||
                (cb.frontier() - release <= 4 * x64::cache_line_size()));
#endif

  fixups.process(nullptr);
  return release;
}

///////////////////////////////////////////////////////////////////////////////

TCA emitCallToExit(CodeBlock& cb, DataBlock& data, const UniqueStubs& us) {
  vixl::MacroAssembler a { cb };
  vixl::Label target_data;
  auto const start = cb.frontier();

  // Emulating the return to enterTCExit. Pop off the FP, LR pair
  a.Add(vixl::sp, vixl::sp, 16);

  // Jump to enterTCExit
  a.Ldr(rAsm, &target_data);
  a.Br(rAsm);
  a.bind(&target_data);
  a.dc64(reinterpret_cast<int64_t>(us.enterTCExit));
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
    // the catch trace (or null) in x0, and the new vmfp in x1.
    v << copy{rvmfp(), rarg(0)};
    v << call{TCA(tc_unwind_resume)};
    v << copy{rarg(1), rvmfp()};

    auto const done2 = v.makeBlock();
    auto const sf2 = v.makeReg();

    v << testq{rarg(0), rarg(0), sf2};
    v << jcci{CC_Z, sf2, done2, resumeCPPUnwind};
    v = done2;

    v << jmpr{rarg(0)};
  });
}

///////////////////////////////////////////////////////////////////////////////

#if defined(__aarch64__)
void enterTCImpl(TCA start, ActRec* stashedAR) {
  CALLEE_SAVED_BARRIER();
  auto& regs = vmRegsUnsafe();
  mcg->ustubs().enterTCHelper(regs.stack.top(), regs.fp, start,
                              vmFirstAR(), rds::tl_base, stashedAR);
  CALLEE_SAVED_BARRIER();
}
#else

/*
 * A partial equivalent of enterTCHelper, used to set up the ARM simulator.
 */
static uintptr_t setupSimRegsAndStack(vixl::Simulator& sim,
                                      ActRec* saved_rStashedAr) {
  auto& vmRegs = vmRegsUnsafe();
  sim.   set_xreg(x2a(rvmfp()).code(), vmRegs.fp);
  sim.   set_xreg(x2a(rvmsp()).code(), vmRegs.stack.top());
  sim.   set_xreg(x2a(rvmtl()).code(), rds::tl_base);

  // Leave space for register spilling and MInstrState.
  assertx(sim.is_on_stack(reinterpret_cast<void*>(sim.sp())));

  auto spOnEntry = sim.sp();

  // Push the link register onto the stack. The link register is technically
  // caller-saved; what this means in practice is that non-leaf functions push
  // it at the very beginning and pop it just before returning (as opposed to
  // just saving it around calls).
  sim.   set_sp(sim.sp() - 16);
  *reinterpret_cast<uint64_t*>(sim.sp()) = sim.lr();

  return spOnEntry;
}

/*
 * Following is the implementation of enterTCImpl for simulated backend
 */
void enterTCImpl(TCA start, ActRec* stashedAR) {
  // This is a pseudo-copy of the logic in enterTCHelper: it sets up the
  // simulator's registers and stack, runs the translation, and gets the
  // necessary information out of the registers when it's done.
  vixl::PrintDisassembler disasm(std::cout);
  vixl::Decoder decoder;
  if (getenv("ARM_DISASM")) {
    decoder.AppendVisitor(&disasm);
  }
  vixl::Simulator sim(&decoder, std::cout);
  SCOPE_EXIT {
    Stats::inc(Stats::vixl_SimulatedInstr, sim.instr_count());
    Stats::inc(Stats::vixl_SimulatedLoad, sim.load_count());
    Stats::inc(Stats::vixl_SimulatedStore, sim.store_count());
  };

  sim.set_exception_hook(arm::simulatorExceptionHook);

  g_context->m_activeSims.push_back(&sim);
  SCOPE_EXIT { g_context->m_activeSims.pop_back(); };

  DEBUG_ONLY auto spOnEntry = setupSimRegsAndStack(sim, stashedAR);

  // The handshake is different when entering at a func prologue. The code
  // we're jumping to expects to find a return address in x30, and a saved
  // return address on the stack.
  if (stashedAR) {
    // Put the call's return address in the link register.
    sim.set_lr(stashedAR->m_savedRip);
  }

  std::cout.flush();
  sim.RunFrom(vixl::Instruction::Cast(start));
  std::cout.flush();

  assertx(sim.sp() == spOnEntry);

  vmRegsUnsafe().fp = (ActRec*)sim.xreg(x2a(rvmfp()).code());
  vmRegsUnsafe().stack.top() = (Cell*)sim.xreg(x2a(rvmsp()).code());
}
#endif

///////////////////////////////////////////////////////////////////////////////

}}}
