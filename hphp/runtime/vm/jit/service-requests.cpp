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

#include "hphp/runtime/vm/jit/service-requests.h"

#include "hphp/runtime/base/arch.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"

#include "hphp/util/data-block.h"
#include "hphp/util/trace.h"

#include "hphp/vixl/a64/macro-assembler-a64.h"

#include <folly/Optional.h>

namespace HPHP { namespace jit { namespace svcreq {

///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(servicereq);

///////////////////////////////////////////////////////////////////////////////

namespace detail {

///////////////////////////////////////////////////////////////////////////////

/*
 * Service request stub emitter.
 *
 * Emit a service request stub of type `sr' at `start' in `cb'.
 */
void emit_svcreq(CodeBlock& cb,
                 TCA start,
                 bool persist,
                 folly::Optional<FPInvOffset> spOff,
                 ServiceRequest sr,
                 const ArgVec& argv) {
  FTRACE(2, "svcreq @{} {}(", start, to_name(sr));

  auto const is_reused = start != cb.frontier();

  CodeBlock stub;
  stub.init(start, stub_size(), "svcreq_stub");

  { Vauto vasm{stub};
    auto& v = vasm.main();

    // If we have an spOff, materialize rvmsp() so that handleSRHelper() can do
    // a VM reg sync.  (When we don't have an spOff, the caller of the service
    // request was responsible for making sure rvmsp already contained the top
    // of the stack.)
    if (spOff) {
      v << lea{rvmfp()[-cellsToBytes(spOff->offset)], rvmsp()};
    }

    auto live_out = leave_trace_regs();

    assert(argv.size() <= kMaxArgs);

    // Pick up CondCode arguments first---vasm may optimize immediate loads
    // into operations which clobber status flags.
    for (auto i = 0; i < argv.size(); ++i) {
      auto const& arg = argv[i];
      if (arg.kind != Arg::Kind::CondCode) continue;

      FTRACE(2, "c({}), ", cc_names[arg.cc]);
      v << setcc{arg.cc, r_svcreq_sf(), rbyte(r_svcreq_arg(i))};
    }

    for (auto i = 0; i < argv.size(); ++i) {
      auto const& arg = argv[i];
      auto const r = r_svcreq_arg(i);

      switch (arg.kind) {
        case Arg::Kind::Immed:
          FTRACE(2, "{}, ", arg.imm);
          v << copy{v.cns(arg.imm), r};
          break;
        case Arg::Kind::Address:
          FTRACE(2, "{}(%rip), ", arg.imm);
          v << leap{reg::rip[arg.imm], r};
          break;
        case Arg::Kind::CondCode:
          break;
      }
      live_out |= r;
    }
    FTRACE(2, ") : stub@");

    if (persist) {
      FTRACE(2, "<none>");
      v << copy{v.cns(0), r_svcreq_stub()};
    } else {
      FTRACE(2, "{}", stub.base());
      v << leap{reg::rip[int64_t(stub.base())], r_svcreq_stub()};
    }
    v << copy{v.cns(sr), r_svcreq_req()};

    live_out |= r_svcreq_stub();
    live_out |= r_svcreq_req();

    v << jmpi{TCA(handleSRHelper), live_out};

    // We pad ephemeral stubs unconditionally.  This is required for
    // correctness by the x64 code relocator.
    vasm.unit().padding = !persist;
  }

  if (!is_reused) cb.skip(stub.used());
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

TCA emit_bindjmp_stub(CodeBlock& cb, FPInvOffset spOff, TCA jmp,
                      SrcKey target, TransFlags trflags) {
  return emit_ephemeral(
    cb,
    mcg->getFreeStub(cb, &mcg->cgFixups()),
    target.resumed() ? folly::none : folly::make_optional(spOff),
    REQ_BIND_JMP,
    jmp,
    target.toAtomicInt(),
    trflags.packed
  );
}

TCA emit_bindjcc1st_stub(CodeBlock& cb, FPInvOffset spOff, TCA jcc,
                         SrcKey taken, SrcKey next, ConditionCode cc) {
  always_assert_flog(taken.resumed() == next.resumed(),
                     "bind_jcc_1st was confused about resumables");
  return emit_ephemeral(
    cb,
    mcg->getFreeStub(cb, &mcg->cgFixups()),
    taken.resumed() ? folly::none : folly::make_optional(spOff),
    REQ_BIND_JCC_FIRST,
    jcc,
    taken.toAtomicInt(),
    next.toAtomicInt(),
    cc
  );
}

TCA emit_bindaddr_stub(CodeBlock& cb, FPInvOffset spOff, TCA* addr,
                       SrcKey target, TransFlags trflags) {
  return emit_ephemeral(
    cb,
    mcg->getFreeStub(cb, &mcg->cgFixups()),
    target.resumed() ? folly::none : folly::make_optional(spOff),
    REQ_BIND_ADDR,
    addr,
    target.toAtomicInt(),
    trflags.packed
  );
}

TCA emit_retranslate_stub(CodeBlock& cb, FPInvOffset spOff,
                          SrcKey target, TransFlags trflags) {
  return emit_persistent(
    cb,
    target.resumed() ? folly::none : folly::make_optional(spOff),
    REQ_RETRANSLATE,
    target.offset(),
    trflags.packed
  );
}

TCA emit_retranslate_opt_stub(CodeBlock& cb, FPInvOffset spOff,
                              SrcKey target, TransID transID) {
  return emit_persistent(
    cb,
    target.resumed() ? folly::none : folly::make_optional(spOff),
    REQ_RETRANSLATE_OPT,
    target.toAtomicInt(),
    transID
  );
}

///////////////////////////////////////////////////////////////////////////////

namespace x64 {
  static constexpr int kMovLen = 10;
  static constexpr int kLeaVmSpLen = 7;
}

size_t stub_size() {
  // The extra args are the request type and the stub address.
  constexpr auto kTotalArgs = kMaxArgs + 2;

  switch (arch()) {
    case Arch::X64:
      return kTotalArgs * x64::kMovLen + x64::kLeaVmSpLen;
    case Arch::ARM:
      not_implemented();
    case Arch::PPC64:
      not_implemented();
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////

FPInvOffset extract_spoff(TCA stub) {
  switch (arch()) {
    case Arch::X64:
      { DecodedInstruction instr(stub);

        // If it's not a lea, vasm optimized a lea{rvmfp, rvmsp} to a mov, so
        // the offset was 0.
        if (!instr.isLea()) return FPInvOffset{0};

        auto const offBytes = safe_cast<int32_t>(instr.offset());
        always_assert((offBytes % sizeof(Cell)) == 0);
        return FPInvOffset{-(offBytes / int32_t{sizeof(Cell)})};
      }

    case Arch::ARM:
      not_implemented();

    case Arch::PPC64:
      not_implemented();
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////

}}}
