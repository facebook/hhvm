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

#include "hphp/runtime/vm/jit/vasm.h"

#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-util.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#include "hphp/util/asm-x64.h"

#include <utility>

TRACE_SET_MOD(vasm);

namespace HPHP { namespace jit {

namespace {

///////////////////////////////////////////////////////////////////////////////

struct Env {
  Vunit& unit;

  // Number of uses of each Vreg.
  jit::vector<uint32_t> use_counts;

  // Instruction which def'd each Vreg.  Probably only useful when the def
  // instruction only has a single dst, but that's all we need right now.
  jit::vector<Vinstr::Opcode> def_insts;
};

template<Vinstr::Opcode op>
using op_type = typename Vinstr::op_matcher<op>::type;

/*
 * Check if the instruction at block `b', index `i' of `env.unit' is an `op'.
 * If so, call `f' on the specific instruction, and return the result.  If not,
 * return a default-constructed instance of f's return type.
 */
template<Vinstr::Opcode op, typename F>
auto if_inst(const Env& env, Vlabel b, size_t i, F f)
  -> decltype(f(std::declval<const op_type<op>&>()))
{
  auto const& code = env.unit.blocks[b].code;
  if (i >= code.size() || code[i].op != op) {
    return decltype(f(code[i].get<op>())){};
  }
  return f(code[i].get<op>());
}

/*
 * Helper for vasm-simplification routines.
 *
 * This routine wraps a call to `simplify' with accounting logic for `env'.
 * `simplify' is passed a scratch Vinstr stream to fill with instructions.  It
 * should return the number of instructions to remove from the Vunit and
 * replace with the contents of its stream.  This instruction splicing occurs
 * at block `b', instruction `i' of the Vunit.
 *
 * If `simplify' both returns 0 and fails to populate the stream, nothing
 * happens, and false is returned; otherwise, return true.
 *
 * The `simplify' implementation should never modify instructions in the
 * stream, due to the required accounting for `env'.  Furthermore, once
 * simplify_impl() is run, all pointers and references to instructions in block
 * `b' should be considered invalidated.
 */
template<typename Simplify>
bool simplify_impl(Env& env, Vlabel b, size_t i, Simplify simplify) {
  auto& unit = env.unit;
  auto& blocks = unit.blocks;
  auto const& vinstr = blocks[b].code[i];

  auto const scratch = unit.makeScratchBlock();
  SCOPE_EXIT { unit.freeScratchBlock(scratch); };
  Vout v(unit, scratch, vinstr.origin);

  auto const nremove = simplify(v);
  if (nremove == 0 && v.empty()) return false;

  // Update use counts for to-be-removed instructions.
  for (auto j = i; j < i + nremove; ++j) {
    visitUses(unit, blocks[b].code[j], [&] (Vreg r) {
      --env.use_counts[r];
    });
  }

  // Update use counts and def instructions for to-be-added instructions.
  for (auto const& inst : blocks[scratch].code) {
    visitUses(unit, inst, [&] (Vreg r) {
      assertx(r < env.use_counts.size());
      ++env.use_counts[r];
    });
    visitDefs(unit, inst, [&] (Vreg r) {
      env.def_insts[r] = inst.op;
    });
  }

  vector_splice(blocks[b].code, i, nremove, blocks[scratch].code);
  return true;
}

/*
 * Simplify an `inst' at block `b', instruction `i', returning whether or not
 * any changes were made.
 *
 * Specializations are below.
 */
template<typename Inst>
bool simplify(Env&, const Inst& inst, Vlabel b, size_t i) { return false; }

///////////////////////////////////////////////////////////////////////////////

bool simplify(Env& env, const andq& vandq, Vlabel b, size_t i) {
  return if_inst<Vinstr::testq>(env, b, i + 1, [&] (const testq& vtestq) {
    // andq{s0, s1, tmp, _}; testq{tmp, tmp, sf} --> testq{s0, s1, sf}
    if (!(env.use_counts[vandq.d] == 2 &&
          env.use_counts[vandq.sf] == 0 &&
          vtestq.s0 == vandq.d &&
          vtestq.s1 == vandq.d)) return false;

    return simplify_impl(env, b, i, [&] (Vout& v) {
      v << testq{vandq.s0, vandq.s1, vtestq.sf};
      return 2;
    });
  });
}

bool simplify(Env& env, const setcc& vsetcc, Vlabel b, size_t i) {
  return if_inst<Vinstr::xorbi>(env, b, i + 1, [&] (const xorbi& vxorbi) {
    // setcc{cc, _, tmp}; xorbi{1, tmp, d, _}; --> setcc{~cc, _, tmp};
    if (!(env.use_counts[vsetcc.d] == 1 &&
          vxorbi.s0.b() == 1 &&
          vxorbi.s1 == vsetcc.d &&
          env.use_counts[vxorbi.sf] == 0)) return false;

    return simplify_impl(env, b, i, [&] (Vout& v) {
      v << setcc{ccNegate(vsetcc.cc), vsetcc.sf, vxorbi.d};
      return 2;
    });
  });
}

bool simplify(Env& env, const copyargs& inst, Vlabel b, size_t i) {
  auto const& srcs = env.unit.tuples[inst.s];
  auto const& dsts = env.unit.tuples[inst.d];
  assertx(srcs.size() == dsts.size());

  for (auto const src : srcs) {
    for (auto const dst : dsts) {
      if (src == dst) return false;
    }
  }

  // If the srcs and dsts don't intersect, simplify to a sequence of copies.
  return simplify_impl(env, b, i, [&] (Vout& v) {
    for (auto i = 0; i < srcs.size(); ++i) {
      v << copy{srcs[i], dsts[i]};
    }
    return 1;
  });
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Perform peephole simplification at instruction `i' of block `b'.
 *
 * Return true if changes were made, else false.
 */
bool simplify(Env& env, Vlabel b, size_t i) {
  assertx(i <= env.unit.blocks[b].code.size());
  auto const& inst = env.unit.blocks[b].code[i];

  switch (inst.op) {
#define O(name, ...)    \
    case Vinstr::name:  \
      return simplify(env, inst.name##_, b, i); \

    VASM_OPCODES
#undef O
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////

}

/*
 * Peephole simplification pass for a Vunit.
 */
void simplify(Vunit& unit) {
  assertx(check(unit));
  auto& blocks = unit.blocks;

  Env env { unit };
  env.use_counts.resize(unit.next_vr);
  env.def_insts.resize(unit.next_vr, Vinstr::nop);

  auto const labels = sortBlocks(unit);

  // Set up Env, only visiting reachable blocks.
  for (auto const b : labels) {
    assertx(!blocks[b].code.empty());

    for (auto const& inst : blocks[b].code) {
      visitDefs(unit, inst, [&] (Vreg r) { env.def_insts[r] = inst.op; });
      visitUses(unit, inst, [&] (Vreg r) { ++env.use_counts[r]; });
    }
  };

  // The simplify() implementations may allocate scratch blocks and modify
  // instruction streams, so we cannot use standard iterators here.
  for (auto const b : labels) {
    for (size_t i = 0; i < blocks[b].code.size(); ++i) {
      // Simplify at this index until no changes are made.
      while (simplify(env, b, i)) {
        // Stop if we simplified away the tail of the block.
        if (i >= blocks[b].code.size()) break;
      }
    }
  };

  printUnit(kVasmSimplifyLevel, "after vasm simplify", unit);
}

///////////////////////////////////////////////////////////////////////////////

}}
