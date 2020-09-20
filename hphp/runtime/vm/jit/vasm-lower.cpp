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

namespace vasm_detail {

constexpr auto kInvalidConstraint = std::numeric_limits<int>::max();

jit::vector<int> computeDominatingConstraints(Vunit& unit) {
  auto vregConstraints = jit::vector<int>(unit.blocks.size(),
                                          kInvalidConstraint);

  auto const rpo = sortBlocks(unit);

  // Calculate each block's dominating Vreg constraint.  The lowerer will use
  // this as the initial Vreg constraint for each block, adjusting as it
  // encounter vregrestrict{}/vregunrestrict{} during lowering.
  vregConstraints[rpo[0]] = 0;
  for (auto const b : rpo) {
    auto con = vregConstraints[b];
    assert_flog(con != kInvalidConstraint,
                "Dominating constraint can't be uninitialized.");

    // Iterate through the instructions, updating the constraint accordingly.
    for (auto const& inst : unit.blocks[b].code) {
      if (inst.op == Vinstr::vregrestrict) {
        con--;
      } else if (inst.op == Vinstr::vregunrestrict) {
        con++;
      }
    }

    // Pass the constraint to each of the successors, enforcing that a block
    // isn't passed conflicting constraints from predecessors.
    for (auto const s : succs(unit.blocks[b])) {
      if (vregConstraints[s] == kInvalidConstraint) {
        vregConstraints[s] = con;
      } else {
        assert_flog(
          vregConstraints[s] == con,
          "Block must be dominated by a single vreg constraint level."
        );
      }
    }
  }

  return vregConstraints;
}

}

///////////////////////////////////////////////////////////////////////////////

namespace {

///////////////////////////////////////////////////////////////////////////////

template<typename Lower>
void lower_impl(Vunit& unit, Vlabel b, size_t i, Lower lower) {
  vmodify(unit, b, i, [&] (Vout& v) { lower(v); return 1; });
}

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
  Vout v(unit, scratch, vinstr.irctx());

  // Total amount of space (in 64-bit words) we've pushed onto the
  // stack.
  int32_t spDelta = 0;
  // If we're going to spill a TypedValue onto the stack, the map of
  // indices of the VregList to their offset (from spDelta) on the
  // stack.
  using SpillOffsets = folly::sorted_vector_map<size_t, int32_t>;
  SpillOffsets argSpillOffsets;
  SpillOffsets stkSpillOffsets;

  // First, if we need to spilled any TypedValues to the stack, do
  // that first. Record their offset in the stack so we can emit the
  // appropriate leas later.
  static_assert(TVOFF(m_data) == 0, "");
  static_assert(TVOFF(m_type) == 8, "");
  for (auto const& p : vargs.argSpills) {
    v << pushp{vargs.args[p.first], p.second};
    spDelta += 2;
    argSpillOffsets.emplace(p.first, spDelta);
  }
  for (auto const& p : vargs.stkSpills) {
    v << pushp{vargs.stkArgs[p.first], p.second};
    spDelta += 2;
    stkSpillOffsets.emplace(p.first, spDelta);
  }

  // If this parameter is actually a spilled TypedValue, pass its
  // address on the stack to the function instead, not its Vreg (we
  // already pushed its Vregs onto the stack). Otherwise use the Vreg
  // as usual.
  auto const spillOverride = [&] (Vreg r,
                                  size_t idx,
                                  const SpillOffsets& spills) {
    auto const it = spills.find(idx);
    if (it == spills.end()) return r;
    auto const temp = v.makeReg();
    v << lea{rsp()[(spDelta - it->second) * sizeof(uintptr_t)], temp};
    return temp;
  };

