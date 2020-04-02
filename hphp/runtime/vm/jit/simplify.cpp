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

#include "hphp/runtime/vm/jit/simplify.h"

#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/array-init.h"
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/double-to-int64.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/packed-array-defs.h"
#include "hphp/runtime/base/repo-auth-type-array.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/base/type-structure-helpers.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/runtime.h"

#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/ir-builder.h"
#include "hphp/runtime/vm/jit/minstr-effects.h"
#include "hphp/runtime/vm/jit/simple-propagation.h"
#include "hphp/runtime/vm/jit/translator-runtime.h"
#include "hphp/runtime/vm/jit/type-array-elem.h"
#include "hphp/runtime/vm/jit/type.h"

#include "hphp/runtime/ext/hh/ext_hh.h"
#include "hphp/runtime/ext/asio/ext_static-wait-handle.h"

#include "hphp/util/overflow.h"
#include "hphp/util/trace.h"

#include <limits>
#include <sstream>
#include <type_traits>

namespace HPHP { namespace jit {

TRACE_SET_MOD(simplify);

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_Array("Array");
const StaticString s_Vec("Vec");
const StaticString s_Dict("Dict");
const StaticString s_Keyset("Keyset");
const StaticString s_isEmpty("isEmpty");
const StaticString s_count("count");
const StaticString s_1("1");
const StaticString s_invoke("__invoke");
const StaticString s_isFinished("isFinished");
const StaticString s_isSucceeded("isSucceeded");
const StaticString s_isFailed("isFailed");
const StaticString s_Awaitable("HH\\Awaitable");

//////////////////////////////////////////////////////////////////////

struct State {
  explicit State(IRUnit& unit) : unit(unit) {}
  IRUnit& unit;

  // The current instruction being simplified is always at insts.top(). This
  // has to be a stack instead of just a pointer because simplify is reentrant.
  jit::stack<const IRInstruction*> insts;
  jit::vector<IRInstruction*> newInsts;
};

//////////////////////////////////////////////////////////////////////

SSATmp* simplifyWork(State&, const IRInstruction*);

template<class... Args>
SSATmp* cns(State& env, Args&&... cns) {
  return env.unit.cns(std::forward<Args>(cns)...);
}

template<class... Args>
SSATmp* gen(State& env, Opcode op, BCContext bcctx, Args&&... args) {
  return makeInstruction(
    [&] (IRInstruction* inst) -> SSATmp* {
      auto const prevNewCount = env.newInsts.size();
      auto const newDest = simplifyWork(env, inst);

      // If any simplification happened to this instruction, drop it. We have to
      // check that nothing was added to newInsts because that's the only way
      // we can tell simplification happened to a no-dest instruction.
      if (newDest || env.newInsts.size() != prevNewCount) {
        return newDest;
      } else {
        assertx(inst->isTransient());
        inst = env.unit.clone(inst);
        env.newInsts.push_back(inst);

        return inst->dst(0);
      }
    },
    op,
    bcctx,
    std::forward<Args>(args)...
  );
}

template<class... Args>
SSATmp* gen(State& env, Opcode op, Args&&... args) {
  assertx(!env.insts.empty());
  return gen(env, op, env.insts.top()->bcctx(), std::forward<Args>(args)...);
}

bool arrayKindNeedsVsize(const ArrayData::ArrayKind kind) {
  return kind == ArrayData::kGlobalsKind;
}

//////////////////////////////////////////////////////////////////////

DEBUG_ONLY bool validate(const State& env,
                         SSATmp* newDst,
                         const IRInstruction* origInst) {
  // simplify() rules are not allowed to add new uses to SSATmps that aren't
  // known to be available.  All the sources to the original instruction must
  // be available, and non-reference counted values reachable through the
  // source chain are also always available.  Anything else requires more
  // complicated analysis than belongs in the simplifier right now.
  auto known_available = [&] (SSATmp* src) -> bool {
    if (!src->type().maybe(TCounted)) return true;
    for (auto& oldSrc : origInst->srcs()) {
      if (oldSrc == src) return true;

      // Some instructions consume a counted SSATmp and produce a new SSATmp
      // which supports the consumed location. If the result of one such
      // instruction is available then the value whose count it supports must
      // also be available. For now CreateSSWH is the only instruction of this
      // form that we care about.
      if (oldSrc->inst()->is(CreateSSWH) && oldSrc->inst()->src(0) == src) {
        return true;
      }
    }
    return false;
  };

  // Return early for the no-simplification case.
  if (env.newInsts.empty() && !newDst) {
    return true;
  }

  const IRInstruction* last = nullptr;

  if (!env.newInsts.empty()) {
    for (size_t i = 0, n = env.newInsts.size(); i < n; ++i) {
      auto const newInst = env.newInsts[i];

      for (auto& src : newInst->srcs()) {
        always_assert_flog(
          known_available(src),
          "A simplification rule produced an instruction that used a value "
          "that wasn't known to be available:\n"
          "  original inst: {}\n"
          "  new inst:      {}\n"
          "  src:           {}\n",
          origInst->toString(),
          newInst->toString(),
          src->toString()
        );
      }

      if (i == n - 1) {
        last = newInst;
        continue;
      }

      always_assert_flog(
        !newInst->isBlockEnd(),
        "Block-ending instruction produced in the middle of a simplified "
        "instruction stream:\n"
        "  original inst: {}\n"
        "  new inst:      {}\n",
        origInst->toString(),
        newInst->toString()
      );
    }
  }

  if (newDst) {
    const bool available = known_available(newDst) ||
      std::any_of(env.newInsts.begin(), env.newInsts.end(),
                  [&] (IRInstruction* inst) { return newDst == inst->dst(); });

    always_assert_flog(
      available,
      "simplify() produced a new destination that wasn't known to be "
      "available:\n"
      "  original inst: {}\n"
      "  new dst:       {}\n",
      origInst->toString(),
      newDst->toString()
    );
  }

  if (!last) return true;

  auto assert_last = [&] (bool cond, const char* msg) {
    always_assert_flog(
      cond,
      "{}:\n"
      "  original inst: {}\n"
      "  last new inst: {}\n",
      msg,
      origInst->toString(),
      last->toString()
    );
  };

  assert_last(
    !origInst->naryDst(),
    "Nontrivial simplification returned for instruction with NaryDest"
  );

  assert_last(
    origInst->hasDst() == (newDst != nullptr),
    "HasDest mismatch between input and output"
  );

  if (last->hasEdges()) {
    assert_last(
      origInst->hasEdges(),
      "Instruction with edges produced for simplification of edge-free "
      "instruction"
    );

    assert_last(
      IMPLIES(last->next(), last->next() == origInst->next() ||
                            last->next() == origInst->taken()),
      "Last instruction of simplified stream has next edge not reachable from "
      "the input instruction"
    );

    assert_last(
      IMPLIES(last->taken(), last->taken() == origInst->next() ||
                             last->taken() == origInst->taken()),
      "Last instruction of simplified stream has taken edge not reachable "
      "from the input instruction"
    );
  }

  return true;
}

//////////////////////////////////////////////////////////////////////

/*
 * Individual simplification routines return nullptr if they don't want to
 * change anything, or they can call gen any number of times to produce a
 * different IR sequence, returning the thing gen'd that should be used as the
 * value of the simplified instruction sequence.
 */

SSATmp* mergeBranchDests(State& env, const IRInstruction* inst) {
  // Replace a conditional branch with a Jmp if both branches go to the same
  // block. Only work if the instruction does not have side effect.
  // JmpZero/JmpNZero is handled separately.
  assertx(inst->is(CheckTypeMem,
                   CheckLoc,
                   CheckStk,
                   CheckMBase,
                   CheckInit,
                   CheckInitMem,
                   CheckRDSInitialized,
                   CheckPackedArrayDataBounds,
                   CheckMixedArrayKeys,
                   CheckMixedArrayOffset,
                   CheckDictOffset,
                   CheckKeysetOffset));
  if (inst->next() != nullptr && inst->next() == inst->taken()) {
    return gen(env, Jmp, inst->next());
  }
  return nullptr;
}

SSATmp* simplifyEqFunc(State& env, const IRInstruction* inst) {
  auto const src0 = inst->src(0);
  auto const src1 = inst->src(1);
  if (src0->hasConstVal() && src1->hasConstVal()) {
    return cns(env, src0->funcVal() == src1->funcVal());
  }
  return nullptr;
}

SSATmp* simplifyFuncHasAttr(State& env, const IRInstruction* inst) {
  auto const funcTmp = inst->src(0);
  return funcTmp->hasConstVal(TFunc)
    ? cns(env, (funcTmp->funcVal()->attrs() & inst->extra<AttrData>()->attr))
    : nullptr;
}

SSATmp* simplifyIsClsDynConstructible(State& env, const IRInstruction* inst) {
  auto const clsTmp = inst->src(0);
  return clsTmp->hasConstVal(TCls)
    ? cns(env, clsTmp->clsVal()->isDynamicallyConstructible())
    : nullptr;
}

SSATmp* simplifyLdFuncRxLevel(State& env, const IRInstruction* inst) {
  auto const funcTmp = inst->src(0);
  return funcTmp->hasConstVal(TFunc)
    ? cns(env, funcTmp->funcVal()->rxLevel())
    : nullptr;
}

SSATmp* simplifyLdCls(State& env, const IRInstruction* inst) {
  if (inst->src(0)->inst()->is(LdClsName)) return inst->src(0)->inst()->src(0);
  return nullptr;
}

SSATmp* simplifyLdClsMethod(State& env, const IRInstruction* inst) {
  auto const clsTmp = inst->src(0);
  auto const idxTmp = inst->src(1);

  if (clsTmp->hasConstVal() && idxTmp->hasConstVal()) {
    auto const cls = clsTmp->clsVal();
    auto const idx = idxTmp->intVal();
    if (idx < cls->numMethods()) {
      return cns(env, cls->getMethod(idx));
    }
  }

  return nullptr;
}

SSATmp* simplifyLdObjClass(State& env, const IRInstruction* inst) {
  auto const ty = inst->src(0)->type();

  if (!(ty < TObj)) return nullptr;

  if (auto const exact = ty.clsSpec().exactCls()) return cns(env, exact);
  return nullptr;
}

SSATmp* simplifyLdFrameCls(State& env, const IRInstruction* inst) {
  auto const ty = inst->typeParam();

  if (!(ty < TCls)) return nullptr;

  if (auto const cls = ty.clsSpec().exactCls()) {
    return cns(env, cls);
  }

  return nullptr;
}

SSATmp* simplifyLdObjInvoke(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (!src->hasConstVal(TCls)) return nullptr;

  auto const meth = src->clsVal()->getCachedInvoke();
  return meth == nullptr ? nullptr : cns(env, meth);
}

SSATmp* simplifyMov(State& /*env*/, const IRInstruction* inst) {
  return inst->src(0);
}

SSATmp* simplifyAbsDbl(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->hasConstVal()) return cns(env, fabs(src->dblVal()));

  return nullptr;
}

template<class Oper>
SSATmp* constImpl(State& env, SSATmp* src1, SSATmp* src2, Oper op) {
  // don't canonicalize to the right, OP might not be commutative
  if (!src1->hasConstVal() || !src2->hasConstVal()) return nullptr;

  auto both = [&](Type ty) { return src1->type() <= ty && src2->type() <= ty; };

  if (both(TBool)) return cns(env, op(src1->boolVal(), src2->boolVal()));
  if (both(TInt))  return cns(env, op(src1->intVal(), src2->intVal()));
  if (both(TDbl))  return cns(env, op(src1->dblVal(), src2->dblVal()));
  return nullptr;
}

template<class Oper>
SSATmp* commutativeImpl(State& env,
                        SSATmp* src1,
                        SSATmp* src2,
                        Opcode opcode,
                        Oper op) {
  if (auto simp = constImpl(env, src1, src2, op)) return simp;

  // Canonicalize constants to the right.
  if (src1->hasConstVal() && !src2->hasConstVal()) {
    return gen(env, opcode, src2, src1);
  }

  // Only handle integer operations for now.
  if (!src1->isA(TInt) || !src2->isA(TInt)) return nullptr;

  auto const inst1 = src1->inst();
  auto const inst2 = src2->inst();
  if (inst1->op() == opcode && inst1->src(1)->hasConstVal()) {
    // (X + C1) + C2 --> X + C3
    if (src2->hasConstVal()) {
      auto const right = op(inst1->src(1)->intVal(), src2->intVal());
      return gen(env, opcode, inst1->src(0), cns(env, right));
    }
    // (X + C1) + (Y + C2) --> X + Y + C3
    if (inst2->op() == opcode && inst2->src(1)->hasConstVal()) {
      auto const right = op(inst1->src(1)->intVal(), inst2->src(1)->intVal());
      auto const left = gen(env, opcode, inst1->src(0), inst2->src(0));
      return gen(env, opcode, left, cns(env, right));
    }
  }
  return nullptr;
}

/*
 * Assumes that outop is commutative, don't use with subtract!
 *
 * Assumes that the values we're going to add new uses to are not reference
 * counted.  (I.e. this is a distributive FooInt opcode.)
 */
template <class OutOper, class InOper>
SSATmp* distributiveImpl(State& env, SSATmp* src1, SSATmp* src2, Opcode outcode,
                         Opcode incode, OutOper outop, InOper /*inop*/) {
  if (auto simp = commutativeImpl(env, src1, src2, outcode, outop)) {
    return simp;
  }

  auto const inst1 = src1->inst();
  auto const inst2 = src2->inst();
  auto const op1 = inst1->op();
  auto const op2 = inst2->op();
  // all combinations of X * Y + X * Z --> X * (Y + Z)
  if (op1 == incode && op2 == incode) {
    if (inst1->src(0) == inst2->src(0)) {
      auto const fold = gen(env, outcode, inst1->src(1), inst2->src(1));
      return gen(env, incode, inst1->src(0), fold);
    }
    if (inst1->src(0) == inst2->src(1)) {
      auto const fold = gen(env, outcode, inst1->src(1), inst2->src(0));
      return gen(env, incode, inst1->src(0), fold);
    }
    if (inst1->src(1) == inst2->src(0)) {
      auto const fold = gen(env, outcode, inst1->src(0), inst2->src(1));
      return gen(env, incode, inst1->src(1), fold);
    }
    if (inst1->src(1) == inst2->src(1)) {
      auto const fold = gen(env, outcode, inst1->src(0), inst2->src(0));
      return gen(env, incode, inst1->src(1), fold);
    }
  }
  return nullptr;
}

SSATmp* simplifyAddInt(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);

  auto add = std::plus<uint64_t>();
  auto mul = std::multiplies<uint64_t>();
  if (auto simp = distributiveImpl(env, src1, src2, AddInt,
                                   MulInt, add, mul)) {
    return simp;
  }
  if (src2->hasConstVal()) {
    int64_t src2Val = src2->intVal();
    // X + 0 --> X
    if (src2Val == 0) return src1;

    // X + -C --> X - C
    // Weird, but can show up as a result of other simplifications.
    auto const min = std::numeric_limits<int64_t>::min();
    if (src2Val < 0 && src2Val > min) {
      return gen(env, SubInt, src1, cns(env, -src2Val));
    }
  }
  // X + (0 - Y) --> X - Y
  auto const inst2 = src2->inst();
  if (inst2->op() == SubInt) {
    auto const src = inst2->src(0);
    if (src->hasConstVal() && src->intVal() == 0) {
      return gen(env, SubInt, src1, inst2->src(1));
    }
  }
  auto const inst1 = src1->inst();

  // (X - C1) + ...
  if (inst1->op() == SubInt && inst1->src(1)->hasConstVal()) {
    auto const x = inst1->src(0);
    auto const c1 = inst1->src(1);

    // (X - C1) + C2 --> X + (C2 - C1)
    if (src2->hasConstVal()) {
      auto const rhs = gen(env, SubInt, cns(env, src2->intVal()), c1);
      return gen(env, AddInt, x, rhs);
    }

    // (X - C1) + (Y +/- C2)
    if ((inst2->op() == AddInt || inst2->op() == SubInt) &&
        inst2->src(1)->hasConstVal()) {
      auto const y = inst2->src(0);
      auto const c2 = inst2->src(1);
      auto const rhs = inst2->op() == SubInt ?
        // (X - C1) + (Y - C2) --> X + Y + (-C1 - C2)
        gen(env, SubInt, gen(env, SubInt, cns(env, 0), c1), c2) :
        // (X - C1) + (Y + C2) --> X + Y + (C2 - C1)
        gen(env, SubInt, c2, c1);

      auto const lhs = gen(env, AddInt, x, y);
      return gen(env, AddInt, lhs, rhs);
    }
    // (X - C1) + (Y + C2) --> X + Y + (C2 - C1)
    if (inst2->op() == AddInt && inst2->src(1)->hasConstVal()) {
      auto const y = inst2->src(0);
      auto const c2 = inst2->src(1);

      auto const lhs = gen(env, AddInt, x, y);
      auto const rhs = gen(env, SubInt, c2, c1);
      return gen(env, AddInt, lhs, rhs);
    }
  }

