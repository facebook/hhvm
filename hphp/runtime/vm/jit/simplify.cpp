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

#include <sstream>
#include <type_traits>
#include <limits>

#include "hphp/util/overflow.h"
#include "hphp/runtime/ext/ext_collections.h"
#include "hphp/runtime/base/type-conversions.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/guard-relaxation.h"
#include "hphp/runtime/vm/jit/ir-builder.h"
#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/runtime.h"
#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/minstr-effects.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_Array("Array");
const StaticString s_isEmpty("isEmpty");
const StaticString s_1("1");
const StaticString s_empty("");
const StaticString s_invoke("__invoke");

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
        assert(inst->isTransient());
        inst = env.unit.cloneInstruction(inst);
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
  assert(!env.insts.empty());
  return gen(env, op, env.insts.top()->marker(), std::forward<Args>(args)...);
}

//////////////////////////////////////////////////////////////////////

/*
 * Simplifier rules are not allowed to add new uses to SSATmps that aren't
 * known to be available.  All the sources to the original instruction must
 * be available, and non-reference counted values reachable through the
 * source chain are also always available.  Anything else requires more
 * complicated analysis than belongs in the simplifier right now.
 */
bool validate(const State& env,
              SSATmp* newDst,
              const IRInstruction* origInst) {
  // Certain opcodes that read stack locations have valid simplification
  // rules (we know values are available because they are on the eval
  // stack) that are not easy to double check here.
  if (origInst->op() == LdStack || origInst->op() == DecRefStack) {
    return true;
  }

  auto known_available = [&] (SSATmp* src) -> bool {
    if (!src->type().maybeCounted()) return true;
    for (auto& oldSrc : origInst->srcs()) {
      if (oldSrc == src) return true;
    }
    return false;
  };

  if (!env.newInsts.empty()) {
    for (auto& newInst : env.newInsts) {
      for (auto& src : newInst->srcs()) {
        always_assert_flog(
          known_available(src),
          "A simplification rule produced an instruction that used a value "
          "that wasn't known to be available:\n"
          "  original inst: {}\n"
          "  new inst     : {}\n"
          "  src          : {}\n",
          origInst->toString(),
          newInst->toString(),
          src->toString()
        );
      }
    }
    return true;
  }

  if (newDst) {
    always_assert_flog(
      known_available(newDst),
      "simplifier produced a new destination that wasn't known to be "
      "available:\n"
      "  original inst: {}\n"
      "  new dst:       {}\n",
      origInst->toString(),
      newDst->toString()
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

SSATmp* simplifySpillStack(State& env, const IRInstruction* inst) {
  auto const sp           = inst->src(0);
  auto const spDeficit    = inst->src(1)->intVal();
  auto const numSpillSrcs = inst->srcs().subpiece(2).size();

  // If there's nothing to spill, and no stack adjustment, we don't
  // need the instruction; the old stack is still accurate.
  if (!numSpillSrcs && spDeficit == 0) return sp;

  return nullptr;
}

SSATmp* simplifyCheckCtxThis(State& env, const IRInstruction* inst) {
  auto const func = inst->marker().func();
  auto const srcTy = inst->src(0)->type();
  if (srcTy <= Type::Obj) return gen(env, Nop);
  if (!func->mayHaveThis() || !srcTy.maybe(Type::Obj)) {
    return gen(env, Jmp, inst->taken());
  }
  return nullptr;
}

SSATmp* simplifyCastCtxThis(State& env, const IRInstruction* inst) {
  // TODO(#5623596): this transformation is required for correctness in
  // refcount opts right now.
  if (inst->src(0)->type() <= Type::Obj) return inst->src(0);
  return nullptr;
}

SSATmp* simplifyLdCtx(State& env, const IRInstruction* inst) {
  auto const func = inst->marker().func();
  if (!func->isStatic()) return nullptr;

  // Change LdCtx in static functions to LdCctx, or if we're inlining try to
  // fish out a constant context.
  auto const src = inst->src(0);
  auto const srcInst = src->inst();
  if (srcInst->is(DefInlineFP)) {
    auto const stackPtr = srcInst->src(0);
    if (auto const spillFrame = findSpillFrame(stackPtr)) {
      auto const cls = spillFrame->src(2);
      if (cls->isConst(Type::Cls)) {
        return cns(env, ConstCctx::cctx(cls->clsVal()));
      }
    }
  }
  // ActRec->m_cls of a static function is always a valid class pointer with
  // the bottom bit set
  return gen(env, LdCctx, src);
}

SSATmp* simplifyLdClsCtx(State& env, const IRInstruction* inst) {
  SSATmp* ctx = inst->src(0);
  if (ctx->isConst(Type::Cctx)) {
    return cns(env, ctx->cctxVal().cls());
  }
  Type ctxType = ctx->type();
  if (ctxType <= Type::Obj) {
    // this pointer... load its class ptr
    return gen(env, LdObjClass, ctx);
  }
  if (ctxType <= Type::Cctx) {
    return gen(env, LdClsCctx, ctx);
  }
  return nullptr;
}

SSATmp* simplifyLdObjClass(State& env, const IRInstruction* inst) {
  auto const ty = inst->src(0)->type();

  if (mightRelax(env, inst->src(0)) || !(ty < Type::Obj)) return nullptr;

  if (auto const exact = ty.getExactClass()) return cns(env, exact);
  return nullptr;
}

SSATmp* simplifyLdObjInvoke(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (!src->isConst()) return nullptr;

  auto const cls = src->clsVal();
  if (!RDS::isPersistentHandle(cls->classHandle())) return nullptr;

  auto const meth = cls->getCachedInvoke();
  return meth == nullptr ? nullptr : cns(env, meth.get());
}

SSATmp* simplifyGetCtxFwdCall(State& env, const IRInstruction* inst) {
  auto const srcCtx = inst->src(0);
  if (srcCtx->isA(Type::Cctx)) return srcCtx;
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
  if (src->isConst()) return cns(env, fabs(src->dblVal()));
  return nullptr;
}

template<class Oper>
SSATmp* constImpl(State& env, SSATmp* src1, SSATmp* src2, Oper op) {
  // don't canonicalize to the right, OP might not be commutative
  if (!src1->isConst() || !src2->isConst()) return nullptr;

  auto both = [&](Type ty) { return src1->type() <= ty && src2->type() <= ty; };

  if (both(Type::Bool)) return cns(env, op(src1->boolVal(), src2->boolVal()));
  if (both(Type::Int))  return cns(env, op(src1->intVal(), src2->intVal()));
  if (both(Type::Dbl))  return cns(env, op(src1->dblVal(), src2->dblVal()));
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
  if (src1->isConst() && !src2->isConst()) {
    return gen(env, opcode, src2, src1);
  }

  // Only handle integer operations for now.
  if (!src1->isA(Type::Int) || !src2->isA(Type::Int)) return nullptr;

  auto const inst1 = src1->inst();
  auto const inst2 = src2->inst();
  if (inst1->op() == opcode && inst1->src(1)->isConst()) {
    // (X + C1) + C2 --> X + C3
    if (src2->isConst()) {
      int64_t right = inst1->src(1)->intVal();
      right = op(right, src2->intVal());
      return gen(env, opcode, inst1->src(0), cns(env, right));
    }
    // (X + C1) + (Y + C2) --> X + Y + C3
    if (inst2->op() == opcode && inst2->src(1)->isConst()) {
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
  if (src2->isConst()) {
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
    if (src->isConst() && src->intVal() == 0) {
      return gen(env, SubInt, src1, inst2->src(1));
    }
  }
  auto inst1 = src1->inst();

  // (X - C1) + ...
  if (inst1->op() == SubInt && inst1->src(1)->isConst()) {
    auto x = inst1->src(0);
    auto c1 = inst1->src(1);

    // (X - C1) + C2 --> X + (C2 - C1)
    if (src2->isConst()) {
      auto rhs = gen(env, SubInt, cns(env, src2->intVal()), c1);
      return gen(env, AddInt, x, rhs);
    }

    // (X - C1) + (Y +/- C2)
    if ((inst2->op() == AddInt || inst2->op() == SubInt) &&
        inst2->src(1)->isConst()) {
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
    if (inst2->op() == AddInt && inst2->src(1)->isConst()) {
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
  if (src1->isConst() && src2->isConst()) {
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

  if (src2->isConst()) {
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
    if (src->isConst(0)) return gen(env, AddInt, src1, inst2->src(1));
  }
  return nullptr;
}

SSATmp* simplifySubIntO(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);
  if (src1->isConst() && src2->isConst()) {
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

  if (!src2->isConst()) return nullptr;

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
    assert(a > 0);
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
  if (src1->isConst() && src2->isConst()) {
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

  if (!src2->isConst()) return nullptr;

  auto const src2Val = src2->intVal();
  if (src2Val == 0 || src2Val == -1) {
    // Undefined behavior, so we might as well constant propagate whatever we
    // want. If we're being asked to simplify this, it better be dynamically
    // unreachable code.
    return cns(env, 0);
  }

  if (src1->isConst()) return cns(env, src1->intVal() % src2Val);
  // X % 1 --> 0
  if (src2Val == 1) return cns(env, 0);

  return nullptr;
}

SSATmp* simplifyDivDbl(State& env, const IRInstruction* inst) {
  auto src1 = inst->src(0);
  auto src2 = inst->src(1);

  if (!src2->isConst()) return nullptr;

  // not supporting integers (#2570625)
  double src2Val = src2->dblVal();

  // X / 0 -> bool(false)
  if (src2Val == 0.0) {
    // Ideally we'd generate a RaiseWarning and return false here, but we need
    // a catch trace for that and we can't make a catch trace without
    // HhbcTranslator.
    return nullptr;
  }

  // statically compute X / Y
  return src1->isConst() ? cns(env, src1->dblVal() / src2Val) : nullptr;
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
  if (src2->isConst()) {
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
  if (src2->isConst()) {
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
  if (src2->isConst(0)) return src1;
  return nullptr;
}

SSATmp* xorTrueImpl(State& env, SSATmp* src) {
  auto const inst = src->inst();
  auto const op = inst->op();

  switch (op) {
  // !!X --> X
  case XorBool:
    if (inst->src(1)->isConst(true)) {
      // This is safe to add a new use to because inst->src(0) is a bool.
      assert(inst->src(0)->isA(Type::Bool));
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
    auto const unsafeTypes = Type::Dbl|Type::Arr;
    auto const safeToFold =
      s0->type().not(unsafeTypes) && s1->type().not(unsafeTypes) &&
      // We can't add new uses to reference counted types without a more
      // advanced availability analysis.
      !s0->type().maybeCounted() && !s1->type().maybeCounted();
    if (safeToFold) {
      return gen(env, negateQueryOp(op), s0, s1);
    }
    break;
  }

  case InstanceOfBitmask:
  case NInstanceOfBitmask:
    // This is safe because instanceofs don't take reference counted arguments.
    assert(!inst->src(0)->type().maybeCounted() &&
           !inst->src(1)->type().maybeCounted());
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
  if (src1->isConst() && src2->isConst()) {
    return cns(env, bool(src1->boolVal() ^ src2->boolVal()));
  }

  // Canonicalize constants to the right.
  if (src1->isConst() && !src2->isConst()) {
    return gen(env, XorBool, src2, src1);
  }

  // X^0 => X
  if (src2->isConst(false)) return src1;

  // X^1 => simplify "not" logic
  if (src2->isConst(true)) return xorTrueImpl(env, src1);
  return nullptr;
}

template<class Oper>
SSATmp* shiftImpl(State& env, const IRInstruction* inst, Oper op) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);

  if (src1->isConst()) {
    if (src1->intVal() == 0) {
      return cns(env, 0);
    }

    if (src2->isConst()) {
      return cns(env, op(src1->intVal(), src2->intVal()));
    }
  }

  if (src2->isConst() && src2->intVal() == 0) {
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
  case Gt:   return a > b;
  case GteInt:
  case Gte:  return a >= b;
  case LtInt:
  case Lt:   return a < b;
  case LteInt:
  case Lte:  return a <= b;
  case Same:
  case EqInt:
  case Eq:   return a == b;
  case NSame:
  case NeqInt:
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
  if (src1 == src2 && type1.not(Type::Dbl)) {
    // (val1 == val1) does not simplify to true when val1 is a NaN
    return cns(env, bool(cmpOp(opName, 0, 0)));
  }

  // Need both types to be unboxed to simplify, and the code below assumes the
  // types are known DataTypes.
  if (!type1.isKnownUnboxedDataType() || !type2.isKnownUnboxedDataType()) {
    return nullptr;
  }

  // OpSame and OpNSame have some special rules
  if (opName == Same || opName == NSame) {
    // OpSame and OpNSame do not perform type juggling
    if (type1.toDataType() != type2.toDataType() &&
        !(type1 <= Type::Str && type2 <= Type::Str)) {
      return cns(env, opName == NSame);
    }
    // Here src1 and src2 are same type, treating Str and StaticStr as the
    // same.

    // Constant fold if they are both constant strings.
    if (type1 <= Type::Str && type2 <= Type::Str) {
      if (src1->isConst() && src2->isConst()) {
        auto const str1 = src1->strVal();
        auto const str2 = src2->strVal();
        bool same = str1->same(str2);
        return cns(env, bool(cmpOp(opName, same, 1)));
      }
      return nullptr;
    }

    // If type is a primitive type - simplify to Eq/Neq.  Str was already
    // removed above.
    auto const badTypes = Type::Obj | Type::Res | Type::Arr;
    if (type1.maybe(badTypes) || type2.maybe(badTypes)) {
      return nullptr;
    }
    return newInst(opName == Same ? Eq : Neq, src1, src2);
  }

  // ---------------------------------------------------------------------
  // We may now perform constant-constant optimizations
  // ---------------------------------------------------------------------

  // Null cmp Null
  if (type1 <= Type::Null && type2 <= Type::Null) {
    return cns(env, bool(cmpOp(opName, 0, 0)));
  }

  // const cmp const
  if (src1->isConst() && src2->isConst()) {
    // StaticStr cmp StaticStr
    if (src1->isA(Type::StaticStr) &&
        src2->isA(Type::StaticStr)) {
      int cmp = src1->strVal()->compare(src2->strVal());
      return cns(env, bool(cmpOp(opName, cmp, 0)));
    }
    // ConstInt cmp ConstInt
    if (src1->isA(Type::Int) && src2->isA(Type::Int)) {
      return cns(env, bool(
        cmpOp(opName, src1->intVal(), src2->intVal())));
    }
    // ConstBool cmp ConstBool
    if (src1->isA(Type::Bool) && src2->isA(Type::Bool)) {
      return cns(env, bool(
        cmpOp(opName, src1->boolVal(), src2->boolVal())));
    }
  }

  // ---------------------------------------------------------------------
  // Constant bool comparisons can be strength-reduced
  // NOTE: Comparisons with bools get juggled to bool.
  // ---------------------------------------------------------------------

  // Perform constant-bool optimizations
  if (src2->isA(Type::Bool) && src2->isConst()) {
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
  if (!isIntQueryOp(opName) && type1 <= Type::Int && type2 <= Type::Int) {
    return newInst(queryToIntQueryOp(opName), src1, src2);
  }

  // Dbl-dbl or dbl-int comparison lower to dbl-comparison
  if (!isDblQueryOp(opName) &&
      (type1 <= Type::Dbl || type2 <= Type::Dbl) &&
      (type1.subtypeOfAny(Type::Int, Type::Dbl) &&
       type2.subtypeOfAny(Type::Int, Type::Dbl))) {
    return newInst(queryToDblQueryOp(opName),
                   gen(env, ConvCellToDbl, src1),
                   gen(env, ConvCellToDbl, src2));
  }

  // ---------------------------------------------------------------------
  // For same-type cmps, canonicalize any constants to the right
  // Then stop - there are no more simplifications left
  // ---------------------------------------------------------------------

  if (type1.toDataType() == type2.toDataType() ||
      (type1 <= Type::Str && type2 <= Type::Str)) {
    if (src1->isConst() && !src2->isConst()) {
      return newInst(commuteQueryOp(opName), src2, src1);
    }
    return nullptr;
  }

  // ---------------------------------------------------------------------
  // Perform type juggling and type canonicalization for different types
  // see http://docs.hhvm.com/manual/en/language.operators.comparison.php
  // ---------------------------------------------------------------------

  // nulls get canonicalized to the right
  if (type1 <= Type::Null) {
    return newInst(commuteQueryOp(opName), src2, src1);
  }

  // case 1a: null cmp string. Convert null to ""
  if (type1 <= Type::Str && type2 <= Type::Null) {
    return newInst(opName, src1, cns(env, s_empty.get()));
  }

  // case 1b: null cmp object. Convert null to false and the object to true
  if (type1 <= Type::Obj && type2 <= Type::Null) {
    return newInst(opName, cns(env, true), cns(env, false));
  }

  // case 2a: null cmp anything. Convert null to false
  if (type2 <= Type::Null) {
    return newInst(opName, src1, cns(env, false));
  }

  // bools get canonicalized to the right
  if (src1->isA(Type::Bool)) {
    return newInst(commuteQueryOp(opName), src2, src1);
  }

  // case 2b: bool cmp anything. Convert anything to bool
  if (src2->isA(Type::Bool)) {
    if (src1->isConst()) {
      if (src1->isA(Type::Int)) {
        return newInst(opName, cns(env, bool(src1->intVal())), src2);
      } else if (src1->isA(Type::Str)) {
        auto str = src1->strVal();
        return newInst(opName, cns(env, str->toBoolean()), src2);
      }
    }

    // Optimize comparison between int and const bool
    if (src1->isA(Type::Int) && src2->isConst()) {
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
  if (type2 <= Type::Str) {
    return newInst(commuteQueryOp(opName), src2, src1);
  }

  // ints get canonicalized to the right
  if (src1->isA(Type::Int)) {
    return newInst(commuteQueryOp(opName), src2, src1);
  }

  // case 4: number/string/resource cmp. Convert to number (int OR double)
  // NOTE: The following if-test only checks for some of the SRON-SRON
  //  cases (specifically, string-int). Other cases (like string-string)
  //  are dealt with earlier, while other cases (like number-resource)
  //  are not caught at all (and end up exiting this macro at the bottom).
  if (src1->isConst(Type::Str) && src2->isA(Type::Int)) {
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
  if (src1->isA(Type::Arr)) {
    return cns(env, bool(cmpOp(opName, 1, 0)));
  }
  if (src2->isA(Type::Arr)) {
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

#undef X

SSATmp* queryJmpImpl(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);
  auto const opc = inst->op();
  // reuse the logic in cmpImpl
  auto const newCmp = cmpImpl(
    env,
    queryJmpToQueryOp(opc),
    nullptr,
    src1,
    src2
  );
  if (!newCmp) return nullptr;

  // Become an equivalent conditional jump and reuse that logic.
  return gen(env, JmpNZero, inst->taken(), newCmp);
}

#define X(x)                                                \
  SSATmp* simplify##x(State& env, const IRInstruction* i) { \
    return queryJmpImpl(env, i);                            \
  }

X(JmpGt)
X(JmpGte)
X(JmpLt)
X(JmpLte)
X(JmpEq)
X(JmpNeq)
X(JmpGtInt)
X(JmpGteInt)
X(JmpLtInt)
X(JmpLteInt)
X(JmpEqInt)
X(JmpNeqInt)
X(JmpSame)
X(JmpNSame)

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
  assert(IMPLIES(type <= Type::Str, type == Type::Str));
  assert(IMPLIES(type <= Type::Arr, type == Type::Arr));

  // The types are disjoint; the result must be false.
  if (srcType.not(type)) {
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

SSATmp* simplifyIsType(State& env, const IRInstruction* i) {
  return isTypeImpl(env, i);
}

SSATmp* simplifyIsNType(State& env, const IRInstruction* i) {
  return isTypeImpl(env, i);
}

SSATmp* simplifyIsScalarType(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->type().isKnownDataType()) {
    return cns(env, src->isA(Type::Int | Type::Dbl | Type::Str | Type::Bool));
  }
  return nullptr;
}

SSATmp* simplifyConcatCellCell(State& env, const IRInstruction* inst) {
  auto const src1       = inst->src(0);
  auto const src2       = inst->src(1);
  auto const catchBlock = inst->taken();

  if (src1->isA(Type::Str) && src2->isA(Type::Str)) { // StrStr
    return gen(env, ConcatStrStr, catchBlock, src1, src2);
  }
  if (src1->isA(Type::Int) && src2->isA(Type::Str)) { // IntStr
    return gen(env, ConcatIntStr, catchBlock, src1, src2);
  }
  if (src1->isA(Type::Str) && src2->isA(Type::Int)) { // StrInt
    return gen(env, ConcatStrInt, catchBlock, src1, src2);
  }

  // XXX: t3770157. All the cases below need two different catch blocks but we
  // only have access to one here.
  return nullptr;

  if (src1->isA(Type::Int)) { // IntCell
    auto* asStr = gen(env, ConvCellToStr, catchBlock, src2);
    auto* result = gen(env, ConcatIntStr, src1, asStr);
    // ConcatIntStr doesn't consume its second input so we have to decref it
    // here.
    gen(env, DecRef, asStr);
    return result;
  }
  if (src2->isA(Type::Int)) { // CellInt
    auto const asStr = gen(env, ConvCellToStr, catchBlock, src1);
    // concat promises to decref its first argument. we need to do it here
    gen(env, DecRef, src1);
    return gen(env, ConcatStrInt, asStr, src2);
  }
  if (src1->isA(Type::Str)) { // StrCell
    auto* asStr = gen(env, ConvCellToStr, catchBlock, src2);
    auto* result = gen(env, ConcatStrStr, src1, asStr);
    // ConcatStrStr doesn't consume its second input so we have to decref it
    // here.
    gen(env, DecRef, asStr);
    return result;
  }
  if (src2->isA(Type::Str)) { // CellStr
    auto const asStr = gen(env, ConvCellToStr, catchBlock, src1);
    // concat promises to decref its first argument. we need to do it here
    gen(env, DecRef, src1);
    return gen(env, ConcatStrStr, asStr, src2);
  }

  return nullptr;
}

SSATmp* simplifyConcatStrStr(State& env, const IRInstruction* inst) {
  auto const src1 = inst->src(0);
  auto const src2 = inst->src(1);
  if (src1->isConst() && src1->isA(Type::StaticStr) &&
      src2->isConst() && src2->isA(Type::StaticStr)) {
    auto const str1 = const_cast<StringData*>(src1->strVal());
    auto const str2 = const_cast<StringData*>(src2->strVal());
    auto const sval = String::attach(concat_ss(str1, str2));
    return cns(env, makeStaticString(sval.get()));
  }

  return nullptr;
}

SSATmp* convToArrImpl(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->isConst()) {
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
  if (src->isConst()) {
    // const_cast is safe. We're only making use of a cell helper.
    auto arr = const_cast<ArrayData*>(src->arrVal());
    return cns(env, cellToBool(make_tv<KindOfArray>(arr)));
  }
  return nullptr;
}

SSATmp* simplifyConvDblToBool(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->isConst()) {
    bool const bval = src->dblVal();
    return cns(env, bval);
  }
  return nullptr;
}

SSATmp* simplifyConvIntToBool(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->isConst()) {
    bool const bval = src->intVal();
    return cns(env, bval);
  }
  return nullptr;
}

SSATmp* simplifyConvStrToBool(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->isConst()) {
    // only the strings "", and "0" convert to false, all other strings
    // are converted to true
    auto const str = src->strVal();
    return cns(env, !str->empty() && !str->isZero());
  }
  return nullptr;
}

SSATmp* simplifyConvArrToDbl(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->isConst()) {
    if (src->arrVal()->empty()) {
      return cns(env, 0.0);
    }
  }
  return nullptr;
}

SSATmp* simplifyConvBoolToDbl(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->isConst()) {
    double const dval = src->boolVal();
    return cns(env, dval);
  }
  return nullptr;
}

SSATmp* simplifyConvIntToDbl(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->isConst()) {
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
  return src->isConst() ? cns(env, src->strVal()->toDouble()) : nullptr;
}

SSATmp* simplifyConvArrToInt(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->isConst()) {
    if (src->arrVal()->empty()) return cns(env, 0);
    return cns(env, 1);
  }
  return nullptr;
}

SSATmp* simplifyConvBoolToInt(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->isConst()) return cns(env, static_cast<int>(src->boolVal()));
  return nullptr;
}

SSATmp* simplifyConvDblToInt(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->isConst()) return cns(env, toInt64(src->dblVal()));
  return nullptr;
}

SSATmp* simplifyConvStrToInt(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  return src->isConst() ? cns(env, src->strVal()->toInt64()) : nullptr;
}

SSATmp* simplifyConvBoolToStr(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->isConst()) {
    if (src->boolVal()) return cns(env, s_1.get());
    return cns(env, s_empty.get());
  }
  return nullptr;
}

SSATmp* simplifyConvDblToStr(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->isConst()) {
    String dblStr(buildStringData(src->dblVal()));
    return cns(env, makeStaticString(dblStr));
  }
  return nullptr;
}

SSATmp* simplifyConvIntToStr(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (src->isConst()) {
    return cns(env,
      makeStaticString(folly::to<std::string>(src->intVal()))
    );
  }
  return nullptr;
}

SSATmp* simplifyConvCellToBool(State& env, const IRInstruction* inst) {
  auto const src     = inst->src(0);
  auto const srcType = src->type();

  if (srcType <= Type::Bool) return src;
  if (srcType <= Type::Null) return cns(env, false);
  if (srcType <= Type::Arr)  return gen(env, ConvArrToBool, src);
  if (srcType <= Type::Dbl)  return gen(env, ConvDblToBool, src);
  if (srcType <= Type::Int)  return gen(env, ConvIntToBool, src);
  if (srcType <= Type::Str)  return gen(env, ConvStrToBool, src);
  if (srcType <= Type::Obj) {
    if (auto cls = srcType.getClass()) {
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
  if (srcType <= Type::Res)  return nullptr; // No specialization yet

  return nullptr;
}

SSATmp* simplifyConvCellToStr(State& env, const IRInstruction* inst) {
  auto const src        = inst->src(0);
  auto const srcType    = src->type();
  auto const catchTrace = inst->taken();

  if (srcType <= Type::Bool)   return gen(env, ConvBoolToStr, src);
  if (srcType <= Type::Null)   return cns(env, s_empty.get());
  if (srcType <= Type::Arr)  {
    gen(env, RaiseNotice, catchTrace,
        cns(env, makeStaticString("Array to string conversion")));
    return cns(env, s_Array.get());
  }
  if (srcType <= Type::Dbl)    return gen(env, ConvDblToStr, src);
  if (srcType <= Type::Int)    return gen(env, ConvIntToStr, src);
  if (srcType <= Type::Str) {
    gen(env, IncRef, src);
    return src;
  }
  if (srcType <= Type::Obj)    return gen(env, ConvObjToStr, catchTrace, src);
  if (srcType <= Type::Res)    return gen(env, ConvResToStr, catchTrace, src);

  return nullptr;
}

SSATmp* simplifyConvCellToInt(State& env, const IRInstruction* inst) {
  auto const src      = inst->src(0);
  auto const srcType  = src->type();

  if (srcType <= Type::Int)  return src;
  if (srcType <= Type::Null) return cns(env, 0);
  if (srcType <= Type::Arr)  return gen(env, ConvArrToInt, src);
  if (srcType <= Type::Bool) return gen(env, ConvBoolToInt, src);
  if (srcType <= Type::Dbl)  return gen(env, ConvDblToInt, src);
  if (srcType <= Type::Str)  return gen(env, ConvStrToInt, src);
  if (srcType <= Type::Obj)  return gen(env, ConvObjToInt, inst->taken(), src);
  if (srcType <= Type::Res)  return nullptr; // No specialization yet

  return nullptr;
}

SSATmp* simplifyConvCellToDbl(State& env, const IRInstruction* inst) {
  auto const src      = inst->src(0);
  auto const srcType  = src->type();

  if (srcType <= Type::Dbl)  return src;
  if (srcType <= Type::Null) return cns(env, 0.0);
  if (srcType <= Type::Arr)  return gen(env, ConvArrToDbl, src);
  if (srcType <= Type::Bool) return gen(env, ConvBoolToDbl, src);
  if (srcType <= Type::Int)  return gen(env, ConvIntToDbl, src);
  if (srcType <= Type::Str)  return gen(env, ConvStrToDbl, src);
  if (srcType <= Type::Obj)  return gen(env, ConvObjToDbl, inst->taken(), src);
  if (srcType <= Type::Res)  return nullptr; // No specialization yet

  return nullptr;
}

SSATmp* simplifyConvObjToBool(State& env, const IRInstruction* inst) {
  auto const ty = inst->src(0)->type();

  if (!typeMightRelax(inst->src(0)) &&
      ty < Type::Obj &&
      ty.getClass() &&
      ty.getClass()->isCollectionClass()) {
    return gen(env, ColIsNEmpty, inst->src(0));
  }
  return nullptr;
}

SSATmp* simplifyConvCellToObj(State& env, const IRInstruction* inst) {
  if (inst->src(0)->isA(Type::Obj)) return inst->src(0);
  return nullptr;
}

SSATmp* simplifyCoerceCellToBool(State& env, const IRInstruction* inst) {
  auto const src     = inst->src(0);
  auto const srcType = src->type();

  if (srcType.subtypeOfAny(Type::Bool, Type::Null, Type::Dbl,
                           Type::Int, Type::Str)) {
    return gen(env, ConvCellToBool, src);
  }

  // We actually know that any other type will fail causing us to side exit
  // but there's no easy way to optimize for that

  return nullptr;
}

SSATmp* simplifyCoerceCellToInt(State& env, const IRInstruction* inst) {
  auto const src      = inst->src(0);
  auto const srcType  = src->type();

  if (srcType.subtypeOfAny(Type::Int, Type::Bool, Type::Null, Type::Dbl,
                           Type::Bool)) {
    return gen(env, ConvCellToInt, inst->taken(), src);
  }

  if (srcType <= Type::Str) return gen(env, CoerceStrToInt, inst->taken(),
                                       *inst->extra<CoerceCellToInt>(), src);

  // We actually know that any other type will fail causing us to side exit
  // but there's no easy way to optimize for that

  return nullptr;
}

SSATmp* simplifyCoerceCellToDbl(State& env, const IRInstruction* inst) {
  auto const src      = inst->src(0);
  auto const srcType  = src->type();

  if (srcType.subtypeOfAny(Type::Int, Type::Bool, Type::Null, Type::Dbl,
                           Type::Bool)) {
    return gen(env, ConvCellToDbl, inst->taken(), src);
  }

  if (srcType <= Type::Str) return gen(env, CoerceStrToDbl, inst->taken(),
                                       *inst->extra<CoerceCellToDbl>(), src);

  // We actually know that any other type will fail causing us to side exit
  // but there's no easy way to optimize for that

  return nullptr;
}

template<class Oper>
SSATmp* roundImpl(State& env, const IRInstruction* inst, Oper op) {
  auto const src  = inst->src(0);

  if (src->isConst()) {
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
  if (inst->src(0)->isA(Type::PtrToCell)) {
    return inst->src(0);
  }
  return nullptr;
}

SSATmp* simplifyBoxPtr(State& env, const IRInstruction* inst) {
  if (inst->src(0)->isA(Type::PtrToBoxedCell)) {
    return inst->src(0);
  }
  return nullptr;
}

SSATmp* simplifyCheckInit(State& env, const IRInstruction* inst) {
  auto const srcType = inst->src(0)->type();
  assert(srcType.notPtr());
  assert(inst->taken());
  if (srcType.not(Type::Uninit)) return gen(env, Nop);
  return nullptr;
}

SSATmp* decRefImpl(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  if (!mightRelax(env, src) && !src->type().maybeCounted()) {
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
  if (!mightRelax(env, src) && !src->type().maybeCounted()) {
    return gen(env, Nop);
  }
  return nullptr;
}

SSATmp* simplifyIncRefCtx(State& env, const IRInstruction* inst) {
  auto const ctx = inst->src(0);
  if (ctx->isA(Type::Obj)) {
    return gen(env, IncRef, ctx);
  } else if (!mightRelax(env, ctx) && ctx->type().notCounted()) {
    return gen(env, Nop);
  }

  return nullptr;
}

SSATmp* condJmpImpl(State& env, const IRInstruction* inst) {
  auto const src       = inst->src(0);
  auto const srcInst   = src->inst();
  auto const srcOpcode = srcInst->op();

  // After other simplifications below (isConvIntOrPtrToBool), we can
  // end up with a non-Bool input.  Nothing more to do in this case.
  if (!src->isA(Type::Bool)) {
    return nullptr;
  }

  // Constant propagate.
  if (src->isConst()) {
    bool val = src->boolVal();
    if (inst->op() == JmpZero) {
      val = !val;
    }
    if (val) {
      return gen(env, Jmp, inst->taken());
    }
    return gen(env, Nop);
  }

  // Pull negations into the jump.
  if (srcOpcode == XorBool && srcInst->src(1)->isConst(true)) {
    if (!srcInst->src(0)->type().maybeCounted()) {
      return gen(
        env,
        inst->op() == JmpZero ? JmpNZero : JmpZero,
        inst->taken(),
        srcInst->src(0)
      );
    }
  }

  /*
   * Try to combine the src inst with the Jmp.  We can't do any combinations of
   * the src instruction with the jump if the src's are refcounted, since we
   * may have dec refs between the src instruction and the jump.
   */
  for (auto& src : srcInst->srcs()) {
    if (src->type().maybeCounted()) return nullptr;
  }

  // If the source is conversion of an int or pointer to boolean, we
  // can test the int/ptr value directly.
  auto isConvIntOrPtrToBool = [&](const IRInstruction* instr) {
    switch (instr->op()) {
    case ConvIntToBool:
      return true;
    case ConvCellToBool:
      return instr->src(0)->type().subtypeOfAny(
        Type::Func, Type::Cls, Type::VarEnv, Type::TCA);
    default:
      return false;
    }
  };
  if (isConvIntOrPtrToBool(srcInst)) {
    // We can just check the int or ptr directly. Borrow the Conv's src.
    return gen(env, inst->op(), inst->taken(), srcInst->src(0));
  }

  auto canCompareFused = [&]() {
    auto src1Type = srcInst->src(0)->type();
    auto src2Type = srcInst->src(1)->type();
    return ((src1Type <= Type::Int && src2Type <= Type::Int) ||
            (src1Type <= Type::Bool && src2Type <= Type::Bool) ||
            (src1Type <= Type::Cls && src2Type <= Type::Cls));
  };

  // Fuse jumps with query operators.
  if (isFusableQueryOp(srcOpcode) && canCompareFused()) {
    auto opc = queryToJmpOp(inst->op() == JmpZero
                            ? negateQueryOp(srcOpcode) : srcOpcode);
    SrcRange ssas = srcInst->srcs();

    return gen(env, opc, inst->maybeTypeParam(), inst->taken(),
               std::make_pair(ssas.size(), ssas.begin()));
  }

  return nullptr;
}

SSATmp* simplifyJmpZero(State& env, const IRInstruction* i) {
  return condJmpImpl(env, i);
}
SSATmp* simplifyJmpNZero(State& env, const IRInstruction* i) {
  return condJmpImpl(env, i);
}

SSATmp* simplifyCastStk(State& env, const IRInstruction* inst) {
  auto const info = getStackValue(inst->src(0),
                                  inst->extra<CastStk>()->offset);
  if (mightRelax(env, info.value)) return nullptr;
  if (inst->typeParam() == Type::NullableObj && info.knownType <= Type::Null) {
    // If we're casting Null to NullableObj, we still need to call
    // tvCastToNullableObjectInPlace. See comment there and t3879280 for
    // details.
    return nullptr;
  } else if (info.knownType <= inst->typeParam()) {
    return inst->src(0);
  }
  return nullptr;
}

SSATmp* simplifyCoerceStk(State& env, const IRInstruction* inst) {
  auto const info = getStackValue(inst->src(0),
                                  inst->extra<CoerceStk>()->offset);
  if (mightRelax(env, info.value)) return nullptr;
  if (info.knownType <= inst->typeParam()) {
    // No need to cast---the type was as good or better.
    return inst->src(0);
  }
  return nullptr;
}

SSATmp* simplifyLdStack(State& env, const IRInstruction* inst) {
  auto const info = getStackValue(inst->src(0),
                                  inst->extra<LdStack>()->offset);

  // We don't want to extend live ranges of tmps across calls, so we
  // don't get the value if spansCall is true; however, we can use
  // any type information known.
  auto* value = info.value;
  if (value && (!info.spansCall || info.value->inst()->is(DefConst))) {
    // The refcount optimizations depend on reliably tracking refcount
    // producers and consumers. LdStack and other raw loads are special cased
    // during the analysis, so if we're going to replace this LdStack with a
    // value that isn't from another raw load, we need to leave something in
    // its place to preserve that information.
    if (!value->inst()->isRawLoad() &&
        (value->type().maybeCounted() || mightRelax(env, info.value))) {
      gen(env, TakeStack, info.value);
    }
    return info.value;
  }

  if (info.knownType < inst->typeParam()) {
    return gen(
      env,
      LdStack,
      *inst->extra<LdStack>(),
      info.knownType,
      inst->src(0)
    );
  }

  return nullptr;
}

SSATmp* simplifyTakeStack(State& env, const IRInstruction* inst) {
  if (inst->src(0)->type().notCounted() &&
      !mightRelax(env, inst->src(0))) {
    return gen(env, Nop);
  }

  return nullptr;
}

SSATmp* simplifyDecRefStack(State& env, const IRInstruction* inst) {
  auto const info = getStackValue(inst->src(0),
                                  inst->extra<StackOffset>()->offset);
  if (info.value && !info.spansCall) {
    if (info.value->type().maybeCounted() || mightRelax(env, info.value)) {
      gen(env, TakeStack, info.value);
    }
    return gen(env, DecRef, info.value);
  }
  if (mightRelax(env, info.value)) {
    return nullptr;
  }

  // NB: strict subtype relation. Non-strict results in infinite recursion.
  if (info.knownType < inst->typeParam()) {
    return gen(
      env,
      DecRefStack,
      *inst->extra<StackOffset>(),
      info.knownType,
      inst->src(0)
    );
  }

  if (inst->typeParam().notCounted()) {
    return gen(env, Nop);
  }
  return nullptr;
}

SSATmp* simplifyAssertNonNull(State& env, const IRInstruction* inst) {
  if (inst->src(0)->type().not(Type::Nullptr)) {
    return inst->src(0);
  }
  return nullptr;
}

SSATmp* simplifyCheckPackedArrayBounds(State& env, const IRInstruction* inst) {
  auto const array = inst->src(0);
  auto const idx   = inst->src(1);
  if (!idx->isConst()) return nullptr;

  auto const idxVal = (uint64_t)idx->intVal();
  if (idxVal >= 0xffffffffull) {
    // ArrayData can't hold more than 2^32 - 1 elements, so this is
    // always going to fail.
    return gen(env, Jmp, inst->taken());
  }

  if (array->isConst()) {
    if (idxVal >= array->arrVal()->size()) {
      return gen(env, Jmp, inst->taken());
    }
    return gen(env, Nop);
  }

  if (packedArrayBoundsCheckUnnecessary(array->type(), idxVal)) {
    return gen(env, Nop);
  }

  return nullptr;
}

SSATmp* arrIntKeyImpl(State& env, const IRInstruction* inst) {
  auto const arr = inst->src(0);
  auto const idx = inst->src(1);
  if (!idx->isConst()) return nullptr;
  auto const value = arr->arrVal()->nvGet(idx->intVal());
  return value ? cns(env, *value) : nullptr;
}

SSATmp* arrStrKeyImpl(State& env, const IRInstruction* inst) {
  auto const arr = inst->src(0);
  auto const idx = inst->src(1);
  if (!idx->isConst()) return nullptr;
  auto const value = [&] {
    int64_t val;
    if (idx->strVal()->isStrictlyInteger(val)) {
      return arr->arrVal()->nvGet(val);
    }
    return arr->arrVal()->nvGet(idx->strVal());
  }();
  return value ? cns(env, *value) : nullptr;
}

SSATmp* simplifyLdPackedArrayElem(State& env, const IRInstruction* inst) {
  if (inst->src(0)->isConst()) return arrIntKeyImpl(env, inst);
  return nullptr;
}

SSATmp* simplifyArrayGet(State& env, const IRInstruction* inst) {
  if (inst->src(0)->isConst()) {
    if (inst->src(1)->type() <= Type::Int) return arrIntKeyImpl(env, inst);
    if (inst->src(1)->type() <= Type::Str) return arrStrKeyImpl(env, inst);
  }
  return nullptr;
}

SSATmp* simplifyCount(State& env, const IRInstruction* inst) {
  auto const val = inst->src(0);
  auto const ty = val->type();

  if (ty <= Type::Null) return cns(env, 0);

  auto const oneTy = Type::Bool | Type::Int | Type::Dbl | Type::Str | Type::Res;
  if (ty <= oneTy) return cns(env, 1);

  if (ty <= Type::Arr) return gen(env, CountArray, val);

  if (ty < Type::Obj) {
    auto const cls = ty.getClass();
    if (!mightRelax(env, val) && cls != nullptr && cls->isCollectionClass()) {
      return gen(env, CountCollection, val);
    }
    return nullptr;
  }
  return nullptr;
}

SSATmp* simplifyCountArray(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  auto const ty = src->type();

  if (src->isConst()) return cns(env, src->arrVal()->size());

  auto const notGlobals =
    ty.hasArrayKind() && ty.getArrayKind() != ArrayData::kGlobalsKind;

  if (!mightRelax(env, src) && notGlobals) {
    return gen(env, CountArrayFast, src);
  }

  return nullptr;
}

SSATmp* simplifyLdClsName(State& env, const IRInstruction* inst) {
  auto const src = inst->src(0);
  return src->isConst(Type::Cls) ? cns(env, src->clsVal()->name()) : nullptr;
}

SSATmp* simplifyCallBuiltin(State& env, const IRInstruction* inst) {
  auto const callee = inst->extra<CallBuiltin>()->callee;
  auto const args = inst->srcs();

  bool const arg0Collection = args.size() >= 1 &&
                              args[0]->type() < Type::Obj &&
                              args[0]->type().getClass() &&
                              args[0]->type().getClass()->isCollectionClass();

  switch (args.size()) {
  case 1:
    if (arg0Collection && callee->name()->isame(s_isEmpty.get())) {
      return gen(env, ColIsEmpty, args[0]);
    }
    break;
  default:
    break;
  }

  return nullptr;
}

SSATmp* simplifyIsWaitHandle(State& env, const IRInstruction* inst) {
  if (inst->src(0)->type() < Type::Obj) {
    auto const cls = inst->src(0)->type().getClass();
    if (cls && cls->classof(c_WaitHandle::classof())) {
      return cns(env, true);
    }
  }
  return nullptr;
}

//////////////////////////////////////////////////////////////////////

SSATmp* simplifyWork(State& env, const IRInstruction* inst) {
  env.insts.push(inst);
  SCOPE_EXIT {
    assert(env.insts.top() == inst);
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
  X(CastStk)
  X(Ceil)
  X(CheckInit)
  X(CheckPackedArrayBounds)
  X(CoerceCellToBool)
  X(CoerceCellToDbl)
  X(CoerceCellToInt)
  X(CoerceStk)
  X(ConcatCellCell)
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
  X(DecRef)
  X(DecRefNZ)
  X(DecRefStack)
  X(DivDbl)
  X(Floor)
  X(GetCtxFwdCall)
  X(IncRef)
  X(IncRefCtx)
  X(IsNType)
  X(IsScalarType)
  X(IsType)
  X(IsWaitHandle)
  X(LdClsCtx)
  X(LdClsName)
  X(LdCtx)
  X(CheckCtxThis)
  X(CastCtxThis)
  X(LdObjClass)
  X(LdObjInvoke)
  X(LdPackedArrayElem)
  X(LdStack)
  X(Mov)
  X(SpillStack)
  X(TakeStack)
  X(UnboxPtr)
  X(JmpGt)
  X(JmpGte)
  X(JmpLt)
  X(JmpLte)
  X(JmpEq)
  X(JmpNeq)
  X(JmpGtInt)
  X(JmpGteInt)
  X(JmpLtInt)
  X(JmpLteInt)
  X(JmpEqInt)
  X(JmpNeqInt)
  X(JmpSame)
  X(JmpNSame)
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
  X(ArrayGet)
  default: break;
  }
#undef X
  return nullptr;
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

SimplifyResult simplify(IRUnit& unit,
                        const IRInstruction* origInst,
                        bool typesMightRelax) {
  auto env = State { unit, typesMightRelax };
  auto const newDst = simplifyWork(env, origInst);
  assert(validate(env, newDst, origInst));
  return SimplifyResult { std::move(env.newInsts), newDst };
}

//////////////////////////////////////////////////////////////////////

StackValueInfo::StackValueInfo(SSATmp* value)
  : value(value)
  , knownType(value->type())
  , predictedInner(Type::Bottom)
  , spansCall(false)
  , typeSrc(value->inst())
{
  ITRACE(5, "{} created\n", show(*this));
}

StackValueInfo::StackValueInfo(IRInstruction* inst,
                               Type type,
                               Type predictedInner)
  : value(nullptr)
  , knownType(type)
  , predictedInner(predictedInner)
  , spansCall(false)
  , typeSrc(inst)
{
  ITRACE(5, "{} created\n", show(*this));
}

std::string show(const StackValueInfo& info) {
  std::string out = "StackValueInfo {";

  if (info.value) {
    out += info.value->inst()->toString();
  } else {
    folly::toAppend(
      info.knownType.toString(),
      info.knownType <= Type::BoxedInitCell
        ? folly::sformat(" ({})", info.predictedInner.toString())
        : std::string{},
      " from ",
      info.typeSrc ? info.typeSrc->toString() : "N/A",
      &out
    );
  }

  if (info.spansCall) out += ", spans call";
  out += "}";

  return out;
}

StackValueInfo getStackValue(SSATmp* sp, uint32_t index) {
  ITRACE(5, "getStackValue: idx = {}, {}\n", index, sp->inst()->toString());
  Trace::Indent _i;

  assert(sp->isA(Type::StkPtr));
  IRInstruction* inst = sp->inst();

  switch (inst->op()) {
  case DefSP:
    // You aren't really allowed to look above your current stack.  We
    // can't assert fail here if the index is too high right now
    // though, because it's currently legal to call getStackValue with
    // invalid stack offsets.  (And this is done in ir-builder; see
    // TODO(#4355796)).
    return StackValueInfo { inst, Type::StackElem };

  case ReDefSP: {
    auto const extra = inst->extra<ReDefSP>();
    auto info = getStackValue(inst->src(0), index);
    if (extra->spansCall) info.spansCall = true;
    return info;
  }

  case ExceptionBarrier:
  case Mov:
    return getStackValue(inst->src(0), index);

  case SideExitGuardStk:
    if (inst->extra<SideExitGuardData>()->checkedSlot == index) {
      return StackValueInfo { inst, inst->typeParam() };
    }
    return getStackValue(inst->src(0), index);

  case CastStk:
  case CastStkIntToDbl:
    // fallthrough
  case GuardStk:
    // We don't have a value, but we may know the type due to guarding
    // on it.
    if (inst->extra<StackOffset>()->offset == index) {
      return StackValueInfo { inst, inst->typeParam() };
    }
    return getStackValue(inst->src(0), index);

  case CoerceStk:
    if (inst->extra<CoerceStk>()->offset == index) {
      return StackValueInfo { inst, inst->typeParam() };
    }
    return getStackValue(inst->src(0), index);

  case AssertStk:
    // fallthrough
  case CheckStk:
    // CheckStk's and AssertStk's resulting type is the intersection
    // of its typeParam with whatever type preceded it.
    if (inst->extra<StackOffset>()->offset == index) {
      auto const prev = getStackValue(inst->src(0), index);
      return StackValueInfo {
        inst,
        refineTypeNoCheck(prev.knownType, inst->typeParam()),
        prev.predictedInner
      };
    }
    return getStackValue(inst->src(0), index);

  case HintStkInner:
    if (inst->extra<HintStkInner>()->offset == index) {
      return StackValueInfo {
        nullptr, Type::BoxedInitCell, inst->typeParam()
      };
    }
    return getStackValue(inst->src(0), index);

  case CallArray: {
    if (index == 0) {
      // return value from call
      return StackValueInfo { inst, Type::Gen };
    }
    auto info =
      getStackValue(inst->src(0),
                    // Pushes a return value, pops an ActRec and args Array
                    index -
                      (1 /* pushed */ - (kNumActRecCells + 1) /* popped */));
    info.spansCall = true;
    return info;
  }

  case Call: {
    if (index == 0) {
      // return value from call
      return StackValueInfo { inst, Type::Gen };
    }
    auto info =
      getStackValue(
        inst->src(0),
        index - (1 /* pushed */ -
                 (kNumActRecCells +
                   inst->extra<Call>()->numParams) /* popped */)
      );
    info.spansCall = true;
    return info;
  }

  case ContEnter: {
    if (index == 0) {
      // return value from call
      return StackValueInfo { inst, Type::Gen };
    }
    auto info = getStackValue(inst->src(0), index);
    info.spansCall = true;
    return info;
  }

  case SpillStack: {
    int64_t numPushed    = 0;
    int32_t numSpillSrcs = inst->numSrcs() - 2;

    for (int i = 0; i < numSpillSrcs; ++i) {
      SSATmp* tmp = inst->src(i + 2);
      if (index == numPushed) {
        return StackValueInfo { tmp };
      }
      ++numPushed;
    }

    // This is not one of the values pushed onto the stack by this
    // spillstack instruction, so continue searching.
    SSATmp* prevSp = inst->src(0);
    int64_t numPopped = inst->src(1)->intVal();
    return getStackValue(prevSp,
                         // pop values pushed by spillstack
                         index - (numPushed - numPopped));
  }

  case InterpOne:
  case InterpOneCF: {
    SSATmp* prevSp = inst->src(0);
    auto const& extra = *inst->extra<InterpOneData>();
    int64_t spAdjustment = extra.cellsPopped - extra.cellsPushed;
    switch (extra.opcode) {
    // some instructions are kinda funny and mess with the stack
    // in places other than the top
    case Op::CGetL2:
      if (index == 1) return StackValueInfo { inst, inst->typeParam() };
      if (index == 0) return getStackValue(prevSp, index);
      break;
    case Op::CGetL3:
      if (index == 2) return StackValueInfo { inst, inst->typeParam() };
      if (index < 2)  return getStackValue(prevSp, index);
      break;
    case Op::FPushCufSafe:
      if (index == kNumActRecCells) return StackValueInfo { inst, Type::Bool };
      if (index == kNumActRecCells + 1) return getStackValue(prevSp, 0);
      break;
    case Op::FPushCtor:
    case Op::FPushCtorD:
      if (index == kNumActRecCells) return StackValueInfo { inst, Type::Obj };
      if (index == kNumActRecCells + 1) return getStackValue(prevSp, 0);
      break;

    default:
      if (index == 0 && inst->hasTypeParam()) {
        return StackValueInfo { inst, inst->typeParam() };
      }
      break;
    }

    // If the index we're looking for is a cell pushed by the InterpOne (other
    // than top of stack), we know nothing about its type.
    if (index < extra.cellsPushed) {
      return StackValueInfo{ inst, Type::StackElem };
    }
    return getStackValue(prevSp, index + spAdjustment);
  }

  case SpillFrame:
  case CufIterSpillFrame:
    // pushes an ActRec
    if (index < kNumActRecCells) {
      return StackValueInfo { inst, Type::StackElem };
    }
    return getStackValue(inst->src(0), index - kNumActRecCells);

  case DefLabel:
    // We could keep tracing back through all the preds of inst's block but
    // there aren't currently any situations where that will add information we
    // wouldn't otherwise have.
    return StackValueInfo { inst, Type::StackElem };

  default:
    {
      // Assume it's a vector instruction.  This will assert in minstrBaseIdx
      // if not.
      auto const base = inst->src(minstrBaseIdx(inst->op()));
      // Currently we require that the stack address is the immediate source of
      // the base tmp.
      always_assert(base->inst()->is(LdStackAddr));
      if (base->inst()->extra<LdStackAddr>()->offset == index) {
        auto const prev = getStackValue(base->inst()->src(0), index);
        MInstrEffects effects(inst->op(), prev.knownType.ptr(Ptr::Stk));
        assert(effects.baseTypeChanged || effects.baseValChanged);
        auto const ty = effects.baseType;
        if (ty.isBoxed()) {
          return StackValueInfo { inst, Type::BoxedInitCell, ty };
        }
        return StackValueInfo { inst, ty.derefIfPtr() };
      }
      return getStackValue(base->inst()->src(0), index);
    }
  }

  not_reached();
}

Type getStackInnerTypePrediction(SSATmp* sp, uint32_t offset) {
  auto const info = getStackValue(sp, offset);
  assert(info.knownType <= Type::BoxedInitCell);
  if (info.predictedInner <= Type::Bottom) {
    ITRACE(2, "no boxed stack prediction {}\n", offset);
    return Type::BoxedInitCell;
  }
  auto t = ldRefReturn(info.predictedInner.unbox());
  ITRACE(2, "stack {} prediction: {}\n", offset, t);
  return t;
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
    assert(!inst->src(i)->inst()->is(Mov));
  }
}

bool packedArrayBoundsCheckUnnecessary(Type arrayType, int64_t idxVal) {
  auto const at = arrayType.getArrayType();
  if (!at) return false;
  using A = RepoAuthType::Array;
  switch (at->tag()) {
  case A::Tag::Packed:
    return idxVal < at->size() && at->emptiness() == A::Empty::No;
  case A::Tag::PackedN:
    return idxVal == 0 && at->emptiness() == A::Empty::No;
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////

}}
