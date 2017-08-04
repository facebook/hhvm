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
#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/double-to-int64.h"
#include "hphp/runtime/base/mixed-array.h"
#include "hphp/runtime/base/mixed-array-defs.h"
#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/base/packed-array-defs.h"
#include "hphp/runtime/base/repo-auth-type-array.h"
#include "hphp/runtime/base/tv-refcount.h"
#include "hphp/runtime/base/tv-type.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/runtime.h"

#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/ir-builder.h"
#include "hphp/runtime/vm/jit/minstr-effects.h"
#include "hphp/runtime/vm/jit/simple-propagation.h"
#include "hphp/runtime/vm/jit/type-array-elem.h"
#include "hphp/runtime/vm/jit/type.h"

#include "hphp/runtime/ext/hh/ext_hh.h"

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
const StaticString s_empty("");
const StaticString s_invoke("__invoke");
const StaticString s_isFinished("isFinished");
const StaticString s_isSucceeded("isSucceeded");
const StaticString s_isFailed("isFailed");
const StaticString s_WaitHandle("HH\\WaitHandle");

//////////////////////////////////////////////////////////////////////

struct State {
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
  switch (kind) {
    case ArrayData::kPackedKind:
    case ArrayData::kMixedKind:
    case ArrayData::kEmptyKind:
    case ArrayData::kVecKind:
    case ArrayData::kApcKind:
    case ArrayData::kDictKind:
    case ArrayData::kKeysetKind:
      return false;
    default:
      return true;
  }
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
                   CheckInitProps,
                   CheckInitSProps,
                   CheckPackedArrayDataBounds,
                   CheckMixedArrayOffset,
                   CheckDictOffset,
                   CheckKeysetOffset,
                   CheckRefInner,
                   CheckCtxThis,
                   CheckFuncStatic));
  if (inst->next() != nullptr && inst->next() == inst->taken()) {
    return gen(env, Jmp, inst->next());
  }
  return nullptr;
}

SSATmp* simplifyCheckCtxThis(State& env, const IRInstruction* inst) {
  auto const func = inst->marker().func();
  auto const srcTy = inst->src(0)->type();
  if (srcTy <= TObj || func->requiresThisInBody()) {
    return gen(env, Nop);
  }
  if (!func->mayHaveThis() || !srcTy.maybe(TObj)) {
    return gen(env, Jmp, inst->taken());
  }
  return mergeBranchDests(env, inst);
}

SSATmp* simplifyCheckFuncStatic(State& env, const IRInstruction* inst) {
  auto const funcTmp = inst->src(0);
  if (funcTmp->hasConstVal()) {
    if (funcTmp->funcVal()->isStatic()) {
      return gen(env, Jmp, inst->taken());
    }
    return gen(env, Nop);
  }

  return mergeBranchDests(env, inst);
}

SSATmp* simplifyRaiseMissingThis(State& env, const IRInstruction* inst) {
  auto const funcTmp = inst->src(0);
  if (funcTmp->hasConstVal()) {
    auto const func = funcTmp->funcVal();
    // Not requiresThisInBody, since this is done in the callee
    // at FPush* time.
    if (func->attrs() & AttrRequiresThis) {
      return gen(env, FatalMissingThis, inst->taken(), funcTmp);
    }
    if (!needs_missing_this_check(func)) {
      return gen(env, Nop);
    }
  }
  return nullptr;
}

