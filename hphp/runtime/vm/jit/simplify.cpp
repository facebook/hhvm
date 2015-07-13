/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2014 Facebook, Inc. (http://www.facebook.com)     |
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

#include <limits>
#include <sstream>
#include <type_traits>

#include "hphp/util/overflow.h"
#include "hphp/util/trace.h"

#include "hphp/runtime/base/array-data-defs.h"
#include "hphp/runtime/base/repo-auth-type-array.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/ext/collections/ext_collections-idl.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/guard-relaxation.h"
#include "hphp/runtime/vm/jit/ir-builder.h"
#include "hphp/runtime/vm/jit/minstr-effects.h"
#include "hphp/runtime/vm/jit/punt.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/runtime.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(simplify);

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_Array("Array");
const StaticString s_isEmpty("isEmpty");
const StaticString s_count("count");
const StaticString s_1("1");
const StaticString s_empty("");
const StaticString s_invoke("__invoke");
const StaticString s_isFinished("isFinished");
const StaticString s_isSucceeded("isSucceeded");
const StaticString s_isFailed("isFailed");
const StaticString s_WaitHandle("HH\\WaitHandle");

//////////////////////////////////////////////////////////////////////

struct State {
  IRUnit& unit;
  const bool typesMightRelax;

  // The current instruction being simplified is always at insts.top(). This
  // has to be a stack instead of just a pointer because simplify is reentrant.
  jit::stack<const IRInstruction*> insts;
  jit::vector<IRInstruction*> newInsts;
};

//////////////////////////////////////////////////////////////////////

SSATmp* simplifyWork(State&, const IRInstruction*);

bool mightRelax(State& env, SSATmp* tmp) {
  if (!env.typesMightRelax) return false;
  return jit::typeMightRelax(tmp);
}

template<class... Args>
SSATmp* cns(State& env, Args&&... cns) {
  return env.unit.cns(std::forward<Args>(cns)...);
}

template<class... Args>
SSATmp* gen(State& env, Opcode op, BCMarker marker, Args&&... args) {
  return makeInstruction(
    [&] (IRInstruction* inst) -> SSATmp* {
      auto prevNewCount = env.newInsts.size();
      auto newDest = simplifyWork(env, inst);

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
    marker,
    std::forward<Args>(args)...
  );
}

template<class... Args>
SSATmp* gen(State& env, Opcode op, Args&&... args) {
  assertx(!env.insts.empty());
  return gen(env, op, env.insts.top()->marker(), std::forward<Args>(args)...);
}

bool arrayKindNeedsVsize(const ArrayData::ArrayKind kind) {
  switch (kind) {
    case ArrayData::kPackedKind:
    case ArrayData::kStructKind:
    case ArrayData::kMixedKind:
    case ArrayData::kEmptyKind:
    case ArrayData::kApcKind:
      return false;
    default:
      return true;
  }
}

//////////////////////////////////////////////////////////////////////

/*
 * Simplifier rules are not allowed to add new uses to SSATmps that aren't
 * known to be available.  All the sources to the original instruction must
 * be available, and non-reference counted values reachable through the
 * source chain are also always available.  Anything else requires more
 * complicated analysis than belongs in the simplifier right now.
 */
DEBUG_ONLY bool validate(const State& env,
                         SSATmp* newDst,
                         const IRInstruction* origInst) {
  auto known_available = [&] (SSATmp* src) -> bool {
    if (!src->type().maybe(TCounted)) return true;
    for (auto& oldSrc : origInst->srcs()) {
      if (oldSrc == src) return true;
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
      auto newInst = env.newInsts[i];

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
      "Simplifier produced a new destination that wasn't known to be "
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
    origInst->hasDst() == (bool)newDst,
    "HasDest mismatch between input and output"
  );

  assert_last(
    IMPLIES(last->isBlockEnd(), origInst->isBlockEnd()),
    "Block-end instruction produced for simplification of non-block-end "
    "instruction"
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
                   CheckInit,
                   CheckInitMem,
                   CheckInitProps,
                   CheckInitSProps,
                   CheckPackedArrayBounds,
                   CheckStaticLocInit,
                   CheckRefInner,
                   CheckCtxThis));
  if (inst->next() != nullptr && inst->next() == inst->taken()) {
    return gen(env, Jmp, inst->next());
  }
  return nullptr;
}

SSATmp* simplifyCheckCtxThis(State& env, const IRInstruction* inst) {
  auto const func = inst->marker().func();
  auto const srcTy = inst->src(0)->type();
  if (srcTy <= TObj) return gen(env, Nop);
  if (!func->mayHaveThis() || !srcTy.maybe(TObj)) {
    return gen(env, Jmp, inst->taken());
  }
  return mergeBranchDests(env, inst);
}

SSATmp* simplifyCastCtxThis(State& env, const IRInstruction* inst) {
  if (inst->src(0)->type() <= TObj) return inst->src(0);
  return nullptr;
}

SSATmp* simplifyLdClsCtx(State& env, const IRInstruction* inst) {
  SSATmp* ctx = inst->src(0);
  if (ctx->hasConstVal(TCctx)) {
    return cns(env, ctx->cctxVal().cls());
  }
  Type ctxType = ctx->type();
  if (ctxType <= TObj) {
    // this pointer... load its class ptr
    return gen(env, LdObjClass, ctx);
  }
  if (ctxType <= TCctx) {
    return gen(env, LdClsCctx, ctx);
  }
  return nullptr;
}

SSATmp* simplifyLdObjClass(State& env, const IRInstruction* inst) {
  auto const ty = inst->src(0)->type();

  if (mightRelax(env, inst->src(0)) || !(ty < TObj)) return nullptr;

  if (auto const exact = ty.clsSpec().exactCls()) return cns(env, exact);
  return nullptr;
}

SSATmp* simplifyLdObjInvoke(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (!src->hasConstVal()) return nullptr;

  auto const cls = src->clsVal();
  if (!rds::isPersistentHandle(cls->classHandle())) return nullptr;

  auto const meth = cls->getCachedInvoke();
  return meth == nullptr ? nullptr : cns(env, meth);
}

SSATmp* simplifyGetCtxFwdCall(State& env, const IRInstruction* inst) {
  auto const srcCtx = inst->src(0);
  if (srcCtx->isA(TCctx)) return srcCtx;
  return nullptr;
}

SSATmp* simplifyConvClsToCctx(State& env, const IRInstruction* inst) {
  auto* srcInst = inst->src(0)->inst();
  if (srcInst->is(LdClsCctx)) return srcInst->src(0);
  return nullptr;
}

SSATmp* simplifyMov(State& env, const IRInstruction* inst) {
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
  if (src1->hasConstVal() && src2->type().hasConstVal()) {
    return gen(env, opcode, src2, src1);
  }

  // Only handle integer operations for now.
  if (!src1->isA(TInt) || !src2->isA(TInt)) return nullptr;

  auto const inst1 = src1->inst();
  auto const inst2 = src2->inst();
  if (inst1->op() == opcode && inst1->src(1)->hasConstVal()) {
    // (X + C1) + C2 --> X + C3
    if (src2->hasConstVal()) {
      int64_t right = inst1->src(1)->intVal();
      right = op(right, src2->intVal());
      return gen(env, opcode, inst1->src(0), cns(env, right));
    }
    // (X + C1) + (Y + C2) --> X + Y + C3
    if (inst2->op() == opcode && inst2->src(1)->hasConstVal()) {
      int64_t right = inst1->src(1)->intVal();
      right = op(right, inst2->src(1)->intVal());
      SSATmp* left = gen(env, opcode, inst1->src(0), inst2->src(0));
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
template<class OutOper, class InOper>
SSATmp* distributiveImpl(State& env,
                         SSATmp* src1,
                         SSATmp* src2,
                         Opcode outcode,
                         Opcode incode,
                         OutOper outop,
                         InOper inop) {
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
      SSATmp* fold = gen(env, outcode, inst1->src(1), inst2->src(1));
      return gen(env, incode, inst1->src(0), fold);
    }
    if (inst1->src(0) == inst2->src(1)) {
      SSATmp* fold = gen(env, outcode, inst1->src(1), inst2->src(0));
      return gen(env, incode, inst1->src(0), fold);
    }
    if (inst1->src(1) == inst2->src(0)) {
      SSATmp* fold = gen(env, outcode, inst1->src(0), inst2->src(1));
      return gen(env, incode, inst1->src(1), fold);
    }
    if (inst1->src(1) == inst2->src(1)) {
      SSATmp* fold = gen(env, outcode, inst1->src(0), inst2->src(0));
      return gen(env, incode, inst1->src(1), fold);
    }
  }
  return nullptr;
}

SSATmp* simplifyAddInt(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);

  auto add = std::plus<int64_t>();
  auto mul = std::multiplies<int64_t>();
  if (auto simp = distributiveImpl(env, src1, src2, AddInt,
                                   MulInt, add, mul)) {
    return simp;
  }
  if (src2->hasConstVal()) {
    int64_t src2Val = src2->intVal();
    // X + 0 --> X
    if (src2Val == 0) return src1;

    // X + -C --> X - C
    // Weird, but can show up as a result of other simplifications. Don't need
    // to check for C == INT_MIN, simplifySubInt already checks.
    if (src2Val < 0) return gen(env, SubInt, src1, cns(env, -src2Val));
  }
  // X + (0 - Y) --> X - Y
  auto inst2 = src2->inst();
  if (inst2->op() == SubInt) {
    SSATmp* src = inst2->src(0);
    if (src->hasConstVal() && src->intVal() == 0) {
      return gen(env, SubInt, src1, inst2->src(1));
    }
  }
  auto inst1 = src1->inst();

  // (X - C1) + ...
  if (inst1->op() == SubInt && inst1->src(1)->hasConstVal()) {
    auto x = inst1->src(0);
    auto c1 = inst1->src(1);

    // (X - C1) + C2 --> X + (C2 - C1)
    if (src2->hasConstVal()) {
      auto rhs = gen(env, SubInt, cns(env, src2->intVal()), c1);
      return gen(env, AddInt, x, rhs);
    }

    // (X - C1) + (Y +/- C2)
    if ((inst2->op() == AddInt || inst2->op() == SubInt) &&
        inst2->src(1)->hasConstVal()) {
      auto y = inst2->src(0);
      auto c2 = inst2->src(1);
      SSATmp* rhs = nullptr;
      if (inst2->op() == SubInt) {
        // (X - C1) + (Y - C2) --> X + Y + (-C1 - C2)
        rhs = gen(env, SubInt, gen(env, SubInt, cns(env, 0), c1), c2);
      } else {
        // (X - C1) + (Y + C2) --> X + Y + (C2 - C1)
        rhs = gen(env, SubInt, c2, c1);
      }
      auto lhs = gen(env, AddInt, x, y);
      return gen(env, AddInt, lhs, rhs);
    }
    // (X - C1) + (Y + C2) --> X + Y + (C2 - C1)
    if (inst2->op() == AddInt && inst2->src(1)->hasConstVal()) {
      auto y = inst2->src(0);
      auto c2 = inst2->src(1);

      auto lhs = gen(env, AddInt, x, y);
      auto rhs = gen(env, SubInt, c2, c1);
      return gen(env, AddInt, lhs, rhs);
    }
  }

  return nullptr;
}

SSATmp* simplifyAddIntO(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);
  if (src1->hasConstVal() && src2->hasConstVal()) {
    int64_t a = src1->intVal();
    int64_t b = src2->intVal();
    return add_overflow(a, b)
      ? cns(env, double(a) + double(b))
      : cns(env, a + b);
  }
  return nullptr;
}

SSATmp* simplifySubInt(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);

  auto sub = std::minus<int64_t>();
  if (auto simp = constImpl(env, src1, src2, sub)) return simp;

  // X - X --> 0
  if (src1 == src2) return cns(env, 0);

  if (src2->hasConstVal()) {
    int64_t src2Val = src2->intVal();
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
  auto inst2 = src2->inst();
  if (inst2->op() == SubInt) {
    SSATmp* src = inst2->src(0);
    if (src->hasConstVal(0)) return gen(env, AddInt, src1, inst2->src(1));
  }
  return nullptr;
}

SSATmp* simplifySubIntO(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);
  if (src1->hasConstVal() && src2->hasConstVal()) {
    int64_t a = src1->intVal();
    int64_t b = src2->intVal();
    return sub_overflow(a, b)
      ? cns(env, double(a) - double(b))
      : cns(env, a - b);
  }
  return nullptr;
}

