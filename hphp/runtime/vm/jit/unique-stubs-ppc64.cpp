/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
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
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/service-requests.h"
#include "hphp/runtime/vm/jit/smashable-instr-ppc64.h"
#include "hphp/runtime/vm/jit/tc.h"
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

/*
 * Helper for the freeLocalsHelpers which does the actual work of decrementing
 * a value's refcount or releasing it.
 *
 * This helper is reached via call from the various freeLocalHelpers.
 * It expects `dataVal' to be a Value, and `typeVal' to be the the
 * associated DataType (with refcounted type) (though it may be
 * static, and we will do nothing in that case).
 *
 * The `live' registers must be preserved across any native calls (and
 * generally left untouched).
 */
static TCA emitDecRefHelper(CodeBlock& cb, DataBlock& data, CGMeta& fixups,
                            PhysReg dataVal, PhysReg typeVal, RegSet live) {
  return vwrap(cb, data, fixups, [&] (Vout& v) {
    auto destroy = [&](Vout& v) {
      PhysRegSaver prs{v, live};

      auto const dword_size = sizeof(int64_t);

      // saving return value on the stack, but keeping it 16-byte aligned
      v << mflr{rfuncln()};
      v << lea {rsp()[-2 * dword_size], rsp()};
      v << store{rfuncln(), rsp()[0]};

      // The refcount is exactly 1; release the value.
      // Avoid 'this' pointer overwriting by reserving it as an argument.
      assertx(dataVal == rarg(0));
      v << callm{lookupDestructor(v, typeVal, true), arg_regs(1)};

      // Between where r1 is now and the saved RIP of the call into the
      // freeLocalsHelpers stub, we have all the live regs we pushed, plus the
      // stack size reserved for the LR saved right above and the LR offset in
      // the frame.
      v << syncpoint{makeIndirectFixup(prs.dwordsPushed())};
      // fallthru

      // restore the return value from the stack
      v << load{rsp()[0], rfuncln()};
      v << lea {rsp()[2 * dword_size], rsp()};
      v << mtlr{rfuncln()};
    };

    auto const sf = emitCmpRefCount(v, OneReference, dataVal);

    if (one_bit_refcount) {
      ifThen(v, CC_E, sf, destroy);
    } else {
      ifThen(v, CC_NL, sf, [&] (Vout& v) {
        // The refcount is positive, so the value is refcounted.  We need to
        // either decref or release.
        ifThen(v, CC_NE, sf, [&] (Vout& v) {
          // The refcount is greater than 1; decref it.
          emitDecRefCount(v, dataVal);
          v << ret{live};
        });

        destroy(v);
      });
    }

    // Either we did a decref, or the value was static.
    v << ret{live};
  });
}

TCA emitFreeLocalsHelpers(CodeBlock& cb, DataBlock& data, UniqueStubs& us) {
  // The address of the first local's type is passed in the second
  // argument register. The address of the first local's value is
  // passed in the third argument register. We use the first to store
  // the loaded value (so its already in the right place when calling
  // the release function).  We use the fourth as a scratch register
  // for loading the type and the fifth as a scratch register for
  // storing the end pointer.
  auto const dataVal = rarg(0);
  auto const typePtr = rarg(1);
  auto const dataPtr = rarg(2);
  auto const typeVal = rarg(3);
  auto const end = rarg(4);
  CGMeta fixups;

  // This stub is very hot; keep it cache-aligned.
  align(cb, &fixups, Alignment::CacheLine, AlignContext::Dead);
  auto const release = emitDecRefHelper(
    cb, data, fixups, dataVal, typeVal,
    dataPtr | typePtr | end
  );

  auto const decref_local = [&] (Vout& v, Vptr d, Vptr t) {
    auto const sf = v.makeReg();

    // We can't do a byte load here---we have to sign-extend since we use
    // `type' as a 64-bit array index to the destructor table.
    v << loadsbq{t, typeVal};
    auto const cc = emitIsTVTypeRefCounted(v, sf, typeVal);

    ifThen(v, cc, sf, [&] (Vout& v) {
      auto const dword_size = sizeof(int64_t);

      // saving return value on the stack, but keeping it 16-byte aligned
      v << mflr{rfuncln()};
      v << lea {rsp()[-2 * dword_size], rsp()};
      v << store{rfuncln(), rsp()[0]};

      v << call{release, dataVal | typeVal};

      // restore the return value from the stack
      v << load{rsp()[0], rfuncln()};
      v << lea {rsp()[2 * dword_size], rsp()};
      v << mtlr{rfuncln()};
    });
  };

  alignJmpTarget(cb);

  us.freeManyLocalsHelper = vwrap(cb, data, fixups, [&] (Vout& v) {
    // We always unroll the final `kNumFreeLocalsHelpers' decrefs, so only loop
    // until we hit that point.
    v << lea{ptrToLocalType(rvmfp(), kNumFreeLocalsHelpers - 1), end};
    doWhile(v, CC_NZ, {}, [&](const VregList&, const VregList&) {
      decref_local(v, *dataPtr, *typePtr);
      auto const sf = v.makeReg();
      prevLocal(v, typePtr, dataPtr, typePtr, dataPtr);
      v << cmpq{typePtr, end, sf};
      return sf;
    });
  });

  for (auto i = kNumFreeLocalsHelpers - 1; i >= 0; --i) {
    us.freeLocalsHelpers[i] = vwrap(cb, data, [&] (Vout& v) {
      decref_local(v, ptrToLocalData(rvmfp(), i), ptrToLocalType(rvmfp(), i));
      if (i != 0) v << fallthru{};
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
  auto const exittc = tc::ustubs().enterTCExit;

  ppc64_asm::DecodedInstruction const di(branch_instr);
  if (di.isJmp()) {
    ppc64_asm::DecodedInstruction const di_target(branch_block);
    always_assert(di.isJmp() && (di_target.farBranchTarget() == exittc));
  } else {
    always_assert(saved_lr == exittc);
  }
}

TCA emitCallToExit(CodeBlock& cb, DataBlock& data, const UniqueStubs& /*us*/) {
  ppc64_asm::Assembler a { cb };
  auto const start = a.frontier();

  if (RuntimeOption::EvalHHIRGenerateAsserts) {
    vwrap(cb, data, [&] (Vout& v) {
      // Not doing it directly as rret(0) == rarg(0) on ppc64
      Vreg ret_addr = v.makeReg();

      // exittc address pushed on resumetc.
      v << copy{rsp(), ret_addr};

      // We need to spill the return registers around the assert call.
      v << push{rret(0)};
      v << push{rret(1)};

      v << copy{ret_addr, rarg(0)};
      v << call{TCA(assert_tc_saved_rip), RegSet(rarg(0))};

      v << pop{rret(1)};
      v << pop{rret(0)};
    });
  }

  // Discard the exittc address pushed on resumetc for balancing the
  // stack next.
  a.addi(rsp(), rsp(), 8);

  // Reinitialize r1 for the external code found after enterTCExit's stubret
  a.addi(rsfp(), rsp(), 8);

  // Restore the rvmfp when leaving the VM, which must be the same of rsfp.
  a.mr(rvmfp(), rsfp());

  // Emulate a ret to enterTCExit without actually doing one to avoid
  // unbalancing the return stack buffer.
  a.branchAuto(TCA(tc::ustubs().enterTCExit));
  return start;
}

///////////////////////////////////////////////////////////////////////////////

}}}