SSATmp* simplifyLdClsCtx(State& env, const IRInstruction* inst) {
  assertx(inst->marker().func()->cls());
  auto const ctx = inst->src(0);

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

SSATmp* simplifyLdClsCctx(State& env, const IRInstruction* inst) {
  assertx(inst->marker().func()->cls());
  SSATmp* ctx = inst->src(0);

  if (ctx->hasConstVal(TCctx)) {
    return cns(env, ctx->cctxVal().cls());
  }
  if (ctx->inst()->op() == ConvClsToCctx) {
    return ctx->inst()->src(0);
  }
  return nullptr;
}

SSATmp* simplifyLdClsMethod(State& env, const IRInstruction* inst) {
  auto const ctx = inst->src(0);

  if (ctx->hasConstVal()) {
    auto const idx = inst->src(1);
    if (idx->hasConstVal()) {
      auto const cls = ctx->hasConstVal(TCls) ?
        ctx->clsVal() : ctx->cctxVal().cls();
      return cns(env, cls->getMethod(-idx->intVal() - 1));
    }
    return nullptr;
  }

  if (ctx->isA(TCls)) {
    auto const src = ctx->inst();
    if (src->op() == LdClsCctx) {
      return gen(env, LdClsMethod, src->src(0), inst->src(1));
    }
  }
  return nullptr;
}

SSATmp* simplifySpillFrame(State& env, const IRInstruction* inst) {
  auto const ctx = inst->src(2);
  if (ctx->isA(TCls)) {
    auto const src = ctx->inst();
    if (src->op() == LdClsCctx) {
      return gen(env, SpillFrame, *inst->extra<SpillFrame>(),
                 inst->src(0), inst->src(1), src->src(0), inst->src(3));
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

SSATmp* simplifyLdObjInvoke(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (!src->hasConstVal(TCls)) return nullptr;

  auto const meth = src->clsVal()->getCachedInvoke();
  return meth == nullptr ? nullptr : cns(env, meth);
}

SSATmp* simplifyFwdCtxStaticCall(State& env, const IRInstruction* inst) {
  auto const srcCtx = inst->src(0);

  if (srcCtx->isA(TCctx)) return srcCtx;
  if (srcCtx->isA(TObj)) {
    auto const cls = gen(env, LdObjClass, srcCtx);
    return gen(env, ConvClsToCctx, cls);
  }
  return nullptr;
}

SSATmp* simplifyConvClsToCctx(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);

  if (src->hasConstVal(TCls)) {
    return cns(env, ConstCctx::cctx(src->clsVal()));
  }

  auto const srcInst = src->inst();
  if (srcInst->is(LdClsCctx)) return srcInst->src(0);
  return nullptr;
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
    }
    return cns(env, a + b);
  }
  return nullptr;
}

SSATmp* simplifySubInt(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);

  auto const sub = std::minus<int64_t>();
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
    }
    return cns(env, a - b);
  }
  return nullptr;
}

