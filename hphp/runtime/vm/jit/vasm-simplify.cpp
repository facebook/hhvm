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

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-info.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-util.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#include "hphp/util/arch.h"
#include "hphp/util/asm-x64.h"

#include <utility>

TRACE_SET_MOD(vasm);

namespace HPHP::jit {

namespace {

///////////////////////////////////////////////////////////////////////////////

enum class ResValid : uint8_t {
  Empty,
  Valid,
  Invalid
};

struct GetMemOp {
  template<class T> void imm (T) {}
  template<class T> void def (T) {}
  template<class T> void use (T) {}
  void use (Vptr mem) {
    if (rv != ResValid::Empty) {
      rv = ResValid::Invalid;
    } else {
      mr = mem;
      rv = ResValid::Valid;
    }
  }
  template<class T> void across (T) {}
  template<class T, class H> void useHint(T,H) {}
  template<class T, class H> void defHint(T,H) {}
  template<Width w> void use(Vp<w> m) { use(static_cast<Vptr>(m)); }

  bool isValid() { return rv == ResValid::Valid;}

  Vptr mr;
  ResValid rv{ResValid::Empty};
};

/*
 * Return the Vptr and size for simple insts that read/write memory.
 *
 * Return a size zero to indicate a non-simple inst, e.g. a call.
 */
std::pair<Vptr, uint8_t> getMemOpAndSize(const Vinstr& inst) {
  GetMemOp g;
  visitOperands(inst, g);

  auto const vop = g.mr;
  if (!g.isValid()) return {vop, 0};

  auto sz = width(inst.op);
  // workaround: load and store opcodes report a width of Octa,
  // while in reality they are Quad.
  if (inst.op == Vinstr::load || inst.op == Vinstr::store) {
    sz = Width::Quad;
  }
  sz &= Width::AnyNF;

  auto const size = [&] {
    switch (sz) {
      case Width::Byte:
        return sz::byte;
      case Width::Word: case Width::WordN:
        return sz::word;
      case Width::Long: case Width::LongN:
        return sz::dword;
      case Width::Quad: case Width::QuadN:
        return sz::qword;
      case Width::Octa: case Width::AnyNF:
        return 16;
      default: return 0;
    }
  }();
  return {vop, size};
}

/*
 * Return true if we are sure that `inst1' does not write to the same memory
 * location that `inst2' reads or writes (and vice versa).
 *
 * (Note that a false return does /not/ make any positive indication about
 * aliasing.)
 */
bool cannot_alias_write(const Vinstr& inst1, const Vinstr& inst2) {
  if (!writesMemory(inst1.op) && !writesMemory(inst2.op)) return true;
  auto const p1 = getMemOpAndSize(inst1);
  auto const p2 = getMemOpAndSize(inst2);
  if (p1.second == 0 || p2.second == 0) return false;
  auto const v1 = p1.first;
  auto const v2 = p2.first;
  auto const size1 = p1.second;
  auto const size2 = p2.second;
  if (v1.base != v2.base || v1.seg != v2.seg) return false;
  if ((v1.index != v2.index) ||
      (v1.index.isValid() && (v1.scale != v2.scale))) {
    return false;
  }
  if (v1.disp == v2.disp) return false;
  if (v1.disp < v2.disp) {
    return v2.disp >= v1.disp + size1;
  } else {
    return v1.disp >= v2.disp + size2;
  }
}

/*
 * Metadata about a potentially-foldable load Vinstr.
 */
struct FoldableLoadInfo {
  /*
   * Source memory location of the load.
   */
  const Vptr* operand;
  /*
   * Index into the instruction stream.
   *
   * Note that this makes FoldableLoadInfo block-context-sensitive.
   */
  size_t idx;
  /*
   * Whether we need to check for interfering writes to the same location.
   */
  bool check_writes;
};

/*
 * If reg is defined by a load in b with index j < i, and there are no
 * interfering stores between j and i, return a pointer to its Vptr argument.
 *
 * Note that during simplification, we can end up with mismatched register
 * sizes, so that a loadzbq feeds a byte sized instruction. In that case, its
 * ok to treat the loadzbq as if it were a loadb, provided we're expecting a
 * byte sized operand. Since Vreg doesn't carry a register-width around, we
 * pass it in via the `size' param, so that we can verify the zero-extend
 * variants.
 */
FoldableLoadInfo foldable_load_helper(Env& env, Vreg reg, int size,
                                      Vlabel b, size_t i, bool mw) {
  if (!i || env.use_counts[reg] != 1) return { nullptr, 0, false };

  auto const def_inst = env.def_insts[reg];
  switch (def_inst) {
    case Vinstr::loadb:
    case Vinstr::loadw:
    case Vinstr::loadl:
    case Vinstr::load:
      break;
    case Vinstr::loadzbq:
    case Vinstr::loadzbl:
      if (size > sz::byte) return { nullptr, 0, false };
      break;
    case Vinstr::loadzwq:
      if (size > sz::word) return { nullptr, 0, false };
      break;
    case Vinstr::loadzlq:
      if (size > sz::dword) return { nullptr, 0, false };
      break;
    case Vinstr::copy:
      break;
    default:
      return { nullptr, 0, false };
  }

#define CHECK_LOAD(load)                        \
  case Vinstr::load:                            \
    if (inst.load##_.d != reg) break;           \
    return { &inst.load##_.s, i, mw };

  while (i--) {
    auto const& inst = env.unit.blocks[b].code[i];
    if (inst.op == def_inst) {
      switch (def_inst) {
        CHECK_LOAD(loadb);
        CHECK_LOAD(loadw);
        CHECK_LOAD(loadl);
        CHECK_LOAD(load);
        CHECK_LOAD(loadzbq);
        CHECK_LOAD(loadzbl);
        CHECK_LOAD(loadzwq);
        CHECK_LOAD(loadzlq);
        case Vinstr::copy:
          if (inst.copy_.d != reg) break;
          return foldable_load_helper(env, inst.copy_.s, size, b, i, mw);
        default:
          not_reached();
      }
    }
    auto const op = env.unit.blocks[b].code[i].op;
    mw |= writesMemory(op);
  }

  return { nullptr, 0, false };
}

const Vptr* foldable_load(Env& env, Vreg reg, int size,
                          Vlabel b, size_t i) {
  auto const info = foldable_load_helper(env, reg, size, b, i, false);
  if (!info.operand || info.idx + 1 == i) return info.operand;

  std::vector<Vreg> nonSSARegs;
  visit(env.unit, *info.operand, [&] (Vreg r, Width) {
    if (r.isPhys()) nonSSARegs.push_back(r);
  });
  if (nonSSARegs.empty() && !info.check_writes) return info.operand;

  // The Vptr contains non-ssa regs, so we need to check that they're
  // not modified between the load and the use.
  auto const loadInst = env.unit.blocks[b].code[info.idx];
  for (auto ix = info.idx; ++ix < i; ) {
    auto const& inst = env.unit.blocks[b].code[ix];
    if (isCall(inst)) return nullptr;
    bool ok = true;
    if (!nonSSARegs.empty()) {
      visitDefs(env.unit, inst, [&] (Vreg r, Width) {
        for (auto const t : nonSSARegs) {
          if (t == r) ok = false;
        }
      });
      if (!ok) return nullptr;
    }
    if (info.check_writes && writesMemory(inst.op) &&
        !cannot_alias_write(loadInst, inst)) {
      return nullptr;
    }
  }
  return info.operand;
}

const Vptr* foldable_load(Env& env, Vreg8 reg, Vlabel b, size_t i) {
  return foldable_load(env, reg, sz::byte, b, i);
}

const Vptr* foldable_load(Env& env, Vreg16 reg, Vlabel b, size_t i) {
  return foldable_load(env, reg, sz::word, b, i);
}

const Vptr* foldable_load(Env& env, Vreg32 reg, Vlabel b, size_t i) {
  return foldable_load(env, reg, sz::dword, b, i);
}

const Vptr* foldable_load(Env& env, Vreg64 reg, Vlabel b, size_t i) {
  return foldable_load(env, reg, sz::qword, b, i);
}

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

template <typename Inst>
bool psimplify(Env&, const Inst& /*inst*/, Vlabel /*b*/, size_t /*i*/) {
  return false;
}

////////////////////////////////////////////////////////////////////////////////
/*
 * Narrow compares
 */

/*
 * Return a bound on the size of a Vreg.
 * If its known to fit in:
 *  - 8 unsigned bits, return sz::byte
 *  - 16 unsigned bits, return sz::word
 *  - 32 unsigned bits, return sz::dword
 *  - otherwise return sz::qword.
 */
int value_width(Env& env, Vreg reg) {
  if (auto const c = env.consts[reg]) {
    if (!c->isUndef && c->kind != Vconst::Double) {
      if (c->val <= 0xff)       return sz::byte;
      if (c->val <= 0xffff)     return sz::word;
      if (c->val <= 0xffffffff) return sz::dword;
    }
    return sz::qword;
  }

  switch (env.def_insts[reg]) {
    case Vinstr::loadzbq:
    case Vinstr::loadzbl:
    case Vinstr::movzbw:
    case Vinstr::movzbl:
    case Vinstr::movzbq:
      return sz::byte;

    case Vinstr::movzwl:
    case Vinstr::movzwq:
    case Vinstr::loadzwq:
      return sz::word;

    case Vinstr::movzlq:
    case Vinstr::loadzlq:
      return sz::dword;
    default:
      break;
  }

  return sz::qword;
}

/*
 * Return a suitable register for r, with width size.
 *
 *  - if r is constant, just return a constant.
 *  - if r is the result of a zero-extending move from the
 *    correct size, return the source of the move
 *  - if r is the result of a zero-extending load with one use,
 *    convert the load to a non-zero extending form
 *  - otherwise we should logically apply a movtq<sz> to the register,
 *    and return the dst. we don't check widths after simplify, however,
 *    and inserting them can prevent other patterns from being recognized,
 *    so just leave them out.
 */
Vreg narrow_reg(Env& env, int size, Vreg r, Vlabel b, size_t i) {
  if (auto const c = env.consts[r]) {
    assertx(!c->isUndef && c->kind != Vconst::Double);
    return r;
  }

  auto reg = [&] () -> Vreg {
    while (i--) {
      auto replace = [&] (const Vinstr& rep) {
        return simplify_impl(env, b, i, [&] (Vout& v) {
          v << rep;
          return 1;
        });
      };
      auto match = [&] (int rsz, size_t rn) {
        if (rsz != size) return Vreg{};
        if (env.use_counts[r] == 1) {
          replace(nop{});
        }
        return Vreg{ rn };
      };

      auto const& inst = env.unit.blocks[b].code[i];
      switch (inst.op) {
        case Vinstr::loadzbq: {
          auto const& load = inst.get<Vinstr::loadzbq>();
          if (load.d == r) {
            if (size == sz::byte && env.use_counts[r] == 1) {
              replace(loadb{load.s, r});
              return r;
            }
            return {};
          }
          break;
        }
        case Vinstr::loadzbl: {
          auto const& load = inst.get<Vinstr::loadzbl>();
          if (load.d == r) {
            if (size == sz::byte && env.use_counts[r] == 1) {
              replace(loadb{load.s, r});
              return r;
            }
            return {};
          }
          break;
        }
        case Vinstr::movzbw:
          if (inst.movzbw_.d == r) return match(sz::byte, inst.movzbw_.s);
          break;
        case Vinstr::movzbl:
          if (inst.movzbl_.d == r) return match(sz::byte, inst.movzbl_.s);
          break;
        case Vinstr::movzbq:
          if (inst.movzbq_.d == r) return match(sz::byte, inst.movzbq_.s);
          break;

        case Vinstr::movzwl:
          if (inst.movzwl_.d == r) return match(sz::word, inst.movzwl_.s);
          break;
        case Vinstr::movzwq:
          if (inst.movzwq_.d == r) return match(sz::word, inst.movzwq_.s);
          break;
        case Vinstr::loadzwq: {
          auto const& load = inst.get<Vinstr::loadzwq>();
          if (load.d == r) {
            if (size == sz::word && env.use_counts[r] == 1) {
              replace(loadw{load.s, r});
              return r;
            }
            return {};
          }
          break;
        }

        case Vinstr::movzlq:
          if (inst.movzlq_.d == r) return match(sz::dword, inst.movzlq_.s);
          break;

        case Vinstr::loadzlq: {
          auto const& load = inst.get<Vinstr::loadzlq>();
          if (load.d == r) {
            if (size == sz::dword && env.use_counts[r] == 1) {
              replace(loadl{load.s, r});
              return r;
            }
            return {};
          }
          break;
        }
        default:
          break;
      }
    }
    return {};
  }();

  return reg.isValid() ? reg : r;
}

template<typename instb, typename instw, typename instl, typename instq>
int narrow_inst(Env& env, int size, const instq& vinst, Vlabel b, size_t i,
                Vout& v) {
  auto const s0 = narrow_reg(env, size, vinst.s0, b, i);
  auto const s1 = narrow_reg(env, size, vinst.s1, b, i);
  switch (size) {
    case sz::byte:
      v << instb{s0, s1, vinst.sf};
      break;
    case sz::word:
      v << instw{s0, s1, vinst.sf};
      break;
    case sz::dword:
      v << instl{s0, s1, vinst.sf};
      break;
    default:
      always_assert(false);
  }
  return 1;
}

enum class SFUsage {
  Ok,
  Fixable,
  Unfixable
};

/*
 * Check whether all uses of sf are suitable, or can be converted to
 * suitable uses, as determined by fun.
 *
 * fun takes a CC as its argument, and returns
 *  - CC_None to indicate the usage is unsuitable
 *  - The input cc to indicate that its already suitable
 *  - Some other cc to indicate that we could convert to that.
 *
 * If fix is true, this will convert any uses as specified.
 */
template<class F>
SFUsage check_sf_usage_helper(Env& env, Vreg sf, Vlabel b, size_t i, bool fix,
                              F fun) {
  auto ret = SFUsage::Ok;
  auto uses = env.use_counts[sf];
  while (uses) {
    auto const& code = env.unit.blocks[b].code;
    if (++i == code.size()) return SFUsage::Unfixable;
    if (getSFUseReg(code[i]) == sf) {
      uses--;
      auto tmp = code[i];
      auto &cc = getConditionCode(tmp);
      auto const newCC = fun(cc);
      if (newCC == CC_None) {
        return SFUsage::Unfixable;
      }
      if (newCC != cc) {
        if (fix) {
          cc = newCC;
          simplify_impl(env, b, i, [&] (Vout& v) {
            v << tmp;
            return 1;
          });
        }
        ret = SFUsage::Fixable;
      }
    }
  }
  return ret;
}

template<class F>
bool check_sf_usage(Env& env, Vreg sf, Vlabel b, size_t i, F fun) {
  switch (check_sf_usage_helper(env, sf, b, i, false, fun)) {
    case SFUsage::Ok:
      return true;
    case SFUsage::Fixable:
      check_sf_usage_helper(env, sf, b, i, true, fun);
      return true;
    case SFUsage::Unfixable:
      return false;
  }
  not_reached();
}

// Try to change the condition codes so they work when the inputs to
// the compare are swapped. Return true if successful (or there was
// nothing to do).
bool flip_ordered_uses(Env& env, Vreg sf, Vlabel b, size_t i) {
  return check_sf_usage(
    env, sf, b, i,
    [] (ConditionCode cc) {
      switch (cc) {
        case CC_None:
          always_assert(false);
        case CC_O:
        case CC_NO:
          // can't be fixed
          return CC_None;
        case CC_B:  return CC_A;
        case CC_AE: return CC_BE;
        case CC_BE: return CC_AE;
        case CC_A:  return CC_B;

        case CC_E:
        case CC_NE:
          // already unordered
          return cc;
        case CC_S:
        case CC_NS:
        case CC_P:
        case CC_NP:
          // can't be fixed
          return CC_None;
        case CC_L:  return CC_G;
        case CC_GE: return CC_LE;
        case CC_LE: return CC_GE;
        case CC_G:  return CC_L;
      }
      not_reached();
    }
  );
}

// Change any signed conditions to the corresponding unsigned
// condition. Return true if successful (or there was nothing to do).
bool fix_signed_uses(Env& env, Vreg sf, Vlabel b, size_t i) {
  return check_sf_usage(
    env, sf, b, i,
    [] (ConditionCode cc) {
      switch (cc) {
        case CC_None:
          always_assert(false);
        case CC_O:
        case CC_NO:
          // can't be fixed
          return CC_None;
        case CC_B:
        case CC_AE:
        case CC_E:
        case CC_NE:
        case CC_BE:
        case CC_A:
          // already unsigned
          return cc;
        case CC_S:
        case CC_NS:
        case CC_P:
        case CC_NP:
          // can't be fixed
          return CC_None;
        case CC_L:  return CC_B;
        case CC_GE: return CC_AE;
        case CC_LE: return CC_BE;
        case CC_G:  return CC_A;
      }
      not_reached();
    }
  );
}

/*
 * Return a (Vptr*, idx in `b') handle to the first store of `reg' within block
 * `b' after position `i'.
 *
 * Returns { nullptr, 0 } if no such store is found, or if that store is not
 * the only use of `reg'.
 */
std::pair<const Vptr*, size_t>
storeq(Env& env, Vreg64 reg, Vlabel b, size_t i) {
  if (env.use_counts[reg] != 1) return { nullptr, 0 };

  while (++i < env.unit.blocks[b].code.size()) {
    auto const& inst = env.unit.blocks[b].code[i];
    if (inst.op == Vinstr::store && inst.store_.s == reg) {
      return { &(inst.store_.d), i };
    }
  }
  return { nullptr, 0 };
}

bool is_reg_const(Env& env, Vreg reg) {
  return env.consts[reg].has_value();
}

template <typename Op>
bool flip_operands_helper(Env& env, const Op& op, Vlabel b, size_t i) {
  // We want any constant register to be in the first operand, and any foldable
  // load to be in the second.
  if (!(is_reg_const(env, op.s0) || foldable_load(env, op.s1, b, i)) &&
      (is_reg_const(env, op.s1) || foldable_load(env, op.s0, b, i))) {
    if (flip_ordered_uses(env, op.sf, b, i)) {
      return simplify_impl(env, b, i, Op { op.s1, op.s0, op.sf });
    }
  }
  return false;
}

bool simplify(Env& env, const addq& vadd, Vlabel b, size_t i) {
  if (arch_any(Arch::ARM)) return false;

  auto stPair = storeq(env, vadd.d, b, i);
  auto const vptrs = stPair.first;
  auto const stIdx = stPair.second;

  if (vptrs) {
    auto const vptrl = foldable_load(env, vadd.s0, b, stIdx);
    if (vptrl && (*vptrl == *vptrs)) {
      bool ret = simplify_impl(env, b, i, addqrm { vadd.s1, *vptrs, vadd.sf });
      // need to do it here because a store will not be removed as dead code
      if (ret) env.unit.blocks[b].code[stIdx] = nop{};
      return ret;
    }
    auto const vptrl2 = foldable_load(env, vadd.s1, b, stIdx);
    if (vptrl2 && (*vptrl2 == *vptrs)) {
      bool ret = simplify_impl(env, b, i, addqrm { vadd.s0, *vptrs, vadd.sf });
      if (ret) env.unit.blocks[b].code[stIdx] = nop{};
      return ret;
    }
  }
  if (auto const vptr = foldable_load(env, vadd.s0, b, i)) {
    return simplify_impl(env, b, i, addqmr{*vptr, vadd.s1, vadd.d, vadd.sf});
  }
  if (auto const vptr = foldable_load(env, vadd.s1, b, i)) {
    return simplify_impl(env, b, i, addqmr{*vptr, vadd.s0, vadd.d, vadd.sf});
  }
  return false;
}

// find two lea that add an immediate to a register and fold into a
// single operation.  given this lea, look for a subsequent lea
// within the same block whose base reg is this lea's dest reg,
// and it is the only use of this lea's dest reg.
//
// first, try to simplify lea (%r1), %r2 into copy %r1,%r2
bool simplify(Env& env, const lea& vlea, Vlabel b, size_t i) {
  if (vlea.s.disp == 0 && !vlea.s.index.isValid() &&
      arch() != Arch::ARM && vlea.s.base.isValid()) {
    simplify_impl(env, b, i, copy { vlea.s.base, vlea.d });
    return true;
  }
  if (!vlea.s.index.isValid() && !vlea.s.base.isValid()) {
    simplify_impl(env, b, i, copy { env.unit.makeConst(vlea.s.disp), vlea.d });
    return true;
  }
  auto xinst = env.unit.blocks[b].code[i];
  bool found_second = false;
  size_t x;
  int disp2;
  if (vlea.d.isPhys()) {
    bool found_interf = false;
    for (x = i+1; x < env.unit.blocks[b].code.size(); ++x) {
      xinst = env.unit.blocks[b].code[x];
      if (xinst.op == Vinstr::lea && xinst.lea_.s.base == vlea.d &&
          xinst.lea_.d == vlea.d && !(xinst.lea_.s.index.isValid())) {
        found_second = true;
        disp2 = xinst.lea_.s.disp;
        break;
      } else {
        visitUses(env.unit, xinst,
            [&] (Vreg r) { if (r == vlea.d) found_interf = true; });
        if (found_interf) return false;
        visitDefs(env.unit, xinst,
            [&] (Vreg r) { if (r == vlea.d) found_interf = true; });
        if (found_interf) return false;
      }
    }
  } else {
    if (env.use_counts[vlea.d] == 1) {
      for (x = i+1 ; x < env.unit.blocks[b].code.size(); ++x) {
        xinst = env.unit.blocks[b].code[x];
        if ((xinst.op == Vinstr::lea) && (xinst.lea_.s.base == vlea.d) &&
              !xinst.lea_.s.index.isValid()) {
          found_second = true;
          disp2 = xinst.lea_.s.disp;
          break;
        }
      }
    }
  }
  if (found_second &&
      deltaFits((int64_t)(vlea.s.disp) + (int64_t)(disp2), sz::dword)) {
     (void) simplify_impl(env, b, i,
         lea { vlea.s+disp2, (xinst).lea_.d });
     // update uses and delete the inst
     (void) simplify_impl(env, b, x, [&] (Vout& v) { return 1; });
     return true;
  }
  return false;
}

// remove compares with unused results. This overlaps with removeDeadCode,
// but does it earlier.
template <typename Cmp>
bool simplify_dead_cmp(Env& env, const Cmp& cmp, Vlabel b, size_t i) {
  if (env.use_counts[cmp.sf] == 0) {
    return simplify_impl(env, b, i, nop{});
  }
  return false;
}

bool simplify(Env& env, const cmpq& vcmp, Vlabel b, size_t i) {
  if (simplify_dead_cmp(env, vcmp, b, i)) return true;
  if (flip_operands_helper(env, vcmp, b, i)) return true;

  if (!arch_any(Arch::ARM)) {
    if (auto const vptr = foldable_load(env, vcmp.s1, b, i)) {
      return simplify_impl(env, b, i, cmpqm { vcmp.s0, *vptr, vcmp.sf });
    }
    if (auto const vptr = foldable_load(env, vcmp.s0, b, i)) {
      if (flip_ordered_uses(env, vcmp.sf, b, i)) {
        return simplify_impl(env, b, i, cmpqm { vcmp.s1, *vptr, vcmp.sf });
      }
    }
  }

  auto const sz0 = value_width(env, vcmp.s0);
  if (sz0 == sz::qword) return false;
  auto const sz1 = value_width(env, vcmp.s1);
  if (sz1 == sz::qword) return false;

  auto const sz = sz1 > sz0 ? sz1 : sz0;

  if (!fix_signed_uses(env, vcmp.sf, b, i)) return false;

  return simplify_impl(env, b, i, [&] (Vout& v) {
    return narrow_inst<cmpb, cmpw, cmpl>(env, sz, vcmp, b, i, v);
  });
}

bool simplify(Env& env, const cmpl& vcmp, Vlabel b, size_t i) {
  if (simplify_dead_cmp(env, vcmp, b, i)) return true;
  if (flip_operands_helper(env, vcmp, b, i)) return true;
  if (arch_any(Arch::ARM)) return false;

  if (auto const vptr = foldable_load(env, vcmp.s1, b, i)) {
    return simplify_impl(env, b, i,
                         cmplm { vcmp.s0, *vptr, vcmp.sf });
  }
  return false;
}

bool simplify(Env& env, const cmpw& vcmp, Vlabel b, size_t i) {
  if (simplify_dead_cmp(env, vcmp, b, i))  return true;
  if (flip_operands_helper(env, vcmp, b, i)) return true;
  if (arch_any(Arch::ARM)) return false;

  if (auto const vptr = foldable_load(env, vcmp.s1, b, i)) {
    return simplify_impl(env, b, i,
                         cmpwm { vcmp.s0, *vptr, vcmp.sf });
  }
  return false;
}

bool simplify(Env& env, const cmpb& vcmp, Vlabel b, size_t i) {
  if (flip_operands_helper(env, vcmp, b, i)) return true;
  if (simplify_dead_cmp(env, vcmp, b, i)) return true;
  if (arch_any(Arch::ARM)) return false;

  if (auto const vptr = foldable_load(env, vcmp.s1, b, i)) {
    return simplify_impl(env, b, i,
                         cmpbm { vcmp.s0, *vptr, vcmp.sf });
  }
  return false;
}

bool simplify(Env& env, const cmpqi& vcmp, Vlabel b, size_t i) {
  if (simplify_dead_cmp(env, vcmp, b, i)) return true;
  if (arch_any(Arch::ARM)) return false;
  if (auto const vptr = foldable_load(env, vcmp.s1, b, i)) {
    return simplify_impl(env, b, i,
                         cmpqim { vcmp.s0, *vptr, vcmp.sf });
  }
  return false;
}

bool simplify(Env& env, const cmpli& vcmp, Vlabel b, size_t i) {
  if (simplify_dead_cmp(env, vcmp, b, i)) return true;
  if (arch_any(Arch::ARM)) return false;
  if (auto const vptr = foldable_load(env, vcmp.s1, b, i)) {
    return simplify_impl(env, b, i,
                         cmplim { vcmp.s0, *vptr, vcmp.sf });
  }
  return false;
}

bool simplify(Env& env, const cmpwi& vcmp, Vlabel b, size_t i) {
  if (simplify_dead_cmp(env, vcmp, b, i)) return true;
  if (arch_any(Arch::ARM)) return false;
  if (auto const vptr = foldable_load(env, vcmp.s1, b, i)) {
    return simplify_impl(env, b, i,
                         cmpwim { vcmp.s0, *vptr, vcmp.sf });
  }
  return false;
}

bool simplify(Env& env, const cmpbi& vcmp, Vlabel b, size_t i) {
  if (simplify_dead_cmp(env, vcmp, b, i)) return true;
  if (arch_any(Arch::ARM)) return false;
  if (auto const vptr = foldable_load(env, vcmp.s1, b, i)) {
    return simplify_impl(env, b, i,
                         cmpbim { vcmp.s0, *vptr, vcmp.sf });
  }
  return false;
}

bool simplify(Env& env, const cmpbim& vcmp, Vlabel b, size_t i) {
  return (simplify_dead_cmp(env, vcmp, b, i));
}

bool simplify(Env& env, const cmpwim& vcmp, Vlabel b, size_t i) {
  return (simplify_dead_cmp(env, vcmp, b, i));
}

bool simplify(Env& env, const cmplim& vcmp, Vlabel b, size_t i) {
  return (simplify_dead_cmp(env, vcmp, b, i));
}

bool simplify(Env& env, const cmpqim& vcmp, Vlabel b, size_t i) {
  return (simplify_dead_cmp(env, vcmp, b, i));
}

bool simplify(Env& env, const cmpbm& vcmp, Vlabel b, size_t i) {
  return (simplify_dead_cmp(env, vcmp, b, i));
}

bool simplify(Env& env, const cmpwm& vcmp, Vlabel b, size_t i) {
  return (simplify_dead_cmp(env, vcmp, b, i));
}

bool simplify(Env& env, const cmplm& vcmp, Vlabel b, size_t i) {
  return (simplify_dead_cmp(env, vcmp, b, i));
}

bool simplify(Env& env, const cmpqm& vcmp, Vlabel b, size_t i) {
  return (simplify_dead_cmp(env, vcmp, b, i));
}

// test sets C=O=0; shift sets them in unknown ways
// Z,S and P are set the same way by both.
bool fix_shift_test_flags(Env& env, Vreg sf, Vlabel b, size_t i) {
  return check_sf_usage(
    env, sf, b, i,
    [] (ConditionCode cc) {
      switch (cc) {
        case CC_None:
          always_assert(false);
        case CC_E:
        case CC_NE:
        case CC_S:
        case CC_NS:
          // only test Z and S, no change
          return cc;
        case CC_A:
          // C=0 && Z=0, but C is always zero
          return CC_NE;
        case CC_BE:
          // C=1 || Z=1, but C is always zero
          return CC_E;
        case CC_L:
          // S != OF, but OF is always zero
          return CC_S;
        case CC_GE:
          // S == OF, but OF is always zero
          return CC_NS;
        case CC_O:
        case CC_NO:
        case CC_P:
        case CC_NP:
        case CC_LE:
        case CC_G:
        case CC_AE:
        case CC_B:
          // can't be fixed
          return CC_None;
      }
      not_reached();
    }
  );
}

///////////////////////////////////////////////////////////////////////////////
/*
 * Test/And
 */

/*
 * If inst (assumed to be an instruction that writes an output
 * register d, and the status flags sf) is followed by a test
 * instruction, see if we can drop the test and just use inst's sf
 * flags.
 *
 * Depending on inst we may have to rewrite the uses of sf (see
 * fix_shift_test_flags), and for some uses, we may have to give
 * up. The caller provides fun to make these decisions.
 */
template<Vinstr::Opcode test, typename Inst, typename FlagsFunc>
bool simplifyInstTest(Env& env, const Inst& inst, Vlabel b, size_t i,
                      FlagsFunc fun) {
  if (env.use_counts[inst.sf]) return false;
  return if_inst<test>(
    env, b, i + 1,
    [&] (const op_type<test>& vtest) {
      if (inst.d != vtest.s0 ||
          inst.d != vtest.s1 ||
          !fun(vtest.sf)) {
        return false;
      }

      return simplify_impl(
        env, b, i,
        [&] (Vout& v) {
          auto repl = inst;
          repl.sf = vtest.sf;
          v << repl;
          return 2;
        }
      );
    }
  );
}

bool simplify(Env& env, const shrqi& vshr, Vlabel b, size_t i) {
  return simplifyInstTest<Vinstr::testq>(
    env, vshr, b, i,
    [&] (Vreg sf) { return fix_shift_test_flags(env, sf, b, i); }
  );
}

template<Vinstr::Opcode test, typename Test, typename And>
bool simplify_and(Env& env, const And& vand, Vlabel b, size_t i) {
  if (!env.use_counts[vand.d]) {
    return simplify_impl(env, b, i, Test{ vand.s0, vand.s1, vand.sf });
  }
  return simplifyInstTest<test>(env, vand, b, i, [] (Vreg) { return true; });
}

bool simplify(Env& env, const andq& vandq, Vlabel b, size_t i) {
  return simplify_and<Vinstr::testq, testq>(env, vandq, b, i);
}

bool simplify(Env& env, const andqi& vandqi, Vlabel b, size_t i) {
  return simplify_and<Vinstr::testq, testqi>(env, vandqi, b, i);
}

/*
 * Simplify masking values with -1 in andXi{}:
 *  andbi{0xff, s, d} -> copy{s, d}
 *  andwi{0xffff, s, d} -> copy{s, d}
 *  andli{0xffffffff, s, d} -> copy{s, d}
 */
template<Vinstr::Opcode test, typename testi, typename andi>
bool simplify_andi(Env& env, const andi& inst, Vlabel b, size_t i) {
  if (inst.s0.l() == -1 && env.use_counts[inst.sf] == 0) {
    return simplify_impl(env, b, i, copy{ inst.s1, inst.d });
  }
  return simplify_and<test, testi>(env, inst, b, i);
}

bool simplify(Env& env, const andbi& andbi, Vlabel b, size_t i) {
  return simplify_andi<Vinstr::testb, testbi>(env, andbi, b, i);
}

bool simplify(Env& env, const andwi& andwi, Vlabel b, size_t i) {
  return simplify_andi<Vinstr::testw, testwi>(env, andwi, b, i);
}

bool simplify(Env& env, const andli& andli, Vlabel b, size_t i) {
  return simplify_andi<Vinstr::testl, testli>(env, andli, b, i);
}

template<typename Out, typename Long, typename In>
bool simplify_signed_test(Env& env, const In& test, uint32_t val,
                          Vlabel b, size_t i) {
  if (val == 0x80000000 &&
      check_sf_usage(
        env, test.sf, b, i,
        [] (ConditionCode cc) {
          switch (cc) {
            case CC_None:
              always_assert(false);
            case CC_E:   return CC_NS;
            case CC_NE:  return CC_S;
            case CC_S:
            case CC_NS:
              return cc;

            case CC_A:
            case CC_BE:
            case CC_L:
            case CC_GE:
            case CC_O:
            case CC_NO:
            case CC_P:
            case CC_NP:
            case CC_LE:
            case CC_G:
            case CC_AE:
            case CC_B:
              // can't be fixed
              return CC_None;
          }
          not_reached();
        }
      ) &&
      simplify_impl(env, b, i, Out { test.s1, test.s1, test.sf })) {

    // This looks like it belongs in shrqi itself. The problem with
    // that is that it relies on the test optimizations already having
    // been done when we reach the shrqi. We could solve that by
    // iterating the simplify pass, but this should be cheaper, and
    // almost as good.
    if_inst<Vinstr::shrqi>(
      env, b, i - 1,
      [&] (const shrqi& vshr) {
        if (vshr.s0.l() != 32) return false;
        return simplify_impl(
          env, b, i - 1,
          [&] (Vout& v) {
            v << Long{ vshr.s1, vshr.s1, test.sf };
            return 2;
          }
        );
      }
    );
    return true;
  }

  return false;
}

template<typename testm, typename test>
bool simplify_testi(Env& env, const test& vtest, Vlabel b, size_t i) {
  if (arch_any(Arch::ARM)) return false;

  if (auto const vptr = foldable_load(env, vtest.s1, b, i)) {
    return simplify_impl(env, b, i, testm { vtest.s0, *vptr, vtest.sf });
  }

  return false;
}

template<typename testm, typename cmpm, typename test>
bool simplify_test(Env& env, const test& vtest, Vlabel b, size_t i) {
  if (arch_any(Arch::ARM)) return false;
  if (vtest.s0 == vtest.s1 && env.use_counts[vtest.s0] == 2) {
    env.use_counts[vtest.s0]--;
    auto const vptr = foldable_load(env, vtest.s0, b, i);
    env.use_counts[vtest.s0]++;
    if (vptr) {
      return simplify_impl(env, b, i, cmpm { 0, *vptr, vtest.sf });
    }
  }

  if (simplify_testi<testm>(env, vtest, b, i)) return true;
  if (auto const vptr = foldable_load(env, vtest.s0, b, i)) {
    return simplify_impl(env, b, i, testm { vtest.s1, *vptr, vtest.sf });
  }

  return false;
}

bool simplify(Env& env, const testqi& test, Vlabel b, size_t i) {
  return simplify_testi<testqim>(env, test, b, i);
}

bool simplify(Env& env, const testli& test, Vlabel b, size_t i) {
  if (simplify_testi<testlim>(env, test, b, i)) return true;

  return simplify_signed_test<testl, testq>(env, test, test.s0.l(), b, i);
}

bool simplify(Env& env, const testwi& test, Vlabel b, size_t i) {
  return simplify_testi<testwim>(env, test, b, i);
}

bool simplify(Env& env, const testbi& test, Vlabel b, size_t i) {
  return simplify_testi<testbim>(env, test, b, i);
}

template<int size, typename testim>
bool shrink_test_immediate(Env& env, uint64_t v, Vptr ptr, Vreg sf,
                           Vlabel b, size_t i) {
  if (!v) return simplify_impl(env, b, i, testbi{ 0, rarg(0), sf });

  auto getNewVal = [&] (uint64_t val, int bits, bool top_bits, Vreg sf)
      -> Optional<int> {
    auto const mask = (1LL << bits) - 1;
    auto const low = mask >> 1;
    val &= mask;
    if (val <= low) return val;
    // If we're not looking at the top bit of the original result, and
    // the top bit of the reduced mask is set, we'll have to give up
    // if anyone cares about the sign bit.
    if (!top_bits && !check_sf_usage(
      env, sf, b, i,
      [] (ConditionCode cc) {
        switch (cc) {
          case CC_None:
            always_assert(false);
          case CC_E:
          case CC_NE:
            return cc;
          case CC_S:
          case CC_NS:
          case CC_A:
          case CC_BE:
          case CC_L:
          case CC_GE:
          case CC_O:
          case CC_NO:
          case CC_P:
          case CC_NP:
          case CC_LE:
          case CC_G:
          case CC_AE:
          case CC_B:
            // can't be fixed
            return CC_None;
        }
        not_reached();
      })) {
      return std::nullopt;
    }

    return (val & low) - (mask - low);
  };

  if (size == 1) return false;
  if (size == 2) {
    if (!(v & 0xff)) {
      auto const newVal = getNewVal(v >> 8, 8, true, sf);
      return newVal && simplify_impl(
        env, b, i, testbim{ *newVal, ptr + 1, sf });
    }
    if (!(v & 0xff00)) {
      auto const newVal = getNewVal(v, 8, false, sf);
      return newVal && simplify_impl(
        env, b, i, testbim{ *newVal, ptr, sf });
    }
  }
  if (size == 4) {
    if (!(v & 0xffff)) {
      auto const newVal = getNewVal(v >> 16, 16, true, sf);
      return newVal && simplify_impl(
        env, b, i, testwim{ *newVal, ptr + 2, sf });
    }
    if (!(v & 0xffff0000)) {
      auto const newVal = getNewVal(v, 16, false, sf);
      return newVal && simplify_impl(
        env, b, i, testwim{ *newVal, ptr, sf });
    }
  }

  if (size == 8) {
    if (!(v & 0xffffffff)) {
      auto const newVal = getNewVal(v >> 32, 32, true, sf);
      return newVal && simplify_impl(
        env, b, i, testlim{ *newVal, ptr + 4, sf });
    }
    if (!(v & 0xffffffff00000000)) {
      auto const newVal = getNewVal(v, 32, false, sf);
      return newVal && simplify_impl(
        env, b, i, testlim{ *newVal, ptr, sf });
    }
  }

  return false;
}

template<int size, typename testim>
bool shrink_testim(Env& env, const testim& test, Vlabel b, size_t i) {
  return shrink_test_immediate<size, testim>(
    env, test.s0.q(), test.s1, test.sf, b, i
  );
}

bool simplify(Env& env, const testqim& test, Vlabel b, size_t i) {
  return shrink_testim<sz::qword>(env, test, b, i);
}

bool simplify(Env& env, const testlim& test, Vlabel b, size_t i) {
  return shrink_testim<sz::dword>(env, test, b, i);
}

bool simplify(Env& env, const testwim& test, Vlabel b, size_t i) {
  return shrink_testim<sz::word>(env, test, b, i);
}

bool simplify(Env& env, const testbim& test, Vlabel b, size_t i) {
  return shrink_testim<sz::byte>(env, test, b, i);
}

bool simplify(Env& env, const testq& test, Vlabel b, size_t i) {
  auto const sz0 = value_width(env, test.s0);
  auto const sz1 = value_width(env, test.s1);

  auto const size = sz1 < sz0 ? sz1 : sz0;
  if (size >= sz::qword) return false;

  return simplify_impl(env, b, i, [&] (Vout& v) {
    return narrow_inst<testb, testw, testl>(env, size, test, b, i, v);
  });

  return simplify_test<testqm, cmpqim>(env, test, b, i);
}

bool simplify(Env& env, const testl& test, Vlabel b, size_t i) {
  return simplify_test<testlm, cmplim>(env, test, b, i);
}

bool simplify(Env& env, const testw& test, Vlabel b, size_t i) {
  return simplify_test<testwm, cmpwim>(env, test, b, i);
}

bool simplify(Env& env, const testb& test, Vlabel b, size_t i) {
  return simplify_test<testbm, cmpbim>(env, test, b, i);
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

///////////////////////////////////////////////////////////////////////////////
/*
 * Or with constant values
 */

namespace {

template <typename Or>
bool implOrSimplify(
  Env& env, const Or& inst, Vlabel b, size_t i, size_t immed
) {
 if (env.use_counts[inst.sf] != 0) return false;
 if (immed == 0) return simplify_impl(env, b, i, copy{inst.s1, inst.d});

 auto const c = env.consts[inst.s1];
 if (!c || c->isUndef) return false;
 return simplify_impl(
   env, b, i,
   [&] (Vout& v) {
     auto const s = v.cns(immed | c->val);
     v << copy{s, inst.d};
     return 1;
   }
 );
}

} // namespace

bool simplify(Env& env, const orwi& inst, Vlabel b, size_t i) {
  auto const immed = inst.s0.w();
  return implOrSimplify(env, inst, b, i, immed);
}

bool simplify(Env& env, const orli& inst, Vlabel b, size_t i) {
  auto const immed = inst.s0.l();
  return implOrSimplify(env, inst, b, i, immed);
}

bool simplify(Env& env, const orqi& inst, Vlabel b, size_t i) {
  auto const immed = inst.s0.q();
  return implOrSimplify(env, inst, b, i, immed);
}

bool simplify(Env& env, const orq& inst, Vlabel b, size_t i) {
  if (env.use_counts[inst.sf] != 0) return false;

  auto const c0 = env.consts[inst.s0];
  auto const c1 = env.consts[inst.s1];
  if (c0 && !c0->isUndef) {
    if (c1 && !c1->isUndef) {
      return simplify_impl(env, b, i, [&] (Vout& v) {
        auto s = v.cns(c0->val | c1->val);
        v << copy{s, inst.d};
        return 1;
      });
    }
    if (c0->val == 0) {
      return simplify_impl(env, b, i, copy{inst.s1, inst.d});
    }
  } else if (c1 && c1->isUndef) {
    if (c1->val == 0) {
      return simplify_impl(env, b, i, copy{inst.s0, inst.d});
    }
  }
  return false;
}

bool simplify(Env& env, const orqim& inst, Vlabel b, size_t i) {
  if (inst.s0.q() == 0 && env.use_counts[inst.sf] == 0) {
    return simplify_impl(env, b, i, nop{});
  }
  return false;
}

/*
 * Fold a cmov of a certain width into a copy if both values are the same
 * register or have the same known constant value.
 */
template <typename Inst>
bool cmov_fold_impl(Env& env, const Inst& inst, Vlabel b, size_t i) {
  auto const equivalent = [&]{
    if (inst.t == inst.f) return true;
    auto const ct = env.consts[inst.t];
    auto const cf = env.consts[inst.f];
    if (!ct || !cf) return false;
    return *ct == *cf;
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
  auto const ct = env.consts[inst.t];
  auto const cf = env.consts[inst.f];
  if (!ct || !cf) return false;

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
  if (!check_const(*ct, t_val)) return false;
  bool f_val;
  if (!check_const(*cf, f_val)) return false;

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

/*
 * Simplify load followed by truncation:
 *  load{s, tmp}; jmpr{tmp, args} -> jmpm{s, args}
 *  load{s, tmp}; movtqb{tmp, d} -> loadtqb{s, d}
 *  load{s, tmp}; movtql{tmp, d} -> loadtql{s, d}
 *  loadzbq{s, tmp}; movtqb{tmp, d} -> loadb{s, d}
 *  loadzwq{s, tmp}; movtqw{tmp, d} -> loadw{s, d}
 *  loadzlq{s, tmp}; movtql{tmp, d} -> loadl{s, d}
 */
bool simplify_load_jmpr(Env& env, const load& load, Vlabel b, size_t i) {
  if (arch() == Arch::ARM) return false;
  if (env.use_counts[load.d] != 1) return false;
  auto const& code = env.unit.blocks[b].code;
  if (i + 1 >= code.size()) return false;

  return if_inst<Vinstr::jmpr>(env, b, i + 1, [&] (const jmpr& jmpr) {
    if (load.d != jmpr.target) return false;
    return simplify_impl(env, b, i, [&] (Vout& v) {
      v << jmpm{load.s, jmpr.args};
      return 2;
    });
  });
}

template<Vinstr::Opcode mov_op, typename loadt, typename load_in>
bool simplify_load_truncate(Env& env, const load_in& load, Vlabel b, size_t i) {
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
    simplify_load_jmpr(env, load, b, i) ||
    simplify_load_truncate<Vinstr::movtqb, loadtqb>(env, load, b, i) ||
    simplify_load_truncate<Vinstr::movtql, loadtql>(env, load, b, i);
}

bool simplify(Env& env, const loadzbq& load, Vlabel b, size_t i) {
  return simplify_load_truncate<Vinstr::movtqb, loadb>(env, load, b, i);
}

bool simplify(Env& env, const loadzwq& load, Vlabel b, size_t i) {
  return simplify_load_truncate<Vinstr::movtqw, loadw>(env, load, b, i);
}

bool simplify(Env& env, const loadzlq& load, Vlabel b, size_t i) {
  return simplify_load_truncate<Vinstr::movtql, loadl>(env, load, b, i);
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

struct regWidth {
  explicit regWidth(Vreg r) { givenReg = r; };
  template<class T> void imm (T) {}
  template<class T> void def (T) {}
  template<class T> void use (T) {}
  void use (Vptr ptr) {
    if (givenReg == ptr.base) {
      has_vptr = true;
      w = Width::Quad;
    }
    if (givenReg == ptr.index) {
      has_vptr = true;
      w = Width::Quad;
    }
  }
  void use (Vreg8 r) { if (!has_vptr && r == givenReg) w = Width::Byte; }
  void use (Vreg16 r) { if (!has_vptr && r == givenReg) w = Width::Word; }
  void use (Vreg32 r) { if (!has_vptr && r == givenReg) w = Width::Long; }
  void use (Vreg64 r) { if (r == givenReg) w = Width::Quad; }
  void use (Vreg r) { if (r == givenReg) w = Width::Quad; }
  void use (RegXMM r) { if (r == givenReg) w = Width::Octa; }

  template<class T> void across (T) {}
  template<class T, class H> void useHint(T s,H d) { use(s); }
  template<class T, class H> void defHint(T,H) {}
  template<Width w> void use(Vp<w>& m) { use(static_cast<Vptr>(m)); }

  Vreg givenReg;
  Width w {Width::None};
  bool has_vptr {false};
};

Width usedWidth(Vreg reg, Vinstr inst) {
  regWidth v{reg};
  visitOperands(inst,v);
  if (v.has_vptr) return Width::Quad;
  if (v.w != Width::None) return v.w;
  return Width::None;
}

// change  loadb into a zero extending load of the dest reg is used as a wide
// reg.  Do it only if there is a use of the reg is a wider width.
// When there is an assignment to a byte reg followed by a use of the full reg,
// the HW has is merge the new 8 bits with the old 24 / 56 bits, which is a penalty
// in all HW we use. The magnitude of the penalty is HW specific, and currently it is not
// huge. Most HW does it via an injection of a micro operation.
// If there is no wide use then a loadb is preferable to a loadzx because the encoding is
// smaller. So only do this if we can find the wide use.
// If there is another def after the loadb then it eliminates the penalty, and therefore
// we avoid the transformation.
// Do not do this is there already is a wide store to this reg.
// register allocation generates register copies and creates opportunities for
// this optimization.

bool psimplify(Env& env, const loadb& vldb, Vlabel b, size_t i) {
  bool found_wide_use = false;
  bool found_def = false;
  Vreg wide_reg;
  for (auto x = i + 1; x < env.unit.blocks[b].code.size(); ++x) {
    const auto xinst = env.unit.blocks[b].code[x];
    if (Vinstr::phpret == xinst.op) continue;
    visitDefs(env.unit, xinst, [&] (Vreg r) {
      if (r == vldb.d) {
        found_def = true;
        return;
      }
    });
    if (found_def) return false;
    visitUses(env.unit, xinst, [&] (Vreg r) {
      if (r == vldb.d) {
        Width w = usedWidth(r, xinst);
        if ((Width::Long == w) || (Width::Quad == w)) {
          wide_reg = r;
          found_wide_use = true;
        }
      }
    });
    if (found_wide_use) {
      auto const zinst = loadzbl { vldb.s, wide_reg };
      return vmodify(env.unit, b, i, [&] (Vout& v) { v << zinst; return 1; } );
    }
  }
  return false;
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

bool simplify(Env& env, const orlim& vorlim, Vlabel b, size_t i) {
  auto const orinst = env.unit.blocks[b].code[i];
  auto orOperand = orinst.orlim_.s0.l();
  const Vptr32 orDest = orinst.orlim_.m;
  for (int x = i - 1; x >= 0; --x) {
    auto xinst = env.unit.blocks[b].code[x];
    if (Vinstr::storeli == xinst.op) {
      const Vptr32 stDest = xinst.storeli_.m;
      if (stDest == orDest) {
        for (auto j = x+1; j < i; ++j) {
          if (!cannot_alias_write(env.unit.blocks[b].code[j],xinst)) {
            return false;
          }
        }
        const auto stOperand = xinst.storeli_.s.l();
        auto newOp = stOperand | orOperand;
        return simplify_impl(env, b, i, storeli { newOp, stDest });
      }
    } else if (Vinstr::storel == xinst.op) {
        const auto srcReg = xinst.storel_.s;
        if (auto const c = env.consts[srcReg]) {
          const Vptr32 stDest = xinst.storel_.m;
          if (orDest == stDest) {
            for (auto j = x + 1; j < i; ++j) {
              if (!cannot_alias_write(env.unit.blocks[b].code[j],xinst)) {
                return false;
              }
            }
            const auto stOperand = c->val;
            const int newOp = stOperand | orOperand;
            return simplify_impl(env, b, i, storeli { newOp, stDest });
          }
        }
    }
  }
  return false;
}

///////////////////////////////////////////////////////////////////////////////

// Try to do constant folding on Vptr operands.

struct SimplifyVptrVisit {
  explicit SimplifyVptrVisit(Env& env) : env{env} {}
  template<class T> void def(T) {}
  template<class T> void imm(const T&) {}
  template<class T> void across(T) {}
  template<class T, class H> void defHint(T, H) {}
  template<class T, class H> void useHint(T r, H) { use(r); }
  template <typename T> void use(T) {}

  void use(Vptr& ptr) {
    // If the base is a constant, we can fold it into the displacement (if it
    // fits).
    if (ptr.base.isValid()) {
      if (auto const c = env.consts[ptr.base]) {
        auto const newDisp = static_cast<int64_t>(ptr.disp) + c->val;
        if (deltaFits(newDisp, sz::dword)) {
          ptr.disp = newDisp;
          ptr.base = Vreg{};
          changed = true;
        }
      }
    }

    // If the index is a constant, we can fold it into the displacement (if it
    // fits).
    if (ptr.index.isValid()) {
      if (auto const c = env.consts[ptr.index]) {
        auto const newDisp =
          static_cast<int64_t>(ptr.disp) + c->val * ptr.scale;
        if (deltaFits(newDisp, sz::dword)) {
          ptr.disp = newDisp;
          ptr.index = Vreg{};
          ptr.scale = 1;
          changed = true;
        }
      }
    }
  }

  Env& env;
  bool changed = false;
};

bool simplify_vptr(Env& env, Vinstr& inst) {
  SimplifyVptrVisit v{env};
  visitOperands(inst, v);
  return v.changed;
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Perform peephole simplification at instruction `i' of block `b'.
 *
 * Return true if changes were made, else false.
 */
bool simplify(Env& env, Vlabel b, size_t i) {
  assertx(i <= env.unit.blocks[b].code.size());
  auto& inst = env.unit.blocks[b].code[i];

  auto const changed = simplify_vptr(env, inst);

  switch (inst.op) {
#define O(name, ...)    \
    case Vinstr::name:  \
      return simplify(env, inst.name##_, b, i) || changed; \

    VASM_OPCODES
#undef O
  }
  not_reached();
}

bool psimplify(Env& env, Vlabel b, size_t i) {
  assertx(i <= env.unit.blocks[b].code.size());
  auto& inst = env.unit.blocks[b].code[i];

  switch (inst.op) {
#define O(name, ...)    \
    case Vinstr::name:  \
      return psimplify(env, inst.name##_, b, i); \

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
  env.consts.resize(unit.next_vr);

  auto const labels = sortBlocks(unit);

  // Set up Env, only visiting reachable blocks.
  for (auto const b : labels) {
    assertx(!blocks[b].code.empty());

    for (auto const& inst : blocks[b].code) {
      visitDefs(unit, inst, [&] (Vreg r) { env.def_insts[r] = inst.op; });
      visitUses(unit, inst, [&] (Vreg r) { ++env.use_counts[r]; });
    }
  };
  for (auto const& p : env.unit.regToConst) {
    env.consts[p.first] = p.second;
  }

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

/*
 * Peephole simplification pass after register allocation, for opportunities
 * that either require physical regs or are created by register allocator.
 */
void postRASimplify(Vunit& unit) {
  assertx(check(unit));
  auto& blocks = unit.blocks;

  Env env { unit };
  auto const labels = sortBlocks(unit);

  // The simplify() implementations may allocate scratch blocks and modify
  // instruction streams, so we cannot use standard iterators here.
  for (auto const b : labels) {
    for (size_t i = 0; i < blocks[b].code.size(); ++i) {
      // Simplify at this index until no changes are made.
      while (psimplify(env, b, i)) {
        // Stop if we simplified away the tail of the block.
        if (i >= blocks[b].code.size()) break;
      }
    }
  };

  printUnit(kVasmSimplifyLevel, "after vasm postRASimplify", unit);
}

///////////////////////////////////////////////////////////////////////////////

}