  return nullptr;
}

SSATmp* simplifyAddIntO(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);
  if (src1->hasConstVal() && src2->hasConstVal()) {
    auto const a = src1->intVal();
    auto const b = src2->intVal();
    if (add_overflow(a, b)) {
      gen(env, Jmp, inst->taken());
      return cns(env, TBottom);
    }
    return cns(env, a + b);
  }
  return nullptr;
}

SSATmp* simplifySubInt(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);

  auto const sub = std::minus<uint64_t>();
  if (auto simp = constImpl(env, src1, src2, sub)) return simp;

  // X - X --> 0
  if (src1 == src2) return cns(env, 0);

  if (src2->hasConstVal()) {
    auto const src2Val = src2->intVal();
    // X - 0 --> X
    if (src2Val == 0) return src1;

    // X - -C --> X + C
    // Need to check for C == INT_MIN, otherwise we'd infinite loop as
    // X + -C would send us back here.
    auto const min = std::numeric_limits<int64_t>::min();
    if (src2Val > min && src2Val < 0) {
      return gen(env, AddInt, src1, cns(env, -src2Val));
    }
  }
  // X - (0 - Y) --> X + Y
  auto const inst2 = src2->inst();
  if (inst2->op() == SubInt) {
    auto const src = inst2->src(0);
    if (src->hasConstVal(0)) return gen(env, AddInt, src1, inst2->src(1));
  }
  return nullptr;
}

SSATmp* simplifySubIntO(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);
  if (src1->hasConstVal() && src2->hasConstVal()) {
    auto const a = src1->intVal();
    auto const b = src2->intVal();
    if (sub_overflow(a, b)) {
      gen(env, Jmp, inst->taken());
      return cns(env, TBottom);
    }
    return cns(env, a - b);
  }
  return nullptr;
}

SSATmp* simplifyMulInt(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);

  auto const mul = std::multiplies<uint64_t>();
  if (auto simp = commutativeImpl(env, src1, src2, MulInt, mul)) return simp;

  if (!src2->hasConstVal()) return nullptr;

  auto const rhs = src2->intVal();

  // X * (-1) --> -X
  if (rhs == -1) return gen(env, SubInt, cns(env, 0), src1);
  // X * 0 --> 0
  if (rhs == 0) return cns(env, 0);
  // X * 1 --> X
  if (rhs == 1) return src1;
  // X * 2 --> X + X
  if (rhs == 2) return gen(env, AddInt, src1, src1);

  auto isPowTwo = [](int64_t a) {
    return a > 0 && folly::isPowTwo<uint64_t>(a);
  };
  auto log2 = [](int64_t a) {
    assertx(a > 0);
    return folly::findLastSet<uint64_t>(a) - 1;
  };

  // X * 2^C --> X << C
  if (isPowTwo(rhs)) return gen(env, Shl, src1, cns(env, log2(rhs)));

  // X * (2^C + 1) --> ((X << C) + X)
  if (isPowTwo(rhs - 1)) {
    auto const lhs = gen(env, Shl, src1, cns(env, log2(rhs - 1)));
    return gen(env, AddInt, lhs, src1);
  }
  // X * (2^C - 1) --> ((X << C) - X)
  if (isPowTwo(rhs + 1)) {
    auto const lhs = gen(env, Shl, src1, cns(env, log2(rhs + 1)));
    return gen(env, SubInt, lhs, src1);
  }

  return nullptr;
}

SSATmp* simplifyAddDbl(State& env, const IRInstruction* inst) {
  return constImpl(env, inst->src(0), inst->src(1), std::plus<double>());
}

SSATmp* simplifySubDbl(State& env, const IRInstruction* inst) {
  return constImpl(env, inst->src(0), inst->src(1), std::minus<double>());
}

SSATmp* simplifyMulDbl(State& env, const IRInstruction* inst) {
  return constImpl(env, inst->src(0), inst->src(1), std::multiplies<double>());
}

SSATmp* simplifyMulIntO(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);
  if (src1->hasConstVal() && src2->hasConstVal()) {
    auto const a = src1->intVal();
    auto const b = src2->intVal();
    if (mul_overflow(a, b)) {
      gen(env, Jmp, inst->taken());
      return cns(env, TBottom);
    }
    return cns(env, a * b);
  }
  return nullptr;
}

/*
Integer Modulo/Remainder Operator: a % b = a - a / b * b
*/
SSATmp* simplifyMod(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);

  if (!src2->hasConstVal()) return nullptr;

  auto const src2Val = src2->intVal();

  if (src2Val == 0) {
    // Undefined behavior, so we might as well constant propagate whatever we
    // want. If we're being asked to simplify this, it better be dynamically
    // unreachable code.
    // TODO we can do `return gen(env, Unreachable, ASSERT_REASON);` here
    return cns(env, 0);
  }

  // X % 1 --> 0, X % -1 --> 0
  if (src2Val == -1 || src2Val == 1) return cns(env, 0);

  if (src1->hasConstVal()) return cns(env, src1->intVal() % src2Val);

  // Optimization: x % 2^n == x & (2^n - 1)
  if (folly::popcount(llabs(src2Val)) == 1) {
    // long shft =
    //  static_cast<unsigned long>(x >> 63) >> (64 - __builtin_ctzll(y));
    // ret ((x + shft) & (y - 1)) - shft;
    auto const divisor = llabs(src2Val);
    auto const trailingZeros = cns(env, 64 - __builtin_ctzll(divisor));
    auto const shft = gen(env, Lshr,
                          gen(env, Shr, src1, cns(env, 63)), trailingZeros);
    return gen(env, SubInt,
               gen(env, AndInt,
                   gen(env, AddInt, src1, shft), cns(env, divisor - 1)), shft);
  }
  return nullptr;
}

SSATmp* simplifyDivDbl(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);

  if (!src2->hasConstVal()) return nullptr;

  auto const src2Val = src2->dblVal();

  if (src2Val == 0.0) {
    // The branch emitted during irgen will deal with this
    return nullptr;
  }

  // statically compute X / Y
  return src1->hasConstVal() ? cns(env, src1->dblVal() / src2Val) : nullptr;
}

SSATmp* simplifyDivInt(State& env, const IRInstruction* inst) {
  auto const dividend = inst->src(0);
  auto const divisor  = inst->src(1);

  if (!divisor->hasConstVal()) return nullptr;

  auto const divisorVal = divisor->intVal();

  if (divisorVal == 0) {
    // The branch emitted during irgen will deal with this
    return nullptr;
  }

  if (!dividend->hasConstVal()) return nullptr;

  auto const dividendVal = dividend->intVal();

  if (dividendVal == LLONG_MIN || dividendVal % divisorVal) {
    // This should be unreachable
    return nullptr;
  }

  return cns(env, dividendVal / divisorVal);
}

SSATmp* simplifyAndInt(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);
  auto bit_and = [](int64_t a, int64_t b) { return a & b; };
  auto bit_or = [](int64_t a, int64_t b) { return a | b; };
  auto const simp = distributiveImpl(env, src1, src2, AndInt, OrInt,
                                     bit_and, bit_or);
  if (simp != nullptr) {
    return simp;
  }
  // X & X --> X
  if (src1 == src2) {
    return src1;
  }
  if (src2->hasConstVal()) {
    // X & 0 --> 0
    if (src2->intVal() == 0) {
      return cns(env, 0);
    }
    // X & (~0) --> X
    if (src2->intVal() == ~0L) {
      return src1;
    }
  }
  return nullptr;
}

SSATmp* simplifyOrInt(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);

  auto bit_and = [](int64_t a, int64_t b) { return a & b; };
  auto bit_or = [](int64_t a, int64_t b) { return a | b; };
  auto const simp = distributiveImpl(env, src1, src2, OrInt, AndInt,
                                     bit_or, bit_and);
  if (simp != nullptr) {
    return simp;
  }
  // X | X --> X
  if (src1 == src2) {
    return src1;
  }
  if (src2->hasConstVal()) {
    // X | 0 --> X
    if (src2->intVal() == 0) {
      return src1;
    }
    // X | (~0) --> ~0
    if (src2->intVal() == ~uint64_t(0)) {
      return cns(env, ~uint64_t(0));
    }
  }
  return nullptr;
}

SSATmp* simplifyXorInt(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);
  auto bitxor = [](int64_t a, int64_t b) { return a ^ b; };
  if (auto simp = commutativeImpl(env, src1, src2, XorInt, bitxor)) {
    return simp;
  }
  // X ^ X --> 0
  if (src1 == src2) return cns(env, 0);
  // X ^ 0 --> X
  if (src2->hasConstVal(0)) return src1;
  return nullptr;
}

SSATmp* xorTrueImpl(State& env, SSATmp* src) {
  auto const inst = src->inst();
  auto const op = inst->op();

  // !(X cmp Y) --> X opposite_cmp Y
  if (auto const negated = negateCmpOp(op)) {
    auto const s0 = inst->src(0);
    auto const s1 = inst->src(1);
    // We can't add new uses to reference counted types without a more
    // advanced availability analysis.
    if (!s0->type().maybe(TCounted) && !s1->type().maybe(TCounted)) {
      return gen(env, *negated, s0, s1);
    }
    return nullptr;
  }

  switch (op) {
    // !!X --> X
    case XorBool:
      if (inst->src(1)->hasConstVal(true)) {
        // This is safe to add a new use to because inst->src(0) is a bool.
        assertx(inst->src(0)->isA(TBool));
        return inst->src(0);
      }
      return nullptr;
    case InstanceOfBitmask:
    case NInstanceOfBitmask:
      // This is safe because instanceofs don't take reference counted
      // arguments.
      assertx(!inst->src(0)->type().maybe(TCounted) &&
              !inst->src(1)->type().maybe(TCounted));
      return gen(
        env,
        (op == InstanceOfBitmask) ?
        NInstanceOfBitmask :
        InstanceOfBitmask,
        inst->src(0),
        inst->src(1)
      );
    default:
      break;
  }

  return nullptr;
}

SSATmp* simplifyXorBool(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);

  // Both constants.
  if (src1->hasConstVal() && src2->hasConstVal()) {
    return cns(env, bool(src1->boolVal() ^ src2->boolVal()));
  }

  // Canonicalize constants to the right.
  if (src1->hasConstVal() && !src2->hasConstVal()) {
    return gen(env, XorBool, src2, src1);
  }

  // X^0 => X
  if (src2->hasConstVal(false)) return src1;

  // X^1 => simplify "not" logic
  if (src2->hasConstVal(true)) return xorTrueImpl(env, src1);

  // X^X => false
  if (src1 == src2) return cns(env, false);

  return nullptr;
}

template<class Oper>
SSATmp* shiftImpl(State& env, const IRInstruction* inst, Oper op) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);

  if (src1->hasConstVal()) {
    if (src1->intVal() == 0) {
      return cns(env, 0);
    }

    if (src2->hasConstVal()) {
      return cns(env, op(src1->intVal(), src2->intVal() & 63));
    }
  }

  if (src2->hasConstVal() && src2->intVal() == 0) {
    return src1;
  }

  return nullptr;
}

SSATmp* simplifyShl(State& env, const IRInstruction* inst) {
  return shiftImpl(env, inst,
                   [] (uint64_t a, int64_t b) -> int64_t {
                     return a << b;
                   });
}

SSATmp* simplifyLshr(State& env, const IRInstruction* inst) {
  return shiftImpl(env, inst,
                   [] (uint64_t a, int64_t b) -> int64_t {
                     return a >> b;
                   });
}

SSATmp* simplifyShr(State& env, const IRInstruction* inst) {
  return shiftImpl(env, inst,
                   [] (int64_t a, int64_t b) {
                     // avoid implementation defined behavior
                     // gcc optimizes this to a signed right shift.
                     return a >= 0 ? a >> b : -(-(a+1) >> b) - 1;
                   });
}

// This function isn't meant to perform the actual comparison at
// compile-time. Instead, it performs the matching comparison against a
// primitive type (usually bool).
template<class T, class U>
static bool cmpOp(Opcode opc, T a, U b) {
  switch (opc) {
  case GtBool:
  case GtInt:
  case GtStr:
  case GtStrInt:
  case GtObj:
  case GtArr:
  case GtVec:
  case GtRes: return a > b;
  case GteBool:
  case GteInt:
  case GteStr:
  case GteStrInt:
  case GteObj:
  case GteArr:
  case GteVec:
  case GteRes: return a >= b;
  case LtBool:
  case LtInt:
  case LtStr:
  case LtStrInt:
  case LtObj:
  case LtArr:
  case LtVec:
  case LtRes: return a < b;
  case LteBool:
  case LteInt:
  case LteStr:
  case LteStrInt:
  case LteObj:
  case LteArr:
  case LteVec:
  case LteRes: return a <= b;
  case SameStr:
  case SameObj:
  case SameArr:
  case SameVec:
  case SameDict:
  case SameKeyset:
  case EqBool:
  case EqInt:
  case EqStr:
  case EqStrInt:
  case EqObj:
  case EqArr:
  case EqVec:
  case EqDict:
  case EqKeyset:
  case EqRes: return a == b;
  case NSameStr:
  case NSameObj:
  case NSameArr:
  case NSameVec:
  case NSameDict:
  case NSameKeyset:
  case NeqBool:
  case NeqInt:
  case NeqStr:
  case NeqStrInt:
  case NeqObj:
  case NeqArr:
  case NeqVec:
  case NeqDict:
  case NeqKeyset:
  case NeqRes: return a != b;
  default:
    always_assert(false);
  }
}

SSATmp* cmpBoolImpl(State& env,
                    Opcode opc,
                    const IRInstruction* const inst,
                    SSATmp* left,
                    SSATmp* right) {
  assertx(left->type() <= TBool);
  assertx(right->type() <= TBool);

  auto newInst = [&](Opcode op, SSATmp* src1, SSATmp* src2) {
    return gen(env, op, inst ? inst->taken() : (Block*)nullptr, src1, src2);
  };

  // Identity optimization
  if (left == right) {
    return cns(env, cmpOp(opc, true, true));
  }

  if (left->hasConstVal()) {
    // If both operands are constants, constant-fold them. Otherwise, move the
    // constant over to the right.
    if (right->hasConstVal()) {
      return cns(env, cmpOp(opc, left->boolVal(), right->boolVal()));
    } else {
      auto const newOpc = [](Opcode opcode) {
        switch (opcode) {
          case GtBool:  return LtBool;
          case GteBool: return LteBool;
          case LtBool:  return GtBool;
          case LteBool: return GteBool;
          case EqBool:  return EqBool;
          case NeqBool: return NeqBool;
          default: always_assert(false);
        }
      }(opc);
      return newInst(newOpc, right, left);
    }
  }

  if (right->hasConstVal()) {
    bool b = right->boolVal();

    // The result of the comparison might be independent of the truth
    // value of the LHS. If so, then simplify.
    if (cmpOp(opc, false, b) == cmpOp(opc, true, b)) {
      return cns(env, cmpOp(opc, false, b));
    }

    // There are only two distinct booleans - false and true (0 and 1).
    // From above, we know that (0 OP b) != (1 OP b).
    // Hence exactly one of (0 OP b) and (1 OP b) is true.
    // Hence there is exactly one boolean value of "left" that results in the
    // overall expression being true.
    // Hence we may check for equality with that boolean.
    if (opc != EqBool) {
      return newInst(EqBool, left, cns(env, !cmpOp(opc, false, b)));
    }

    // If we reach here, this is an equality comparison against a
    // constant. Testing for equality with true simplifies to just the left
    // operand, while equality with false is the negation of the left operand
    // (equivalent to XORing with true).
    return b ? left : newInst(XorBool, left, cns(env, true));
  }

  return nullptr;
}

