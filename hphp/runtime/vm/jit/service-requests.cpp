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

#include "hphp/runtime/vm/jit/service-requests.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/align.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/jit/stub-alloc.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/tc-internal.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/resumable.h"

#include "hphp/util/arch.h"
#include "hphp/util/data-block.h"
#include "hphp/util/trace.h"

#include "hphp/vixl/a64/macro-assembler-a64.h"
#include "hphp/vixl/a64/disasm-a64.h"

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
                 DataBlock& data,
                 CGMeta& meta,
                 TCA start,
                 bool persist,
                 FPInvOffset spOff,
                 ServiceRequest sr,
                 const ArgVec& argv) {
  FTRACE(2, "svcreq @{} {}(spOff={}, ", start, to_name(sr), spOff.offset);

  tracing::Pause _p;
  tracing::Block _{
    "emit-svcreq",
    [&] {
      return tracing::Props{}
        .add("service_request", to_name(sr))
        .add("persist", persist);
    }
  };

  auto const is_reused = start != cb.frontier();

  if (!is_reused) cb.assertCanEmit(stub_size());

  CodeBlock stub;
  auto const realAddr = is_reused ? start : cb.toDestAddress(start);
  stub.init(start, realAddr, stub_size(), "svcreq_stub");

  {
    Vauto vasm{stub, stub, data, meta};
    auto& v = vasm.main();

    auto live_out = leave_trace_regs();

    assertx(argv.size() <= kMaxArgs);

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
          always_assert(deltaFits(arg.addr - start, sz::dword));
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
    v << copy{v.cns(spOff.offset), r_svcreq_spoff()};

    live_out |= r_svcreq_stub();
    live_out |= r_svcreq_req();
    live_out |= r_svcreq_spoff();

    v << jmpi{tc::ustubs().handleSRHelper, live_out};

    // We pad ephemeral stubs unconditionally.  This is required for
    // correctness by the x64 code relocator.
    vasm.unit().padding = !persist;
  }

  if (!is_reused) cb.skip(stub.used());
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

TCA emit_bindjmp_stub(CodeBlock& cb, DataBlock& data, CGMeta& fixups,
                      FPInvOffset spOff, TCA jmp, SrcKey target) {
  return emit_ephemeral(
    cb,
    data,
    fixups,
    allocTCStub(cb, &fixups),
    spOff,
    REQ_BIND_JMP,
    jmp,
    target.toAtomicInt()
  );
}

TCA emit_bindaddr_stub(CodeBlock& cb, DataBlock& data, CGMeta& fixups,
                       FPInvOffset spOff, TCA* addr, SrcKey target) {
  // Right now it's possible that addr may not belong to the data segment,
  // as is the case with SSwitchMap (see #10347945) and thus may not be PIC
  // addressable. Passing a TCA generates an RIP relative address which can
  // be handled by the relocation logic, while a TCA* will generate an immediate
  // address which will not be remapped.
  if (data.contains((TCA)addr)) {
    return emit_ephemeral(
      cb,
      data,
      fixups,
      allocTCStub(cb, &fixups),
      spOff,
      REQ_BIND_ADDR,
      (TCA)addr, // needs to be RIP relative so that we can relocate it
      target.toAtomicInt()
    );
  }

  return emit_ephemeral(
    cb,
    data,
    fixups,
    allocTCStub(cb, &fixups),
    spOff,
    REQ_BIND_ADDR,
    addr,
    target.toAtomicInt()
  );
}

TCA emit_retranslate_opt_stub(CodeBlock& cb, DataBlock& data, CGMeta& fixups,
                              FPInvOffset spOff, SrcKey sk) {
  return emit_persistent(
    cb,
    data,
    fixups,
    spOff,
    REQ_RETRANSLATE_OPT,
    sk.toAtomicInt()
  );
}

///////////////////////////////////////////////////////////////////////////////

namespace {
std::atomic<bool> s_fullForStub{false};
}

TCA emit_interp_no_translate_stub(FPInvOffset spOff, SrcKey sk) {
  FTRACE(2, "interp_no_translate_stub @{} {}\n", showShort(sk), spOff.offset);

  // No point on trying to emit if we already failed once.
  if (s_fullForStub.load(std::memory_order_relaxed)) {
    FTRACE(4, "  no space for {}, bailing\n", showShort(sk));
    return nullptr;
  }

  tracing::Pause _p;
  tracing::Block _{"emit-interp-no-translate-stub"};

  auto codeLock = tc::lockCode();
  auto metaLock = tc::lockMetadata();

  auto view = tc::code().view();
  auto& cb = view.frozen();
  auto& data = view.data();

  auto const start = vwrap(
    cb,
    data,
    [&] (Vout& v) { emitInterpReqNoTranslate(v, sk, spOff); },
    false,
    true /* nullOnFull */
  );

  // We passed true to nullOnFull, so if the TC was out of space, we
  // just get a nullptr address.
  if (!start) {
    FTRACE(4, "  ran out of space while making stub for {}\n", showShort(sk));
    s_fullForStub.store(true, std::memory_order_relaxed);
  }
  FTRACE(4, "  emitted stub {} for {}\n", start, showShort(sk));
  return start;
}

///////////////////////////////////////////////////////////////////////////////

}}}
