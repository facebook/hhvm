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

#include "hphp/runtime/vm/jit/unique-stubs-ppc64.h"

#include "hphp/runtime/base/header-kind.h"
#include "hphp/runtime/base/rds-header.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/event-hook.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi-ppc64.h"
#include "hphp/runtime/vm/jit/align-ppc64.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/code-gen-tls.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/smashable-instr-ppc64.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/unwind-itanium.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"

#include "hphp/ppc64-asm/asm-ppc64.h"
#include "hphp/ppc64-asm/decoded-instr-ppc64.h"
#include "hphp/util/data-block.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(ustubs);

///////////////////////////////////////////////////////////////////////////////

namespace ppc64 {

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
    // in other stubs because we need the return IP for this frame in the vmfp
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
      // is exactly ppc64_asm::min_frame_size bytes.
      v << lea{rsp()[ppc64_asm::min_frame_size], rsp()};

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

      PhysRegSaver prs{v, live};

      auto const dword_size = sizeof(int64_t);

      // saving return value on the stack, but keeping it 16-byte aligned
      v << mflr{rfuncln()};
      v << lea {rsp()[-2 * dword_size], rsp()};
      v << store{rfuncln(), rsp()[0]};

      // The refcount is exactly 1; release the value.
      // Avoid 'this' pointer overwriting by reserving it as an argument.
      v << callm{lookupDestructor(v, type), arg_regs(1)};

      // Between where r1 is now and the saved RIP of the call into the
      // freeLocalsHelpers stub, we have all the live regs we pushed, plus the
      // stack size reserved for the LR saved right above and the LR offset in
      // the frame.
      Fixup fixup = makeIndirectFixup(prs.dwordsPushed() + 2 +
          AROFF(m_savedRip) / dword_size);
      v << syncpoint{fixup};
      // fallthru

      // restore the return value from the stack
      v << load{rsp()[0], rfuncln()};
      v << lea {rsp()[2 * dword_size], rsp()};
      v << mtlr{rfuncln()};
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
      auto const dword_size = sizeof(int64_t);

      // saving return value on the stack, but keeping it 16-byte aligned
      v << mflr{rfuncln()};
      v << lea {rsp()[-2 * dword_size], rsp()};
      v << store{rfuncln(), rsp()[0]};

      v << call{release, arg_regs(3)};

      // restore the return value from the stack
      v << load{rsp()[0], rfuncln()};
      v << lea {rsp()[2 * dword_size], rsp()};
      v << mtlr{rfuncln()};
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
#if 0
  // TODO(gut): Currently this assert fails.
  // Take a closer look when looking at performance
  always_assert(Stats::enabled() ||
                (cb.frontier() - release <= 4 * cache_line_size()));
#endif

  fixups.process(nullptr);
  return release;
}

///////////////////////////////////////////////////////////////////////////////

void assert_tc_saved_rip(void* saved_lr_pointer) {
  // saved on enterTCHelper
  auto const saved_lr = *reinterpret_cast<uint8_t**>(saved_lr_pointer);
  auto const branch_block = saved_lr; // next instruction after resumetc's callr
  auto const jccLen = smashableJccLen() - ppc64_asm::instr_size_in_bytes;
  auto const branch_instr = branch_block + jccLen;
  auto const exittc = mcg->ustubs().enterTCExit;

  ppc64_asm::DecodedInstruction di(branch_instr);
  if (di.isJmp()) {
    auto const jmp_target = TCA(ppc64_asm::Assembler::getLi64(branch_block));
    always_assert(di.isJmp() && jmp_target == exittc);
  } else {
    always_assert(saved_lr == exittc);
  }
}

TCA emitCallToExit(CodeBlock& cb, DataBlock& data, const UniqueStubs& us) {
  ppc64_asm::Assembler a { cb };
  auto const start = a.frontier();

  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    vwrap(cb, data, [&] (Vout& v) {
      // Not doing it directly as rret(0) == rarg(0) on ppc64
      Vreg ret_addr = v.makeReg();
      v << lea{rsp()[ppc64_asm::exittc_position_on_frame], ret_addr};

      // We need to spill the return registers around the assert call.
      v << push{rret(0)};
      v << push{rret(1)};

      v << copy{ret_addr, rarg(0)};
      v << call{TCA(assert_tc_saved_rip), RegSet(rarg(0))};

      v << pop{rret(1)};
      v << pop{rret(0)};
    });
  }

  // Reinitialize r1 for the external code found after enterTCExit's stubret
  a.mr(ppc64_asm::reg::r1, rsp());

  // r31 should have the same value as caller's r1. Loading it soon on stubret.
  // (this corrupts the backchain, but it's not relevant as this frame will be
  // destroyed soon)
  a.std(ppc64_asm::reg::r1, rsp()[0]);

  // Simply go to enterTCExit
  a.branchAuto(TCA(mcg->ustubs().enterTCExit));
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
    // the catch trace (or null) in r3, and the new vmfp in r4.
    v << copy{rvmfp(), rarg(0)};
    v << call{TCA(tc_unwind_resume)};
    v << copy{ppc64_asm::reg::r4, rvmfp()};

    auto const done2 = v.makeBlock();
    auto const sf2 = v.makeReg();

    v << testq{ppc64_asm::reg::r3, ppc64_asm::reg::r3, sf2};
    v << jcci{CC_Z, sf2, done2, resumeCPPUnwind};
    v = done2;

    v << jmpr{ppc64_asm::reg::r3};
  });
}

///////////////////////////////////////////////////////////////////////////////

void enterTCImpl(TCA start, ActRec* stashedAR) {
  // We have to force C++ to spill anything that might be in a callee-saved
  // register (aside from vmfp), since enterTCHelper does not save them.
  CALLEE_SAVED_BARRIER();
  auto& regs = vmRegsUnsafe();
  mcg->ustubs().enterTCHelper(regs.stack.top(), regs.fp, start,
                              vmFirstAR(), rds::tl_base, stashedAR);
  CALLEE_SAVED_BARRIER();
}

///////////////////////////////////////////////////////////////////////////////

}}}