SSATmp* cmpIntImpl(State& env,
                   Opcode opc,
                   const IRInstruction* const inst,
                   SSATmp* left,
                   SSATmp* right) {
  assertx(left->type() <= TInt);
  assertx(right->type() <= TInt);

  auto newInst = [&](Opcode op, SSATmp* src1, SSATmp* src2) {
    return gen(env, op, inst ? inst->taken() : (Block*)nullptr, src1, src2);
  };

  // Identity optimization
  if (left == right) {
    return cns(env, cmpOp(opc, true, true));
  }

  if (left->hasConstVal()) {
    // If both operands are constants, constant-fold them. Otherwise, move the
    // constant over to the right.
    if (right->hasConstVal()) {
      return cns(env, cmpOp(opc, left->intVal(), right->intVal()));
    } else {
      auto const newOpc = [](Opcode opcode) {
        switch (opcode) {
          case GtInt:  return LtInt;
          case GteInt: return LteInt;
          case LtInt:  return GtInt;
          case LteInt: return GteInt;
          case EqInt:  return EqInt;
          case NeqInt: return NeqInt;
          default: always_assert(false);
        }
      }(opc);
      return newInst(newOpc, right, left);
    }
  }

  return nullptr;
}

SSATmp* cmpStrImpl(State& env,
                   Opcode opc,
                   const IRInstruction* const inst,
                   SSATmp* left,
                   SSATmp* right) {
  assertx(left->type() <= TStr);
  assertx(right->type() <= TStr);

  auto newInst = [&](Opcode op, SSATmp* src1, SSATmp* src2) {
    return gen(env, op, inst ? inst->taken() : (Block*)nullptr, src1, src2);
  };

  // Identity optimization
  if (left == right) {
    return cns(env, cmpOp(opc, true, true));
  }

  if (left->hasConstVal()) {
    // If both operands are constants, constant-fold them. Otherwise, move the
    // constant over to the right.
    if (right->hasConstVal()) {
      if (opc == SameStr || opc == NSameStr) {
        return cns(
          env,
          cmpOp(opc, left->strVal()->same(right->strVal()), true)
        );
      } else {
        return cns(
          env,
          cmpOp(opc, left->strVal()->compare(right->strVal()), 0)
        );
      }
    } else {
      auto const newOpc = [](Opcode opcode) {
        switch (opcode) {
          case GtStr:    return LtStr;
          case GteStr:   return LteStr;
          case LtStr:    return GtStr;
          case LteStr:   return GteStr;
          case EqStr:    return EqStr;
          case NeqStr:   return NeqStr;
          case SameStr:  return SameStr;
          case NSameStr: return NSameStr;
          default: always_assert(false);
        }
      }(opc);
      return newInst(newOpc, right, left);
    }
  }

  // Comparisons against the empty string can be optimized to checks on the
  // string length.
  if (right->hasConstVal() && right->strVal()->empty()) {
    switch (opc) {
      case EqStr:
      case SameStr:
      case LteStr: return newInst(EqInt, gen(env, LdStrLen, left), cns(env, 0));
      case NeqStr:
      case NSameStr:
      case GtStr: return newInst(NeqInt, gen(env, LdStrLen, left), cns(env, 0));
      case LtStr: return cns(env, false);
      case GteStr: return cns(env, true);
      default: always_assert(false);
    }
  }

  return nullptr;
}

SSATmp* cmpStrIntImpl(State& env,
                      Opcode opc,
                      const IRInstruction* const inst,
                      SSATmp* left,
                      SSATmp* right) {
  assertx(left->type() <= TStr);
  assertx(right->type() <= TInt);

  auto newInst = [&](Opcode op, SSATmp* src1, SSATmp* src2) {
    return gen(env, op, inst ? inst->taken() : (Block*)nullptr, src1, src2);
  };

  // If the string operand has a constant value, convert it to the appropriate
  // numeric and lower to a numeric comparison.
  if (left->hasConstVal()) {
    int64_t si;
    double sd;
    auto const type =
      left->strVal()->isNumericWithVal(si, sd, true /* allow errors */);
    if (type == KindOfDouble) {
      auto const dblOpc = [](Opcode opcode) {
        switch (opcode) {
          case GtStrInt:  return GtDbl;
          case GteStrInt: return GteDbl;
          case LtStrInt:  return LtDbl;
          case LteStrInt: return LteDbl;
          case EqStrInt:  return EqDbl;
          case NeqStrInt: return NeqDbl;
          default: always_assert(false);
        }
      }(opc);
      return newInst(
        dblOpc,
        cns(env, sd),
        gen(env, ConvIntToDbl, right)
      );
    } else {
      auto const intOpc = [](Opcode opcode) {
        switch (opcode) {
          case GtStrInt:  return GtInt;
          case GteStrInt: return GteInt;
          case LtStrInt:  return LtInt;
          case LteStrInt: return LteInt;
          case EqStrInt:  return EqInt;
          case NeqStrInt: return NeqInt;
          default: always_assert(false);
        }
      }(opc);
      return newInst(
        intOpc,
        cns(env, type == KindOfNull ? (int64_t)0 : si),
        right
      );
    }
  }

  return nullptr;
}

SSATmp* cmpObjImpl(State& env, Opcode opc, const IRInstruction* const /*inst*/,
                   SSATmp* left, SSATmp* right) {
  assertx(left->type() <= TObj);
  assertx(right->type() <= TObj);

  // Identity optimization. Object comparisons can produce arbitrary
  // side-effects, so we can only eliminate the comparison if its checking for
  // sameness.
  if ((opc == SameObj || opc == NSameObj) && left == right) {
    return cns(env, cmpOp(opc, true, true));
  }

  return nullptr;
}

SSATmp* cmpArrImpl(State& env, Opcode opc, const IRInstruction* const /*inst*/,
                   SSATmp* left, SSATmp* right) {
  assertx(left->type() <= TArr);
  assertx(right->type() <= TArr);

  // Identity optimization. Array comparisons can produce arbitrary
  // side-effects, so we can only eliminate the comparison if its checking for
  // sameness.
  if ((opc == SameArr || opc == NSameArr) && left == right) {
    return cns(env, cmpOp(opc, true, true));
  }

  return nullptr;
}

SSATmp* cmpVecImpl(State& env, Opcode opc, const IRInstruction* const /*inst*/,
                   SSATmp* left, SSATmp* right) {
  assertx(left->type() <= TVec);
  assertx(right->type() <= TVec);

  // Identity optimization. Vec comparisons can produce arbitrary side-effects,
  // so we can only eliminate the comparison if its checking for sameness.
  if ((opc == SameVec || opc == NSameVec) && left == right) {
    return cns(env, cmpOp(opc, true, true));
  }

  return nullptr;
}

SSATmp* cmpDictImpl(State& env, Opcode opc, const IRInstruction* const /*inst*/,
                    SSATmp* left, SSATmp* right) {
  assertx(left->type() <= TDict);
  assertx(right->type() <= TDict);

  // Identity optimization. Dict comparisons can produce arbitrary side-effects,
  // so we can only eliminate the comparison if its checking for sameness.
  if ((opc == SameDict || opc == NSameDict) && left == right) {
    return cns(env, cmpOp(opc, true, true));
  }

  return nullptr;
}

SSATmp*
cmpKeysetImpl(State& env, Opcode opc, const IRInstruction* const /*inst*/,
              SSATmp* left, SSATmp* right) {
  assertx(left->type() <= TKeyset);
  assertx(right->type() <= TKeyset);

  // Unlike other array types, keyset comparisons can never throw or re-enter
  // (because they can only store integers and strings). Therefore we can fully
  // simplify equality comparisons if both arrays are constants.
  if (left->hasConstVal() && right->hasConstVal()) {
    auto const leftVal = left->keysetVal();
    auto const rightVal = right->keysetVal();
    switch (opc) {
      case EqKeyset:
        return cns(env, SetArray::Equal(leftVal, rightVal));
      case SameKeyset:
        return cns(env, SetArray::Same(leftVal, rightVal));
      case NeqKeyset:
        return cns(env, SetArray::NotEqual(leftVal, rightVal));
      case NSameKeyset:
        return cns(env, SetArray::NotSame(leftVal, rightVal));
      default:
        break;
    }
  }

  // Even if not a constant, we can apply an identity simplification as long as
  // we're doing an equality comparison.
  if ((opc == SameKeyset || opc == NSameKeyset ||
       opc == EqKeyset || opc == NeqKeyset)
      && left == right) {
    return cns(env, cmpOp(opc, true, true));
  }

  return nullptr;
}

SSATmp* cmpResImpl(State& env, Opcode opc, const IRInstruction* const /*inst*/,
                   SSATmp* left, SSATmp* right) {
  assertx(left->type() <= TRes);
  assertx(right->type() <= TRes);

  // Identity optimization.
  if (left == right) {
    return cns(env, cmpOp(opc, true, true));
  }

  return nullptr;
}

#define X(name, type)                                                   \
  SSATmp* simplify##name(State& env, const IRInstruction* i) {          \
    return cmp##type##Impl(env, i->op(), i, i->src(0), i->src(1));      \
  }

X(GtBool, Bool)
X(GteBool, Bool)
X(LtBool, Bool)
X(LteBool, Bool)
X(EqBool, Bool)
X(NeqBool, Bool)

X(GtInt, Int)
X(GteInt, Int)
X(LtInt, Int)
X(LteInt, Int)
X(EqInt, Int)
X(NeqInt, Int)

X(GtStr, Str)
X(GteStr, Str)
X(LtStr, Str)
X(LteStr, Str)
X(EqStr, Str)
X(NeqStr, Str)
X(SameStr, Str)
X(NSameStr, Str)

X(GtStrInt, StrInt)
X(GteStrInt, StrInt)
X(LtStrInt, StrInt)
X(LteStrInt, StrInt)
X(EqStrInt, StrInt)
X(NeqStrInt, StrInt)

X(GtObj, Obj)
X(GteObj, Obj)
X(LtObj, Obj)
X(LteObj, Obj)
X(EqObj, Obj)
X(NeqObj, Obj)
X(SameObj, Obj)
X(NSameObj, Obj)

X(GtArr, Arr)
X(GteArr, Arr)
X(LtArr, Arr)
X(LteArr, Arr)
X(EqArr, Arr)
X(NeqArr, Arr)
X(SameArr, Arr)
X(NSameArr, Arr)

X(GtVec, Vec)
X(GteVec, Vec)
X(LtVec, Vec)
X(LteVec, Vec)
X(EqVec, Vec)
X(NeqVec, Vec)
X(SameVec, Vec)
X(NSameVec, Vec)

X(EqDict, Dict)
X(NeqDict, Dict)
X(SameDict, Dict)
X(NSameDict, Dict)

X(EqKeyset, Keyset)
X(NeqKeyset, Keyset)
X(SameKeyset, Keyset)
X(NSameKeyset, Keyset)

X(GtRes, Res)
X(GteRes, Res)
X(LtRes, Res)
X(LteRes, Res)
X(EqRes, Res)
X(NeqRes, Res)

#undef X

SSATmp* simplifyEqCls(State& env, const IRInstruction* inst) {
  auto const left = inst->src(0);
  auto const right = inst->src(1);
  if (left->hasConstVal() && right->hasConstVal()) {
    return cns(env, left->clsVal() == right->clsVal());
  }
  return nullptr;
}

SSATmp* simplifyEqRecDesc(State& env, const IRInstruction* inst) {
  auto const left = inst->src(0);
  auto const right = inst->src(1);
  if (left->hasConstVal() && right->hasConstVal()) {
    return cns(env, left->recVal() == right->recVal());
  }
  return nullptr;
}

SSATmp* simplifyEqStrPtr(State& env, const IRInstruction* inst) {
  auto const left = inst->src(0);
  auto const right = inst->src(1);
  if (left == right) return cns(env, true);
  if (left->hasConstVal() && right->hasConstVal()) {
    return cns(env, left->strVal() == right->strVal());
  }
  return nullptr;
}

SSATmp* simplifyEqArrayDataPtr(State& env, const IRInstruction* inst) {
  auto const left = inst->src(0);
  auto const right = inst->src(1);
  if (left == right) return cns(env, true);
  if (!left->type().maybe(right->type())) return cns(env, false);
  if (left->hasConstVal() && right->hasConstVal()) {
    return cns(env, left->arrLikeVal() == right->arrLikeVal());
  }
  return nullptr;
}

SSATmp* simplifyCmpBool(State& env, const IRInstruction* inst) {
  auto const left = inst->src(0);
  auto const right = inst->src(1);
  assertx(left->type() <= TBool);
  assertx(right->type() <= TBool);
  if (left->hasConstVal() && right->hasConstVal()) {
    return cns(env, HPHP::compare(left->boolVal(), right->boolVal()));
  }
  return nullptr;
}

SSATmp* simplifyCmpInt(State& env, const IRInstruction* inst) {
  auto const left = inst->src(0);
  auto const right = inst->src(1);
  assertx(left->type() <= TInt);
  assertx(right->type() <= TInt);
  if (left->hasConstVal() && right->hasConstVal()) {
    return cns(env, HPHP::compare(left->intVal(), right->intVal()));
  }
  return nullptr;
}

SSATmp* simplifyCmpStr(State& env, const IRInstruction* inst) {
  auto const left = inst->src(0);
  auto const right = inst->src(1);

  assertx(left->type() <= TStr);
  assertx(right->type() <= TStr);

  auto newInst = [&](Opcode op, SSATmp* src1, SSATmp* src2) {
    return gen(env, op, inst ? inst->taken() : (Block*)nullptr, src1, src2);
  };

  if (left->hasConstVal()) {
    if (right->hasConstVal()) {
      return cns(env, HPHP::compare(left->strVal(), right->strVal()));
    } else if (left->strVal()->empty()) {
      // Comparisons against the empty string can be optimized to a comparison
      // on the string length.
      return newInst(CmpInt, cns(env, 0), gen(env, LdStrLen, right));
    }
  } else if (right->hasConstVal() && right->strVal()->empty()) {
    return newInst(CmpInt, gen(env, LdStrLen, left), cns(env, 0));
  }

  return nullptr;
}

SSATmp* simplifyCmpStrInt(State& env, const IRInstruction* inst) {
  auto const left = inst->src(0);
  auto const right = inst->src(1);

  assertx(left->type() <= TStr);
  assertx(right->type() <= TInt);

  auto newInst = [&](Opcode op, SSATmp* src1, SSATmp* src2) {
    return gen(env, op, inst ? inst->taken() : (Block*)nullptr, src1, src2);
  };

  // If the string operand has a constant value, convert it to the appropriate
  // numeric and lower to a numeric comparison.
  if (left->hasConstVal()) {
    int64_t si;
    double sd;
    auto const type =
      left->strVal()->isNumericWithVal(si, sd, true /* allow errors */);
    if (type == KindOfDouble) {
      return newInst(
        CmpDbl,
        cns(env, sd),
        gen(env, ConvIntToDbl, right)
      );
    } else {
      return newInst(
        CmpInt,
        cns(env, type == KindOfNull ? (int64_t)0 : si),
        right
      );
    }
  }

  return nullptr;
}

SSATmp* simplifyCmpRes(State& env, const IRInstruction* inst) {
  auto const left = inst->src(0);
  auto const right = inst->src(1);
  assertx(left->type() <= TRes);
  assertx(right->type() <= TRes);
  return (left == right) ? cns(env, 0) : nullptr;
}

SSATmp* isTypeImpl(State& env, const IRInstruction* inst) {
  bool const trueSense = inst->op() == IsType;
  auto const type      = inst->typeParam();
  auto const src       = inst->src(0);
  auto const srcType   = src->type();

  // Testing for StaticStr will make you miss out on CountedStr, and vice versa,
  // and similarly for arrays. PHP treats both types of string the same, so if
  // the distinction matters to you here, be careful.
  assertx(IMPLIES(type <= TStr, type == TStr));
  assertx(IMPLIES(type <= TArr, type == TArr));
  assertx(IMPLIES(type <= TVec, type == TVec));
  assertx(IMPLIES(type <= TDict, type == TDict));
  assertx(IMPLIES(type <= TKeyset, type == TKeyset));

  // Specially handle checking if an uninit var's type is null. The Type class
  // doesn't fully correctly handle the fact that the earlier stages of the
  // compiler consider null to be either initalized or uninitalized, so we need
  // to do this check first. Right here is really the only place in the backend
  // it seems to matter, especially since manipulating an uninit is kind of
  // weird; as of this writing, "$uninitalized_variable ?? 42" is the only
  // place it really comes up.
  if (srcType <= TUninit && type <= TNull) {
    return cns(env, trueSense);
  }

  // The types are disjoint; the result must be false.
  if (!srcType.maybe(type)) {
    return cns(env, !trueSense);
  }

  // The src type is a subtype of the tested type; the result must be true.
  if (srcType <= type) {
    return cns(env, trueSense);
  }

  // At this point, either the tested type is a subtype of the src type, or they
  // are non-disjoint but neither is a subtype of the other. We can't simplify
  // this away.
  return nullptr;
}