  // Now push stack arguments, in reverse order. Push in pairs without
  // padding except for the last argument (pushed first) which should
  // be padded if there are an odd number of arguments.
  auto numArgs = vargs.stkArgs.size();
  int32_t const adjust = (numArgs & 0x1) ? sizeof(uintptr_t) : 0;
  if (adjust) {
    // Using InvalidReg below fails SSA checks and simplify pass, so just
    // push the arg twice. It's on the same cacheline and will actually
    // perform faster than an explicit lea.
    auto const r =
      spillOverride(vargs.stkArgs[numArgs - 1], numArgs - 1, stkSpillOffsets);
    v << pushp{r, r};
    spDelta += 2;
    --numArgs;
  }
  for (auto i2 = numArgs; i2 >= 2; i2 -= 2) {
    auto const r1 =
      spillOverride(vargs.stkArgs[i2 - 1], i2 - 1, stkSpillOffsets);
    auto const r2 =
      spillOverride(vargs.stkArgs[i2 - 2], i2 - 2, stkSpillOffsets);
    v << pushp{r1, r2};
    spDelta += 2;
  }

  // Get the arguments in the proper registers.
  RegSet argRegs;
  auto doArgs = [&] (const VregList& srcs,
                     PhysReg (*r)(size_t),
                     const SpillOffsets* spills) {
    VregList argSrcs;
    VregList argDests;
    for (size_t i2 = 0, n = srcs.size(); i2 < n; ++i2) {
      auto const src = spills ? spillOverride(srcs[i2], i2, *spills) : srcs[i2];
      auto const dst = r(i2);
      argSrcs.push_back(src);
      argDests.push_back(dst);
      argRegs |= dst;
    }
    if (argDests.size()) {
      v << copyargs{v.makeTuple(std::move(argSrcs)),
                    v.makeTuple(std::move(argDests))};
    }
  };
  doArgs(vargs.indRetArgs, rarg_ind_ret, nullptr);
  doArgs(vargs.args, rarg, &argSpillOffsets);
  doArgs(vargs.simdArgs, rarg_simd, nullptr);

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
    if (spDelta > 0) {
      auto& taken = unit.blocks[targets[1]].code;
      assertx(taken.front().op == Vinstr::landingpad ||
              taken.front().op == Vinstr::jmp);

      Vinstr vi {
        lea{rsp()[spDelta*sizeof(uintptr_t)], rsp()},
        taken.front().irctx()
      };

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
        switch (arch()) {
          case Arch::X64: // fall through
          case Arch::PPC64:
            v << copyargs{
              v.makeTuple({rret(0), rret(1)}),
              v.makeTuple({dests[0], dests[1]})
            };
            break;
          case Arch::ARM:
            // For ARM64 we need to clear the bits 8..31 from the type value.
            // That allows us to use the resulting register values in
            // type comparisons without the need for truncation there.
            // We must not touch bits 63..32 as they contain the AUX data.
            v << copy{rret(0), dests[0]};
            v << andq{v.cns(0xffffffff000000ff),
                      rret(1), dests[1], v.makeReg()};
            break;
        }
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
      assertx(dests.size() == 1 || dests.size() == 2);
      assertx(dests[0].isValid());

      if (dests.size() == 1) {
        // Copy the single-register result to dests[0].
        v << copy{rret(0), dests[0]};
      } else {
        assertx(dests[1].isValid());
        // Copy the result pair to dests.
        v << copyargs{
          v.makeTuple({rret(0), rret(1)}),
          v.makeTuple({dests[0], dests[1]})
        };
      }
      break;

    case DestType::Dbl:
      // Copy the single-register result to dests[0].
      assertx(dests.size() == 1);
      assertx(dests[0].isValid());
      v << copy{rret_simd(0), dests[0]};
      break;

    case DestType::Indirect:
      // Already asserted above
      break;

    case DestType::None:
      assertx(dests.empty());
      break;
  }