SSATmp* simplifyMulInt(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);

  auto mul = std::multiplies<int64_t>();
  if (auto simp = commutativeImpl(env, src1, src2, MulInt, mul)) return simp;

  if (!src2->hasConstVal()) return nullptr;

  int64_t rhs = src2->intVal();

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
    auto lhs = gen(env, Shl, src1, cns(env, log2(rhs - 1)));
    return gen(env, AddInt, lhs, src1);
  }
  // X * (2^C - 1) --> ((X << C) - X)
  if (isPowTwo(rhs + 1)) {
    auto lhs = gen(env, Shl, src1, cns(env, log2(rhs + 1)));
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
    int64_t a = src1->intVal();
    int64_t b = src2->intVal();
    return mul_overflow(a, b)
      ? cns(env, double(a) * double(b))
      : cns(env, a * b);
  }
  return nullptr;
}

SSATmp* simplifyMod(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);

  if (!src2->hasConstVal()) return nullptr;

  auto const src2Val = src2->intVal();
  if (src2Val == 0 || src2Val == -1) {
    // Undefined behavior, so we might as well constant propagate whatever we
    // want. If we're being asked to simplify this, it better be dynamically
    // unreachable code.
    return cns(env, 0);
  }

  if (src1->hasConstVal()) return cns(env, src1->intVal() % src2Val);
  // X % 1 --> 0
  if (src2Val == 1) return cns(env, 0);

  return nullptr;
}

SSATmp* simplifyDivDbl(State& env, const IRInstruction* inst) {
  auto src1 = inst->src(0);
  auto src2 = inst->src(1);

  if (!src2->hasConstVal()) return nullptr;

  // not supporting integers (#2570625)
  double src2Val = src2->dblVal();

  // X / 0 -> bool(false)
  if (src2Val == 0.0) {
    // Ideally we'd generate a RaiseWarning and return false here, but we need
    // a catch trace for that and we can't make a catch trace here.
    return nullptr;
  }

  // statically compute X / Y
  return src1->hasConstVal() ? cns(env, src1->dblVal() / src2Val) : nullptr;
}