SSATmp* instanceOfImpl(State& env, SSATmp* ssatmp1, ClassSpec spec2) {
  assertx(ssatmp1->type() <= TCls);
  if (!spec2) return nullptr;
  auto const cls2 = spec2.cls();

  ClassSpec spec1 = ssatmp1->type().clsSpec();
  if (spec2.exact() && spec1 && spec1.cls()->classof(cls2)) {
    return cns(env, true);
  }

  // If spec2 is exact and we have an instance bit for it, use
  // InstanceOfBitmask.
  const bool useInstanceBits = InstanceBits::initted() ||
                               env.unit.context().kind == TransKind::Optimize;
  if (spec2.exact() && useInstanceBits) {
    InstanceBits::init();
    if (InstanceBits::lookup(cls2->name()) != 0) {
      return gen(env, InstanceOfBitmask, ssatmp1, cns(env, cls2->name()));
    }
  }

  if (!spec1) return nullptr;
  auto const cls1 = spec1.cls();

  if (cls1->classof(cls2)) {
    assertx(!spec2.exact()); // the exact case is handled above
    return nullptr;
  }

  if (isInterface(cls1)) return nullptr;

  if (spec1.exact()) return cns(env, false);

  // At this point cls1 is not a cls2, and its not exact, so:
  //
  //  - If cls2 is an interface, a descendent of cls1 could implement
  //    that interface
  //  - If cls2 is a descendent of cls1, then (clearly) a descendent
  //    of cls1 could be a cls2
  //  - Otherwise no descendent of cls1 can be a cls2 or any class
  //    that isa cls2
  if (!isInterface(cls2) && !cls2->classof(cls1)) {
    return cns(env, false);
  }

  return nullptr;
}

SSATmp* simplifyInstanceOf(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);

  if (src2->isA(TNullptr)) {
    return cns(env, false);
  }

  auto const spec2 = src2->type().clsSpec();

  if (auto const cls = spec2.exactCls()) {
    if (isNormalClass(cls) && (cls->attrs() & AttrUnique)) {
      return gen(env, ExtendsClass, ExtendsClassData{ cls }, src1);
    }
    if (isInterface(cls) && (cls->attrs() & AttrUnique)) {
      return gen(env, InstanceOfIface, src1, cns(env, cls->name()));
    }
  }

  return instanceOfImpl(env, src1, src2->type().clsSpec());
}

SSATmp* simplifyExtendsClass(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const cls2 = inst->extra<ExtendsClassData>()->cls;
  assertx(cls2 && isNormalClass(cls2));
  auto const spec2 = ClassSpec{cls2, ClassSpec::ExactTag{}};
  return instanceOfImpl(env, src1, spec2);
}

SSATmp* simplifyInstanceOfBitmask(State& env, const IRInstruction* inst) {
  auto const cls = inst->src(0);
  auto const name = inst->src(1);

  if (!name->hasConstVal(TStr)) return nullptr;

  auto const bit = InstanceBits::lookup(name->strVal());
  always_assert(bit && "cgInstanceOfBitmask had no bitmask");

  if (cls->type().clsSpec() &&
      cls->type().clsSpec().cls()->checkInstanceBit(bit)) {
    return cns(env, true);
  }

  if (!cls->hasConstVal(TCls)) return nullptr;
  return cns(env, false);
}

SSATmp* simplifyNInstanceOfBitmask(State& env, const IRInstruction* inst) {
  auto const cls = inst->src(0);
  auto const name = inst->src(1);

  if (!name->hasConstVal(TStr)) return nullptr;

  auto const bit = InstanceBits::lookup(name->strVal());
  always_assert(bit && "cgNInstanceOfBitmask had no bitmask");

  if (cls->type().clsSpec() &&
      cls->type().clsSpec().cls()->checkInstanceBit(bit)) {
    return cns(env, false);
  }

  if (!cls->hasConstVal(TCls)) return nullptr;
  return cns(env, true);
}

SSATmp* simplifyInstanceOfIface(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);

  auto const cls2 = Unit::lookupUniqueClassInContext(src2->strVal(),
                                                     inst->ctx(), nullptr);
  assertx(cls2 && isInterface(cls2));
  auto const spec2 = ClassSpec{cls2, ClassSpec::ExactTag{}};

  return instanceOfImpl(env, src1, spec2);
}

SSATmp* simplifyInstanceOfIfaceVtable(State& env, const IRInstruction* inst) {
  if (!inst->extra<InstanceOfIfaceVtable>()->canOptimize) return nullptr;
  auto const cls = inst->src(0);
  auto const iface = inst->extra<InstanceOfIfaceVtable>()->cls;
  if (cls->type().clsSpec() &&
      cls->type().clsSpec().cls()->classof(iface)) {
    return cns(env, true);
  }

  const bool useInstanceBits = InstanceBits::initted() ||
                               env.unit.context().kind == TransKind::Optimize;
  if (useInstanceBits) {
    InstanceBits::init();
    auto const ifaceName = iface->name();
    if (InstanceBits::lookup(ifaceName) != 0) {
      return gen(env, InstanceOfBitmask, cls, cns(env, ifaceName));
    }
  }
  return nullptr;
}

SSATmp* simplifyIsType(State& env, const IRInstruction* i) {
  return isTypeImpl(env, i);
}

SSATmp* simplifyIsNType(State& env, const IRInstruction* i) {
  return isTypeImpl(env, i);
}

SSATmp* simplifyMethodExists(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);

  auto const clsSpec = src1->type().clsSpec();

  if (clsSpec.cls() != nullptr && src2->hasConstVal(TStr)) {
    // If we don't have an exact type, then we can't say for sure the class
    // doesn't have the method.
    auto const result = clsSpec.cls()->lookupMethod(src2->strVal()) != nullptr;
    return (clsSpec.exact() || result) ? cns(env, result) : nullptr;
  }
  return nullptr;
}

static auto concat_litstrs(State& env, SSATmp* s1, SSATmp* s2) {
  auto const str1 = const_cast<StringData*>(s1->strVal());
  auto const str2 = const_cast<StringData*>(s2->strVal());
  auto const sval = String::attach(concat_ss(str1, str2));
  return cns(env, makeStaticString(sval.get()));
}

SSATmp* simplifyConcatStrStr(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);
  if (src1->hasConstVal(TStaticStr) &&
      src2->hasConstVal(TStaticStr)) {
    return concat_litstrs(env, src1, src2);
  }

  // ConcatStrStr consumes a reference to src1 and produces a reference, so
  // anything we replace it with must do the same thing.
  if (src1->hasConstVal(staticEmptyString())) {
    // Produce a reference on src2.
    gen(env, IncRef, src2);
    return src2;
  }
  if (src2->hasConstVal(staticEmptyString())) {
    // Forward the reference on src1 from input to output.
    return src1;
  }

  return nullptr;
}

SSATmp* simplifyConcatStr3(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);
  auto const src3 = inst->src(2);

  if (src2->hasConstVal(TStaticStr)) {
    if (src1->hasConstVal(TStaticStr)) {
      return gen(env, ConcatStrStr, inst->taken(),
                 concat_litstrs(env, src1, src2), src3);
    }
    if (src3->hasConstVal(TStaticStr)) {
      return gen(env, ConcatStrStr, inst->taken(),
                 src1, concat_litstrs(env, src2, src3));
    }
    if (src2->hasConstVal(staticEmptyString())) {
      return gen(env, ConcatStrStr, inst->taken(), src1, src3);
    }
  }

  // ConcatStr3 consumes a reference to src1 and produces a reference, so
  // anything we replace it with must do the same thing.
  if (src1->hasConstVal(staticEmptyString())) {
    // Compensate for ConcatStrStr consuming src2.
    gen(env, IncRef, src2);
    return gen(env, ConcatStrStr, inst->taken(), src2, src3);
  }
  if (src3->hasConstVal(staticEmptyString())) {
    // ConcatStrStr also consumes a reference to src1
    return gen(env, ConcatStrStr, inst->taken(), src1, src2);
  }

  return nullptr;
}

SSATmp* simplifyConcatStr4(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);
  auto const src3 = inst->src(2);
  auto const src4 = inst->src(3);

  if (src2->hasConstVal(TStaticStr)) {
    if (src1->hasConstVal(TStaticStr)) {
      return gen(env, ConcatStr3, inst->taken(),
                 concat_litstrs(env, src1, src2), src3, src4);
    }
    if (src3->hasConstVal(TStaticStr)) {
      return gen(env, ConcatStr3, inst->taken(),
                 src1, concat_litstrs(env, src2, src3), src4);
    }
    if (src2->hasConstVal(staticEmptyString())) {
      return gen(env, ConcatStr3, inst->taken(), src1, src3, src4);
    }
  }

  if (src3->hasConstVal(TStaticStr)) {
    if (src4->hasConstVal(TStaticStr)) {
      return gen(env, ConcatStr3, inst->taken(),
                 src1, src2, concat_litstrs(env, src3, src4));
    }
    if (src3->hasConstVal(staticEmptyString())) {
      return gen(env, ConcatStr3, inst->taken(), src1, src2, src4);
    }
  }

  // ConcatStr4 consumes a reference to src1 and produces a reference, so
  // anything we replace it with must do the same thing.
  if (src1->hasConstVal(staticEmptyString())) {
    // Compensate for ConcatStrStr consuming src2.
    gen(env, IncRef, src2);
    return gen(env, ConcatStr3, inst->taken(), src2, src3, src4);
  }
  if (src4->hasConstVal(staticEmptyString())) {
    // ConcatStr3 also consumes a reference to src1
    return gen(env, ConcatStr3, inst->taken(), src1, src2, src3);
  }

  return nullptr;
}

SSATmp* simplifyConcatIntStr(State& env, const IRInstruction* inst) {
  auto const lhs = inst->src(0);
  auto const rhs = inst->src(1);

  if (lhs->hasConstVal()) {
    auto const lhsStr =
      cns(env, makeStaticString(folly::to<std::string>(lhs->intVal())));
    return gen(env, ConcatStrStr, inst->taken(), lhsStr, rhs);
  }
  if (rhs->hasConstVal(staticEmptyString())) {
    return gen(env, ConvIntToStr, lhs);
  }

  return nullptr;
}

SSATmp* simplifyConcatStrInt(State& env, const IRInstruction* inst) {
  auto const lhs = inst->src(0);
  auto const rhs = inst->src(1);

  if (rhs->hasConstVal()) {
    auto const rhsStr =
      cns(env, makeStaticString(folly::to<std::string>(rhs->intVal())));
    return gen(env, ConcatStrStr, inst->taken(), lhs, rhsStr);
  }
  if (lhs->hasConstVal(staticEmptyString())) {
    return gen(env, ConvIntToStr, rhs);
  }

  return nullptr;
}

namespace {

template <typename C>
SSATmp* arrayLikeConvImpl(State& env, const IRInstruction* inst, C convert) {
  arrprov::TagOverride ap_override{arrprov::tagFromSK(inst->marker().sk())};
  auto const src = inst->src(0);
  if (!src->hasConstVal()) return nullptr;
  /* we can't fold the conversion if this function is skip frame
   * and we would assign a new tag at this location */
  if (RO::EvalArrayProvenance &&
      inst->func()->isProvenanceSkipFrame()) return nullptr;
  auto const before = src->arrLikeVal();
  auto converted = convert(const_cast<ArrayData*>(before));
  if (!converted) return nullptr;
  ArrayData::GetScalarArray(&converted);
  return cns(env, converted);
}

SSATmp* convToArrImpl(State& env, const IRInstruction* inst) {
  return arrayLikeConvImpl(
    env, inst,
    [&](ArrayData* a) { return a->toPHPArray(true); }
  );
}

SSATmp* convToVecImpl(State& env, const IRInstruction* inst) {
  return arrayLikeConvImpl(
    env, inst,
    [&](ArrayData* a) { return a->toVec(true); }
  );
}

SSATmp* convToDictImpl(State& env, const IRInstruction* inst) {
  return arrayLikeConvImpl(
    env, inst,
    [&](ArrayData* a) { return a->toDict(true); }
  );
}

SSATmp* convToNonDVArrImpl(State& env, const IRInstruction* inst) {
  return arrayLikeConvImpl(
    env, inst,
    [&](ArrayData* a) { return a->toPHPArray(true); }
  );
}

SSATmp* convToKeysetImpl(State& env, const IRInstruction* inst) {
  return arrayLikeConvImpl(
    env, inst,
    [&](ArrayData* a) {
      // We need to check if the array contains values suitable for keyset
      // before attempting the conversion. Otherwise, toKeyset() might re-enter
      // which we can't do from the simplifier.
      bool keylike = true;
      IterateV(
        a,
        [&](TypedValue v) {
          if (!isIntType(v.m_type) && !isStringType(v.m_type)) {
            keylike = false;
            return true;
          }
          return false;
        }
      );
      return keylike ? a->toKeyset(true) : nullptr;
    }
  );
}

SSATmp* convToVArrImpl(State& env, const IRInstruction* inst) {
  return arrayLikeConvImpl(
    env, inst,
    [&](ArrayData* a) { return a->toVArray(true); }
  );
}

SSATmp* convToDArrImpl(State& env, const IRInstruction* inst) {
  return arrayLikeConvImpl(
    env, inst,
    [&](ArrayData* a) { return a->toDArray(true); }
  );
}

SSATmp* convNonArrToArrImpl(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->hasConstVal()) {
    auto arr = make_packed_array(src->variantVal());
    return cns(env, ArrayData::GetScalarArray(std::move(arr)));
  }
  return nullptr;
}

}

#define X(FromTy, ToTy)                                                   \
SSATmp*                                                                   \
simplifyConv##FromTy##To##ToTy(State& env, const IRInstruction* inst) {   \
  return convTo##ToTy##Impl(env, inst);                                   \
}

X(Vec, Arr)
X(Dict, Arr)
X(Keyset, Arr)

X(Arr, Vec)
X(Dict, Vec)
X(Keyset, Vec)

X(Arr, Dict)
X(Vec, Dict)
X(Keyset, Dict)

X(Arr, Keyset)
X(Vec, Keyset)
X(Dict, Keyset)

X(Arr, VArr)
X(Vec, VArr)
X(Dict, VArr)
X(Keyset, VArr)

X(Arr, DArr)
X(Vec, DArr)
X(Dict, DArr)
X(Keyset, DArr)

X(Arr, NonDVArr)

#undef X

SSATmp* simplifyConvTVToArr(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->isA(TArr))    return gen(env, ConvArrToNonDVArr, src);
  if (src->isA(TVec))    return gen(env, ConvVecToArr, src);
  if (src->isA(TDict))   return gen(env, ConvDictToArr, inst->taken(), src);
  if (src->isA(TKeyset)) return gen(env, ConvKeysetToArr, inst->taken(), src);
  if (src->isA(TNull))   return cns(env, ArrayData::Create());
  if (src->isA(TBool))   return gen(env, ConvBoolToArr, src);
  if (src->isA(TDbl))    return gen(env, ConvDblToArr, src);
  if (src->isA(TInt))    return gen(env, ConvIntToArr, src);
  if (src->isA(TStr))    return gen(env, ConvStrToArr, src);
  if (src->isA(TObj))    return gen(env, ConvObjToArr, inst->taken(), src);
  if (src->isA(TFunc))   return gen(env, ConvFuncToArr, src);
  if (src->isA(TClsMeth)) return gen(env, ConvClsMethToArr, inst->taken(), src);
  return nullptr;
}

SSATmp* simplifyConvBoolToArr(State& env, const IRInstruction* inst) {
  return convNonArrToArrImpl(env, inst);
}

SSATmp* simplifyConvIntToArr(State& env, const IRInstruction* inst) {
  return convNonArrToArrImpl(env, inst);
}

SSATmp* simplifyConvDblToArr(State& env, const IRInstruction* inst) {
  return convNonArrToArrImpl(env, inst);
}

SSATmp* simplifyConvStrToArr(State& env, const IRInstruction* inst) {
  return convNonArrToArrImpl(env, inst);
}

SSATmp* simplifyConvFuncToArr(State& env, const IRInstruction* inst) {
  return convNonArrToArrImpl(env, inst);
}

