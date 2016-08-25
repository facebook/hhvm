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

#include "hphp/util/arch.h"
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
 * This just wraps vmodify() with some accounting logic for `env'.
 */
template<typename Simplify>
bool simplify_impl(Env& env, Vlabel b, size_t i, Simplify simplify) {
  auto& unit = env.unit;

  return vmodify(unit, b, i, [&] (Vout& v) {
    auto& blocks = unit.blocks;
    auto const nremove = simplify(v);

    // Update use counts for to-be-removed instructions.
    for (auto j = i; j < i + nremove; ++j) {
      visitUses(unit, blocks[b].code[j], [&] (Vreg r) {
        --env.use_counts[r];
      });
    }

    // Update use counts and def instructions for to-be-added instructions.
    for (auto const& inst : blocks[Vlabel(v)].code) {
      visitUses(unit, inst, [&] (Vreg r) {
        if (r >= env.use_counts.size()) {
          env.use_counts.resize(size_t{r}+1);
        }
        ++env.use_counts[r];
      });
      visitDefs(unit, inst, [&] (Vreg r) {
        if (r >= env.def_insts.size()) {
          env.def_insts.resize(size_t{r}+1, Vinstr::nop);
        }
        env.def_insts[r] = inst.op;
      });
    }

    return nremove;
  });
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

template<typename Test, typename And>
bool simplify_and(Env& env, const And& vandq, Vlabel b, size_t i) {
  return if_inst<Vinstr::testq>(env, b, i + 1, [&] (const testq& vtestq) {
    // And{s0, s1, tmp, _}; testq{tmp, tmp, sf} --> Test{s0, s1, sf}
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

// Turn a cmov of a certain width into a matching setcc instruction if the
// conditions are correct (both sources are constants of value 0 or 1).
template <typename Inst, typename Extend>
bool cmov_impl(Env& env, const Inst& inst, Vlabel b, size_t i, Extend extend) {
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

  return simplify_impl(
    env, b, i, [&] (Vout& v) {
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
    }
  );
}

bool simplify(Env& env, const cmovb& inst, Vlabel b, size_t i) {
  return cmov_impl(
    env, inst, b, i,
    [](Vout& v, Vreg8 src, Vreg dest) { v << copy{src, dest}; }
  );
}

bool simplify(Env& env, const cmovw& inst, Vlabel b, size_t i) {
  return cmov_impl(
    env, inst, b, i,
    [](Vout& v, Vreg8 src, Vreg dest) { v << movzbw{src, dest}; }
  );
}

bool simplify(Env& env, const cmovl& inst, Vlabel b, size_t i) {
  return cmov_impl(
    env, inst, b, i,
    [](Vout& v, Vreg8 src, Vreg dest) { v << movzbl{src, dest}; }
  );
}

bool simplify(Env& env, const cmovq& inst, Vlabel b, size_t i) {
  return cmov_impl(
    env, inst, b, i,
    [](Vout& v, Vreg8 src, Vreg dest) { v << movzbq{src, dest}; }
  );
}

////////////////////////////////////////////////////////////////////////////////

/*
 * Simplify equal/not-equal comparisons against zero into test
 * instructions. Only perform this simplification if the comparison is
 * immediately followed by an instruction which uses the flag result with an
 * equals or not-equals condition code. Furthermore, that must be the only usage
 * of that flag result.
 */

// Check if an instruction uses a particular flag register and contains an equal
// or not-equal condition code. If more than one condition code or flag register
// is used, we don't match against the instruction. This is preferred over
// maintaining an explicit list of allowed instructions.
struct CmpUseChecker {
  explicit CmpUseChecker(VregSF target) : target{target} {}

  void imm(ConditionCode cc) {
    cc_result = !cc_result ? (cc == CC_E || cc == CC_NE) : false;
  }
  void use(VregSF sf) {
    use_result = !use_result ? (sf == target) : false;
  }
  template<class H> void useHint(VregSF sf, H) { use(sf); }
  void across(VregSF sf) { use(sf); }

  template<class T> void imm(const T&) {}
  template<class T> void def(T) {}
  template<class T, class H> void defHint(T r, H) {}
  template<class T> void use(T) {}
  template<class T, class H> void useHint(T r, H) {}
  template<class T> void across(T r) {}

  VregSF target;
  folly::Optional<bool> cc_result;
  folly::Optional<bool> use_result;
};

// Transform a cmp* instruction into a test* instruction if all the above
// conditions are met.
template <typename Out, typename In, typename Reg>
bool cmp_zero_impl(Env& env, const In& inst, Reg r, Vlabel b, size_t i) {
  if (env.use_counts[inst.sf] != 1) return false;

  auto const suitable_use = [&]{
    // "cmp zero -> test" simplification is arch specific so it's considered an
    // opt-in optimization.
    if (arch() != Arch::X64) return false;

    auto const& code = env.unit.blocks[b].code;
    if (i + 1 >= code.size()) return false;
    CmpUseChecker c{inst.sf};
    visitOperands(code[i+1], c);
    return c.cc_result == true && c.use_result == true;
  }();
  if (!suitable_use) return false;

  return simplify_impl(
    env, b, i,
    [&] (Vout& v) {
      v << Out{r, r, inst.sf};
      return 1;
    }
  );
}

// Determine if either register in the instruction represents a constant zero,
// returning it. Returns an invalid register if neither does.
template <typename In>
auto get_cmp_zero_reg(Env& env, const In& inst) -> decltype(inst.s0) {
  auto const s0_it = env.unit.regToConst.find(inst.s0);
  auto const s1_it = env.unit.regToConst.find(inst.s1);
  if (s0_it != env.unit.regToConst.end() && s0_it->second.val == 0) {
    return inst.s1;
  } else if (s1_it != env.unit.regToConst.end() && s1_it->second.val == 0) {
    return inst.s0;
  } else {
    return decltype(inst.s0){Vreg::kInvalidReg};
  }
}

// Comparisons against another register

bool simplify(Env& env, const cmpb& inst, Vlabel b, size_t i) {
  auto const reg = get_cmp_zero_reg(env, inst);
  return reg.isValid() ? cmp_zero_impl<testb>(env, inst, reg, b, i) : false;
}

bool simplify(Env& env, const cmpl& inst, Vlabel b, size_t i) {
  auto const reg = get_cmp_zero_reg(env, inst);
  return reg.isValid() ? cmp_zero_impl<testl>(env, inst, reg, b, i) : false;
}

bool simplify(Env& env, const cmpq& inst, Vlabel b, size_t i) {
  auto const reg = get_cmp_zero_reg(env, inst);
  return reg.isValid() ? cmp_zero_impl<testq>(env, inst, reg, b, i) : false;
}

// Comparisons against literals

bool simplify(Env& env, const cmpbi& inst, Vlabel b, size_t i) {
  return (inst.s0.q() == 0)
    ? cmp_zero_impl<testb>(env, inst, inst.s1, b, i) : false;
}

bool simplify(Env& env, const cmpli& inst, Vlabel b, size_t i) {
  return (inst.s0.q() == 0)
    ? cmp_zero_impl<testl>(env, inst, inst.s1, b, i) : false;
}

bool simplify(Env& env, const cmpqi& inst, Vlabel b, size_t i) {
  return (inst.s0.q() == 0)
    ? cmp_zero_impl<testq>(env, inst, inst.s1, b, i) : false;
}

////////////////////////////////////////////////////////////////////////////////

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

bool simplify(Env& env, const movzlq& inst, Vlabel b, size_t i) {
  if (arch() != Arch::X64) return false;
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