  if (spDelta > 0) {
    v << lea{rsp()[spDelta * sizeof(uintptr_t)], rsp()};
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

template <typename Inst>
void lower(VLS& /*env*/, Inst& /*inst*/, Vlabel /*b*/, size_t /*i*/) {}

void lower(VLS& env, vcall& inst, Vlabel b, size_t i) {
  lower_vcall(env.unit, inst, b, i);
}
void lower(VLS& env, vinvoke& inst, Vlabel b, size_t i) {
  lower_vcall(env.unit, inst, b, i);
}

void lower(VLS& env, defvmsp& inst, Vlabel b, size_t i) {
  env.unit.blocks[b].code[i] = copy{rvmsp(), inst.d};
}
void lower(VLS& env, defvmfp& inst, Vlabel b, size_t i) {
  env.unit.blocks[b].code[i] = copy{rvmfp(), inst.d};
}
void lower(VLS& env, pushvmfp& inst, Vlabel b, size_t i) {
  env.unit.blocks[b].code[i] = copy{inst.s, rvmfp()};
}
void lower(VLS& env, popvmfp& inst, Vlabel b, size_t i) {
  env.unit.blocks[b].code[i] = copy{inst.s, rvmfp()};
}
void lower(VLS& env, syncvmsp& inst, Vlabel b, size_t i) {
  env.unit.blocks[b].code[i] = copy{inst.s, rvmsp()};
}

void lower(VLS& env, defvmretdata& inst, Vlabel b, size_t i) {
  env.unit.blocks[b].code[i] = copy{rret_data(), inst.data};
}
void lower(VLS& env, defvmrettype& inst, Vlabel b, size_t i) {
  switch (arch()) {
    case Arch::X64: // fall through
    case Arch::PPC64:
      env.unit.blocks[b].code[i] = copy{rret_type(), inst.type};
      break;
    case Arch::ARM:
      // For ARM64 we need to clear the bits 8..31 from the type value.
      // That allows us to use the resulting register values in
      // type comparisons without the need for truncation there.
      // We must not touch bits 63..32 as they contain the AUX data.
      env.unit.blocks[b].code[i] = andq{
        env.unit.makeConst(Vconst{0xffffffff000000ff}),
        rret_type(), inst.type, env.unit.makeReg()};
      break;
  }
}
void lower(VLS& env, syncvmret& inst, Vlabel b, size_t i) {
  switch (arch()) {
    case Arch::X64: // fall through
    case Arch::PPC64:
      env.unit.blocks[b].code[i] = copyargs{
        env.unit.makeTuple({inst.data, inst.type}),
        env.unit.makeTuple({rret_data(), rret_type()})
      };
      break;
    case Arch::ARM:
      // For ARM64 we need to clear the bits 8..31 from the type value.
      // That allows us to use the resulting register values in
      // type comparisons without the need for truncation there.
      // We must not touch bits 63..32 as they contain the AUX data.
      lower_impl(env.unit, b, i, [&] (Vout& v) {
        v << copy{inst.data, rret_data()};
        v << andq{v.cns(0xffffffff000000ff),
                  inst.type, rret_type(), v.makeReg()};
      });
      break;
  }
}
void lower(VLS& env, syncvmrettype& inst, Vlabel b, size_t i) {
  switch (arch()) {
    case Arch::X64: // fall through
    case Arch::PPC64:
      env.unit.blocks[b].code[i] = copy{inst.type, rret_type()};
      break;
    case Arch::ARM:
      // For ARM64 we need to clear the bits 8..31 from the type value.
      // That allows us to use the resulting register values in
      // type comparisons without the need for truncation there.
      // We must not touch bits 63..32 as they contain the AUX data.
      lower_impl(env.unit, b, i, [&] (Vout& v) {
        v << andq{v.cns(0xffffffff000000ff),
                  inst.type, rret_type(), v.makeReg()};
      });
      break;
  }
}
void lower(VLS& env, vregrestrict& /*inst*/, Vlabel b, size_t i) {
  env.vreg_restrict_level--;
  env.unit.blocks[b].code[i] = nop{};
}
void lower(VLS& env, vregunrestrict& /*inst*/, Vlabel b, size_t i) {
  env.vreg_restrict_level++;
  env.unit.blocks[b].code[i] = nop{};
}

///////////////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

void vlower(VLS& env, Vlabel b, size_t i) {
  auto& inst = env.unit.blocks[b].code[i];

  switch (inst.op) {
#define O(name, ...)                    \
    case Vinstr::name:                  \
      lower(env, inst.name##_, b, i);  \
      break;

    VASM_OPCODES
#undef O
  }
}

///////////////////////////////////////////////////////////////////////////////

}}