SSATmp* simplifyConvClsMethToArr(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->hasConstVal()) {
    auto const clsmeth = src->clsmethVal();
    auto arr = make_packed_array(clsmeth->getClsStr(), clsmeth->getFuncStr());
    return cns(env, ArrayData::GetScalarArray(std::move(arr)));
  }
  return nullptr;
}

SSATmp* simplifyConvClsMethToVArr(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->hasConstVal()) {
    auto const clsmeth = src->clsmethVal();
    auto arr = make_varray(clsmeth->getClsStr(), clsmeth->getFuncStr());
    return cns(env, ArrayData::GetScalarArray(std::move(arr)));
  }
  return nullptr;
}

SSATmp* simplifyConvClsMethToVec(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->hasConstVal()) {
    auto const clsmeth = src->clsmethVal();
    auto arr = make_vec_array(clsmeth->getClsStr(), clsmeth->getFuncStr());
    return cns(env, ArrayData::GetScalarArray(std::move(arr)));
  }
  return nullptr;
}

SSATmp* simplifyConvClsMethToDArr(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->hasConstVal()) {
    auto const clsmeth = src->clsmethVal();
    auto arr = make_darray(0, clsmeth->getClsStr(), 1, clsmeth->getFuncStr());
    return cns(env, ArrayData::GetScalarArray(std::move(arr)));
  }
  return nullptr;
}

SSATmp* simplifyConvClsMethToDict(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->hasConstVal()) {
    auto const clsmeth = src->clsmethVal();
    auto arr = make_dict_array(
      0, clsmeth->getClsStr(), 1, clsmeth->getFuncStr());
    return cns(env, ArrayData::GetScalarArray(std::move(arr)));
  }
  return nullptr;
}

SSATmp* simplifyConvClsMethToKeyset(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->hasConstVal()) {
    auto const clsmeth = src->clsmethVal();
    auto arr = make_keyset_array(
      const_cast<StringData*>(clsmeth->getCls()->name()),
      const_cast<StringData*>(clsmeth->getFunc()->name()));
    return cns(env, ArrayData::GetScalarArray(std::move(arr)));
  }
  return nullptr;
}

SSATmp* simplifyConvArrToBool(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  auto const kind = src->type().arrSpec().kind();
  if (src->isA(TStaticArr) || (kind && !arrayKindNeedsVsize(*kind))) {
    return gen(env, ConvIntToBool, gen(env, CountArrayFast, src));
  }
  return nullptr;
}

SSATmp* simplifyConvDblToBool(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->hasConstVal()) {
    bool const bval = src->dblVal();
    return cns(env, bval);
  }
  return nullptr;
}

SSATmp* simplifyConvIntToBool(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->hasConstVal()) {
    bool const bval = src->intVal();
    return cns(env, bval);
  }
  return nullptr;
}

SSATmp* simplifyConvStrToBool(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->hasConstVal()) {
    // only the strings "", and "0" convert to false, all other strings
    // are converted to true
    auto const str = src->strVal();
    return cns(env, !str->empty() && !str->isZero());
  }
  return nullptr;
}

SSATmp* simplifyConvArrToDbl(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->hasConstVal()) {
    if (src->arrVal()->empty()) {
      return cns(env, 0.0);
    }
  }
  return nullptr;
}

SSATmp* simplifyConvBoolToDbl(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->hasConstVal()) {
    double const dval = src->boolVal();
    return cns(env, dval);
  }
  return nullptr;
}

SSATmp* simplifyConvIntToDbl(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->hasConstVal()) {
    double const dval = src->intVal();
    return cns(env, dval);
  }
  if (src->inst()->is(ConvBoolToInt)) {
    // This is safe, because the bool src is not reference counted.
    return gen(env, ConvBoolToDbl, src->inst()->src(0));
  }
  return nullptr;
}

SSATmp* simplifyConvStrToDbl(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  return src->hasConstVal() ? cns(env, src->strVal()->toDouble()) : nullptr;
}

SSATmp* simplifyConvBoolToInt(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->hasConstVal()) return cns(env, static_cast<int>(src->boolVal()));
  return nullptr;
}

SSATmp* simplifyConvDblToInt(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->hasConstVal()) return cns(env, double_to_int64(src->dblVal()));
  return nullptr;
}

SSATmp* simplifyConvStrToInt(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  return src->hasConstVal() ? cns(env, src->strVal()->toInt64()) : nullptr;
}

SSATmp* simplifyConvDblToStr(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->hasConstVal()) {
    auto const dblStr = String::attach(buildStringData(src->dblVal()));
    return cns(env, makeStaticString(dblStr));
  }
  return nullptr;
}

SSATmp* simplifyConvIntToStr(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->hasConstVal()) {
    return cns(env,
      makeStaticString(folly::to<std::string>(src->intVal()))
    );
  }
  return nullptr;
}

namespace {
const StaticString
    s_msgArrToStr("Array to string conversion"),
    s_msgVecToStr("Vec to string conversion"),
    s_msgDictToStr("Dict to string conversion"),
    s_msgKeysetToStr("Keyset to string conversion"),
    s_msgFuncToStr("Func to string conversion"),
    s_msgFuncToInt("Func to int conversion"),
    s_msgFuncToDbl("Func to double conversion"),
    s_msgClsMethToStr("Implicit clsmeth to string conversion"),
    s_msgClsMethToInt("Implicit clsmeth to int conversion"),
    s_msgClsMethToDbl("Implicit clsmeth to double conversion");
}

SSATmp* simplifyConvTVToBool(State& env, const IRInstruction* inst) {
  auto const src     = inst->src(0);
  auto const srcType = src->type();

  if (srcType <= TBool) return src;
  if (srcType <= TNull) return cns(env, false);
  if (srcType <= TArr)  return gen(env, ConvArrToBool, src);
  if (srcType <= TVec) {
    auto const length = gen(env, CountVec, src);
    return gen(env, NeqInt, length, cns(env, 0));
  }
  if (srcType <= TDict) {
    auto const length = gen(env, CountDict, src);
    return gen(env, NeqInt, length, cns(env, 0));
  }
  if (srcType <= TKeyset) {
    auto const length = gen(env, CountKeyset, src);
    return gen(env, NeqInt, length, cns(env, 0));
  }
  if (srcType <= TDbl)  return gen(env, ConvDblToBool, src);
  if (srcType <= TInt)  return gen(env, ConvIntToBool, src);
  if (srcType <= TStr)  return gen(env, ConvStrToBool, src);
  if (srcType <= TObj) {
    if (auto const cls = srcType.clsSpec().cls()) {
      // We need to exclude interfaces like ConstSet.  For now, just
      // skip anything that's an interface or extension.
      if (!(cls->attrs() & AttrInterface)) {
        if (!cls->instanceCtor()) {
          return cns(env, true);
        }
      }
    }
    return gen(env, ConvObjToBool, inst->taken(), src);
  }
  if (srcType <= TRes)  return cns(env, true);
  if (srcType <= TFunc) return cns(env, true);
  if (srcType <= TClsMeth) return cns(env, true);

  return nullptr;
}

SSATmp* simplifyConvTVToStr(State& env, const IRInstruction* inst) {
  auto const src        = inst->src(0);
  auto const srcType    = src->type();
  auto const catchTrace = inst->taken();

  if (srcType <= TBool) {
    return gen(
      env,
      Select,
      src,
      cns(env, s_1.get()),
      cns(env, staticEmptyString())
    );
  }
  if (srcType <= TNull)   return cns(env, staticEmptyString());
  if (srcType <= TArr){
    gen(env, RaiseNotice, catchTrace, cns(env, s_msgArrToStr.get()));
    return cns(env, s_Array.get());
  }
  if (srcType <= TVec) {
    gen(env, RaiseNotice, catchTrace, cns(env, s_msgVecToStr.get()));
    return cns(env, s_Vec.get());
  }
  if (srcType <= TDict) {
    gen(env, RaiseNotice, catchTrace, cns(env, s_msgDictToStr.get()));
    return cns(env, s_Dict.get());
  }
  if (srcType <= TKeyset) {
    gen(env, RaiseNotice, catchTrace, cns(env, s_msgKeysetToStr.get()));
    return cns(env, s_Keyset.get());
  }
  if (srcType <= TDbl)    return gen(env, ConvDblToStr, src);
  if (srcType <= TInt)    return gen(env, ConvIntToStr, src);
  if (srcType <= TStr) {
    gen(env, IncRef, src);
    return src;
  }
  if (srcType <= TObj)    return gen(env, ConvObjToStr, catchTrace, src);
  if (srcType <= TRes)    return gen(env, ConvResToStr, catchTrace, src);
  if (srcType <= TFunc) {
    auto const ret = gen(env, LdFuncName, src);
    if (RuntimeOption::EvalRaiseFuncConversionWarning) {
      gen(env, RaiseWarning, catchTrace, cns(env, s_msgFuncToStr.get()));
    }
    return ret;
  }
  if (srcType <= TClsMeth) {
    if (RuntimeOption::EvalRaiseClsMethConversionWarning) {
      gen(env, RaiseNotice, catchTrace, cns(env, s_msgClsMethToStr.get()));
    }
    if (RuntimeOption::EvalHackArrDVArrs) {
      return cns(env, s_Vec.get());
    } else {
      return cns(env, s_Array.get());
    }
  }

  return nullptr;
}

SSATmp* simplifyConvTVToInt(State& env, const IRInstruction* inst) {
  auto const src      = inst->src(0);
  auto const srcType  = src->type();
  auto const catchTrace = inst->taken();

  if (srcType <= TInt)  return src;
  if (srcType <= TNull) return cns(env, 0);
  if (srcType <= TArr)  {
    auto const length = gen(env, Count, src);
    return gen(env, Select, length, cns(env, 1), cns(env, 0));
  }
  if (srcType <= TVec) {
    auto const length = gen(env, CountVec, src);
    return gen(env, Select, length, cns(env, 1), cns(env, 0));
  }
  if (srcType <= TDict) {
    auto const length = gen(env, CountDict, src);
    return gen(env, Select, length, cns(env, 1), cns(env, 0));
  }
  if (srcType <= TKeyset) {
    auto const length = gen(env, CountKeyset, src);
    return gen(env, Select, length, cns(env, 1), cns(env, 0));
  }
  if (srcType <= TBool) return gen(env, ConvBoolToInt, src);
  if (srcType <= TDbl)  return gen(env, ConvDblToInt, src);
  if (srcType <= TStr)  return gen(env, ConvStrToInt, src);
  if (srcType <= TObj)  return gen(env, ConvObjToInt, inst->taken(), src);
  if (srcType <= TRes)  return gen(env, ConvResToInt, src);
  if (srcType <= TFunc) {
    if (RuntimeOption::EvalRaiseFuncConversionWarning) {
      gen(env, RaiseWarning, catchTrace, cns(env, s_msgFuncToInt.get()));
    }
    return cns(env, 0);
  }
  if (srcType <= TClsMeth)  {
    if (RuntimeOption::EvalRaiseClsMethConversionWarning) {
      gen(env, RaiseNotice, catchTrace, cns(env, s_msgClsMethToInt.get()));
    }
    return cns(env, 1);
  }
  return nullptr;
}

SSATmp* simplifyConvTVToDbl(State& env, const IRInstruction* inst) {
  auto const src      = inst->src(0);
  auto const srcType  = src->type();
  auto const catchTrace = inst->taken();

  if (srcType <= TDbl)  return src;
  if (srcType <= TNull) return cns(env, 0.0);
  if (srcType <= TArr)  return gen(env, ConvArrToDbl, src);
  if (srcType <= TVec) {
    auto const length = gen(env, CountVec, src);
    return gen(env, ConvBoolToDbl, gen(env, ConvIntToBool, length));
  }
  if (srcType <= TDict) {
    auto const length = gen(env, CountDict, src);
    return gen(env, ConvBoolToDbl, gen(env, ConvIntToBool, length));
  }
  if (srcType <= TKeyset) {
    auto const length = gen(env, CountKeyset, src);
    return gen(env, ConvBoolToDbl, gen(env, ConvIntToBool, length));
  }
  if (srcType <= TBool) return gen(env, ConvBoolToDbl, src);
  if (srcType <= TInt)  return gen(env, ConvIntToDbl, src);
  if (srcType <= TStr)  return gen(env, ConvStrToDbl, src);
  if (srcType <= TObj)  return gen(env, ConvObjToDbl, inst->taken(), src);
  if (srcType <= TRes)  return gen(env, ConvResToDbl, src);
  if (srcType <= TFunc) {
    if (RuntimeOption::EvalRaiseFuncConversionWarning) {
      gen(env, RaiseWarning, catchTrace, cns(env, s_msgFuncToDbl.get()));
    }
    return cns(env, 0.0);
  }
  if (srcType <= TClsMeth)  {
    if (RuntimeOption::EvalRaiseClsMethConversionWarning) {
      gen(env, RaiseNotice, catchTrace, cns(env, s_msgClsMethToDbl.get()));
    }
    return cns(env, 1.0);
  }

  return nullptr;
}

SSATmp* simplifyConvObjToBool(State& env, const IRInstruction* inst) {
  auto const ty = inst->src(0)->type();

  if (ty < TObj &&
      ty.clsSpec().cls() &&
      ty.clsSpec().cls()->isCollectionClass()) {
    if (RuntimeOption::EvalNoticeOnCollectionToBool) return nullptr;
    return gen(env, ColIsNEmpty, inst->src(0));
  }
  return nullptr;
}


SSATmp* simplifyDblAsBits(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->hasConstVal()) {
    union {
      int64_t i;
      double d;
    };
    d = src->dblVal();
    return cns(env, i);
  }
  return nullptr;
}

SSATmp* roundImpl(State& env, const IRInstruction* inst, double (*op)(double)) {
  auto const src  = inst->src(0);

  if (src->hasConstVal()) {
    return cns(env, op(src->dblVal()));
  }

  auto const srcInst = src->inst();
  if (srcInst->op() == ConvIntToDbl || srcInst->op() == ConvBoolToDbl) {
    return src;
  }

  return nullptr;
}

SSATmp* simplifyFloor(State& env, const IRInstruction* inst) {
  return roundImpl(env, inst, floor);
}

SSATmp* simplifyCeil(State& env, const IRInstruction* inst) {
  return roundImpl(env, inst, ceil);
}

SSATmp* simplifyCheckInit(State& env, const IRInstruction* inst) {
  auto const srcType = inst->src(0)->type();
  assertx(!srcType.maybe(TMemToCell));
  assertx(inst->taken());
  if (!srcType.maybe(TUninit)) return gen(env, Nop);
  return mergeBranchDests(env, inst);
}

SSATmp* simplifyCheckInitMem(State& env, const IRInstruction* inst) {
  return mergeBranchDests(env, inst);
}

SSATmp* simplifyCheckRDSInitialized(State& env, const IRInstruction* inst) {
  auto const handle = inst->extra<CheckRDSInitialized>()->handle;
  if (!rds::isNormalHandle(handle)) return gen(env, Jmp, inst->next());
  return mergeBranchDests(env, inst);
}

SSATmp* simplifyMarkRDSInitialized(State& env, const IRInstruction* inst) {
  auto const handle = inst->extra<MarkRDSInitialized>()->handle;
  if (!rds::isNormalHandle(handle)) return gen(env, Nop);
  return nullptr;
}

SSATmp* simplifyInitObjProps(State& env, const IRInstruction* inst) {
  auto const cls = inst->extra<InitObjProps>()->cls;
  if (cls->numDeclProperties() == 0 && !cls->hasMemoSlots()) {
    return gen(env, Nop);
  }
  return nullptr;
}

SSATmp* simplifyInitObjMemoSlots(State& env, const IRInstruction* inst) {
  auto const cls = inst->extra<InitObjMemoSlots>()->cls;
  if (!cls->hasMemoSlots()) {
    return gen(env, Nop);
  }
  return nullptr;
}

SSATmp* simplifyCheckType(State& env, const IRInstruction* inst) {
  auto const typeParam = inst->typeParam();
  auto const srcType = inst->src(0)->type();

  if (!srcType.maybe(typeParam) || inst->next() == inst->taken()) {
    /*
     * Convert the check into a Jmp.  The dest of the CheckType (which would've
     * been Bottom) is now never going to be defined, so we return a Bottom.
     */
    gen(env, Jmp, inst->taken());
    return cns(env, TBottom);
  }

  auto const newType = srcType & typeParam;
  if (srcType <= newType) {
    // The type of the src is the same or more refined than type, so the guard
    // is unnecessary.
    return inst->src(0);
  }

  return nullptr;
}

