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

#include "hphp/runtime/vm/jit/unique-stubs-arm.h"

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/stats.h"
#include "hphp/runtime/vm/bytecode.h"
#include "hphp/runtime/vm/jit/abi-arm.h"
#include "hphp/runtime/vm/jit/align-arm.h"
#include "hphp/runtime/vm/jit/code-gen-cf.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/code-gen-tls.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
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
    // Set up frame linkage to avoid an indirect fixup.
    v << pushp{rlr(), rfp()};
    v << copy{rsp(), rfp()};

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
        v << popp{rfp(), rlr()};
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
      v << syncpoint{makeIndirectFixup(prs.dwordsPushed())};
      // fallthru
    });

    // Either we did a decref, or the value was static.
    // Pop FP/LR and return
    v << popp{rfp(), rlr()};
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
  TCA freeLocalsHelpers[kNumFreeLocalsHelpers];
  TCA freeManyLocalsHelper;

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
      v << call{release, arg_regs(3)};
    });
  };

  auto const next_local = [&] (Vout& v) {
    v << addqi{static_cast<int>(sizeof(TypedValue)),
               local, local, v.makeReg()};
  };

  alignJmpTarget(cb);

  freeManyLocalsHelper = vwrap(cb, data, [&] (Vout& v) {
    // We always unroll the final `kNumFreeLocalsHelpers' decrefs, so only loop
    // until we hit that point.
    v << lea{rvmfp()[localOffset(kNumFreeLocalsHelpers - 1)], last};

    // Set up frame linkage to avoid an indirect fixup.
    v << copy{rsp(), rfp()};

    doWhile(v, CC_NZ, {}, [&](const VregList& /*in*/, const VregList& /*out*/) {
      auto const sf = v.makeReg();

      decref_local(v);
      next_local(v);
      v << cmpq{ local, last, sf };
      return sf;
    });
  });

  for (auto i = kNumFreeLocalsHelpers - 1; i >= 0; --i) {
    freeLocalsHelpers[i] = vwrap(cb, data, [&] (Vout& v) {
      decref_local(v);
      if (i != 0) next_local(v);
    });
  }

  // All the stub entrypoints share the same ret.
  vwrap(cb, data, fixups, [] (Vout& v) {
    v << popp{rfp(), rlr()};
    v << ret{};
  });

  // Create a table of branches
  us.freeManyLocalsHelper = vwrap(cb, data, [&] (Vout& v) {
    v << pushp{rlr(), rfp()};

    // rvmfp() is needed by the freeManyLocalsHelper stub above, so frame
    // linkage setup is deferred until after its use in freeManyLocalsHelper.
    v << jmpi{freeManyLocalsHelper};
  });
  for (auto i = kNumFreeLocalsHelpers - 1; i >= 0; --i) {
    us.freeLocalsHelpers[i] = vwrap(cb, data, [&] (Vout& v) {
      // We set up frame linkage to avoid an indirect fixup.
      v << pushp{rlr(), rfp()};
      v << copy{rsp(), rfp()};
      v << jmpi{freeLocalsHelpers[i]};
    });
  }

  // FIXME: This stub is hot, so make sure to keep it small.
#if 0
  always_assert(Stats::enabled() ||
                (cb.frontier() - release <= 4 * x64::cache_line_size()));
#endif

  fixups.process(nullptr);
  return release;
}

///////////////////////////////////////////////////////////////////////////////

TCA emitCallToExit(CodeBlock& cb, DataBlock& /*data*/, const UniqueStubs& us) {
  vixl::MacroAssembler a { cb };
  vixl::Label target_data;
  auto const start = cb.frontier();

  // Jump to enterTCExit
  a.Ldr(rAsm, &target_data);
  a.Br(rAsm);
  a.bind(&target_data);
  a.dc64(us.enterTCExit);

  __builtin___clear_cache(reinterpret_cast<char*>(start),
                          reinterpret_cast<char*>(cb.frontier()));
  return start;
}

///////////////////////////////////////////////////////////////////////////////

}}}