SSATmp* simplifyAndInt(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);
  auto bit_and = [](int64_t a, int64_t b) { return a & b; };
  auto bit_or = [](int64_t a, int64_t b) { return a | b; };
  auto simp = distributiveImpl(env, src1, src2, AndInt, OrInt,
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
  auto simp = distributiveImpl(env, src1, src2, OrInt, AndInt,
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

  switch (op) {
  // !!X --> X
  case XorBool:
    if (inst->src(1)->hasConstVal(true)) {
      // This is safe to add a new use to because inst->src(0) is a bool.
      assertx(inst->src(0)->isA(TBool));
      return inst->src(0);
    }
    return nullptr;

  // !(X cmp Y) --> X opposite_cmp Y
  case Lt:
  case Lte:
  case Gt:
  case Gte:
  case Eq:
  case Neq:
  case Same:
  case NSame: {
    auto const s0 = inst->src(0);
    auto const s1 = inst->src(1);
    // Not for Dbl:  (x < NaN) != !(x >= NaN)
    //
    // Also don't do it for arrays; we haven't thought through whether this
    // transformation always holds on that type (and we could only plausibly do
    // it for static ones anyway, because of the !maybeCounted restriction we
    // have below).
    auto const unsafeTypes = TDbl|TArr;
    auto const safeToFold =
      !s0->type().maybe(unsafeTypes) && !s1->type().maybe(unsafeTypes) &&
      // We can't add new uses to reference counted types without a more
      // advanced availability analysis.
      !s0->type().maybe(TCounted) && !s1->type().maybe(TCounted);
    if (safeToFold) {
      return gen(env, negateQueryOp(op), s0, s1);
    }
    break;
  }

  case InstanceOfBitmask:
  case NInstanceOfBitmask:
    // This is safe because instanceofs don't take reference counted arguments.
    assertx(!inst->src(0)->type().maybe(TCounted) &&
           !inst->src(1)->type().maybe(TCounted));
    return gen(
      env,
      negateQueryOp(op),
      inst->src(0),
      inst->src(1)
    );
    return nullptr;
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
      return cns(env, op(src1->intVal(), src2->intVal()));
    }
  }

  if (src2->hasConstVal() && src2->intVal() == 0) {
    return src1;
  }

  return nullptr;
}

SSATmp* simplifyShl(State& env, const IRInstruction* inst) {
  return shiftImpl(env, inst, [] (int64_t a, int64_t b) { return a << b; });
}

SSATmp* simplifyShr(State& env, const IRInstruction* inst) {
  return shiftImpl(env, inst, [] (int64_t a, int64_t b) { return a >> b; });
}

template<class T, class U>
static typename std::common_type<T,U>::type cmpOp(Opcode opName, T a, U b) {
  switch (opName) {
  case GtInt:
  case GtStr:
  case Gt:   return a > b;
  case GteInt:
  case GteStr:
  case Gte:  return a >= b;
  case LtInt:
  case LtStr:
  case Lt:   return a < b;
  case LteInt:
  case LteStr:
  case Lte:  return a <= b;
  case Same:
  case SameStr:
  case EqInt:
  case EqStr:
  case Eq:   return a == b;
  case NSame:
  case NSameStr:
  case NeqInt:
  case NeqStr:
  case Neq:  return a != b;
  default:
    not_reached();
  }
}

SSATmp* cmpImpl(State& env,
                Opcode opName,
                const IRInstruction* const inst,
                SSATmp* src1,
                SSATmp* src2) {
  auto newInst = [&] (Opcode op, SSATmp* src1, SSATmp* src2) {
    return gen(env, op, inst ? inst->taken() : (Block*)nullptr, src1, src2);
  };

  auto const type1 = src1->type();
  auto const type2 = src2->type();

  // Identity optimization
  if (src1 == src2 && !type1.maybe(TDbl)) {
    // (val1 == val1) does not simplify to true when val1 is a NaN
    return cns(env, bool(cmpOp(opName, 0, 0)));
  }

  assertx(type1 <= TGen && type2 <= TGen);

  // Need both types to be unboxed to simplify, and the code below assumes the
  // types are known DataTypes.
  if (!type1.isKnownDataType() || type1.maybe(TBoxedCell) ||
      !type2.isKnownDataType() || type2.maybe(TBoxedCell)) {
    return nullptr;
  }

  // OpSame and OpNSame have some special rules
  if (opName == Same || opName == NSame ||
      opName == SameStr || opName == NSameStr) {
    // OpSame and OpNSame do not perform type juggling
    if (type1.toDataType() != type2.toDataType() &&
        !(type1 <= TStr && type2 <= TStr)) {
      return cns(env, bool(cmpOp(opName, 0, 1)));
    }
    // Here src1 and src2 are same type, treating Str and StaticStr as the
    // same.

    if (type1 <= TStr && type2 <= TStr) {
      // Constant fold if they are both constant strings.
      if (src1->hasConstVal() && src2->hasConstVal()) {
        auto const str1 = src1->strVal();
        auto const str2 = src2->strVal();
        bool same = str1->same(str2);
        return cns(env, bool(cmpOp(opName, same, 1)));
      } else if (!isStrQueryOp(opName)) {
        // Otherwise, lower to str comparison if possible.
        return newInst(queryToStrQueryOp(opName), src1, src2);
      }
      return nullptr;
    }
    assertx(opName != SameStr && opName != NSameStr);

    // If type is a primitive type - simplify to Eq/Neq.  Str was already
    // removed above.
    auto const badTypes = TObj | TRes | TArr;
    if (type1.maybe(badTypes) || type2.maybe(badTypes)) {
      return nullptr;
    }
    return newInst(opName == Same ? Eq : Neq, src1, src2);
  }

  // ---------------------------------------------------------------------
  // We may now perform constant-constant optimizations
  // ---------------------------------------------------------------------

  // Null cmp Null
  if (type1 <= TNull && type2 <= TNull) {
    return cns(env, bool(cmpOp(opName, 0, 0)));
  }

  // const cmp const
  if (src1->hasConstVal() && src2->hasConstVal()) {
    // StaticStr cmp StaticStr
    if (src1->isA(TStaticStr) &&
        src2->isA(TStaticStr)) {
      int cmp = src1->strVal()->compare(src2->strVal());
      return cns(env, bool(cmpOp(opName, cmp, 0)));
    }
    // ConstInt cmp ConstInt
    if (src1->isA(TInt) && src2->isA(TInt)) {
      return cns(env, bool(
        cmpOp(opName, src1->intVal(), src2->intVal())));
    }
    // ConstBool cmp ConstBool
    if (src1->isA(TBool) && src2->isA(TBool)) {
      return cns(env, bool(
        cmpOp(opName, src1->boolVal(), src2->boolVal())));
    }
  }

  // ---------------------------------------------------------------------
  // Constant bool comparisons can be strength-reduced
  // NOTE: Comparisons with bools get juggled to bool.
  // ---------------------------------------------------------------------

  // Perform constant-bool optimizations
  if (src2->hasConstVal(TBool)) {
    bool b = src2->boolVal();

    // The result of the comparison might be independent of the truth
    // value of the LHS. If so, then simplify.
    // E.g. `some-int > true`. some-int may juggle to false or true
    //  (0 or 1), but `0 > true` and `1 > true` are both false, so we can
    //  simplify to false immediately.
    if (cmpOp(opName, false, b) == cmpOp(opName, true, b)) {
      return cns(env, bool(cmpOp(opName, false, b)));
    }

    // There are only two distinct booleans - false and true (0 and 1).
    // From above, we know that (0 OP b) != (1 OP b).
    // Hence exactly one of (0 OP b) and (1 OP b) is true.
    // Hence there is exactly one boolean value of src1 that results in the
    // overall expression being true (after type-juggling).
    // Hence we may check for equality with that boolean.
    // E.g. `some-int > false` is equivalent to `some-int == true`
    if (opName != Eq) {
      bool const res = cmpOp(opName, false, b);
      return newInst(Eq, src1, cns(env, !res));
    }
  }

  // Lower to int-comparison if possible.
  if (!isIntQueryOp(opName) && type1 <= TInt && type2 <= TInt) {
    return newInst(queryToIntQueryOp(opName), src1, src2);
  }

  // Dbl-dbl or dbl-int comparison lower to dbl-comparison
  if (!isDblQueryOp(opName) &&
      (type1 <= TDbl || type2 <= TDbl) &&
      (type1.subtypeOfAny(TInt, TDbl) &&
       type2.subtypeOfAny(TInt, TDbl))) {
    return newInst(queryToDblQueryOp(opName),
                   gen(env, ConvCellToDbl, src1),
                   gen(env, ConvCellToDbl, src2));
  }

  // Lower to str-comparison if possible.
  if (!isStrQueryOp(opName) && type1 <= TStr && type2 <= TStr) {
    return newInst(queryToStrQueryOp(opName), src1, src2);
  }

  // ---------------------------------------------------------------------
  // For same-type cmps, canonicalize any constants to the right
  // Then stop - there are no more simplifications left
  // ---------------------------------------------------------------------

  if (type1.toDataType() == type2.toDataType() ||
      (type1 <= TStr && type2 <= TStr)) {
    if (src1->hasConstVal() && !src2->hasConstVal()) {
      return newInst(commuteQueryOp(opName), src2, src1);
    }
    return nullptr;
  }

  // ---------------------------------------------------------------------
  // Perform type juggling and type canonicalization for different types
  // see http://docs.hhvm.com/manual/en/language.operators.comparison.php
  // ---------------------------------------------------------------------

  // nulls get canonicalized to the right
  if (type1 <= TNull) {
    return newInst(commuteQueryOp(opName), src2, src1);
  }

  // case 1a: null cmp string. Convert null to ""
  if (type1 <= TStr && type2 <= TNull) {
    return newInst(opName, src1, cns(env, s_empty.get()));
  }

  // case 1b: null cmp object. Convert null to false and the object to true
  if (type1 <= TObj && type2 <= TNull) {
    return newInst(opName, cns(env, true), cns(env, false));
  }

  // case 2a: null cmp anything. Convert null to false
  if (type2 <= TNull) {
    return newInst(opName, src1, cns(env, false));
  }

  // bools get canonicalized to the right
  if (src1->isA(TBool)) {
    return newInst(commuteQueryOp(opName), src2, src1);
  }

  // case 2b: bool cmp anything. Convert anything to bool
  if (src2->isA(TBool)) {
    if (src1->hasConstVal()) {
      if (src1->isA(TInt)) {
        return newInst(opName, cns(env, bool(src1->intVal())), src2);
      } else if (src1->isA(TStr)) {
        auto str = src1->strVal();
        return newInst(opName, cns(env, str->toBoolean()), src2);
      }
    }

    // Optimize comparison between int and const bool
    if (src1->isA(TInt) && src2->hasConstVal()) {
      // Based on the const bool optimization (above) opName should be Eq
      always_assert(opName == Eq);
      return newInst(src2->boolVal() ? Neq : Eq, src1, cns(env, 0));
    }

    // Nothing fancy to do - perform juggling as normal.
    return newInst(opName, gen(env, ConvCellToBool, src1), src2);
  }

  // case 3: object cmp object. No juggling to do same-type simplification is
  // performed above.

  // strings get canonicalized to the left
  if (type2 <= TStr) {
    return newInst(commuteQueryOp(opName), src2, src1);
  }

  // ints get canonicalized to the right
  if (src1->isA(TInt)) {
    return newInst(commuteQueryOp(opName), src2, src1);
  }

  // case 4: number/string/resource cmp. Convert to number (int OR double)
  // NOTE: The following if-test only checks for some of the SRON-SRON
  //  cases (specifically, string-int). Other cases (like string-string)
  //  are dealt with earlier, while other cases (like number-resource)
  //  are not caught at all (and end up exiting this macro at the bottom).
  if (src1->hasConstVal(TStr) && src2->isA(TInt)) {
    auto str = src1->strVal();
    int64_t si; double sd;
    auto st = str->isNumericWithVal(si, sd, true /* allow errors */);
    if (st == KindOfDouble) {
      return newInst(opName, cns(env, sd), src2);
    }
    if (st == KindOfNull) {
      si = 0;
    }
    return newInst(opName, cns(env, si), src2);
  }

  // case 5: array cmp array. No juggling to do same-type simplification is
  // performed above

  // case 6: array cmp anything. Array is greater
  if (src1->isA(TArr)) {
    return cns(env, bool(cmpOp(opName, 1, 0)));
  }
  if (src2->isA(TArr)) {
    return cns(env, bool(cmpOp(opName, 0, 1)));
  }

  return nullptr;
}