SSATmp* simplifyMulInt(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);

  auto const mul = std::multiplies<int64_t>();
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
    }
    return cns(env, a * b);
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
  if (left->hasConstVal() && right->hasConstVal()) {
    // this assumes that all the ArrayData* in Type::m_extra overlap exactly
    // so we can use arrVal without having to check the type
    return cns(env, left->arrVal() == right->arrVal());
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

SSATmp* instanceOfImpl(State& env, ClassSpec spec1, ClassSpec spec2) {
  if (!spec1 || !spec2) return nullptr;

  auto const cls1 = spec1.cls();
  auto const cls2 = spec2.cls();

  if (cls1->classof(cls2)) {
    return spec2.exact() ? cns(env, true) : nullptr;
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

  return instanceOfImpl(env, src1->type().clsSpec(), src2->type().clsSpec());
}

SSATmp* simplifyExtendsClass(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const cls2 = inst->extra<ExtendsClassData>()->cls;
  assertx(cls2 && isNormalClass(cls2));
  auto const spec2 = ClassSpec{cls2, ClassSpec::ExactTag{}};
  return instanceOfImpl(env, src1->type().clsSpec(), spec2);
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
                                                     inst->ctx());
  assertx(cls2 && isInterface(cls2));
  auto const spec2 = ClassSpec{cls2, ClassSpec::ExactTag{}};

  return instanceOfImpl(env, src1->type().clsSpec(), spec2);
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

template <typename G, typename C>
SSATmp* arrayLikeConvImpl(State& env, const IRInstruction* inst,
                          G get, C convert) {
  auto const src = inst->src(0);
  if (!src->hasConstVal()) return nullptr;
  auto const before = get(src);
  auto const converted = convert(const_cast<ArrayData*>(before));
  if (!converted) return nullptr;
  auto const scalar = ArrayData::GetScalarArray(converted);
  decRefArr(converted);
  return cns(env, scalar);
}

template <typename G>
SSATmp* convToArrImpl(State& env, const IRInstruction* inst, G get) {
  return arrayLikeConvImpl(
    env, inst, get,
    [&](ArrayData* a) { return a->toPHPArray(true); }
  );
}

template <typename G>
SSATmp* convToVecImpl(State& env, const IRInstruction* inst, G get) {
  return arrayLikeConvImpl(
    env, inst, get,
    [&](ArrayData* a) { return a->toVec(true); }
  );
}

template <typename G>
SSATmp* convToDictImpl(State& env, const IRInstruction* inst, G get) {
  return arrayLikeConvImpl(
    env, inst, get,
    [&](ArrayData* a) { return a->toDict(true); }
  );
}

template <typename G>
SSATmp* convToKeysetImpl(State& env, const IRInstruction* inst, G get) {
  return arrayLikeConvImpl(
    env, inst, get,
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

template <typename G>
SSATmp* convToVArrImpl(State& env, const IRInstruction* inst, G get) {
  return arrayLikeConvImpl(
    env, inst, get,
    [&](ArrayData* a) { return a->toVArray(true); }
  );
}

SSATmp* convNonArrToArrImpl(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->hasConstVal()) {
    Array arr = Array::Create(src->variantVal());
    return cns(env, ArrayData::GetScalarArray(arr.get()));
  }
  return nullptr;
}

}

#define X(FromTy, FromTy2, ToTy)                                          \
SSATmp*                                                                   \
simplifyConv##FromTy##To##ToTy(State& env, const IRInstruction* inst) {   \
  return convTo##ToTy##Impl(                                              \
    env, inst,                                                            \
    [&](const SSATmp* s) { return s->FromTy2(); });                       \
}

X(Vec, vecVal, Arr)
X(Dict, dictVal, Arr)
X(Keyset, keysetVal, Arr)

X(Arr, arrVal, Vec)
X(Dict, dictVal, Vec)
X(Keyset, keysetVal, Vec)

X(Arr, arrVal, Dict)
X(Vec, vecVal, Dict)
X(Keyset, keysetVal, Dict)

X(Arr, arrVal, Keyset)
X(Vec, vecVal, Keyset)
X(Dict, dictVal, Keyset)

//X(Arr, arrVal, VArr) // Below
X(Vec, vecVal, VArr)
X(Dict, dictVal, VArr)
X(Keyset, keysetVal, VArr)

#undef X

SSATmp* simplifyConvArrToVArr(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);

  auto const packedArrType = Type::Array(ArrayData::kPackedKind);
  auto const emptyArrType  = Type::Array(ArrayData::kEmptyKind);

  if (src->isA(emptyArrType) || src->isA(packedArrType)) return src;

  return convToVArrImpl(
    env, inst,
    [&](const SSATmp* s) { return s->arrVal(); });
}

SSATmp* simplifyConvCellToArr(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->isA(TArr))    return src;
  if (src->isA(TVec))    return gen(env, ConvVecToArr, src);
  if (src->isA(TDict))   return gen(env, ConvDictToArr, src);
  if (src->isA(TKeyset)) return gen(env, ConvKeysetToArr, src);
  if (src->isA(TNull))   return cns(env, staticEmptyArray());
  if (src->isA(TBool))   return gen(env, ConvBoolToArr, src);
  if (src->isA(TDbl))    return gen(env, ConvDblToArr, src);
  if (src->isA(TInt))    return gen(env, ConvIntToArr, src);
  if (src->isA(TStr))    return gen(env, ConvStrToArr, src);
  if (src->isA(TObj))    return gen(env, ConvObjToArr, inst->taken(), src);
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

