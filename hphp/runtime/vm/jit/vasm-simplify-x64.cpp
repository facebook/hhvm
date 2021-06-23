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

#include "hphp/runtime/vm/jit/vasm-simplify-internal.h"

#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-util.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

namespace HPHP { namespace jit { namespace x64 {

namespace {

///////////////////////////////////////////////////////////////////////////////

template <typename Inst>
bool simplify(Env&, const Inst& /*inst*/, Vlabel /*b*/, size_t /*i*/) {
  return false;
}

///////////////////////////////////////////////////////////////////////////////

bool simplify(Env& env, const andqi& vandqi, Vlabel b, size_t i) {
  return simplify_impl(env, b, i, [&] (Vout& v) {
    if (env.use_counts[vandqi.sf] == 0 && vandqi.s0.q() == 0xff) {
      // andqi{0xff, s, d} -> movzbq{s, d}
      auto const tmp = v.makeReg();
      v << movtqb{vandqi.s1, tmp};
      v << movzbq{tmp, vandqi.d};
      return 1;
    }
    return 0;
  });
}

////////////////////////////////////////////////////////////////////////////////
/*
 * Simplify equal/not-equal comparisons against zero into test instructions.
 *
 * Only perform this simplification if the comparison is immediately followed
 * by an instruction which uses the flag result with an equals or not-equals
 * condition code.  Furthermore, that must be the only usage of that flag
 * result.
 */

/*
 * Check if an instruction uses a particular flag register and contains an equal
 * or not-equal condition code.
 *
 * If more than one condition code or flag register is used, we don't match
 * against the instruction.  This is preferred over maintaining an explicit
 * list of allowed instructions.
 */
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
  template <class T, class H>
  void defHint(T /*r*/, H) {}
  template<class T> void use(T) {}
  template <class T, class H>
  void useHint(T /*r*/, H) {}
  template <class T>
  void across(T /*r*/) {}