#define X(x)                                                \
  SSATmp* simplify##x(State& env, const IRInstruction* i) { \
    return cmpImpl(env, i->op(), i, i->src(0), i->src(1));  \
  }

X(Gt)
X(Gte)
X(Lt)
X(Lte)
X(Eq)
X(Neq)
X(GtInt)
X(GteInt)
X(LtInt)
X(LteInt)
X(EqInt)
X(NeqInt)
X(Same)
X(NSame)
X(GtStr)
X(GteStr)
X(LtStr)
X(LteStr)
X(EqStr)
X(NeqStr)
X(SameStr)
X(NSameStr)

#undef X

SSATmp* isTypeImpl(State& env, const IRInstruction* inst) {
  bool const trueSense = inst->op() == IsType;
  auto const type      = inst->typeParam();
  auto const src       = inst->src(0);
  auto const srcType   = src->type();

  // If mightRelax(src) returns true, we can't generally depend on the src's
  // type. However, we always constrain the input to this opcode with at least
  // DataTypeSpecific, so we only have to skip the optimization if the
  // typeParam is specialized.
  if (mightRelax(env, src) && type.isSpecialized()) return nullptr;

  // Testing for StaticStr will make you miss out on CountedStr, and vice versa,
  // and similarly for arrays. PHP treats both types of string the same, so if
  // the distinction matters to you here, be careful.
  assertx(IMPLIES(type <= TStr, type == TStr));
  assertx(IMPLIES(type <= TArr, type == TArr));

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

SSATmp* simplifyInstanceOf(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);

  if (src2->isA(TNullptr)) return cns(env, false);

  if (!src1->hasConstVal() || !src2->hasConstVal()) return nullptr;

  return cns(env, src1->clsVal()->classof(src2->clsVal()));
}

SSATmp* simplifyExtendsClass(State& env, const IRInstruction* i) {
  return simplifyInstanceOf(env, i);
}

SSATmp* simplifyInstanceOfIface(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);

  if (!src1->hasConstVal() || !src2->hasConstVal()) return nullptr;

  return cns(env, src1->clsVal()->ifaceofDirect(src2->strVal()));
}

SSATmp* simplifyIsType(State& env, const IRInstruction* i) {
  return isTypeImpl(env, i);
}

SSATmp* simplifyIsNType(State& env, const IRInstruction* i) {
  return isTypeImpl(env, i);
}

SSATmp* simplifyIsScalarType(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->type().isKnownDataType()) {
    return cns(env, src->isA(TInt | TDbl | TStr | TBool));
  }
  return nullptr;
}

SSATmp* simplifyConcatStrStr(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);
  if (src1->hasConstVal(TStaticStr) &&
      src2->hasConstVal(TStaticStr)) {
    auto const str1 = const_cast<StringData*>(src1->strVal());
    auto const str2 = const_cast<StringData*>(src2->strVal());
    auto const sval = String::attach(concat_ss(str1, str2));
    return cns(env, makeStaticString(sval.get()));
  }

  return nullptr;
}

