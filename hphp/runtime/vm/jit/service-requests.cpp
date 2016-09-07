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

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/jit/stub-alloc.h"
#include "hphp/runtime/vm/jit/tc.h"
#include "hphp/runtime/vm/jit/translator-inline.h"
#include "hphp/runtime/vm/jit/unique-stubs.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"

#include "hphp/util/arch.h"
#include "hphp/util/data-block.h"
#include "hphp/util/trace.h"

#include "hphp/vixl/a64/macro-assembler-a64.h"

#include "hphp/ppc64-asm/decoded-instr-ppc64.h"

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
                 TCA start,
                 bool persist,
                 folly::Optional<FPInvOffset> spOff,
                 ServiceRequest sr,
                 const ArgVec& argv) {
  FTRACE(2, "svcreq @{} {}(", start, to_name(sr));

  auto const is_reused = start != cb.frontier();

  CodeBlock stub;
  stub.init(start, stub_size(), "svcreq_stub");

  {
    CGMeta fixups;
    SCOPE_EXIT { assert(fixups.empty()); };

    Vauto vasm{stub, stub, data, fixups};
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
                      FPInvOffset spOff,
                      TCA jmp, SrcKey target, TransFlags trflags) {
  return emit_ephemeral(
    cb,
    data,
    allocTCStub(cb, &fixups),
    target.resumed() ? folly::none : folly::make_optional(spOff),
    REQ_BIND_JMP,
    jmp,
    target.toAtomicInt(),
    trflags.packed
  );
}

TCA emit_bindaddr_stub(CodeBlock& cb, DataBlock& data, CGMeta& fixups,
                       FPInvOffset spOff,
                       TCA* addr, SrcKey target, TransFlags trflags) {
  return emit_ephemeral(
    cb,
    data,
    allocTCStub(cb, &fixups),
    target.resumed() ? folly::none : folly::make_optional(spOff),
    REQ_BIND_ADDR,
    addr,
    target.toAtomicInt(),
    trflags.packed
  );
}

TCA emit_retranslate_stub(CodeBlock& cb, DataBlock& data, FPInvOffset spOff,
                          SrcKey target, TransFlags trflags) {
  return emit_persistent(
    cb,
    data,
    target.resumed() ? folly::none : folly::make_optional(spOff),
    REQ_RETRANSLATE,
    target.offset(),
    trflags.packed
  );
}

TCA emit_retranslate_opt_stub(CodeBlock& cb, DataBlock& data, FPInvOffset spOff,
                              SrcKey target, TransID transID) {
  return emit_persistent(
    cb,
    data,
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

namespace arm {
  // 'lea' results in atmost 4 instructions (see vasm-arm.cpp)
  static constexpr int kMovLen = 4 * 4;
  static constexpr int kLeaVmSpLen = 4 * 4;
}

namespace ppc64 {
  // Standard ppc64 instructions are 4 bytes long
  static constexpr int kStdIns = 4;
  // Leap for ppc64, in worst case, have 5 standard ppc64 instructions.
  static constexpr int kLeaVMSpLen = kStdIns * 5;
}

size_t stub_size() {
  // The extra args are the request type and the stub address.
  constexpr auto kTotalArgs = kMaxArgs + 2;

  switch (arch()) {
    case Arch::X64:
      return kTotalArgs * x64::kMovLen + x64::kLeaVmSpLen;
    case Arch::ARM:
      return kTotalArgs * arm::kMovLen + arm::kLeaVmSpLen;
    case Arch::PPC64:
      // This calculus was based on the amount of emitted instructions in
      // emit_svcreq.
      return (ppc64::kStdIns + ppc64::kLeaVMSpLen) * kTotalArgs +
          ppc64::kLeaVMSpLen + 3 * ppc64::kStdIns;
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////

FPInvOffset extract_spoff(TCA stub) {
  switch (arch()) {
    case Arch::X64: {
      HPHP::jit::x64::DecodedInstruction instr(stub);

      // If it's not a lea, vasm optimized a lea{rvmfp, rvmsp} to a mov, so
      // the offset was 0.
      if (!instr.isLea()) return FPInvOffset{0};

      auto const offBytes = safe_cast<int32_t>(instr.offset());
      always_assert((offBytes % sizeof(Cell)) == 0);
      return FPInvOffset{-(offBytes / int32_t{sizeof(Cell)})};
    }

    case Arch::ARM: {
      struct Decoder : public vixl::Decoder {
        void VisitAddSubImmediate(vixl::Instruction* inst) {
          // For immediate operands, shift can be '0' or '12'
          int64_t immed =
            inst->ImmAddSub() << ((inst->ShiftAddSub() == 1) ? 12 : 0);
          switch (inst->Mask(vixl::AddSubOpMask)) {
            case vixl::ADD: offset = immed; break;
            case vixl::SUB: offset = -immed; break;
            default: break;
          }
        }
        void VisitMoveWideImmediate(vixl::Instruction* inst) {
          // For wide moves, shift can be 0, 16, 32 or 64
          int64_t immed = safe_cast<int64_t>(
            inst->ImmMoveWide() << (inst->ShiftMoveWide() << 4));
          switch (inst->Mask(vixl::MoveWideImmediateMask)) {
            case vixl::MOVN_w:
            case vixl::MOVN_x:
              immed = safe_cast<int64_t>(~immed);
              break;
          }
          offset = immed;
        }
        folly::Optional<int32_t> offset;
      };
      Decoder decoder;
      decoder.Decode((vixl::Instruction*)(stub));

      // 'lea' becomes
      //   a. 'add dst, base, #imm' or
      //   b. 'mov r, #imm'
      //      'add dst, base, r'
      // FIXME: Return '0' if vasm optimizes 'lea' to 'mov'
      if (!decoder.offset) return FPInvOffset{0};
      always_assert(decoder.offset && (*decoder.offset % sizeof(Cell)) == 0);
      return FPInvOffset{-(*decoder.offset / int32_t{sizeof(Cell)})};
    }

    case Arch::PPC64: {
      ppc64_asm::DecodedInstruction instr(stub);
      if (!instr.isSpOffsetInstr()) {
        return FPInvOffset{0};
      } else {
        auto const offBytes = safe_cast<int32_t>(instr.offset());
        return FPInvOffset{-(offBytes / int32_t{sizeof(Cell)})};
      }
    }
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////

}}}