  VregSF target;
  Optional<bool> cc_result;
  Optional<bool> use_result;
};

/*
 * Transform a cmp* instruction into a test* instruction if all the above
 * conditions are met.
 */
template <typename Out, typename In, typename Reg>
bool cmp_zero_impl(Env& env, const In& inst, Reg r, Vlabel b, size_t i) {
  if (env.use_counts[inst.sf] != 1) return false;

  auto const suitable_use = [&]{
    auto const& code = env.unit.blocks[b].code;
    if (i + 1 >= code.size()) return false;
    CmpUseChecker c{inst.sf};
    visitOperands(code[i+1], c);
    return c.cc_result == true && c.use_result == true;
  }();
  if (!suitable_use) return false;

  return simplify_impl(env, b, i, [&] (Vout& v) {
    v << Out{r, r, inst.sf};
    return 1;
  });
}

/*
 * Determine if either register in the instruction represents a constant zero
 * and return it.  Returns an invalid register if neither does.
 */
template <typename In>
auto get_cmp_zero_reg(Env& env, const In& inst) -> decltype(inst.s0) {
  auto const& consts = env.unit.regToConst;

  auto const s0_it = consts.find(inst.s0);
  auto const s1_it = consts.find(inst.s1);
  if (s0_it != consts.end() && s0_it->second.val == 0) {
    return inst.s1;
  } else if (s1_it != consts.end() && s1_it->second.val == 0) {
    return inst.s0;
  } else {
    return decltype(inst.s0){Vreg::kInvalidReg};
  }
}

/*
 * Comparisons against another register.
 */
bool simplify(Env& env, const cmpb& inst, Vlabel b, size_t i) {
  auto const reg = get_cmp_zero_reg(env, inst);
  return reg.isValid() ? cmp_zero_impl<testb>(env, inst, reg, b, i) : false;
}

bool simplify(Env& env, const cmpw& inst, Vlabel b, size_t i) {
  auto const reg = get_cmp_zero_reg(env, inst);
  return reg.isValid() ? cmp_zero_impl<testw>(env, inst, reg, b, i) : false;
}

bool simplify(Env& env, const cmpl& inst, Vlabel b, size_t i) {
  auto const reg = get_cmp_zero_reg(env, inst);
  return reg.isValid() ? cmp_zero_impl<testl>(env, inst, reg, b, i) : false;
}

bool simplify(Env& env, const cmpq& inst, Vlabel b, size_t i) {
  auto const reg = get_cmp_zero_reg(env, inst);
  return reg.isValid() ? cmp_zero_impl<testq>(env, inst, reg, b, i) : false;
}

/*
 * Comparisons against literals.
 */
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

// extract the bottom size bits of value as an int64_t, treating bit
// (size-1) as the sign bit, and avoiding undefined/unspecified
// behavior.
static int64_t extract_signed_value(uint64_t value, int size) {
  assertx(size && size <= std::numeric_limits<uint64_t>::digits);

  auto const value_mask = (int64_t{1} << (size - 1)) - 1;
  auto const sign_bit = -(int64_t{1} << (size - 1));
  int64_t val = value & value_mask;
  int64_t sgn = (value >> (size - 1)) & 1 ? sign_bit : 0;
  return val + sgn;
}

template<typename testi, typename Arg0, typename Arg1>
bool simplify_test_imm(Env& env, Arg0 r0, Arg1 r1, Vreg sf,
                       Vlabel b, size_t i) {
  auto const size = static_cast<int>(width(r0));
  assertx(1 <= size && size <= 8 && !(size & (size - 1)));
  auto const it = env.unit.regToConst.find(r0);
  if (it == env.unit.regToConst.end() ||
      it->second.isUndef ||
      it->second.kind == Vconst::Double) {
    return false;
  }
  // andqi/testqi only accepts 32-bit immediates, and will do sign extension.
  if (size == sz::qword && !deltaFits(it->second.val, sz::dword)) {
    return false;
  }
  const int val = extract_signed_value(it->second.val,
                                       size < sz::qword ? size * 8 : 32);
  return simplify_impl(env, b, i, testi{ val, r1, sf });
}

template<typename testi, typename test>
bool simplify_test_imm(Env& env, const test& vtest, Vlabel b, size_t i) {
  return
    simplify_test_imm<testi>(
      env, vtest.s0, vtest.s1, vtest.sf, b, i) ||
    simplify_test_imm<testi>(
      env, vtest.s1, vtest.s0, vtest.sf, b, i);
}

bool simplify(Env& env, const testq& test, Vlabel b, size_t i) {
  return simplify_test_imm<testqi>(env, test, b, i);
}

bool simplify(Env& env, const testl& test, Vlabel b, size_t i) {
  return simplify_test_imm<testli>(env, test, b, i);
}

bool simplify(Env& env, const testw& test, Vlabel b, size_t i) {
  return simplify_test_imm<testwi>(env, test, b, i);
}

bool simplify(Env& env, const testb& test, Vlabel b, size_t i) {
  return simplify_test_imm<testbi>(env, test, b, i);
}

bool simplify(Env& env, const testqm& test, Vlabel b, size_t i) {
  return simplify_test_imm<testqim>(env, test.s0, test.s1, test.sf , b, i);
}

bool simplify(Env& env, const testlm& test, Vlabel b, size_t i) {
  return simplify_test_imm<testlim>(env, test.s0, test.s1, test.sf, b, i);
}

bool simplify(Env& env, const testwm& test, Vlabel b, size_t i) {
  return simplify_test_imm<testwim>(env, test.s0, test.s1, test.sf, b, i);
}

bool simplify(Env& env, const testbm& test, Vlabel b, size_t i) {
  return simplify_test_imm<testbim>(env, test.s0, test.s1, test.sf, b, i);
}

///////////////////////////////////////////////////////////////////////////////

}

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

}}}