SSATmp* convToArrImpl(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->hasConstVal()) {
    Array arr = Array::Create(src->variantVal());
    return cns(env, ArrayData::GetScalarArray(arr.get()));
  }
  return nullptr;
}

SSATmp* simplifyConvBoolToArr(State& env, const IRInstruction* inst) {
  return convToArrImpl(env, inst);
}

SSATmp* simplifyConvIntToArr(State& env, const IRInstruction* inst) {
  return convToArrImpl(env, inst);
}

SSATmp* simplifyConvDblToArr(State& env, const IRInstruction* inst) {
  return convToArrImpl(env, inst);
}

SSATmp* simplifyConvStrToArr(State& env, const IRInstruction* inst) {
  return convToArrImpl(env, inst);
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

SSATmp* simplifyConvArrToInt(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->hasConstVal()) {
    if (src->arrVal()->empty()) return cns(env, 0);
    return cns(env, 1);
  }
  return nullptr;
}

SSATmp* simplifyConvBoolToInt(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->hasConstVal()) return cns(env, static_cast<int>(src->boolVal()));
  return nullptr;
}

SSATmp* simplifyConvDblToInt(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->hasConstVal()) return cns(env, toInt64(src->dblVal()));
  return nullptr;
}

SSATmp* simplifyConvStrToInt(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  return src->hasConstVal() ? cns(env, src->strVal()->toInt64()) : nullptr;
}

SSATmp* simplifyConvBoolToStr(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->hasConstVal()) {
    if (src->boolVal()) return cns(env, s_1.get());
    return cns(env, s_empty.get());
  }
  return nullptr;
}