namespace {

SSATmp* implSimplifyHackArrTypehint(
    State& env, const IRInstruction* inst, SSATmp* arr) {
  auto const extra = inst->extra<RaiseHackArrTypehintNoticeData>();
  auto const type = [&]{
    if (RO::EvalHackArrCompatTypeHintPolymorphism) return TBottom;
    switch (extra->tc.type()) {
      case AnnotType::VArrOrDArr: return TVArr | TDArr;
      case AnnotType::VArray:     return TVArr;
      case AnnotType::DArray:     return TDArr;
      default:                    return TBottom;
    }
  }();
  assertx(arr->isA(TArr));
  return arr->isA(type) ? gen(env, Nop) : nullptr;
}

}

SSATmp* simplifyRaiseHackArrParamNotice(State& env, const IRInstruction* inst) {
  return implSimplifyHackArrTypehint(env, inst, inst->src(0));
}

SSATmp* simplifyRaiseHackArrPropNotice(State& env, const IRInstruction* inst) {
  return implSimplifyHackArrTypehint(env, inst, inst->src(1));
}

SSATmp* simplifyCheckVArray(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->type() <= TVArr) {
    gen(env, Nop);
    return src;
  } else if (!src->type().maybe(TVArr.widenToBespoke())) {
    gen(env, Jmp, inst->taken());
    return cns(env, TBottom);
  }
  return nullptr;
}

SSATmp* simplifyCheckDArray(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->type() <= TDArr) {
    gen(env, Nop);
    return src;
  } else if (!src->type().maybe(TDArr.widenToBespoke())) {
    gen(env, Jmp, inst->taken());
    return cns(env, TBottom);
  }
  return nullptr;
}

SSATmp* simplifyCheckTypeMem(State& env, const IRInstruction* inst) {
  if (inst->next() == inst->taken() ||
      inst->typeParam() == TBottom) {
    return gen(env, Jmp, inst->taken());
  }

  return nullptr;
}

SSATmp* simplifyAssertType(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);

  auto const newType = src->type() & inst->typeParam();
  if (newType == TBottom) {
    gen(env, Unreachable, ASSERT_REASON);
    return cns(env, TBottom);
  }

  return src->isA(newType) ? src : nullptr;
}

SSATmp* simplifyCheckLoc(State& env, const IRInstruction* inst) {
  return mergeBranchDests(env, inst);
}

SSATmp* simplifyCheckStk(State& env, const IRInstruction* inst) {
  return mergeBranchDests(env, inst);
}

SSATmp* simplifyCheckMBase(State& env, const IRInstruction* inst) {
  return mergeBranchDests(env, inst);
}

SSATmp* simplifyCheckNonNull(State& env, const IRInstruction* inst) {
  auto const type = inst->src(0)->type();

  if (type <= TNullptr || inst->next() == inst->taken()) {
    gen(env, Jmp, inst->taken());
    return cns(env, TBottom);
  }

  if (!type.maybe(TNullptr)) return inst->src(0);

  return nullptr;
}

SSATmp* simplifyCheckInOuts(State& env, const IRInstruction* inst) {
  if (!inst->src(0)->hasConstVal()) return nullptr;

  auto const func = inst->src(0)->funcVal();
  auto const extra = inst->extra<CheckInOuts>();
  auto i = extra->firstBit;
  auto m = extra->mask;
  auto v = extra->vals;
  while (m) {
    if (m & 1) {
      if (func->isInOut(i) != (v & 1)) {
        // This shouldn't happen - the mask/value are predictions
        // based on previously seen Funcs; but we're now claiming its
        // always this Func. But unreachable code mumble mumble.
        return gen(env, Jmp, inst->taken());
      }
    }
    m >>= 1;
    v >>= 1;
    i++;
  }

  return gen(env, Nop);
}

SSATmp* simplifyDefLabel(State& env, const IRInstruction* inst) {
  if (inst->numDsts() == 0) {
    return gen(env, Nop);
  }
  return nullptr;
}

SSATmp* decRefImpl(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (noop_decref || !src->type().maybe(TCounted)) {
    return gen(env, Nop);
  }
  return nullptr;
}

SSATmp* simplifyDecRef(State& env, const IRInstruction* inst) {
  return decRefImpl(env, inst);
}

SSATmp* simplifyDecRefNZ(State& env, const IRInstruction* inst) {
  if (one_bit_refcount) return gen(env, Nop);

  return decRefImpl(env, inst);
}

SSATmp* simplifyIncRef(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (!src->type().maybe(TCounted)) {
    return gen(env, Nop);
  }
  return nullptr;
}

SSATmp* condJmpImpl(State& env, const IRInstruction* inst) {
  assertx(inst->is(JmpZero, JmpNZero));
  // Both ways go to the same block.
  if (inst->taken() == inst->next()) {
    assertx(inst->taken() != nullptr);
    return gen(env, Jmp, inst->taken());
  }

  auto const src = inst->src(0);
  auto const srcInst = src->inst();

  // Constant propagate.
  if (src->hasConstVal()) {
    bool val = src->isA(TBool) ? src->boolVal()
                               : static_cast<bool>(src->intVal());
    if (val == inst->is(JmpNZero)) {
      // always taken
      assertx(inst->taken());
      return gen(env, Jmp, inst->taken());
    }
    // Never taken. Since simplify() is also run when building the IR,
    // inst->next() could be nullptr at this moment.
    return gen(env, Nop);
  }

  auto absorb = [&](){
    return gen(env, inst->op(), inst->taken(), srcInst->src(0));
  };
  auto absorbOpp = [&](){
    return gen(env,
               inst->op() == JmpZero ? JmpNZero : JmpZero,
               inst->taken(),
               srcInst->src(0)
              );
  };

  // Absorb negations.
  if (srcInst->is(XorBool) && srcInst->src(1)->hasConstVal(true)) {
    return absorbOpp();
  }

  // Absorb ConvIntToBool.
  if (srcInst->is(ConvIntToBool)) {
    return absorb();
  }
  if (srcInst->is(ConvBoolToInt)) {
    return absorb();
  }

  // Absorb boolean comparisons.
  if (srcInst->is(EqBool) && srcInst->src(1)->hasConstVal()) {
    return srcInst->src(1)->boolVal() ? absorb() : absorbOpp();
  }
  if (srcInst->is(NeqBool) && srcInst->src(1)->hasConstVal()) {
    return srcInst->src(1)->boolVal() ? absorbOpp() : absorb();
  }

  // Absorb integer comparisons against constant zero.
  if (srcInst->is(EqInt) && srcInst->src(1)->hasConstVal(0)) {
    return absorbOpp();
  }
  if (srcInst->is(NeqInt) && srcInst->src(1)->hasConstVal(0)) {
    return absorb();
  }

  return nullptr;
}

SSATmp* simplifyJmpZero(State& env, const IRInstruction* i) {
  return condJmpImpl(env, i);
}

SSATmp* simplifyJmpNZero(State& env, const IRInstruction* i) {
  return condJmpImpl(env, i);
}

SSATmp* simplifySelect(State& env, const IRInstruction* inst) {
  auto const cond = inst->src(0);
  auto const tval = inst->src(1);
  auto const fval = inst->src(2);

  // Simplifications based on condition:

  if (cond->hasConstVal()) {
    auto const cval = cond->isA(TBool)
      ? cond->boolVal()
      : static_cast<bool>(cond->intVal());
    return cval ? tval : fval;
  }

  // The condition isn't statically known, but could be computed from a
  // different operation we can absorb.
  auto const condInst = cond->inst();
  auto const absorb = [&]{
    return gen(env, Select, condInst->src(0), tval, fval);
  };
  auto const absorbOpp = [&]{
    return gen(env, Select, condInst->src(0), fval, tval);
  };

  // Conversions between int and bool can't change which value is selected.
  if (condInst->is(ConvIntToBool)) return absorb();
  if (condInst->is(ConvBoolToInt)) return absorb();

  // Condition is negated, so check the pre-negated condition with flipped
  // values.
  if (condInst->is(XorBool) && condInst->src(1)->hasConstVal(true)) {
    return absorbOpp();
  }

  // Condition comes from comparisons against true/false or 0, which is
  // equivalent to selecting on original value (possibly with values flipped).
  if (condInst->is(EqBool) && condInst->src(1)->hasConstVal()) {
    return condInst->src(1)->boolVal() ? absorb() : absorbOpp();
  }
  if (condInst->is(NeqBool) && condInst->src(1)->hasConstVal()) {
    return condInst->src(1)->boolVal() ? absorbOpp() : absorb();
  }
  if (condInst->is(EqInt) && condInst->src(1)->hasConstVal(0)) {
    return absorbOpp();
  }
  if (condInst->is(NeqInt) && condInst->src(1)->hasConstVal(0)) {
    return absorb();
  }

  // Condition comes from Select C, X, 0 or Select C, 0, X (where X != 0), which
  // is equivalent to selecting on C directly.
  if (condInst->is(Select)) {
    auto const condInstT = condInst->src(1);
    auto const condInstF = condInst->src(2);
    if (condInstT->hasConstVal(TInt) &&
        condInstT->intVal() != 0 &&
        condInstF->hasConstVal(0)) {
      return absorb();
    }
    if (condInstT->hasConstVal(0) &&
        condInstF->hasConstVal(TInt) &&
        condInstF->intVal() != 0) {
      return absorbOpp();
    }
  }

  // Simplifications based on known value choices:

  // If the two values are the same tmp, or if they're both constants with the
  // same value, no need to do a select, as we'll always get the same value.
  if (tval == fval) return tval;
  if ((tval->type() == fval->type()) &&
      (tval->type().hasConstVal() ||
       tval->type().subtypeOfAny(TUninit, TInitNull, TNullptr))) {
    return tval;
  }

  // If either value isn't satisfiable, assume it comes from unreachable code.
  if (tval->isA(TBottom)) return fval;
  if (fval->isA(TBottom)) return tval;

  // If the values are true and false (and vice-versa), then its equal to the
  // value of the condition itself (or inverted).
  if (tval->hasConstVal(true) && fval->hasConstVal(false)) {
    if (cond->isA(TBool)) return cond;
    if (cond->isA(TInt)) return gen(env, NeqInt, cond, cns(env, 0));
  }
  if (tval->hasConstVal(false) && fval->hasConstVal(true)) {
    if (cond->isA(TBool)) return gen(env, XorBool, cond, cns(env, true));
    if (cond->isA(TInt)) return gen(env, EqInt, cond, cns(env, 0));
  }

  // Select C, 0, 1 or Select C, 1, 0 is the same as a bool -> int conversion.
  if (tval->hasConstVal(1) && fval->hasConstVal(0) && cond->isA(TBool)) {
    return gen(env, ConvBoolToInt, cond);
  }
  if (tval->hasConstVal(0) && fval->hasConstVal(1) && cond->isA(TBool)) {
    return gen(env, ConvBoolToInt, gen(env, XorBool, cond, cns(env, true)));
  }

  return nullptr;
}

SSATmp* simplifyAssertNonNull(State& /*env*/, const IRInstruction* inst) {
  if (!inst->src(0)->type().maybe(TNullptr)) {
    return inst->src(0);
  }
  return nullptr;
}

SSATmp* simplifyCheckPackedArrayDataBounds(State& env,
                                           const IRInstruction* inst) {
  auto const array = inst->src(0);
  auto const idx   = inst->src(1);

  auto const idxVal = idx->hasConstVal()
    ? folly::Optional<int64_t>(idx->intVal())
    : folly::none;
  switch (packedArrayBoundsStaticCheck(array->type(), idxVal)) {
  case PackedBounds::In:       return gen(env, Nop);
  case PackedBounds::Out:      return gen(env, Jmp, inst->taken());
  case PackedBounds::Unknown:  break;
  }

  return mergeBranchDests(env, inst);
}

SSATmp* simplifyReservePackedArrayDataNewElem(State& env,
                                              const IRInstruction* inst) {
  auto const base = inst->src(0);

  if (base->type() <= (TPersistentArr|TPersistentVec)) {
    gen(env, Unreachable, ASSERT_REASON);
    return cns(env, TBottom);
  }
  return nullptr;
}

SSATmp* arrKeyImpl(State& env, const IRInstruction* inst) {
  auto const arr = inst->src(0);
  auto const key = inst->src(1);
  assertx(arr->hasConstVal(TArr));
  assertx(key->hasConstVal(TInt|TStr));
  assertx(arr->arrVal()->isPHPArrayType());
  auto const rval = key->isA(TInt) ? arr->arrVal()->rval(key->intVal())
                                   : arr->arrVal()->rval(key->strVal());
  return rval ? cns(env, rval.tv()) : nullptr;
}

SSATmp* simplifyArrayGet(State& env, const IRInstruction* inst) {
  if (inst->src(0)->hasConstVal() && inst->src(1)->hasConstVal()) {
    if (auto const result = arrKeyImpl(env, inst)) return result;
    auto const mode = inst->extra<ArrayGet>()->mode;
    if (mode == MOpMode::None) return cns(env, TInitNull);
    auto const data = ArrayGetExceptionData { mode == MOpMode::InOut };
    auto const op = inst->src(1)->isA(TInt) ? ThrowArrayIndexException
                                            : ThrowArrayKeyException;
    gen(env, op, data, inst->taken(), inst->src(1));
    return cns(env, TBottom);
  }
  return nullptr;
}

SSATmp* simplifyArrayIsset(State& env, const IRInstruction* inst) {
  if (inst->src(0)->hasConstVal() && inst->src(1)->hasConstVal()) {
    auto const result = arrKeyImpl(env, inst);
    return cns(env, result && !result->isA(TInitNull));
  }
  return nullptr;
}

SSATmp* simplifyArrayIdx(State& env, const IRInstruction* inst) {
  if (inst->src(0)->hasConstVal() && inst->src(1)->hasConstVal()) {
    auto const result = arrKeyImpl(env, inst);
    return result ? result : inst->src(2);
  }
  return nullptr;
}

SSATmp* simplifyAKExistsArr(State& env, const IRInstruction* inst) {
  if (inst->src(0)->hasConstVal() && inst->src(1)->hasConstVal()) {
    auto const result = arrKeyImpl(env, inst);
    return cns(env, (bool)result);
  }
  return nullptr;
}

