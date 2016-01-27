/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/vasm-lower.h"

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/code-gen-helpers.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-util.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#include <folly/ScopeGuard.h>

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

template<class Inst>
void lower_vcall(Vunit& unit, Inst& inst, Vlabel b, size_t i) {
  auto& blocks = unit.blocks;
  auto const& vinstr = blocks[b].code[i];

  auto const is_vcall = vinstr.op == Vinstr::vcall;
  auto const vcall = vinstr.vcall_;
  auto const vinvoke = vinstr.vinvoke_;

  // We lower vinvoke in two phases, and `inst' is overwritten after the first
  // phase.  We need to save any of its parameters that we care about in the
  // second phase ahead of time.
  auto const& vargs = unit.vcallArgs[inst.args];
  auto const dests = unit.tuples[inst.d];
  auto const destType = inst.destType;

  auto const scratch = unit.makeScratchBlock();
  SCOPE_EXIT { unit.freeScratchBlock(scratch); };
  Vout v(unit, scratch, vinstr.origin);

  int32_t const adjust = (vargs.stkArgs.size() & 0x1) ? sizeof(uintptr_t) : 0;
  if (adjust) v << lea{rsp()[-adjust], rsp()};

  // Push stack arguments, in reverse order.
  for (int i = vargs.stkArgs.size() - 1; i >= 0; --i) {
    v << push{vargs.stkArgs[i]};
  }

  // Get the arguments in the proper registers.
  RegSet argRegs;
  auto doArgs = [&] (const VregList& srcs, PhysReg (*r)(size_t)) {
    VregList argDests;
    for (size_t i = 0, n = srcs.size(); i < n; ++i) {
      auto const reg = r(i);
      argDests.push_back(reg);
      argRegs |= reg;
    }
    if (argDests.size()) {
      v << copyargs{v.makeTuple(srcs),
                    v.makeTuple(std::move(argDests))};
    }
  };
  doArgs(vargs.args, rarg);
  doArgs(vargs.simdArgs, rarg_simd);

  // Emit the appropriate call instruction sequence.
  emitCall(v, inst.call, argRegs);

  // Handle fixup and unwind information.
  if (inst.fixup.isValid()) {
    v << syncpoint{inst.fixup};
  }

  if (!is_vcall) {
    auto& targets = vinvoke.targets;
    v << unwind{{targets[0], targets[1]}};

    // Insert an lea fixup for any stack args at the beginning of the catch
    // block.
    if (auto rspOffset = ((vargs.stkArgs.size() + 1) & ~1) *
                         sizeof(uintptr_t)) {
      auto& taken = unit.blocks[targets[1]].code;
      assertx(taken.front().op == Vinstr::landingpad ||
              taken.front().op == Vinstr::jmp);

      Vinstr vi { lea{rsp()[rspOffset], rsp()} };
      vi.origin = taken.front().origin;

      if (taken.front().op == Vinstr::jmp) {
        taken.insert(taken.begin(), vi);
      } else {
        taken.insert(taken.begin() + 1, vi);
      }
    }

    // Write out the code so far to the end of b.  Remaining code will be
    // emitted to the next block.
    vector_splice(blocks[b].code, i, 1, blocks[scratch].code);
  } else if (vcall.nothrow) {
    v << nothrow{};
  }
  // For vinvoke, `inst' is no longer valid after this point.

  // Copy the call result to the destination register(s).
  switch (destType) {
    case DestType::TV:
      static_assert(offsetof(TypedValue, m_data) == 0, "");
      static_assert(offsetof(TypedValue, m_type) == 8, "");

      if (dests.size() == 2) {
        v << copy2{rret(0), rret(1), dests[0], dests[1]};
      } else {
        // We have cases where we statically know the type but need the value
        // from native call.  Even if the type does not really need a register
        // (e.g., InitNull), a Vreg is still allocated in assignRegs(), so the
        // following assertion holds.
        assertx(dests.size() == 1);
        v << copy{rret(0), dests[0]};
      }
      break;

    case DestType::SIMD:
      static_assert(offsetof(TypedValue, m_data) == 0, "");
      static_assert(offsetof(TypedValue, m_type) == 8, "");
      assertx(dests.size() == 1);

      pack2(v, rret(0), rret(1), dests[0]);
      break;

    case DestType::SSA:
    case DestType::Byte:
      assertx(dests.size() == 1);
      assertx(dests[0].isValid());

      // Copy the single-register result to dests[0].
      v << copy{rret(0), dests[0]};
      break;

    case DestType::Dbl:
      // Copy the single-register result to dests[0].
      assertx(dests.size() == 1);
      assertx(dests[0].isValid());
      v << copy{rret_simd(0), dests[0]};
      break;

    case DestType::None:
      assertx(dests.empty());
      break;
  }

  if (vargs.stkArgs.size() > 0) {
    auto const delta = safe_cast<int32_t>(
      vargs.stkArgs.size() * sizeof(uintptr_t) + adjust
    );
    v << lea{rsp()[delta], rsp()};
  }

  // Insert new instructions to the appropriate block.
  if (is_vcall) {
    vector_splice(blocks[b].code, i, 1, blocks[scratch].code);
  } else {
    vector_splice(blocks[vinvoke.targets[0]].code, 0, 0,
                  blocks[scratch].code);
  }
}

///////////////////////////////////////////////////////////////////////////////

template<typename Inst>
void lower(Vunit& unit, Inst& inst, Vlabel b, size_t i) {}

void lower(Vunit& unit, vcall& inst, Vlabel b, size_t i) {
  lower_vcall(unit, inst, b, i);
}
void lower(Vunit& unit, vinvoke& inst, Vlabel b, size_t i) {
  lower_vcall(unit, inst, b, i);
}

void lower(Vunit& unit, defvmsp& inst, Vlabel b, size_t i) {
  unit.blocks[b].code[i] = copy{rvmsp(), inst.d};
}
void lower(Vunit& unit, syncvmsp& inst, Vlabel b, size_t i) {
  unit.blocks[b].code[i] = copy{inst.s, rvmsp()};
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

void vlower(Vunit& unit, Vlabel b, size_t i) {
  Timer _t(Timer::vasm_lower);

  auto& inst = unit.blocks[b].code[i];

  switch (inst.op) {
#define O(name, ...)                    \
    case Vinstr::name:                  \
      lower(unit, inst.name##_, b, i);  \
      break;

    VASM_OPCODES
#undef O
  }
}

void vlower(Vunit& unit) {
  // This pass relies on having no critical edges in the unit.
  splitCriticalEdges(unit);

  auto& blocks = unit.blocks;

  // The lowering operations for individual instructions may allocate scratch
  // blocks, which may invalidate iterators on `blocks'.  Correctness of this
  // pass relies on PostorderWalker /not/ using standard iterators on `blocks'.
  PostorderWalker{unit}.dfs([&] (Vlabel b) {
    assertx(!blocks[b].code.empty());
    for (size_t i = 0; i < blocks[b].code.size(); ++i) {
      vlower(unit, b, i);
    }
  });
}

///////////////////////////////////////////////////////////////////////////////

}}