SSATmp* simplifyConvCellToBool(State& env, const IRInstruction* inst) {
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
  if (srcType <= TRes)  return cns(env, true);

  return nullptr;
}

SSATmp* simplifyConvCellToStr(State& env, const IRInstruction* inst) {
  auto const src        = inst->src(0);
  auto const srcType    = src->type();
  auto const catchTrace = inst->taken();

  if (srcType <= TBool) {
    return gen(
      env,
      Select,
      src,
      cns(env, s_1.get()),
      cns(env, s_empty.get())
    );
  }
  if (srcType <= TNull)   return cns(env, s_empty.get());
  if (srcType <= TArr)  {
    gen(env, RaiseNotice, catchTrace,
        cns(env, makeStaticString("Array to string conversion")));
    return cns(env, s_Array.get());
  }
  if (srcType <= TVec) {
    gen(env, RaiseNotice, catchTrace,
        cns(env, makeStaticString("Vec to string conversion")));
    return cns(env, s_Vec.get());
  }
  if (srcType <= TDict) {
    gen(env, RaiseNotice, catchTrace,
        cns(env, makeStaticString("Dict to string conversion")));
    return cns(env, s_Dict.get());
  }
  if (srcType <= TKeyset) {
    gen(env, RaiseNotice, catchTrace,
        cns(env, makeStaticString("Keyset to string conversion")));
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

  return nullptr;
}

SSATmp* simplifyConvCellToInt(State& env, const IRInstruction* inst) {
  auto const src      = inst->src(0);
  auto const srcType  = src->type();

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

  return nullptr;
}

SSATmp* simplifyConvCellToDbl(State& env, const IRInstruction* inst) {
  auto const src      = inst->src(0);
  auto const srcType  = src->type();

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

  return nullptr;
}

SSATmp* simplifyConvObjToBool(State& env, const IRInstruction* inst) {
  auto const ty = inst->src(0)->type();

  if (ty < TObj &&
      ty.clsSpec().cls() &&
      ty.clsSpec().cls()->isCollectionClass()) {
    return gen(env, ColIsNEmpty, inst->src(0));
  }
  return nullptr;
}

SSATmp* simplifyConvCellToObj(State& /*env*/, const IRInstruction* inst) {
  if (inst->src(0)->isA(TObj)) return inst->src(0);
  return nullptr;
}

namespace {

ALWAYS_INLINE bool isSimplifyOkay(const IRInstruction* inst) {
  // We want to be able to simplify the coerce calls away if possible.
  // These serve as huristics to help remove CoerceCell* IT ops.
  // We will let tvCoerceIfStrict handle the exact checking at runtime.
  auto const f = inst->marker().func();

  return !RuntimeOption::PHP7_ScalarTypes ||
         (f && !f->unit()->useStrictTypes());
}

}

SSATmp* simplifyCoerceCellToBool(State& env, const IRInstruction* inst) {
  auto const src     = inst->src(0);
  auto const srcType = src->type();

  if (srcType <= TBool ||
       (isSimplifyOkay(inst)
        && srcType.subtypeOfAny(TNull, TDbl, TInt, TStr))) {
    return gen(env, ConvCellToBool, src);
  }

  // We actually know that any other type will fail causing us to side exit
  // but there's no easy way to optimize for that

  return nullptr;
}

SSATmp* simplifyCoerceCellToInt(State& env, const IRInstruction* inst) {
  auto const src      = inst->src(0);
  auto const srcType  = src->type();

  if (srcType <= TInt ||
       (isSimplifyOkay(inst) && srcType.subtypeOfAny(TBool, TNull, TDbl))) {
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

  if (srcType.subtypeOfAny(TInt, TDbl) ||
       (isSimplifyOkay(inst) && srcType.subtypeOfAny(TBool, TNull))) {
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

SSATmp* simplifyUnboxPtr(State& /*env*/, const IRInstruction* inst) {
  if (inst->src(0)->isA(TPtrToCell)) {
    return inst->src(0);
  }
  return nullptr;
}

SSATmp* simplifyBoxPtr(State& /*env*/, const IRInstruction* inst) {
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

  auto const newType = src->type() & inst->typeParam();
  if (newType == TBottom) {
    gen(env, Unreachable);
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

SSATmp* simplifyCheckRefs(State& env, const IRInstruction* inst) {
  if (!inst->src(0)->hasConstVal()) return nullptr;

  auto const func = inst->src(0)->funcVal();
  auto const extra = inst->extra<CheckRefs>();
  auto i = extra->firstBit;
  auto m = extra->mask;
  auto v = extra->vals;
  while (m) {
    if (m & 1) {
      if (func->byRef(i) != (v & 1)) {
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
  if (!src->type().maybe(TCounted)) {
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
  if (!idx->hasConstVal()) return mergeBranchDests(env, inst);

  auto const idxVal = idx->intVal();
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
    gen(env, Unreachable);
    return cns(env, TBottom);
  }
  return nullptr;
}

SSATmp* arrIntKeyImpl(State& env, const IRInstruction* inst) {
  auto const arr = inst->src(0);
  auto const idx = inst->src(1);
  assertx(arr->hasConstVal(TArr));
  assertx(idx->hasConstVal(TInt));
  assertx(arr->arrVal()->isPHPArray());
  auto const rval = arr->arrVal()->rval(idx->intVal());
  return rval ? cns(env, rval.tv()) : nullptr;
}

SSATmp* arrStrKeyImpl(State& env, const IRInstruction* inst, bool& skip) {
  auto const arr = inst->src(0);
  auto const idx = inst->src(1);
  assertx(arr->hasConstVal(TArr));
  assertx(idx->hasConstVal(TStr));
  assertx(arr->arrVal()->isPHPArray());

  skip = false;
  auto const rval = [&] {
    int64_t val;
    if (arr->arrVal()->convertKey(idx->strVal(), val, false)) {
      if (RuntimeOption::EvalHackArrCompatNotices) {
        skip = true;
        return member_rval{};
      }
      return arr->arrVal()->rval(val);
    }
    return arr->arrVal()->rval(idx->strVal());
  }();
  return rval ? cns(env, rval.tv()) : nullptr;
}

SSATmp* simplifyArrayGet(State& env, const IRInstruction* inst) {
  if (inst->src(0)->hasConstVal() && inst->src(1)->hasConstVal()) {
    if (inst->src(1)->type() <= TInt) {
      if (auto const result = arrIntKeyImpl(env, inst)) {
        return result;
      }
      gen(env, RaiseArrayIndexNotice, inst->taken(), inst->src(1));
      return cns(env, TInitNull);
    }
    if (inst->src(1)->type() <= TStr) {
      bool skip;
      if (auto const result = arrStrKeyImpl(env, inst, skip)) {
        return result;
      }
      if (skip) return nullptr;
      gen(env, RaiseArrayKeyNotice, inst->taken(), inst->src(1));
      return cns(env, TInitNull);
    }
  }
  return nullptr;
}

SSATmp* simplifyArrayIsset(State& env, const IRInstruction* inst) {
  if (inst->src(0)->hasConstVal() && inst->src(1)->hasConstVal()) {
    if (inst->src(1)->type() <= TInt) {
      if (auto const result = arrIntKeyImpl(env, inst)) {
        return cns(env, !result->isA(TInitNull));
      }
      return cns(env, false);
    }
    if (inst->src(1)->type() <= TStr) {
      bool skip;
      if (auto const result = arrStrKeyImpl(env, inst, skip)) {
        return cns(env, !result->isA(TInitNull));
      }
      if (skip) return nullptr;
      return cns(env, false);
    }
  }
  return nullptr;
}

SSATmp* simplifyArrayIdx(State& env, const IRInstruction* inst) {
  if (inst->src(0)->hasConstVal() && inst->src(1)->hasConstVal()) {
    if (inst->src(1)->isA(TInt)) {
      if (auto const result = arrIntKeyImpl(env, inst)) {
        return result;
      }
      return inst->src(2);
    }
    if (inst->src(1)->isA(TStr)) {
      bool skip;
      if (auto const result = arrStrKeyImpl(env, inst, skip)) {
        return result;
      }
      if (skip) return nullptr;
      return inst->src(2);
    }
  }
  return nullptr;
}

SSATmp* simplifyAKExistsArr(State& env, const IRInstruction* inst) {
  if (inst->src(0)->hasConstVal() && inst->src(1)->hasConstVal()) {
    if (inst->src(1)->isA(TInt)) {
      if (arrIntKeyImpl(env, inst)) {
        return cns(env, true);
      }
    } else if (inst->src(1)->isA(TStr)) {
      bool skip;
      if (arrStrKeyImpl(env, inst, skip)) {
        return cns(env, true);
      }
      if (skip) return nullptr;
    }
    return cns(env, false);
  }
  return nullptr;
}

namespace {

template <typename G>
SSATmp* arrGetKImpl(State& env, const IRInstruction* inst, G get) {
  auto const arr = inst->src(0);
  auto const& extra = inst->extra<IndexData>();

  assertx(validPos(ssize_t(extra->index)));
  if (!arr->hasConstVal()) return nullptr;

  auto const mixed = MixedArray::asMixed(get(arr));
  auto const tv = mixed->getArrayElmPtr(extra->index);

  // The array doesn't contain a valid element at that offset. Since this
  // instruction should be guarded by a check, this (should be) unreachable.
  if (!tv) {
    gen(env, Unreachable);
    return cns(env, TBottom);
  }

  assertx(tvIsPlausible(*tv));
  return cns(env, *tv);
}

template <typename G>
SSATmp* checkOffsetImpl(State& env, const IRInstruction* inst, G get) {
  auto const arr = inst->src(0);
  auto const key = inst->src(1);
  auto const& extra = inst->extra<IndexData>();

  assertx(validPos(ssize_t(extra->index)));
  if (!arr->hasConstVal()) return mergeBranchDests(env, inst);

  auto const mixed = MixedArray::asMixed(get(arr));

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
    [&] (member_rval rval) {
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
    [&] (member_rval rval) {
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
    [&] (member_rval rval) { return cns(env, rval && !cellIsNull(rval.tv())); }
  );
}

template <typename I, typename S>
SSATmp* hackArrEmptyElemImpl(State& env, const IRInstruction* inst,
                             I getInt, S getStr) {
  return hackArrQueryImpl(
    env, inst,
    getInt, getStr,
    [&] (member_rval rval) { return cns(env, !rval || !cellToBool(rval.tv())); }
  );
}

template <typename I, typename S>
SSATmp* hackArrIdxImpl(State& env, const IRInstruction* inst,
                       I getInt, S getStr) {
  return hackArrQueryImpl(
    env, inst,
    getInt, getStr,
    [&] (member_rval rval) { return rval ? cns(env, rval.tv()) : inst->src(2); }
  );
}

template <typename I, typename S>
SSATmp* hackArrAKExistsImpl(State& env, const IRInstruction* inst,
                            I getInt, S getStr) {
  return hackArrQueryImpl(
    env, inst,
    getInt, getStr,
    [&] (member_rval rval) { return cns(env, !!rval); }
  );
}

}

#define X(Name, Action, Get)                                          \
SSATmp* simplify##Name(State& env, const IRInstruction* inst) {       \
  return hackArr##Action##Impl(                                       \
    env, inst,                                                        \
    [](SSATmp* a, int64_t k) {                                        \
      return MixedArray::RvalIntDict(a->Get(), k);                    \
    },                                                                \
    [](SSATmp* a, const StringData* k) {                              \
      return MixedArray::RvalStrDict(a->Get(), k);                    \
    }                                                                 \
  );                                                                  \
}

X(DictGet, Get, dictVal)
X(DictGetQuiet, GetQuiet, dictVal)
X(DictIsset, Isset, dictVal)
X(DictEmptyElem, EmptyElem, dictVal)
X(DictIdx, Idx, dictVal)
X(AKExistsDict, AKExists, dictVal)

#undef X

#define X(Name, Action, Get)                                          \
SSATmp* simplify##Name(State& env, const IRInstruction* inst) {       \
  return hackArr##Action##Impl(                                       \
    env, inst,                                                        \
    [](SSATmp* a, int64_t k) {                                        \
      return SetArray::RvalInt(a->Get(), k);                          \
    },                                                                \
    [](SSATmp* a, const StringData* k) {                              \
      return SetArray::RvalStr(a->Get(), k);                          \
    }                                                                 \
  );                                                                  \
}

X(KeysetGet, Get, keysetVal)
X(KeysetGetQuiet, GetQuiet, keysetVal)
X(KeysetIsset, Isset, keysetVal)
X(KeysetEmptyElem, EmptyElem, keysetVal)
X(KeysetIdx, Idx, keysetVal)
X(AKExistsKeyset, AKExists, keysetVal)

#undef X

SSATmp* simplifyMixedArrayGetK(State& env, const IRInstruction* inst) {
  return arrGetKImpl(env, inst, [](SSATmp* a) { return a->arrVal(); });
}

SSATmp* simplifyDictGetK(State& env, const IRInstruction* inst) {
  return arrGetKImpl(env, inst, [](SSATmp* a) { return a->dictVal(); });
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
    gen(env, Unreachable);
    return cns(env, TBottom);
  }

  assertx(tvIsPlausible(*tv));
  assertx(isStringType(tv->m_type) || isIntType(tv->m_type));
  return cns(env, *tv);
}

SSATmp* simplifyCheckMixedArrayOffset(State& env, const IRInstruction* inst) {
  return checkOffsetImpl(env, inst, [](SSATmp* a) { return a->arrVal(); });
}

SSATmp* simplifyCheckDictOffset(State& env, const IRInstruction* inst) {
  return checkOffsetImpl(env, inst, [](SSATmp* a) { return a->dictVal(); });
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

  if (ty <= TArr) return gen(env, CountArray, val);
  if (ty <= TVec) return gen(env, CountVec, val);
  if (ty <= TDict) return gen(env, CountDict, val);
  if (ty <= TKeyset) return gen(env, CountKeyset, val);

  if (ty < TObj) {
    auto const cls = ty.clsSpec().cls();
    if (cls != nullptr && cls->isCollectionClass()) {
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

  if (kind && !arrayKindNeedsVsize(*kind))
    return gen(env, CountArrayFast, src);

  return nullptr;
}

SSATmp* simplifyCountVec(State& env, const IRInstruction* inst) {
  auto const vec = inst->src(0);
  return vec->hasConstVal(TVec) ? cns(env, vec->vecVal()->size()) : nullptr;
}

SSATmp* simplifyCountDict(State& env, const IRInstruction* inst) {
  auto const dict = inst->src(0);
  return dict->hasConstVal(TDict) ? cns(env, dict->dictVal()->size()) : nullptr;
}

SSATmp* simplifyCountKeyset(State& env, const IRInstruction* inst) {
  auto const keyset = inst->src(0);
  return keyset->hasConstVal(TKeyset)
    ? cns(env, keyset->keysetVal()->size()) : nullptr;
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

SSATmp* simplifyLdStrLen(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  return src->hasConstVal(TStr) ? cns(env, src->strVal()->size()) : nullptr;
}

SSATmp* simplifyLdVecElem(State& env, const IRInstruction* inst) {
  auto const src0 = inst->src(0);
  auto const src1 = inst->src(1);
  if (src0->hasConstVal(TVec) && src1->hasConstVal(TInt)) {
    auto const vec = src0->vecVal();
    auto const idx = src1->intVal();
    assertx(vec->isVecArray());
    if (idx >= 0) {
      auto const rval = PackedArray::RvalIntVec(vec, idx);
      return rval ? cns(env, rval.tv()) : nullptr;
    }
  }
  return nullptr;
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

      if (cls->classof(c_WaitHandle::classof())) {
        auto const genState = [&] (Opcode op, int64_t whstate) -> SSATmp* {
          // these methods all spring from the base class
          assert(callee->cls()->name()->isame(s_WaitHandle.get()));
          auto const state = gen(env, LdWHState, thiz);
          return gen(env, op, state, cns(env, whstate));
        };
        auto const methName = callee->name();
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
    });
}

SSATmp* simplifyIsWaitHandle(State& env, const IRInstruction* inst) {
  return simplifyByClass(
    env, inst->src(0),
    [&](const Class* cls, bool) -> SSATmp* {
      if (cls->classof(c_WaitHandle::classof())) return cns(env, true);
      if (!isInterface(cls) &&
          !c_WaitHandle::classof()->classof(cls)) {
        return cns(env, false);
      }
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
    return gen(env, Unreachable);
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

SSATmp* simplifyGetMemoKey(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);

  // Note: this all uses the fully generic memo key scheme. If we used a more
  // specific scheme, then the GetMemoKey op wouldn't have been emitted.

  if (src->hasConstVal()) {
    try {
      ThrowAllErrorsSetter taes;
      auto const key =
        HHVM_FN(serialize_memoize_param)(*src->variantVal().asTypedValue());
      SCOPE_EXIT { tvDecRefGen(key); };
      assertx(cellIsPlausible(key));
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

SSATmp* simplifyStrictlyIntegerConv(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (!src->hasConstVal()) return nullptr;
  int64_t n;
  if (src->strVal()->isStrictlyInteger(n)) return cns(env, n);
  gen(env, IncRef, src);
  return src;
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
  X(CheckMBase)
  X(CheckRefs)
  X(CheckRefInner)
  X(CheckStk)
  X(CheckType)
  X(CheckTypeMem)
  X(AssertType)
  X(CheckNonNull)
  X(CheckPackedArrayDataBounds)
  X(ReservePackedArrayDataNewElem)
  X(CoerceCellToBool)
  X(CoerceCellToDbl)
  X(CoerceCellToInt)
  X(ConcatStrStr)
  X(ConcatIntStr)
  X(ConcatStrInt)
  X(ConvArrToBool)
  X(ConvArrToDbl)
  X(ConvBoolToArr)
  X(ConvBoolToDbl)
  X(ConvBoolToInt)
  X(ConvCellToBool)
  X(ConvCellToDbl)
  X(ConvCellToInt)
  X(ConvCellToObj)
  X(ConvCellToStr)
  X(ConvCellToArr)
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
  X(ConvVecToArr)
  X(ConvDictToArr)
  X(ConvKeysetToArr)
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
  X(ExtendsClass)
  X(InstanceOfBitmask)
  X(NInstanceOfBitmask)
  X(Floor)
  X(FwdCtxStaticCall)
  X(IncRef)
  X(InitObjProps)
  X(InstanceOf)
  X(InstanceOfIface)
  X(IsNType)
  X(IsScalarType)
  X(IsType)
  X(IsWaitHandle)
  X(IsCol)
  X(HasToString)
  X(LdClsCtx)
  X(LdClsCctx)
  X(LdClsName)
  X(LookupClsRDS)
  X(LdClsMethod)
  X(LdStrLen)
  X(LdVecElem)
  X(MethodExists)
  X(CheckCtxThis)
  X(CheckFuncStatic)
  X(RaiseMissingThis)
  X(LdObjClass)
  X(LdObjInvoke)
  X(Mov)
  X(UnboxPtr)
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
  X(CheckMixedArrayOffset)
  X(CheckDictOffset)
  X(CheckKeysetOffset)
  X(CheckArrayCOW)
  X(ArrayIsset)
  X(DictIsset)
  X(KeysetIsset)
  X(DictEmptyElem)
  X(KeysetEmptyElem)
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
  X(SpillFrame)
  X(GetMemoKey)
  X(StrictlyIntegerConv)
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