namespace {

SSATmp* arrGetKImpl(State& env, const IRInstruction* inst) {
  auto const arr = inst->src(0);
  auto const& extra = inst->extra<IndexData>();

  assertx(validPos(ssize_t(extra->index)));
  if (!arr->hasConstVal()) return nullptr;

  auto const mixed = MixedArray::asMixed(arr->arrLikeVal());
  auto const tv = mixed->getArrayElmPtr(extra->index);

  // The array doesn't contain a valid element at that offset. Since this
  // instruction should be guarded by a check, this (should be) unreachable.
  if (!tv) {
    gen(env, Unreachable, ASSERT_REASON);
    return cns(env, TBottom);
  }

  assertx(tvIsPlausible(*tv));
  return cns(env, *tv);
}

SSATmp* checkOffsetImpl(State& env, const IRInstruction* inst) {
  auto const arr = inst->src(0);
  auto const key = inst->src(1);
  auto const& extra = inst->extra<IndexData>();

  assertx(validPos(ssize_t(extra->index)));
  if (!arr->hasConstVal()) return mergeBranchDests(env, inst);

  auto const mixed = MixedArray::asMixed(arr->arrLikeVal());

  auto const dataTV = mixed->getArrayElmPtr(extra->index);
  if (!dataTV) return gen(env, Jmp, inst->taken());
  assertx(tvIsPlausible(*dataTV));

  auto const keyTV = mixed->getArrayElmKey(extra->index);
  assertx(isIntType(keyTV.m_type) || isStringType(keyTV.m_type));

  if (key->isA(TInt)) {
    if (isIntType(keyTV.m_type)) {
      auto const cmp = gen(env, EqInt, key, cns(env, keyTV));
      return gen(env, JmpZero, inst->taken(), cmp);
    }
    return gen(env, Jmp, inst->taken());
  } else if (key->isA(TStr)) {
    if (isStringType(keyTV.m_type)) {
      auto const cmp = gen(env, EqStrPtr, key, cns(env, keyTV));
      return gen(env, JmpZero, inst->taken(), cmp);
    }
    return gen(env, Jmp, inst->taken());
  }

  return mergeBranchDests(env, inst);
}

template <typename I, typename S, typename F>
SSATmp* hackArrQueryImpl(State& /*env*/, const IRInstruction* inst, I getInt,
                         S getStr, F finish) {
  auto const arr = inst->src(0);
  auto const key = inst->src(1);

  if (!arr->hasConstVal()) return nullptr;
  if (!key->hasConstVal(TInt) && !key->hasConstVal(TStr)) return nullptr;

  auto const value = key->hasConstVal(TInt)
    ? getInt(arr, key->intVal())
    : getStr(arr, key->strVal());
  return finish(value);
}

template <typename I, typename S>
SSATmp* hackArrGetImpl(State& env, const IRInstruction* inst,
                       I getInt, S getStr) {
  return hackArrQueryImpl(
    env, inst,
    getInt, getStr,
    [&] (tv_rval rval) {
      if (rval) return cns(env, rval.tv());
      gen(env, ThrowOutOfBounds, inst->taken(), inst->src(0), inst->src(1));
      return cns(env, TBottom);
    }
  );
}

template <typename I, typename S>
SSATmp* hackArrGetQuietImpl(State& env, const IRInstruction* inst,
                            I getInt, S getStr) {
  return hackArrQueryImpl(
    env, inst,
    getInt, getStr,
    [&] (tv_rval rval) {
      return rval ? cns(env, rval.tv()) : cns(env, TInitNull);
    }
  );
}

template <typename I, typename S>
SSATmp* hackArrIssetImpl(State& env, const IRInstruction* inst,
                         I getInt, S getStr) {
  return hackArrQueryImpl(
    env, inst,
    getInt, getStr,
    [&] (tv_rval rval) { return cns(env, rval && !tvIsNull(rval.tv())); }
  );
}

template <typename I, typename S>
SSATmp* hackArrIdxImpl(State& env, const IRInstruction* inst,
                       I getInt, S getStr) {
  return hackArrQueryImpl(
    env, inst,
    getInt, getStr,
    [&] (tv_rval rval) { return rval ? cns(env, rval.tv()) : inst->src(2); }
  );
}

template <typename I, typename S>
SSATmp* hackArrAKExistsImpl(State& env, const IRInstruction* inst,
                            I getInt, S getStr) {
  return hackArrQueryImpl(
    env, inst,
    getInt, getStr,
    [&] (tv_rval rval) { return cns(env, !!rval); }
  );
}

}

#define X(Name, Action, Get)                                          \
SSATmp* simplify##Name(State& env, const IRInstruction* inst) {       \
  return hackArr##Action##Impl(                                       \
    env, inst,                                                        \
    [](SSATmp* a, int64_t k) {                                        \
      return MixedArray::NvGetIntDict(a->Get(), k);                   \
    },                                                                \
    [](SSATmp* a, const StringData* k) {                              \
      return MixedArray::NvGetStrDict(a->Get(), k);                   \
    }                                                                 \
  );                                                                  \
}

X(DictGet, Get, dictVal)
X(DictGetQuiet, GetQuiet, dictVal)
X(DictIsset, Isset, dictVal)
X(DictIdx, Idx, dictVal)
X(AKExistsDict, AKExists, dictVal)

#undef X

#define X(Name, Action, Get)                                          \
SSATmp* simplify##Name(State& env, const IRInstruction* inst) {       \
  return hackArr##Action##Impl(                                       \
    env, inst,                                                        \
    [](SSATmp* a, int64_t k) {                                        \
      return SetArray::NvGetInt(a->Get(), k);                         \
    },                                                                \
    [](SSATmp* a, const StringData* k) {                              \
      return SetArray::NvGetStr(a->Get(), k);                         \
    }                                                                 \
  );                                                                  \
}

X(KeysetGet, Get, keysetVal)
X(KeysetGetQuiet, GetQuiet, keysetVal)
X(KeysetIsset, Isset, keysetVal)
X(KeysetIdx, Idx, keysetVal)
X(AKExistsKeyset, AKExists, keysetVal)

#undef X

SSATmp* simplifyMixedArrayGetK(State& env, const IRInstruction* inst) {
  return arrGetKImpl(env, inst);
}

SSATmp* simplifyDictGetK(State& env, const IRInstruction* inst) {
  return arrGetKImpl(env, inst);
}

SSATmp* simplifyKeysetGetK(State& env, const IRInstruction* inst) {
  auto const arr = inst->src(0);
  auto const& extra = inst->extra<IndexData>();

  assertx(validPos(ssize_t(extra->index)));
  if (!arr->hasConstVal()) return nullptr;

  auto const set = SetArray::asSet(arr->keysetVal());
  auto const tv = set->tvOfPos(extra->index);

  // The array doesn't contain a valid element at that offset. Since this
  // instruction should be guarded by a check, this (should be) unreachable.
  if (!tv) {
    gen(env, Unreachable, ASSERT_REASON);
    return cns(env, TBottom);
  }

  assertx(tvIsPlausible(*tv));
  assertx(isStringType(tv->m_type) || isIntType(tv->m_type));
  return cns(env, *tv);
}

SSATmp* simplifyGetMixedPtrIter(State& env, const IRInstruction* inst) {
  auto const arr = inst->src(0);
  auto const idx = inst->src(1);
  if (!arr->hasConstVal(TArrLike)) return nullptr;
  if (!idx->hasConstVal(TInt)) return nullptr;
  auto const ad  = MixedArray::asMixed(arr->arrLikeVal());
  auto const elm = ad->data() + idx->intVal();
  return cns(env, Type::cns(elm, outputType(inst)));
}

SSATmp* simplifyGetPackedPtrIter(State& env, const IRInstruction* inst) {
  auto const arr = inst->src(0);
  auto const idx = inst->src(1);
  if (!arr->hasConstVal(TArrLike)) return nullptr;
  if (!idx->hasConstVal(TInt)) return nullptr;
  auto const elm = packedData(arr->arrLikeVal()) + idx->intVal();
  return cns(env, Type::cns(elm, outputType(inst)));
}

SSATmp* simplifyCheckMixedArrayKeys(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (!src->hasConstVal()) return mergeBranchDests(env, inst);
  auto const arr = src->arrLikeVal();

  auto match = true;
  IterateKVNoInc(arr, [&](TypedValue key, TypedValue val){
    match &= Type::cns(key) <= inst->typeParam();
    return !match;
  });
  return match ? gen(env, Nop) : gen(env, Jmp, inst->taken());
}

SSATmp* simplifyCheckMixedArrayOffset(State& env, const IRInstruction* inst) {
  return checkOffsetImpl(env, inst);
}

SSATmp* simplifyCheckDictOffset(State& env, const IRInstruction* inst) {
  return checkOffsetImpl(env, inst);
}

SSATmp* simplifyCheckKeysetOffset(State& env, const IRInstruction* inst) {
  auto const arr = inst->src(0);
  auto const key = inst->src(1);
  auto const& extra = inst->extra<IndexData>();

  assertx(validPos(ssize_t(extra->index)));
  if (!arr->hasConstVal()) return mergeBranchDests(env, inst);

  auto const set = SetArray::asSet(arr->keysetVal());
  auto const tv = set->tvOfPos(extra->index);
  if (!tv) return gen(env, Jmp, inst->taken());
  assertx(tvIsPlausible(*tv));
  assertx(isStringType(tv->m_type) || isIntType(tv->m_type));

  if (key->isA(TInt)) {
    if (isIntType(tv->m_type)) {
      auto const cmp = gen(env, EqInt, key, cns(env, *tv));
      return gen(env, JmpZero, inst->taken(), cmp);
    }
    return gen(env, Jmp, inst->taken());
  } else if (key->isA(TStr)) {
    if (isStringType(tv->m_type)) {
      auto const cmp = gen(env, EqStrPtr, key, cns(env, *tv));
      return gen(env, JmpZero, inst->taken(), cmp);
    }
    return gen(env, Jmp, inst->taken());
  }

  return mergeBranchDests(env, inst);
}

SSATmp* simplifyCheckArrayCOW(State& env, const IRInstruction* inst) {
  auto const arr = inst->src(0);
  if (arr->isA(TPersistentArrLike)) return gen(env, Jmp, inst->taken());
  return nullptr;
}

SSATmp* simplifyCount(State& env, const IRInstruction* inst) {
  auto const val = inst->src(0);
  auto const ty = val->type();

  if (ty <= TNull) return cns(env, 0);

  auto const oneTy = TBool | TInt | TDbl | TStr | TRes;
  if (ty <= oneTy) return cns(env, 1);

  if (!RO::EvalAllowBespokeArrayLikes || ty.arrSpec().vanilla()) {
    if (ty <= TArr) return gen(env, CountArray, val);
    if (ty <= TVec) return gen(env, CountVec, val);
    if (ty <= TDict) return gen(env, CountDict, val);
    if (ty <= TKeyset) return gen(env, CountKeyset, val);
  }

  if (ty < TObj) {
    auto const cls = ty.clsSpec().cls();
    if (cls != nullptr && cls->isCollectionClass()) {
      return gen(env, CountCollection, val);
    }
  }
  return nullptr;
}


SSATmp* simplifyCountArray(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  auto const ty = src->type();

  if (src->hasConstVal()) return cns(env, src->arrVal()->size());

  auto const kind = ty.arrSpec().kind();

  if (kind && !arrayKindNeedsVsize(*kind))
    return gen(env, CountArrayFast, src);

  return nullptr;
}

namespace {
SSATmp* simplifyCountHelper(
  State& env,
  const IRInstruction* inst,
  const Type& ty
) {
  auto const src = inst->src(0);
  if (src->hasConstVal(ty)) return cns(env, src->arrLikeVal()->size());

  auto const at = src->type().arrSpec().type();
  if (!at) return nullptr;
  using A = RepoAuthType::Array;
  switch (at->tag()) {
  case A::Tag::Packed:
    if (at->emptiness() == A::Empty::No) return cns(env, at->size());
    break;
  case A::Tag::PackedN:
    break;
  }
  return nullptr;
}
}

SSATmp* simplifyCountArrayFast(State& env, const IRInstruction* inst) {
  return simplifyCountHelper(env, inst, TArr);
}

SSATmp* simplifyCountVec(State& env, const IRInstruction* inst) {
  return simplifyCountHelper(env, inst, TVec);
}

SSATmp* simplifyCountDict(State& env, const IRInstruction* inst) {
  return simplifyCountHelper(env, inst, TDict);
}

SSATmp* simplifyCountKeyset(State& env, const IRInstruction* inst) {
  return simplifyCountHelper(env, inst, TKeyset);
}

SSATmp* simplifyLdClsName(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  return src->hasConstVal(TCls) ? cns(env, src->clsVal()->name()) : nullptr;
}

SSATmp* simplifyLookupClsRDS(State& /*env*/, const IRInstruction* inst) {
  auto const name = inst->src(0);
  if (name->inst()->is(LdClsName)) {
    return name->inst()->src(0);
  }
  return nullptr;
}

SSATmp* simplifyLookupSPropSlot(State& env, const IRInstruction* inst) {
  auto const cls = inst->src(0);
  auto const name = inst->src(1);
  if (!cls->hasConstVal(TCls) || !name->hasConstVal(TStr)) return nullptr;
  return cns(env, cls->clsVal()->lookupSProp(name->strVal()));
}

SSATmp* simplifyLdStrLen(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  return src->hasConstVal(TStr) ? cns(env, src->strVal()->size()) : nullptr;
}

namespace {

SSATmp* packedLayoutLoadImpl(State& env,
                             const IRInstruction* inst,
                             bool isVec) {
  auto const src0 = inst->src(0);
  auto const src1 = inst->src(1);
  if (src0->hasConstVal() && src1->hasConstVal(TInt)) {
    auto const arr = src0->arrLikeVal();
    auto const idx = src1->intVal();
    assertx(arr->hasVanillaPackedLayout());
    if (idx >= 0) {
      auto const rval = isVec
        ? PackedArray::NvGetIntVec(arr, idx)
        : PackedArray::NvGetInt(arr, idx);
      return rval ? cns(env, rval.tv()) : nullptr;
    }
  }
  return nullptr;
}

}

SSATmp* simplifyLdVecElem(State& env, const IRInstruction* inst) {
  return packedLayoutLoadImpl(env, inst, true);
}

SSATmp* simplifyLdPackedElem(State& env, const IRInstruction* inst) {
  return packedLayoutLoadImpl(env, inst, false);
}

template <class F>
SSATmp* simplifyByClass(State& /*env*/, const SSATmp* src, F f) {
  if (!src->isA(TObj)) return nullptr;
  if (auto const spec = src->type().clsSpec()) {
    return f(spec.cls(), spec.exact());
  }
  return nullptr;
}

SSATmp* simplifyCallBuiltin(State& env, const IRInstruction* inst) {
  if (inst->numSrcs() != 3) return nullptr;

  auto const thiz = inst->src(2);
  return simplifyByClass(
    env, thiz,
    [&](const Class* cls, bool) -> SSATmp* {
      auto const callee = inst->extra<CallBuiltin>()->callee;
      if (cls->isCollectionClass()) {
        if (callee->name()->isame(s_isEmpty.get())) {
          FTRACE(3, "simplifying collection: {}\n", callee->name()->data());
          return gen(env, ColIsEmpty, thiz);
        }
        if (callee->name()->isame(s_count.get())) {
          FTRACE(3, "simplifying collection: {}\n", callee->name()->data());
          return gen(env, CountCollection, thiz);
        }
        return nullptr;
      }

      if (cls->classof(c_Awaitable::classof())) {
        auto const genState = [&] (Opcode op, int64_t whstate) -> SSATmp* {
          // these methods all spring from the base class
          assertx(callee->cls()->name()->isame(s_Awaitable.get()));
          auto const state = gen(env, LdWHState, thiz);
          return gen(env, op, state, cns(env, whstate));
        };
        auto const methName = callee->name();
        if (methName->isame(s_isFinished.get())) {
          return genState(LteInt, int64_t{c_Awaitable::STATE_FAILED});
        }
        if (methName->isame(s_isSucceeded.get())) {
          return genState(EqInt, int64_t{c_Awaitable::STATE_SUCCEEDED});
        }
        if (methName->isame(s_isFailed.get())) {
          return genState(EqInt, int64_t{c_Awaitable::STATE_FAILED});
        }
      }

      return nullptr;
    });
}

SSATmp* simplifyIsWaitHandle(State& env, const IRInstruction* inst) {
  return simplifyByClass(
    env, inst->src(0),
    [&](const Class* cls, bool) -> SSATmp* {
      if (cls->classof(c_Awaitable::classof())) return cns(env, true);
      if (!isInterface(cls) &&
          !c_Awaitable::classof()->classof(cls)) {
        return cns(env, false);
      }
      return nullptr;
    });
}

SSATmp* simplifyLdWHState(State& env, const IRInstruction* inst) {
  auto const wh = canonical(inst->src(0));
  if (wh->inst()->is(CreateSSWH)) {
    return cns(env, int64_t{c_Awaitable::STATE_SUCCEEDED});
  }
  return nullptr;
}

SSATmp* simplifyLdWHResult(State& env, const IRInstruction* inst) {
  auto const wh = canonical(inst->src(0));
  if (wh->inst()->is(CreateSSWH)) {
    return wh->inst()->src(0);
  }
  return nullptr;
}

SSATmp* simplifyLdWHNotDone(State& env, const IRInstruction* inst) {
  return simplifyByClass(
    env, inst->src(0),
    [&](const Class* cls, bool) -> SSATmp* {
      if (cls->classof(c_StaticWaitHandle::classof())) return cns(env, 0);
      return nullptr;
    });
}

SSATmp* simplifyIsCol(State& env, const IRInstruction* inst) {
  return simplifyByClass(
    env, inst->src(0),
    [&](const Class* cls, bool) -> SSATmp* {
      if (cls->isCollectionClass()) return cns(env, true);
      if (!isInterface(cls)) return cns(env, false);
      return nullptr;
    });
}

SSATmp* simplifyHasToString(State& env, const IRInstruction* inst) {
  return simplifyByClass(
    env, inst->src(0),
    [&](const Class* cls, bool exact) -> SSATmp* {
      if (cls->getToString() != nullptr) return cns(env, true);
      if (exact) return cns(env, false);
      return nullptr;
    });
}

SSATmp* simplifyChrInt(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->hasConstVal(TInt)) {
    auto const str = makeStaticString(char(src->intVal() & 255));
    return cns(env, str);
  }
  return nullptr;
}

