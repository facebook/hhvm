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

#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-simplify-internal.h"

#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-util.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#include "hphp/util/arch.h"
#include "hphp/util/asm-x64.h"

#include <utility>

TRACE_SET_MOD(vasm);

namespace HPHP { namespace jit {

namespace {

///////////////////////////////////////////////////////////////////////////////

/*
 * Simplify an `inst' at block `b', instruction `i', returning whether or not
 * any changes were made.
 *
 * Specializations are below.
 */
template <typename Inst>
bool simplify(Env&, const Inst& /*inst*/, Vlabel /*b*/, size_t /*i*/) {
  return false;
}

///////////////////////////////////////////////////////////////////////////////
/*
 * Arithmetic instructions.
 */

template<typename Test, typename And>
bool simplify_and(Env& env, const And& vandq, Vlabel b, size_t i) {
  return if_inst<Vinstr::testq>(env, b, i + 1, [&] (const testq& vtestq) {
    // And{s0, s1, tmp, _}; testq{tmp, tmp, sf} -> Test{s0, s1, sf}
    // where And/Test is either andq/testq, or andqi/testqi.
    if (!(env.use_counts[vandq.d] == 2 &&
          env.use_counts[vandq.sf] == 0 &&
          vtestq.s0 == vandq.d &&
          vtestq.s1 == vandq.d)) return false;

    return simplify_impl(env, b, i, [&] (Vout& v) {
      v << Test{vandq.s0, vandq.s1, vtestq.sf};
      return 2;
    });
  });
}

bool simplify(Env& env, const andq& vandq, Vlabel b, size_t i) {
  return simplify_and<testq>(env, vandq, b, i);
}

bool simplify(Env& env, const andqi& vandqi, Vlabel b, size_t i) {
  return simplify_and<testqi>(env, vandqi, b, i);
}

/*
 * Simplify masking values with -1 in andXi{}:
 *  andbi{0xff, s, d} -> copy{s, d}
 *  andli{0xffffffff, s, d} -> copy{s, d}
 */
template<typename andi>
bool simplify_andi(Env& env, const andi& inst, Vlabel b, size_t i) {
  if (inst.s0.l() != -1 ||
      env.use_counts[inst.sf] != 0) return false;
  return simplify_impl(env, b, i, [&] (Vout& v) {
    v << copy{inst.s1, inst.d};
    return 1;
  });
}

bool simplify(Env& env, const andbi& andbi, Vlabel b, size_t i) {
  return simplify_andi(env, andbi, b, i);
}

bool simplify(Env& env, const andli& andli, Vlabel b, size_t i) {
  return simplify_andi(env, andli, b, i);
}

////////////////////////////////////////////////////////////////////////////////
/*
 * Conditional operations.
 */

bool simplify(Env& env, const setcc& vsetcc, Vlabel b, size_t i) {
  return if_inst<Vinstr::xorbi>(env, b, i + 1, [&] (const xorbi& vxorbi) {
    // setcc{cc, _, tmp}; xorbi{1, tmp, d, _}; -> setcc{~cc, _, tmp};
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

/*
 * Fold a cmov of a certain width into a copy if both values are the same
 * register or have the same known constant value.
 */
template <typename Inst>
bool cmov_fold_impl(Env& env, const Inst& inst, Vlabel b, size_t i) {
  auto const equivalent = [&]{
    if (inst.t == inst.f) return true;

    auto const t_it = env.unit.regToConst.find(inst.t);
    if (t_it == env.unit.regToConst.end()) return false;
    auto const f_it = env.unit.regToConst.find(inst.f);
    if (f_it == env.unit.regToConst.end()) return false;

    auto const t_const = t_it->second;
    auto const f_const = f_it->second;
    if (t_const.isUndef || f_const.isUndef) return false;
    if (t_const.kind != f_const.kind) return false;
    return t_const.val == f_const.val;
  }();
  if (!equivalent) return false;

  return simplify_impl(
    env, b, i,
    [&] (Vout& v) {
      v << copy{inst.t, inst.d};
      return 1;
    }
  );
}

/*
 * Turn a cmov of a certain width into a matching setcc instruction if the
 * conditions are correct (both sources are constants of value 0 or 1).
 */
template <typename Inst, typename Extend>
bool cmov_setcc_impl(Env& env, const Inst& inst, Vlabel b,
                     size_t i, Extend extend) {
  auto const t_it = env.unit.regToConst.find(inst.t);
  if (t_it == env.unit.regToConst.end()) return false;
  auto const f_it = env.unit.regToConst.find(inst.f);
  if (f_it == env.unit.regToConst.end()) return false;

  auto const check_const = [](Vconst c, bool& val) {
    if (c.isUndef) return false;
    switch (c.kind) {
      case Vconst::Quad:
      case Vconst::Long:
      case Vconst::Byte:
        if (c.val == 0) {
          val = false;
          return true;
        } else if (c.val == 1) {
          val = true;
          return true;
        } else {
          return false;
        }
      case Vconst::Double:
        return false;
    }
    not_reached();
  };

  bool t_val;
  if (!check_const(t_it->second, t_val)) return false;
  bool f_val;
  if (!check_const(f_it->second, f_val)) return false;

  return simplify_impl(env, b, i, [&] (Vout& v) {
    auto const d = env.unit.makeReg();
    if (t_val == f_val) {
      v << copy{env.unit.makeConst(t_val), d};
    } else if (t_val) {
      v << setcc{inst.cc, inst.sf, d};
    } else {
      v << setcc{ccNegate(inst.cc), inst.sf, d};
    }
    extend(v, d, inst.d);
    return 1;
  });
}

bool simplify(Env& env, const cmovb& inst, Vlabel b, size_t i) {
  if (cmov_fold_impl(env, inst, b, i)) return true;
  return cmov_setcc_impl(
    env, inst, b, i,
    [](Vout& v, Vreg8 src, Vreg dest) { v << copy{src, dest}; }
  );
}

bool simplify(Env& env, const cmovw& inst, Vlabel b, size_t i) {
  if (cmov_fold_impl(env, inst, b, i)) return true;
  return cmov_setcc_impl(
    env, inst, b, i,
    [](Vout& v, Vreg8 src, Vreg dest) { v << movzbw{src, dest}; }
  );
}

bool simplify(Env& env, const cmovl& inst, Vlabel b, size_t i) {
  if (cmov_fold_impl(env, inst, b, i)) return true;
  return cmov_setcc_impl(
    env, inst, b, i,
    [](Vout& v, Vreg8 src, Vreg dest) { v << movzbl{src, dest}; }
  );
}

bool simplify(Env& env, const cmovq& inst, Vlabel b, size_t i) {
  if (cmov_fold_impl(env, inst, b, i)) return true;
  return cmov_setcc_impl(
    env, inst, b, i,
    [](Vout& v, Vreg8 src, Vreg dest) { v << movzbq{src, dest}; }
  );
}

////////////////////////////////////////////////////////////////////////////////
/*
 * Copies, loads, and stores.
 */

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

/*
 * Simplify load followed by truncation:
 *  loadq{s, tmp}; movtqb{tmp, d} -> loadtqb{s, d}
 *  loadq{s, tmp}; movtql{tmp, d} -> loadtql{s, d}
 */
template<Vinstr::Opcode mov_op, typename loadt>
bool simplify_load_truncate(Env& env, const load& load, Vlabel b, size_t i) {
  if (env.use_counts[load.d] != 1) return false;
  auto const& code = env.unit.blocks[b].code;
  if (i + 1 >= code.size()) return false;

  return if_inst<mov_op>(env, b, i + 1, [&] (const op_type<mov_op>& mov) {
    if (load.d != mov.s) return false;
    return simplify_impl(env, b, i, [&] (Vout& v) {
      v << loadt{load.s, mov.d};
      return 2;
    });
  });
}

bool simplify(Env& env, const load& load, Vlabel b, size_t i) {
  return
    simplify_load_truncate<Vinstr::movtqb, loadtqb>(env, load, b, i) ||
    simplify_load_truncate<Vinstr::movtql, loadtql>(env, load, b, i);
}

bool simplify(Env& env, const movzlq& inst, Vlabel b, size_t i) {
  auto const def_op = env.def_insts[inst.s];

  // Check if `inst.s' was defined by an instruction with Vreg32 operands, or
  // movzbl{} in particular (which lowers to a movl{}).
  if (width(def_op) != Width::Long &&
      def_op != Vinstr::movzbl) {
    return false;
  }

  // If so, the movzlq{} is redundant---instructions on 32-bit registers on x64
  // always zero the upper bits.
  return simplify_impl(env, b, i, [&] (Vout& v) {
    v << copy{inst.s, inst.d};
    return 1;
  });
}

///////////////////////////////////////////////////////////////////////////////
/*
 * Pushes and pops.
 */

bool simplify(Env& env, const pop& inst, Vlabel b, size_t i) {
  if (env.use_counts[inst.d]) return false;

  // Convert to an lea when popping to a reg without any uses.
  return simplify_impl(env, b, i, [&] (Vout& v) {
    v << lea{reg::rsp[8], reg::rsp};
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

/*
 * Perform architecture-specific peephole simplification.
 */
bool simplify_arch(Env& env, Vlabel b, size_t i) {
  return ARCH_SWITCH_CALL(simplify, env, b, i);
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
      while (simplify(env, b, i) || simplify_arch(env, b, i)) {
        // Stop if we simplified away the tail of the block.
        if (i >= blocks[b].code.size()) break;
      }
    }
  };

  printUnit(kVasmSimplifyLevel, "after vasm simplify", unit);
}

///////////////////////////////////////////////////////////////////////////////

}}
