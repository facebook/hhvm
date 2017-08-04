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

#include "hphp/runtime/vm/jit/unique-stubs-x64.h"

#include "hphp/runtime/base/header-kind.h"
#include "hphp/runtime/base/runtime-option.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/bytecode.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/align-x64.h"
#include "hphp/runtime/vm/jit/cg-meta.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/fixup.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/phys-reg-saver.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"

#include "hphp/util/asm-x64.h"
#include "hphp/util/data-block.h"
#include "hphp/util/trace.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(ustubs);

///////////////////////////////////////////////////////////////////////////////

namespace x64 {

///////////////////////////////////////////////////////////////////////////////

static void alignJmpTarget(CodeBlock& cb) {
  align(cb, nullptr, Alignment::JmpTarget, AlignContext::Dead);
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
static TCA emitDecRefHelper(CodeBlock& cb, DataBlock& data,
                            PhysReg tv, PhysReg type, RegSet live) {
  CGMeta meta;
  auto addr = vwrap(cb, data, meta, [&] (Vout& v) {
    // We use the first argument register for the TV data because we might pass
    // it to the native release call.  It's not live when we enter the helper.
    auto const data = rarg(0);
    v << load{tv[TVOFF(m_data)], data};

    auto const sf = v.makeReg();
    v << cmplim{1, data[FAST_REFCOUNT_OFFSET], sf};

    auto skipref = v.makeBlock();
    auto destroy = v.makeBlock();
    auto chkref  = v.makeBlock();
    auto decref  = v.makeBlock();

    // We can't quite get the layout we want from two nested ifThens,
    // because we want the else case from the first to jmp to the middle
    // of the then case of the second (we want to share the ret).
    v << jcc{CC_L, sf, {chkref, skipref}, StringTag{}};
    v = chkref;
    v << jcc{CC_NE, sf, {destroy, decref}, StringTag{}};
    v = decref;
    v << declm{data[FAST_REFCOUNT_OFFSET], v.makeReg()};
    v << jmp{skipref};
    v = skipref;
    v << ret{live};
    v = destroy;

    {
      // Note that the stack is aligned since we called to this helper from an
      // stack-unaligned stub.
      PhysRegSaver prs{v, live};

      // The refcount is exactly 1; release the value.
      // Avoid 'this' pointer overwriting by reserving it as an argument.
      v << callm{lookupDestructor(v, type, true), arg_regs(1)};

      // Between where %rsp is now and the saved RIP of the call into the
      // freeLocalsHelpers stub, we have all the live regs we pushed, plus the
      // saved RIP of the call from the stub to this helper.
      v << syncpoint{makeIndirectFixup(prs.dwordsPushed())};
    }
    v << ret{live};
  }, CodeKind::CrossTrace, true);

  meta.process(nullptr);
  return addr;
}

TCA emitFreeLocalsHelpers(CodeBlock& cb, DataBlock& data, UniqueStubs& us) {
  // The address of the first local is passed in the second argument register.
  // We use the third and fourth as scratch registers.
  auto const local = rarg(1);
  auto const last = rarg(2);
  auto const type = rarg(3);

  // This stub is very hot; keep it cache-aligned.
  align(cb, nullptr, Alignment::CacheLine, AlignContext::Dead);

  auto const start = cb.frontier();
  // We want the release function to come last; we enter the slide at
  // several different points, but always execute through to the end
  // of the slide - so its better to put the end of the slide close to
  // the release helper. Since we don't know exactly where the release
  // helper will be, use a fake address that we can recognize and
  // fixup after the fact.
  auto const releaseFake = start - 1;

  auto const decref_local = [&] (Vout& v) {
    auto const sf = v.makeReg();

    // We can't do a byte load here---we have to sign-extend since we use
    // `type' as a 32-bit array index to the destructor table.
    v << loadzbl{local[TVOFF(m_type)], type};
    emitCmpTVType(v, sf, KindOfRefCountThreshold, type);

    ifThen(v, CC_G, sf, [&] (Vout& v) {
      v << call{releaseFake, local | type};
    });
  };

  auto const next_local = [&] (Vout& v) {
    v << addqi{static_cast<int>(sizeof(TypedValue)),
               local, local, v.makeReg()};
  };

  us.freeManyLocalsHelper = vwrap(cb, data, [&] (Vout& v) {
    // We always unroll the final `kNumFreeLocalsHelpers' decrefs, so only loop
    // until we hit that point.
    v << lea{rvmfp()[localOffset(kNumFreeLocalsHelpers - 1)], last};

    doWhile(v, CC_NZ, {}, [&](const VregList& /*in*/, const VregList& /*out*/) {
      auto const sf = v.makeReg();

      decref_local(v);
      next_local(v);
      v << cmpq{ local, last, sf };
      return sf;
    });
  }, true);

  for (auto i = kNumFreeLocalsHelpers - 1; i >= 0; --i) {
    us.freeLocalsHelpers[i] = vwrap(cb, data, [&] (Vout& v) {
      decref_local(v);
      if (i == 0) {
        // The helpers all fall through to each other. Only the last
        // one needs a ret.
        v << ret{};
      } else {
        next_local(v);
      }
    }, true);
  }

  auto const release = emitDecRefHelper(cb, data, local, type, local | last);
  // Now we know where release is, we can patch the calls to
  // releaseFake and point them to the correct address.
  for (auto addr = start; addr < release; ) {
    x64::DecodedInstruction di(addr, addr);
    if (di.hasPicOffset() && di.picAddress() == releaseFake) {
      always_assert(di.isCall());
      di.setPicAddress(release);
    }
    addr += di.size();
  }

  return start;
}

///////////////////////////////////////////////////////////////////////////////

void assert_tc_saved_rip(void* sp) {
  auto const saved_rip = *reinterpret_cast<uint8_t**>(sp);
  auto const exittc = tc::ustubs().enterTCExit;

  DecodedInstruction di(saved_rip);
  auto const jmp_target = [&] { return saved_rip + di.size() + di.offset(); };

  // We should either be returning to enterTCExit, or to a jmp to enterTCExit.
  always_assert(saved_rip == exittc || (di.isJmp() && jmp_target() == exittc));
}

TCA emitCallToExit(CodeBlock& cb, DataBlock& /*data*/, const UniqueStubs& us) {
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

///////////////////////////////////////////////////////////////////////////////

}}}