SSATmp* simplifyOrdStr(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->hasConstVal(TStr)) {
    // a static string is passed in, resolve with a constant.
    unsigned char first = src->strVal()->data()[0];
    return cns(env, int64_t{first});
  }
  if (src->inst()->is(ChrInt)) {
    return gen(env, AndInt, src->inst()->src(0), cns(env, 255));
  }
  return nullptr;
}

SSATmp* simplifyJmpSwitchDest(State& env, const IRInstruction* inst) {
  auto const index = inst->src(0);
  if (!index->hasConstVal(TInt)) return nullptr;

  auto const indexVal = index->intVal();
  auto const sp = inst->src(1);
  auto const fp = inst->src(2);
  auto const& extra = *inst->extra<JmpSwitchDest>();

  if (indexVal < 0 || indexVal >= extra.cases) {
    // Instruction is unreachable.
    return gen(env, Unreachable, ASSERT_REASON);
  }

  auto const newExtra = ReqBindJmpData {
    extra.targets[indexVal],
    extra.spOffBCFromFP,
    extra.spOffBCFromIRSP,
    TransFlags{}
  };
  return gen(env, ReqBindJmp, newExtra, sp, fp);
}

SSATmp* simplifyCheckRange(State& env, const IRInstruction* inst) {
  auto const val = inst->src(0);
  auto const limit = inst->src(1);

  // CheckRange returns (0 <= val < limit).
  if (val && val->hasConstVal(TInt)) {
    if (val->intVal() < 0) return cns(env, false);

    if (limit && limit->hasConstVal(TInt)) {
      return cns(env, val->intVal() < limit->intVal());
    }
  }

  if (limit && limit->hasConstVal(TInt) && limit->intVal() <= 0) {
    return cns(env, false);
  }

  return nullptr;
}

SSATmp* simplifyGetMemoKeyScalar(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);

  // Note: this all uses the fully generic memo key scheme. If we used a more
  // specific scheme, then the GetMemoKey op wouldn't have been emitted.

  if (src->hasConstVal()) {
    try {
      ThrowAllErrorsSetter taes;
      auto const key =
        HHVM_FN(serialize_memoize_param)(*src->variantVal().asTypedValue());
      SCOPE_EXIT { tvDecRefGen(key); };
      assertx(tvIsPlausible(key));
      if (tvIsString(&key)) {
        return cns(env, makeStaticString(key.m_data.pstr));
      } else {
        assertx(key.m_type == KindOfInt64);
        return cns(env, key.m_data.num);
      }
    } catch (...) {
    }
  }

  if (src->isA(TInt)) return src;
  if (src->isA(TBool)) {
    return gen(
      env,
      Select,
      src,
      cns(env, s_trueMemoKey.get()),
      cns(env, s_falseMemoKey.get())
    );
  }
  if (src->isA(TNull)) return cns(env, s_nullMemoKey.get());

  return nullptr;
}

SSATmp* simplifyGetMemoKey(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);

  if (auto ret = simplifyGetMemoKeyScalar(env, inst)) return ret;
  if (src->isA(TUncounted|TStr)) return gen(env, GetMemoKeyScalar, src);
  return nullptr;
}

SSATmp* simplifyStrictlyIntegerConv(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (!src->hasConstVal()) return nullptr;
  int64_t n;
  if (src->strVal()->isStrictlyInteger(n)) return cns(env, n);
  gen(env, IncRef, src);
  return src;
}

SSATmp* simplifyRaiseErrorOnInvalidIsAsExpressionType(
  State& env,
  const IRInstruction* inst
) {
  auto const ts = inst->src(0);
  if (!ts->hasConstVal(RuntimeOption::EvalHackArrDVArrs ? TDict : TArr)) {
    return nullptr;
  }
  auto const tsVal = ts->arrLikeVal();
  if (errorOnIsAsExpressionInvalidTypes(ArrNR(tsVal), true)) {
    gen(env, Unreachable, ASSERT_REASON);
    return cns(env, TBottom);
  }
  return ts;
}

//////////////////////////////////////////////////////////////////////

SSATmp* simplifyWork(State& env, const IRInstruction* inst) {
  env.insts.push(inst);
  SCOPE_EXIT {
    assertx(env.insts.top() == inst);
    env.insts.pop();
  };

#define X(x) case x: return simplify##x(env, inst);
  switch (inst->op()) {
  X(Shl)
  X(Shr)
  X(Lshr)
  X(AbsDbl)
  X(AssertNonNull)
  X(CallBuiltin)
  X(Ceil)
  X(CheckInit)
  X(CheckInitMem)
  X(CheckRDSInitialized)
  X(MarkRDSInitialized)
  X(CheckLoc)
  X(CheckMBase)
  X(CheckInOuts)
  X(CheckStk)
  X(CheckType)
  X(CheckTypeMem)
  X(RaiseHackArrParamNotice)
  X(RaiseHackArrPropNotice)
  X(CheckVArray)
  X(CheckDArray)
  X(AssertType)
  X(CheckNonNull)
  X(CheckPackedArrayDataBounds)
  X(ReservePackedArrayDataNewElem)
  X(ConcatStrStr)
  X(ConcatStr3)
  X(ConcatStr4)
  X(ConcatIntStr)
  X(ConcatStrInt)
  X(ConvArrToBool)
  X(ConvArrToDbl)
  X(ConvBoolToArr)
  X(ConvBoolToDbl)
  X(ConvBoolToInt)
  X(ConvTVToBool)
  X(ConvTVToDbl)
  X(ConvTVToInt)
  X(ConvTVToStr)
  X(ConvTVToArr)
  X(ConvDblToArr)
  X(ConvDblToBool)
  X(ConvDblToInt)
  X(ConvDblToStr)
  X(ConvIntToArr)
  X(ConvIntToBool)
  X(ConvIntToDbl)
  X(ConvIntToStr)
  X(ConvObjToBool)
  X(ConvStrToArr)
  X(ConvFuncToArr)
  X(ConvVecToArr)
  X(ConvDictToArr)
  X(ConvKeysetToArr)
  X(ConvClsMethToArr)
  X(ConvClsMethToVArr)
  X(ConvClsMethToDArr)
  X(ConvClsMethToVec)
  X(ConvClsMethToDict)
  X(ConvClsMethToKeyset)
  X(ConvStrToBool)
  X(ConvStrToDbl)
  X(ConvStrToInt)
  X(ConvArrToVec)
  X(ConvDictToVec)
  X(ConvKeysetToVec)
  X(ConvArrToDict)
  X(ConvVecToDict)
  X(ConvKeysetToDict)
  X(ConvArrToKeyset)
  X(ConvVecToKeyset)
  X(ConvDictToKeyset)
  X(ConvArrToVArr)
  X(ConvVecToVArr)
  X(ConvDictToVArr)
  X(ConvKeysetToVArr)
  X(ConvArrToDArr)
  X(ConvVecToDArr)
  X(ConvDictToDArr)
  X(ConvKeysetToDArr)
  X(DblAsBits)
  X(Count)
  X(CountArray)
  X(CountArrayFast)
  X(CountVec)
  X(CountDict)
  X(CountKeyset)
  X(DecRef)
  X(DecRefNZ)
  X(DefLabel)
  X(DivDbl)
  X(DivInt)
  X(EqFunc)
  X(ExtendsClass)
  X(InstanceOfBitmask)
  X(NInstanceOfBitmask)
  X(Floor)
  X(IncRef)
  X(InitObjProps)
  X(InitObjMemoSlots)
  X(InstanceOf)
  X(InstanceOfIface)
  X(InstanceOfIfaceVtable)
  X(IsNType)
  X(IsType)
  X(IsWaitHandle)
  X(IsCol)
  X(HasToString)
  X(LdCls)
  X(LdClsName)
  X(LdWHResult)
  X(LdWHState)
  X(LdWHNotDone)
  X(LookupClsRDS)
  X(LookupSPropSlot)
  X(LdClsMethod)
  X(LdStrLen)
  X(LdVecElem)
  X(LdPackedElem)
  X(MethodExists)
  X(FuncHasAttr)
  X(IsClsDynConstructible)
  X(LdFuncRxLevel)
  X(LdObjClass)
  X(LdObjInvoke)
  X(Mov)
  X(JmpZero)
  X(JmpNZero)
  X(Select)
  X(OrInt)
  X(AddInt)
  X(SubInt)
  X(MulInt)
  X(AddDbl)
  X(SubDbl)
  X(MulDbl)
  X(Mod)
  X(AndInt)
  X(XorInt)
  X(XorBool)
  X(AddIntO)
  X(SubIntO)
  X(MulIntO)
  X(GtBool)
  X(GteBool)
  X(LtBool)
  X(LteBool)
  X(EqBool)
  X(NeqBool)
  X(CmpBool)
  X(GtInt)
  X(GteInt)
  X(LtInt)
  X(LteInt)
  X(EqInt)
  X(NeqInt)
  X(CmpInt)
  X(GtStr)
  X(GteStr)
  X(LtStr)
  X(LteStr)
  X(EqStr)
  X(NeqStr)
  X(SameStr)
  X(NSameStr)
  X(CmpStr)
  X(GtStrInt)
  X(GteStrInt)
  X(LtStrInt)
  X(LteStrInt)
  X(EqStrInt)
  X(NeqStrInt)
  X(CmpStrInt)
  X(GtObj)
  X(GteObj)
  X(LtObj)
  X(LteObj)
  X(EqObj)
  X(NeqObj)
  X(SameObj)
  X(NSameObj)
  X(GtArr)
  X(GteArr)
  X(LtArr)
  X(LteArr)
  X(EqArr)
  X(NeqArr)
  X(SameArr)
  X(NSameArr)
  X(GtVec)
  X(GteVec)
  X(LtVec)
  X(LteVec)
  X(EqVec)
  X(NeqVec)
  X(SameVec)
  X(NSameVec)
  X(EqDict)
  X(NeqDict)
  X(SameDict)
  X(NSameDict)
  X(EqKeyset)
  X(NeqKeyset)
  X(SameKeyset)
  X(NSameKeyset)
  X(GtRes)
  X(GteRes)
  X(LtRes)
  X(LteRes)
  X(EqRes)
  X(NeqRes)
  X(CmpRes)
  X(EqCls)
  X(EqStrPtr)
  X(EqArrayDataPtr)
  X(ArrayGet)
  X(MixedArrayGetK)
  X(DictGet)
  X(DictGetQuiet)
  X(DictGetK)
  X(KeysetGet)
  X(KeysetGetQuiet)
  X(KeysetGetK)
  X(GetMixedPtrIter)
  X(GetPackedPtrIter)
  X(CheckMixedArrayKeys)
  X(CheckMixedArrayOffset)
  X(CheckDictOffset)
  X(CheckKeysetOffset)
  X(CheckArrayCOW)
  X(ArrayIsset)
  X(DictIsset)
  X(KeysetIsset)
  X(ArrayIdx)
  X(AKExistsArr)
  X(DictIdx)
  X(AKExistsDict)
  X(KeysetIdx)
  X(AKExistsKeyset)
  X(OrdStr)
  X(ChrInt)
  X(JmpSwitchDest)
  X(CheckRange)
  X(GetMemoKey)
  X(GetMemoKeyScalar)
  X(StrictlyIntegerConv)
  X(RaiseErrorOnInvalidIsAsExpressionType)
  X(LdFrameCls)
  default: break;
  }
#undef X
  return nullptr;
}

//////////////////////////////////////////////////////////////////////

}

///////////////////////////////////////////////////////////////////////////////

SimplifyResult simplify(IRUnit& unit, const IRInstruction* origInst) {
  auto env = State { unit };

  auto const newDst = simplifyWork(env, origInst);

  assertx(validate(env, newDst, origInst));

  return SimplifyResult { std::move(env.newInsts), newDst };
}

void simplifyInPlace(IRUnit& unit, IRInstruction* origInst) {
  assertx(!origInst->isTransient());

  copyProp(origInst);
  constProp(unit, origInst);
  auto res = simplify(unit, origInst);

  // No simplification occurred; nothing to do.
  if (res.instrs.empty() && !res.dst) return;

  FTRACE(1, "simplifying: {}\n", origInst->toString());

  if (origInst->isBlockEnd()) {
    auto const next = origInst->block()->next();

    if (res.instrs.empty() || !res.instrs.back()->isBlockEnd()) {
      // Our block-end instruction was eliminated (most likely a Jmp* converted
      // to a Nop).  Replace it with a Jmp to the next block.
      res.instrs.push_back(unit.gen(Jmp, origInst->bcctx(), next));
    }

    auto const last = res.instrs.back();
    assertx(last->isBlockEnd());

    if (!last->isTerminal() && !last->next()) {
      // We converted the block-end instruction to a different one.  Set its
      // next block appropriately.
      last->setNext(next);
    }
  }

  size_t out_size = 0;
  bool need_mov = res.dst;
  IRInstruction* last = nullptr;

  for (auto const inst : res.instrs) {
    if (inst->is(Nop)) continue;

    ++out_size;
    last = inst;

    if (res.dst && res.dst == inst->dst()) {
      // One of the new instructions produced the new dst.  Since we're going
      // to drop `origInst', just use origInst->dst() instead.
      inst->setDst(origInst->dst());
      inst->dst()->setInstruction(inst);
      need_mov = false;
    }
  }

  auto const block = origInst->block();
  auto pos = ++block->iteratorTo(origInst);

  if (last != nullptr && last->isTerminal()) {
    // Delete remaining instructions in the block if a terminal is created.
    // This can happen, e.g., 'Unreachable' may be created when the block is
    // unreachable.
    while (pos != block->end()) pos = block->erase(pos);
  }

  if (need_mov) {
    /*
     * In `killed_edge_defining' we have the case that an instruction defining
     * a temp on an edge (like CheckType) determined it can never define that
     * tmp.  In this situation we just Nop out the instruction and leave the
     * old tmp dangling.  The reason this is ok is that one of the following
     * two things are happening:
     *
     *    o The old next() block is becoming unreachable.  It's ok not to make
     *      a new definition of this tmp, because the code running simplify is
     *      going to have to track unreachable blocks and avoid looking at
     *      them.  It will also have to remove unreachable blocks when it's
     *      finished to maintain IR invariants (e.g. through DCE::Minimal),
     *      which will mean the uses of the no-longer-defined tmp will go away.
     *
     *    o The old next() block is still reachable (e.g. if we're removing a
     *      CheckType because it had next == taken).  But in this case, the
     *      next() edge must have been a critical edge, and therefore nothing
     *      could have any use of the old destination of the CheckType, or the
     *      program would already not have been in SSA, because it was only
     *      defined in blocks dominated by the next edge.
     */
    auto const killed_edge_defining = res.dst->type() <= TBottom &&
      origInst->isBlockEnd();
    if (killed_edge_defining) {
      origInst->convertToNop();
    } else {
      unit.replace(origInst, Mov, res.dst);
      // Force the existing dst type to match that of `res.dst'.
      origInst->dst()->setType(res.dst->type());
    }
  } else {
    if (out_size == 1) {
      assertx(origInst->dst() == last->dst());
      FTRACE(1, "    {}\n", last->toString());

      // We only have a single instruction, so just become it.
      origInst->become(unit, last);

      // Make sure to reset our dst's inst pointer, if we have one.  (It may
      // have been set to `last'.
      if (origInst->dst()) {
        origInst->dst()->setInstruction(origInst);
      }

      // And we also need to kill `last', to update preds.
      last->convertToNop();
      return;
    }

    origInst->convertToNop();
  }

  FTRACE(1, "    {}\n", origInst->toString());

  for (auto const inst : res.instrs) {
    if (inst->is(Nop)) continue;
    block->insert(pos, inst);
    FTRACE(1, "    {}\n", inst->toString());
  }
}

////////////////////////////////////////////////////////////////////////////////

void simplifyPass(IRUnit& unit) {
  auto reachable = boost::dynamic_bitset<>(unit.numBlocks());
  reachable.set(unit.entry()->id());

  for (auto block : rpoSortCfg(unit)) {
    if (!reachable.test(block->id())) continue;

    if (block->back().is(Unreachable)) {
      // Any code that's postdominated by Unreachable is also unreachable, so
      // erase everything else in this block.
      for (auto it = block->skipHeader(), end = block->backIter(); it != end;) {
        auto toErase = it;
        ++it;
        block->erase(toErase);
      }
    } else {
      for (auto& inst : *block) simplifyInPlace(unit, &inst);
    }

    if (auto const b = block->next())  reachable.set(b->id());
    if (auto const b = block->taken()) reachable.set(b->id());
  }
}

////////////////////////////////////////////////////////////////////////////////

}}