SSATmp* simplifyConvDblToStr(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->hasConstVal()) {
    auto dblStr = String::attach(buildStringData(src->dblVal()));
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

SSATmp* simplifyConvCellToBool(State& env, const IRInstruction* inst) {
  auto const src     = inst->src(0);
  auto const srcType = src->type();

  if (srcType <= TBool) return src;
  if (srcType <= TNull) return cns(env, false);
  if (srcType <= TArr)  return gen(env, ConvArrToBool, src);
  if (srcType <= TDbl)  return gen(env, ConvDblToBool, src);
  if (srcType <= TInt)  return gen(env, ConvIntToBool, src);
  if (srcType <= TStr)  return gen(env, ConvStrToBool, src);
  if (srcType <= TObj) {
    if (auto cls = srcType.clsSpec().cls()) {
      // We need to exclude interfaces like ConstSet.  For now, just
      // skip anything that's an interface.
      if (!(cls->attrs() & AttrInterface)) {
        // t3429711 we should test cls->m_ODAttr
        // here, but currently it doesnt have all
        // the flags set.
        if (!cls->instanceCtor()) {
          return cns(env, true);
        }
      }
    }
    return gen(env, ConvObjToBool, src);
  }
  if (srcType <= TRes)  return nullptr; // No specialization yet

  return nullptr;
}

SSATmp* simplifyConvCellToStr(State& env, const IRInstruction* inst) {
  auto const src        = inst->src(0);
  auto const srcType    = src->type();
  auto const catchTrace = inst->taken();

  if (srcType <= TBool)   return gen(env, ConvBoolToStr, src);
  if (srcType <= TNull)   return cns(env, s_empty.get());
  if (srcType <= TArr)  {
    gen(env, RaiseNotice, catchTrace,
        cns(env, makeStaticString("Array to string conversion")));
    return cns(env, s_Array.get());
  }
  if (srcType <= TDbl)    return gen(env, ConvDblToStr, src);
  if (srcType <= TInt)    return gen(env, ConvIntToStr, src);
  if (srcType <= TStr) {
    gen(env, IncRef, src);
    return src;
  }
  if (srcType <= TObj)    return gen(env, ConvObjToStr, catchTrace, src);
  if (srcType <= TRes)    return gen(env, ConvResToStr, catchTrace, src);

  return nullptr;
}

SSATmp* simplifyConvCellToInt(State& env, const IRInstruction* inst) {
  auto const src      = inst->src(0);
  auto const srcType  = src->type();

  if (srcType <= TInt)  return src;
  if (srcType <= TNull) return cns(env, 0);
  if (srcType <= TArr)  return gen(env, ConvArrToInt, src);
  if (srcType <= TBool) return gen(env, ConvBoolToInt, src);
  if (srcType <= TDbl)  return gen(env, ConvDblToInt, src);
  if (srcType <= TStr)  return gen(env, ConvStrToInt, src);
  if (srcType <= TObj)  return gen(env, ConvObjToInt, inst->taken(), src);
  if (srcType <= TRes)  return nullptr; // No specialization yet

  return nullptr;
}

SSATmp* simplifyConvCellToDbl(State& env, const IRInstruction* inst) {
  auto const src      = inst->src(0);
  auto const srcType  = src->type();

  if (srcType <= TDbl)  return src;
  if (srcType <= TNull) return cns(env, 0.0);
  if (srcType <= TArr)  return gen(env, ConvArrToDbl, src);
  if (srcType <= TBool) return gen(env, ConvBoolToDbl, src);
  if (srcType <= TInt)  return gen(env, ConvIntToDbl, src);
  if (srcType <= TStr)  return gen(env, ConvStrToDbl, src);
  if (srcType <= TObj)  return gen(env, ConvObjToDbl, inst->taken(), src);
  if (srcType <= TRes)  return nullptr; // No specialization yet

  return nullptr;
}

SSATmp* simplifyConvObjToBool(State& env, const IRInstruction* inst) {
  auto const ty = inst->src(0)->type();

  if (!typeMightRelax(inst->src(0)) &&
      ty < TObj &&
      ty.clsSpec().cls() &&
      ty.clsSpec().cls()->isCollectionClass()) {
    return gen(env, ColIsNEmpty, inst->src(0));
  }
  return nullptr;
}

SSATmp* simplifyConvCellToObj(State& env, const IRInstruction* inst) {
  if (inst->src(0)->isA(TObj)) return inst->src(0);
  return nullptr;
}

SSATmp* simplifyCoerceCellToBool(State& env, const IRInstruction* inst) {
  auto const src     = inst->src(0);
  auto const srcType = src->type();

  if (srcType.subtypeOfAny(TBool, TNull, TDbl,
                           TInt, TStr)) {
    return gen(env, ConvCellToBool, src);
  }

  // We actually know that any other type will fail causing us to side exit
  // but there's no easy way to optimize for that

  return nullptr;
}

SSATmp* simplifyCoerceCellToInt(State& env, const IRInstruction* inst) {
  auto const src      = inst->src(0);
  auto const srcType  = src->type();

  if (srcType.subtypeOfAny(TInt, TBool, TNull, TDbl,
                           TBool)) {
    return gen(env, ConvCellToInt, inst->taken(), src);
  }

  if (srcType <= TStr) return gen(env, CoerceStrToInt, inst->taken(),
                                       *inst->extra<CoerceCellToInt>(), src);

  // We actually know that any other type will fail causing us to side exit
  // but there's no easy way to optimize for that

  return nullptr;
}

SSATmp* simplifyCoerceCellToDbl(State& env, const IRInstruction* inst) {
  auto const src      = inst->src(0);
  auto const srcType  = src->type();

  if (srcType.subtypeOfAny(TInt, TBool, TNull, TDbl,
                           TBool)) {
    return gen(env, ConvCellToDbl, inst->taken(), src);
  }

  if (srcType <= TStr) return gen(env, CoerceStrToDbl, inst->taken(),
                                       *inst->extra<CoerceCellToDbl>(), src);

  // We actually know that any other type will fail causing us to side exit
  // but there's no easy way to optimize for that

  return nullptr;
}

SSATmp* roundImpl(State& env, const IRInstruction* inst, double (*op)(double)) {
  auto const src  = inst->src(0);

  if (src->hasConstVal()) {
    return cns(env, op(src->dblVal()));
  }

  auto srcInst = src->inst();
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

SSATmp* simplifyUnboxPtr(State& env, const IRInstruction* inst) {
  if (inst->src(0)->isA(TPtrToCell)) {
    return inst->src(0);
  }
  return nullptr;
}

SSATmp* simplifyBoxPtr(State& env, const IRInstruction* inst) {
  if (inst->src(0)->isA(TPtrToBoxedCell)) {
    return inst->src(0);
  }
  return nullptr;
}

SSATmp* simplifyCheckInit(State& env, const IRInstruction* inst) {
  auto const srcType = inst->src(0)->type();
  assertx(!srcType.maybe(TPtrToGen));
  assertx(inst->taken());
  if (!srcType.maybe(TUninit)) return gen(env, Nop);
  return mergeBranchDests(env, inst);
}

SSATmp* simplifyCheckInitMem(State& env, const IRInstruction* inst) {
  return mergeBranchDests(env, inst);
}

SSATmp* simplifyCheckInitProps(State& env, const IRInstruction* inst) {
  return mergeBranchDests(env, inst);
}

SSATmp* simplifyCheckInitSProps(State& env, const IRInstruction* inst) {
  return mergeBranchDests(env, inst);
}

SSATmp* simplifyInitObjProps(State& env, const IRInstruction* inst) {
  auto const cls = inst->extra<InitObjProps>()->cls;
  if (cls->getODAttrs() == 0 && cls->numDeclProperties() == 0) {
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

SSATmp* simplifyCheckTypeMem(State& env, const IRInstruction* inst) {
  if (inst->next() == inst->taken() ||
      inst->typeParam() == TBottom) {
    return gen(env, Jmp, inst->taken());
  }

  return nullptr;
}

SSATmp* simplifyAssertType(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);

  return canSimplifyAssertType(inst, src->type(), mightRelax(env, src))
    ? src
    : nullptr;
}

SSATmp* simplifyCheckLoc(State& env, const IRInstruction* inst) {
  return mergeBranchDests(env, inst);
}

SSATmp* simplifyCheckStk(State& env, const IRInstruction* inst) {
  return mergeBranchDests(env, inst);
}

SSATmp* simplifyCheckStaticLocInit(State& env, const IRInstruction* inst) {
  return mergeBranchDests(env, inst);
}

SSATmp* simplifyCheckRefInner(State& env, const IRInstruction* inst) {
  // Ref inner cells are at worst InitCell, so don't bother checking for that.
  if (TInitCell <= inst->typeParam()) {
    return gen(env, Nop);
  }
  return mergeBranchDests(env, inst);
}

SSATmp* simplifyDefLabel(State& env, const IRInstruction* inst) {
  if (inst->numDsts() == 0) {
    return gen(env, Nop);
  }
  return nullptr;
}

SSATmp* decRefImpl(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (!mightRelax(env, src) && !src->type().maybe(TCounted)) {
    return gen(env, Nop);
  }
  return nullptr;
}

SSATmp* simplifyDecRef(State& env, const IRInstruction* inst) {
  return decRefImpl(env, inst);
}

SSATmp* simplifyDecRefNZ(State& env, const IRInstruction* inst) {
  return decRefImpl(env, inst);
}

SSATmp* simplifyIncRef(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (!mightRelax(env, src) && !src->type().maybe(TCounted)) {
    return gen(env, Nop);
  }
  return nullptr;
}

SSATmp* simplifyIncRefCtx(State& env, const IRInstruction* inst) {
  auto const ctx = inst->src(0);
  if (ctx->isA(TObj)) {
    return gen(env, IncRef, ctx);
  } else if (!mightRelax(env, ctx) && !ctx->type().maybe(TCounted)) {
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

  // Absorb negations.
  if (srcInst->is(XorBool) && srcInst->src(1)->hasConstVal(true)) {
    return gen(env,
               inst->op() == JmpZero ? JmpNZero : JmpZero,
               inst->taken(),
               srcInst->src(0)
              );
  }
  // Absorb ConvIntToBool.
  if (srcInst->is(ConvIntToBool)) {
    return gen(env, inst->op(), inst->taken(), srcInst->src(0));
  }

  return nullptr;
}

SSATmp* simplifyJmpZero(State& env, const IRInstruction* i) {
  return condJmpImpl(env, i);
}

SSATmp* simplifyJmpNZero(State& env, const IRInstruction* i) {
  return condJmpImpl(env, i);
}


SSATmp* simplifyAssertNonNull(State& env, const IRInstruction* inst) {
  if (!inst->src(0)->type().maybe(TNullptr)) {
    return inst->src(0);
  }
  return nullptr;
}

SSATmp* simplifyCheckPackedArrayBounds(State& env, const IRInstruction* inst) {
  auto const array = inst->src(0);
  auto const idx   = inst->src(1);
  if (!idx->hasConstVal()) return mergeBranchDests(env, inst);

  auto const idxVal = idx->intVal();
  switch (packedArrayBoundsStaticCheck(array->type(), idxVal)) {
  case PackedBounds::In:       return gen(env, Nop);
  case PackedBounds::Out:      return gen(env, Jmp, inst->taken());
  case PackedBounds::Unknown:  break;
  }

  return mergeBranchDests(env, inst);
}

SSATmp* arrIntKeyImpl(State& env, const IRInstruction* inst) {
  auto const arr = inst->src(0);
  auto const idx = inst->src(1);
  assertx(arr->hasConstVal(TArr));
  if (!idx->hasConstVal()) return nullptr;
  auto const value = arr->arrVal()->nvGet(idx->intVal());
  return value ? cns(env, *value) : nullptr;
}

SSATmp* arrStrKeyImpl(State& env, const IRInstruction* inst) {
  auto const arr = inst->src(0);
  auto const idx = inst->src(1);
  assertx(arr->hasConstVal(TArr));
  if (!idx->hasConstVal()) return nullptr;
  auto const value = [&] {
    int64_t val;
    if (idx->strVal()->isStrictlyInteger(val)) {
      return arr->arrVal()->nvGet(val);
    }
    return arr->arrVal()->nvGet(idx->strVal());
  }();
  return value ? cns(env, *value) : nullptr;
}

SSATmp* simplifyArrayGet(State& env, const IRInstruction* inst) {
  if (inst->src(0)->hasConstVal()) {
    if (inst->src(1)->type() <= TInt) return arrIntKeyImpl(env, inst);
    if (inst->src(1)->type() <= TStr) return arrStrKeyImpl(env, inst);
  }
  return nullptr;
}

SSATmp* simplifyCount(State& env, const IRInstruction* inst) {
  auto const val = inst->src(0);
  auto const ty = val->type();

  if (ty <= TNull) return cns(env, 0);

  auto const oneTy = TBool | TInt | TDbl | TStr | TRes;
  if (ty <= oneTy) return cns(env, 1);

  if (ty <= TArr) return gen(env, CountArray, val);

  if (ty < TObj) {
    auto const cls = ty.clsSpec().cls();
    if (!mightRelax(env, val) && cls != nullptr && cls->isCollectionClass()) {
      return gen(env, CountCollection, val);
    }
  }
  return nullptr;
}

SSATmp* simplifyCountArrayFast(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (inst->src(0)->hasConstVal(TArr)) return cns(env, src->arrVal()->size());
  auto const arrSpec = src->type().arrSpec();
  auto const at = arrSpec.type();
  if (!at) return nullptr;
  using A = RepoAuthType::Array;
  switch (at->tag()) {
  case A::Tag::Packed:
    if (at->emptiness() == A::Empty::No) {
      return cns(env, at->size());
    }
    break;
  case A::Tag::PackedN:
    break;
  }
  return nullptr;
}

SSATmp* simplifyCountArray(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  auto const ty = src->type();

  if (src->hasConstVal()) return cns(env, src->arrVal()->size());

  auto const kind = ty.arrSpec().kind();

  if (kind && !mightRelax(env, src) && !arrayKindNeedsVsize(*kind))
    return gen(env, CountArrayFast, src);
  else
    return nullptr;
}

SSATmp* simplifyLdClsName(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  return src->hasConstVal(TCls) ? cns(env, src->clsVal()->name()) :nullptr;
}

SSATmp* simplifyCallBuiltin(State& env, const IRInstruction* inst) {
  auto const callee = inst->extra<CallBuiltin>()->callee;
  auto const args = inst->srcs();


  bool const arg2IsCollection = args.size() == 3 &&
    args[2]->isA(TObj) &&
    args[2]->type().clsSpec() &&
    args[2]->type().clsSpec().cls()->isCollectionClass() &&
    !mightRelax(env, args[2]);

  if (arg2IsCollection) {
    if (callee->name()->isame(s_isEmpty.get())) {
      FTRACE(3, "simplifying collection: {}\n", callee->name()->data());
      return gen(env, ColIsEmpty, args[2]);
    }
    if (callee->name()->isame(s_count.get())) {
      FTRACE(3, "simplifying collection: {}\n", callee->name()->data());
      return gen(env, CountCollection, args[2]);
    }
  }

  bool const arg2IsWaitHandle = !arg2IsCollection &&
    args.size() == 3 &&
    args[2]->isA(TObj) &&
    args[2]->type().clsSpec() &&
    args[2]->type().clsSpec().cls()->classof(c_WaitHandle::classof()) &&
    !mightRelax(env, args[2]);

  if (arg2IsWaitHandle) {
    const auto genState = [&] (Opcode op, int64_t whstate) -> SSATmp* {
      // these methods all spring from the base class
      assert(callee->cls()->name()->isame(s_WaitHandle.get()));
      const auto state = gen(env, LdWHState, args[2]);
      return gen(env, op, state, cns(env, whstate));
    };
    const auto methName = callee->name();
    if (methName->isame(s_isFinished.get())) {
      return genState(LteInt, int64_t{c_WaitHandle::STATE_FAILED});
    }
    if (methName->isame(s_isSucceeded.get())) {
      return genState(EqInt, int64_t{c_WaitHandle::STATE_SUCCEEDED});
    }
    if (methName->isame(s_isFailed.get())) {
      return genState(EqInt, int64_t{c_WaitHandle::STATE_FAILED});
    }
  }

  return nullptr;
}

SSATmp* simplifyIsWaitHandle(State& env, const IRInstruction* inst) {
  if (mightRelax(env, inst->src(0))) return nullptr;

  bool baseIsWaitHandle = inst->src(0)->isA(TObj) &&
    inst->src(0)->type().clsSpec() &&
    inst->src(0)->type().clsSpec().cls()->classof(c_WaitHandle::classof());
  if (baseIsWaitHandle) {
    return cns(env, true);
  }
  return nullptr;
}

SSATmp* simplifyOrdStr(State& env, const IRInstruction* inst) {
  const auto src = inst->src(0);
  if (src->hasConstVal(TStr)) {
    // a static string is passed in, resolve with a constant.
    unsigned char first = src->strVal()->data()[0];
    return cns(env, int64_t{first});
  }
  return nullptr;
}

SSATmp* ldImpl(State& env, const IRInstruction* inst) {
  if (env.typesMightRelax) return nullptr;

  auto const t = inst->typeParam();

  return t.hasConstVal() ||
         t.subtypeOfAny(TUninit, TInitNull, TNullptr)
    ? cns(env, t)
    : nullptr;
}

SSATmp* simplifyLdLoc(State& env, const IRInstruction* inst) {
  return ldImpl(env, inst);
}

SSATmp* simplifyLdStk(State& env, const IRInstruction* inst) {
  return ldImpl(env, inst);
}

SSATmp* simplifyJmpSwitchDest(State& env, const IRInstruction* inst) {
  auto const index = inst->src(0);
  if (!index->hasConstVal(TInt)) return nullptr;

  auto indexVal = index->intVal();
  auto const sp = inst->src(1);
  auto const fp = inst->src(2);
  auto const& extra = *inst->extra<JmpSwitchDest>();

  if (indexVal < 0 || indexVal >= extra.cases) {
    // Instruction is unreachable.
    return gen(env, Halt);
  }

  auto const newExtra = ReqBindJmpData{extra.targets[indexVal], extra.invSPOff,
                                       extra.irSPOff, TransFlags{}};
  return gen(env, ReqBindJmp, newExtra, sp, fp);
}

SSATmp* simplifyCheckRange(State& env, const IRInstruction* inst) {
  auto val = inst->src(0);
  auto limit = inst->src(1);

  // CheckRange returns (0 <= val < limit).
  if (val->hasConstVal(TInt)) {
    if (val->intVal() < 0) return cns(env, false);

    if (limit->hasConstVal(TInt)) {
      return cns(env, val->intVal() < limit->intVal());
    }
  }

  if (limit->hasConstVal(TInt) && limit->intVal() <= 0) return cns(env, false);

  return nullptr;
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
  X(AbsDbl)
  X(AssertNonNull)
  X(BoxPtr)
  X(CallBuiltin)
  X(Ceil)
  X(CheckInit)
  X(CheckInitMem)
  X(CheckInitProps)
  X(CheckInitSProps)
  X(CheckLoc)
  X(CheckRefInner)
  X(CheckStk)
  X(CheckStaticLocInit)
  X(CheckType)
  X(CheckTypeMem)
  X(AssertType)
  X(CheckPackedArrayBounds)
  X(CoerceCellToBool)
  X(CoerceCellToDbl)
  X(CoerceCellToInt)
  X(ConcatStrStr)
  X(ConvArrToBool)
  X(ConvArrToDbl)
  X(ConvArrToInt)
  X(ConvBoolToArr)
  X(ConvBoolToDbl)
  X(ConvBoolToInt)
  X(ConvBoolToStr)
  X(ConvCellToBool)
  X(ConvCellToDbl)
  X(ConvCellToInt)
  X(ConvCellToObj)
  X(ConvCellToStr)
  X(ConvClsToCctx)
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
  X(ConvStrToBool)
  X(ConvStrToDbl)
  X(ConvStrToInt)
  X(Count)
  X(CountArray)
  X(CountArrayFast)
  X(DecRef)
  X(DecRefNZ)
  X(DefLabel)
  X(DivDbl)
  X(ExtendsClass)
  X(Floor)
  X(GetCtxFwdCall)
  X(IncRef)
  X(IncRefCtx)
  X(InitObjProps)
  X(InstanceOf)
  X(InstanceOfIface)
  X(IsNType)
  X(IsScalarType)
  X(IsType)
  X(IsWaitHandle)
  X(LdClsCtx)
  X(LdClsName)
  X(CheckCtxThis)
  X(CastCtxThis)
  X(LdObjClass)
  X(LdObjInvoke)
  X(Mov)
  X(UnboxPtr)
  X(JmpZero)
  X(JmpNZero)
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
  X(Gt)
  X(Gte)
  X(Lt)
  X(Lte)
  X(Eq)
  X(Neq)
  X(GtInt)
  X(GteInt)
  X(LtInt)
  X(LteInt)
  X(EqInt)
  X(NeqInt)
  X(Same)
  X(NSame)
  X(GtStr)
  X(GteStr)
  X(LtStr)
  X(LteStr)
  X(EqStr)
  X(NeqStr)
  X(SameStr)
  X(NSameStr)
  X(ArrayGet)
  X(OrdStr)
  X(LdLoc)
  X(LdStk)
  X(JmpSwitchDest)
  X(CheckRange)
  default: break;
  }
#undef X
  return nullptr;
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

bool canSimplifyAssertType(const IRInstruction* inst,
                           const Type srcType,
                           bool srcMightRelax) {
  assert(inst->is(AssertType, AssertLoc, AssertStk));

  auto const typeParam = inst->typeParam();

  if (!srcType.maybe(typeParam)) {
    // If both types are boxed, this is okay and even expected as a means to
    // update the hint for the inner type.
    if (srcType <= TBoxedCell &&
        typeParam <= TBoxedCell) {
      return false;
    }

    // We got external information (probably from static analysis) that
    // conflicts with what we've built up so far.  There's no reasonable way to
    // continue here: we can't properly fatal the request because we can't make
    // a catch trace or SpillStack without IRGS, and we can't punt on
    // just this instruction because we might not be in the initial translation
    // phase, and we can't just plow on forward since we'll probably generate
    // malformed IR.  Since this case is very rare, just punt on the whole
    // trace so it gets interpreted.
    TRACE_PUNT("Invalid AssertTypeOp");
  }

  // Asserting in these situations doesn't add any information.
  if (typeParam == TCls && srcType <= TCls) return true;
  if (typeParam == TGen && srcType <= TGen) return true;

  auto const newType = srcType & typeParam;

  if (srcType <= newType) {
    // The src type is at least as good as the new type.  Eliminate this
    // AssertType if the src type won't relax.  We do this to avoid eliminating
    // apparently redundant assert opcodes that may become useful after prior
    // guards are relaxed.
    if (!srcMightRelax) return true;

    if (srcType < newType) {
      // This can happen because of limitations in how Toperator& handles
      // specialized types: sometimes it returns a Type that's wider than it
      // needs to be.  It shouldn't affect correctness but it can cause us to
      // miss out on some perf.
      FTRACE_MOD(Trace::hhir, 1,
                 "Suboptimal AssertTypeOp: refineType({}, {}) -> {} in {}\n",
                 srcType, typeParam, newType, *inst);

      // We don't currently support intersecting RepoAuthType::Arrays
      // (t4473238), so we might be here because srcType and typeParam have
      // different RATArrays.  If that's the case, and if typeParam provides no
      // other useful information, we can unconditionally eliminate this
      // instruction: RATArrays never come from guards so we can't miss out on
      // anything by doing so.
      if (srcType < TArr &&
          srcType.arrSpec().type() &&
          typeParam < TArr &&
          typeParam.arrSpec().type() &&
          !typeParam.arrSpec().kind()) {
        return true;
      }
    }
  }

  return false;
}

//////////////////////////////////////////////////////////////////////

SimplifyResult simplify(IRUnit& unit,
                        const IRInstruction* origInst,
                        bool typesMightRelax) {
  auto env = State { unit, typesMightRelax };
  auto const newDst = simplifyWork(env, origInst);

  assertx(validate(env, newDst, origInst));

  return SimplifyResult { std::move(env.newInsts), newDst };
}

void simplify(IRUnit& unit, IRInstruction* origInst) {
  assertx(!origInst->isTransient());

  copyProp(origInst);
  auto res = simplify(unit, origInst, false);

  // No simplification occurred; nothing to do.
  if (res.instrs.empty() && !res.dst) return;

  FTRACE(1, "simplifying: {}\n", origInst->toString());

  if (origInst->isBlockEnd()) {
    auto const next = origInst->block()->next();

    if (res.instrs.empty() || !res.instrs.back()->isBlockEnd()) {
      // Our block-end instruction was eliminated (most likely a Jmp* converted
      // to a Nop).  Replace it with a Jmp to the next block.
      res.instrs.push_back(unit.gen(Jmp, origInst->marker(), next));
    }

    auto last = res.instrs.back();
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

  for (auto inst : res.instrs) {
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
  auto const pos = ++block->iteratorTo(origInst);

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

//////////////////////////////////////////////////////////////////////

void simplifyPass(IRUnit& unit) {
  auto reachable = boost::dynamic_bitset<>(unit.numBlocks());
  reachable.set(unit.entry()->id());

  for (auto block : rpoSortCfg(unit)) {
    if (!reachable.test(block->id())) continue;

    for (auto& inst : *block) simplify(unit, &inst);

    if (auto const b = block->next())  reachable.set(b->id());
    if (auto const b = block->taken()) reachable.set(b->id());
  }
}

//////////////////////////////////////////////////////////////////////

void copyProp(IRInstruction* inst) {
  for (uint32_t i = 0; i < inst->numSrcs(); i++) {
    auto tmp     = inst->src(i);
    auto srcInst = tmp->inst();

    if (srcInst->is(Mov)) {
      inst->setSrc(i, srcInst->src(0));
      }

    // We're assuming that all of our src instructions have already been
    // copyPropped.
    assertx(!inst->src(i)->inst()->is(Mov));
    }
  }

PackedBounds packedArrayBoundsStaticCheck(Type arrayType, int64_t idxVal) {
  if (idxVal < 0 || idxVal > PackedArray::MaxSize) return PackedBounds::Out;

  if (arrayType.hasConstVal()) {
    return idxVal < arrayType.arrVal()->size()
      ? PackedBounds::In
      : PackedBounds::Out;
  }

  auto const at = arrayType.arrSpec().type();
  if (!at) return PackedBounds::Unknown;

  using A = RepoAuthType::Array;
  switch (at->tag()) {
  case A::Tag::Packed:
    if (idxVal < at->size() && at->emptiness() == A::Empty::No) {
      return PackedBounds::In;
    }
    // fallthrough
  case A::Tag::PackedN:
    if (idxVal == 0 && at->emptiness() == A::Empty::No) {
      return PackedBounds::In;
    }
  }
  return PackedBounds::Unknown;
}

Type packedArrayElemType(SSATmp* arr, SSATmp* idx) {
  assertx(arr->isA(TArr) &&
          arr->type().arrSpec().kind() == ArrayData::kPackedKind &&
          idx->isA(TInt));

  if (arr->hasConstVal() && idx->hasConstVal()) {
    auto const idxVal = idx->intVal();
    if (idxVal >= 0 && idxVal < arr->arrVal()->size()) {
      return Type(arr->arrVal()->nvGet(idxVal)->m_type);
    }
    return TInitNull;
  }

  Type t = arr->isA(TStaticArr) ? TInitCell : TGen;

  auto const at = arr->type().arrSpec().type();
  if (!at) return t;

  switch (at->tag()) {
    case RepoAuthType::Array::Tag::Packed:
    {
      if (idx->hasConstVal(TInt)) {
        auto const idxVal = idx->intVal();
        if (idxVal >= 0 && idxVal < at->size()) {
          return typeFromRAT(at->packedElem(idxVal)) & t;
        }
        return TInitNull;
      }
      Type elemType = TBottom;
      for (uint32_t i = 0; i < at->size(); ++i) {
        elemType |= typeFromRAT(at->packedElem(i));
      }
      return elemType & t;
    }
    case RepoAuthType::Array::Tag::PackedN:
      return typeFromRAT(at->elemType()) & t;
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

}}
