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
#pragma once

#include "hphp/runtime/vm/jit/irgen.h"

#include <vector>
#include <algorithm>
#include <utility>

#include "hphp/util/trace.h"

#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/guard-constraint.h"
#include "hphp/runtime/vm/jit/punt.h"
#include "hphp/runtime/vm/jit/simplify.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/resumable.h"
#include "hphp/runtime/vm/rx.h"
#include "hphp/runtime/vm/srckey.h"

namespace HPHP { namespace jit { namespace irgen {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////
// Convenient short-hand state accessors

inline SSATmp* fp(const IRGS& env) { return env.irb->fs().fp(); }
inline SSATmp* sp(const IRGS& env) { return env.irb->fs().sp(); }

inline Offset bcOff(const IRGS& env) {
  return env.bcState.offset();
}

inline bool hasThis(const IRGS& env) {
  return env.bcState.hasThis();
}

inline ResumeMode resumeMode(const IRGS& env) {
  return env.bcState.resumeMode();
}

inline const Func* curFunc(const IRGS& env) {
  return env.bcState.func();
}

inline const Unit* curUnit(const IRGS& env) {
  return curFunc(env)->unit();
}

inline const Class* curClass(const IRGS& env) {
  return curFunc(env)->cls();
}

inline SrcKey curSrcKey(const IRGS& env) {
  return env.bcState;
}

inline SrcKey nextSrcKey(const IRGS& env) {
  auto srcKey = curSrcKey(env);
  srcKey.advance(curFunc(env));
  return srcKey;
}

inline Offset nextBcOff(const IRGS& env) {
  return nextSrcKey(env).offset();
}

inline RxLevel curRxLevel(const IRGS& env) {
  // Pessimize enforcements for conditional reactivity, as it is not tracked yet
  if (curFunc(env)->isRxConditional()) return RxLevel::None;
  return curFunc(env)->rxLevel();
}

//////////////////////////////////////////////////////////////////////
// Control-flow helpers.

inline Block* defBlock(IRGS& env, Block::Hint hint = Block::Hint::Neither) {
  return env.unit.defBlock(curProfCount(env), hint);
}

inline void hint(IRGS& env, Block::Hint h) {
  env.irb->curBlock()->setHint(h);
}

/*
 * BranchImpl is used by cond to support branch and next lambdas with different
 * signatures. See cond for details.
 */

template<class T> struct BranchImpl;

template<> struct BranchImpl<void> {
  template<class Branch, class Next>
  static SSATmp* go(Branch branch, Block* taken, Next next) {
    branch(taken);
    return next();
  }
};

template<> struct BranchImpl<SSATmp*> {
  template<class Branch, class Next>
  static SSATmp* go(Branch branch, Block* taken, Next next) {
    return next(branch(taken));
  }
};

template<class T> struct BranchPairImpl;

template<> struct BranchPairImpl<void> {
  template<class Branch, class Next>
  static std::pair<SSATmp*, SSATmp*> go(Branch branch,
                                        Block* taken,
                                        Next next) {
    branch(taken);
    return next();
  }
};

template<> struct BranchPairImpl<SSATmp*> {
  template<class Branch, class Next>
  static std::pair<SSATmp*, SSATmp*> go(Branch branch,
                                        Block* taken,
                                        Next next) {
    return next(branch(taken));
  }
};

/*
 * cond() generates if-then-else blocks within a trace.  The caller supplies
 * lambdas to create the branch, next-body, and taken-body.  The next and
 * taken lambdas must return one SSATmp* value, or they must both return
 * nullptr; cond() returns the SSATmp for the merged value, or nullptr.
 *
 * If branch returns void, next must take zero arguments. If branch returns
 * SSATmp*, next must take one SSATmp* argument. This allows branch to return
 * an SSATmp* that is only defined in the next branch, without letting it
 * escape into the caller's scope (most commonly used with things like
 * LdMem).
 */
template<class Branch, class Next, class Taken>
SSATmp* cond(IRGS& env, Branch branch, Next next, Taken taken) {
  auto const taken_block = defBlock(env);
  auto const done_block  = defBlock(env);

  using T = decltype(branch(taken_block));
  SSATmp* v1 = BranchImpl<T>::go(branch, taken_block, next);
  if (v1) {
    gen(env, Jmp, done_block, v1);
  } else {
    gen(env, Jmp, done_block);
  }
  env.irb->appendBlock(taken_block);
  SSATmp* v2 = taken();
  assertx(!v1 == !v2);
  if (v2) {
    gen(env, Jmp, done_block, v2);
  } else {
    gen(env, Jmp, done_block);
  }

  env.irb->appendBlock(done_block);
  if (v1) {
    auto const bcctx = env.irb->nextBCContext();
    auto const label = env.unit.defLabel(1, done_block, bcctx);
    auto const result = label->dst(0);
    result->setType(v1->type() | v2->type());
    return result;
  }
  return nullptr;
}

template<class Branch, class Next, class Taken>
std::pair<SSATmp*, SSATmp*> condPair(IRGS& env,
                                     Branch branch,
                                     Next next,
                                     Taken taken) {
  auto const taken_block = defBlock(env);
  auto const done_block  = defBlock(env);

  using T = decltype(branch(taken_block));
  auto const v1 = BranchPairImpl<T>::go(branch, taken_block, next);
  assertx(v1.first);
  assertx(v1.second);
  gen(env, Jmp, done_block, v1.first, v1.second);

  env.irb->appendBlock(taken_block);
  auto const v2 = taken();
  assertx(v2.first);
  assertx(v2.second);
  gen(env, Jmp, done_block, v2.first, v2.second);

  env.irb->appendBlock(done_block);
  auto const bcctx = env.irb->nextBCContext();
  auto const label = env.unit.defLabel(2, done_block, bcctx);
  auto const result1 = label->dst(0);
  auto const result2 = label->dst(1);
  result1->setType(v1.first->type() | v2.first->type());
  result2->setType(v1.second->type() | v2.second->type());
  return std::make_pair(result1, result2);
}

/*
 * Generate an if-then-else block construct.
 *
 * Code emitted in the {next,taken} lambda will be executed iff the branch
 * emitted in the branch lambda is {not,} taken.
 *
 * TODO(#11019533): Fix undefined behavior if any of the blocks ends up
 * unreachable as a result of simplification (or funky irgen).
 */
template<class Branch, class Next, class Taken>
void ifThenElse(IRGS& env, Branch branch, Next next, Taken taken) {
  auto const next_block  = defBlock(env);
  auto const taken_block = defBlock(env);
  auto const done_block  = defBlock(env);

  branch(taken_block);
  auto const branch_block = env.irb->curBlock();

  if (branch_block->empty() || !branch_block->back().isBlockEnd()) {
    gen(env, Jmp, next_block);
  } else if (!branch_block->back().isTerminal()) {
    branch_block->back().setNext(next_block);
  }
  // The above logic ensures that `branch_block' always ends with an
  // isBlockEnd() instruction, so its out state is meaningful.
  env.irb->fs().setSaveOutState(branch_block);
  env.irb->appendBlock(next_block);
  next();

  // Patch the last block added by the Next lambda to jump to the done block.
  // Note that last might not be taken_block.
  auto const cur = env.irb->curBlock();
  if (cur->empty() || !cur->back().isBlockEnd()) {
    gen(env, Jmp, done_block);
  } else if (!cur->back().isTerminal()) {
    cur->back().setNext(done_block);
  }
  env.irb->appendBlock(taken_block);
  taken();

  // Patch the last block added by the Taken lambda to jump to the done block.
  // Note that last might not be taken_block.
  auto const last = env.irb->curBlock();
  if (last->empty() || !last->back().isBlockEnd()) {
    gen(env, Jmp, done_block);
  } else if (!last->back().isTerminal()) {
    last->back().setNext(done_block);
  }
  env.irb->appendBlock(done_block);
}

/*
 * Implementation for ifThen() and ifElse().
 *
 * TODO(#11019533): Fix undefined behavior if any of the blocks ends up
 * unreachable as a result of simplification (or funky irgen).
 */
template<bool on_true, class Branch, class Succ>
void ifBranch(IRGS& env, Branch branch, Succ succ) {
  auto const succ_block = defBlock(env);
  auto const done_block = defBlock(env);

  auto const cond_block    = on_true ? succ_block : done_block;
  auto const default_block = on_true ? done_block : succ_block;

  branch(cond_block);
  auto const branch_block = env.irb->curBlock();

  if (branch_block->empty() || !branch_block->back().isBlockEnd()) {
    gen(env, Jmp, default_block);
  } else if (!branch_block->back().isTerminal()) {
    branch_block->back().setNext(default_block);
  }
  // The above logic ensures that `branch_block' always ends with an
  // isBlockEnd() instruction, so its out state is meaningful.
  env.irb->fs().setSaveOutState(branch_block);
  env.irb->appendBlock(succ_block);
  succ();

  // Patch the last block added by `succ' to jump to the done block.  Note that
  // `last' might not be `succ_block'.
  auto const last = env.irb->curBlock();
  if (last->empty() || !last->back().isBlockEnd()) {
    gen(env, Jmp, done_block);
  } else if (!last->back().isTerminal()) {
    last->back().setNext(done_block);
  }
  env.irb->appendBlock(done_block);
}

/*
 * Generate an if-then block construct.
 *
 * Code emitted in the `taken' lambda will be executed iff the branch emitted
 * in the `branch' lambda is taken.
 */
template<class Branch, class Taken>
void ifThen(IRGS& env, Branch branch, Taken taken) {
  ifBranch<true>(env, branch, taken);
}

/*
 * Generate an if-then-else block construct with an empty `then' block.
 *
 * Code emitted in the `next' lambda will be executed iff the branch emitted in
 * the branch lambda is not taken.
 */
template<class Branch, class Next>
void ifElse(IRGS& env, Branch branch, Next next) {
  ifBranch<false>(env, branch, next);
}

/*
 * Code emitted in the `then' lambda will be executed iff `tmp' isn't a Nullptr.
 */
template<class Then>
void ifNonNull(IRGS& env, SSATmp* tmp, Then then) {
  cond(
    env,
    [&] (Block* taken) { return gen(env, CheckNonNull, taken, tmp); },
    [&] (SSATmp* s) { then(s); return nullptr; },
    [] { return nullptr; }
  );
}

//////////////////////////////////////////////////////////////////////
// Multi cond (Chonky cond)

/* This is a helper for generating multi-branch diamonds, or 'chonky conds'
 *
 * Usage is something like this:
 *
 *
 *   MultiCond mc{env};
 *   mc.ifThen(
 *     [&](Block* taken) { return gen(env, CheckType, TInt, tmp, taken); },
 *     [&](SSATmp* refined) {
 *       // this code is executed when we ***don't*** jump to `taken`
 *       ...
 *     });
 *   mc.ifTypeThen( // equivalent to passing a lambda that does CheckType on src
 *     src, TBool
 *     [&](SSATmp* refined) {
 *       ...
 *     });
 *   SSATmp* result = mc.elseDo([&]{ return cns(env, false); });
 *   // mc is logically dead here
 *
 *
 * This will produce the following control flow:
 *
 *
 * B0:
 *  x1 : Int := CheckType<Int>(x0 : Cell) -> B1
 *  [ here refined === x1 ]
 *  phijmp <something> -> B3
 *
 * B1:
 *  x2 : Bool := CheckType<Bool>(x0 : Cell) -> B2
 *  ...
 *  phijmp <something> -> B3
 *
 * B2:
 *   phijmp (false : Bool) -> B3
 *
 * B3:
 *   x3 := DefLabel<Bool>
 *
 *
 *
 * While it won't _necessarily_ break anything to use the IRGS outside of
 * `if(Type)Then` and `elseDo` while there's a live MultiCond, you are cautioned
 * to do so at your own peril
 */

struct MultiCond {
  explicit MultiCond(IRGS& env)
    : env{env}
    , done{defBlock(env)} {}

  ~MultiCond() {
    assertx(finished);
  }

  template <typename Branch, typename Then>
  void ifThen(Branch&& branch, Then&& then) {
    assertx(!finished);
    auto const taken = defBlock(env);
    auto const refined = branch(taken);
    auto const result = then(refined);
    resultTy |= result->type();
    gen(env, Jmp, done, result);
    env.irb->appendBlock(taken);
  }

  template <typename Then>
  void ifTypeThen(SSATmp* src, Type type, Then&& then) {
    ifThen([&](Block* taken) { return gen(env, CheckType, type, taken, src); },
           std::forward<Then>(then));
  }

  template <typename Else>
  SSATmp* elseDo(Else&& els) {
    assertx(!finished);
    auto const result = els();
    assertx(result);
    resultTy |= result->type();
    gen(env, Jmp, done, result);
    finished = true;

    env.irb->appendBlock(done);
    auto const label = env.unit.defLabel(1, done, env.irb->nextBCContext());
    auto const finalResult = label->dst(0);
    finalResult->setType(resultTy);
    return finalResult;
  }

private:
  IRGS& env;
  Block* done;
  Type resultTy{TBottom};
  bool finished{false};
};


//////////////////////////////////////////////////////////////////////
// Eval stack manipulation

inline SSATmp* assertType(SSATmp* tmp, Type type) {
  assertx(!tmp || tmp->isA(type));
  return tmp;
}

inline FPInvOffset offsetFromFP(const IRGS& env, IRSPRelOffset irSPRel) {
  auto const irSPOff = env.irb->fs().irSPOff();
  return irSPRel.to<FPInvOffset>(irSPOff);
}

inline IRSPRelOffset offsetFromIRSP(const IRGS& env, FPInvOffset fpRel) {
  auto const irSPOff = env.irb->fs().irSPOff();
  return fpRel.to<IRSPRelOffset>(irSPOff);
}

inline IRSPRelOffset offsetFromIRSP(const IRGS& env, BCSPRelOffset bcSPRel) {
  auto const fpRel = bcSPRel.to<FPInvOffset>(env.irb->fs().bcSPOff());
  return offsetFromIRSP(env, fpRel);
}

inline BCSPRelOffset offsetFromBCSP(const IRGS& env, FPInvOffset fpRel) {
  auto const bcSPOff = env.irb->fs().bcSPOff();
  return fpRel.to<BCSPRelOffset>(bcSPOff);
}

inline BCSPRelOffset offsetFromBCSP(const IRGS& env, IRSPRelOffset irSPRel) {
  auto const fpRel = irSPRel.to<FPInvOffset>(env.irb->fs().irSPOff());
  return offsetFromBCSP(env, fpRel);
}

/*
 * Offset of the bytecode stack pointer.
 */
inline FPInvOffset spOffBCFromFP(const IRGS& env) {
  return env.irb->fs().bcSPOff();
}
inline IRSPRelOffset spOffBCFromIRSP(const IRGS& env) {
  return offsetFromIRSP(env, BCSPRelOffset { 0 });
}

/*
 * Offset of empty evaluation stack.
 */
inline FPInvOffset spOffEmpty(const IRGS& env) {
  return FPInvOffset{
    resumeMode(env) == ResumeMode::None ? curFunc(env)->numSlotsInFrame() : 0
  };
}

inline SSATmp* pop(IRGS& env, GuardConstraint gc = DataTypeSpecific) {
  auto const offset = offsetFromIRSP(env, BCSPRelOffset{0});
  auto const knownType = env.irb->stack(offset, gc).type;
  auto value = gen(env, LdStk, knownType, IRSPRelOffsetData{offset}, sp(env));
  env.irb->fs().decBCSPDepth();
  FTRACE(2, "popping {}\n", *value->inst());
  return value;
}

inline SSATmp* popC(IRGS& env, GuardConstraint gc = DataTypeSpecific) {
  return assertType(pop(env, gc), TCell);
}

inline SSATmp* popCU(IRGS& env) { return assertType(pop(env), TCell); }
inline SSATmp* popU(IRGS& env) { return assertType(pop(env), TUninit); }

inline void discard(IRGS& env, uint32_t n = 1) {
  env.irb->fs().decBCSPDepth(n);
}

inline void decRef(IRGS& env, SSATmp* tmp, int locId=-1) {
  gen(env, DecRef, DecRefData(locId), tmp);
}

inline void popDecRef(IRGS& env, GuardConstraint gc = DataTypeCountness) {
  auto const val = pop(env, gc);
  decRef(env, val);
}

inline SSATmp* push(IRGS& env, SSATmp* tmp) {
  FTRACE(2, "pushing {}\n", *tmp->inst());
  env.irb->fs().incBCSPDepth();
  auto const offset = offsetFromIRSP(env, BCSPRelOffset{0});
  gen(env, StStk, IRSPRelOffsetData{offset}, sp(env), tmp);
  return tmp;
}

inline SSATmp* pushIncRef(IRGS& env, SSATmp* tmp,
                          GuardConstraint gc = DataTypeCountness) {
  env.irb->constrainValue(tmp, gc);
  gen(env, IncRef, tmp);
  return push(env, tmp);
}

inline void allocActRec(IRGS& env) {
  env.irb->fs().incBCSPDepth(kNumActRecCells);
}

inline Type topType(IRGS& env, BCSPRelOffset idx = BCSPRelOffset{0},
                    GuardConstraint gc = DataTypeSpecific) {
  FTRACE(5, "Asking for type of stack elem {}\n", idx.offset);
  return env.irb->stack(offsetFromIRSP(env, idx), gc).type;
}

inline SSATmp* top(IRGS& env, BCSPRelOffset index = BCSPRelOffset{0},
                   GuardConstraint gc = DataTypeSpecific) {
  auto const offset = offsetFromIRSP(env, index);
  auto const knownType = env.irb->stack(offset, gc).type;
  return gen(env, LdStk, IRSPRelOffsetData{offset}, knownType, sp(env));
}

inline SSATmp* topC(IRGS& env, BCSPRelOffset i = BCSPRelOffset{0},
                    GuardConstraint gc = DataTypeSpecific) {
  return assertType(top(env, i, gc), TCell);
}

//////////////////////////////////////////////////////////////////////

inline BCMarker makeMarker(IRGS& env, Offset bcOff) {
  auto const stackOff = spOffBCFromFP(env);

  FTRACE(2, "makeMarker: bc {} sp {} fn {}\n",
         bcOff, stackOff.offset, curFunc(env)->fullName()->data());

  return BCMarker {
    SrcKey(curSrcKey(env), bcOff),
    stackOff,
    env.profTransIDs,
    env.irb->fs().fp()
  };
}

inline void updateMarker(IRGS& env) {
  env.irb->setCurMarker(makeMarker(env, bcOff(env)));
}

//////////////////////////////////////////////////////////////////////
// Frame

inline SSATmp* ldThis(IRGS& env) {
  assertx(hasThis(env));
  auto const func = curFunc(env);
  auto const thisType = func ? thisTypeFromFunc(func) : TObj;
  return gen(env, LdFrameThis, thisType, fp(env));
}

inline SSATmp* ldCtx(IRGS& env) {
  if (!curClass(env))                return cns(env, nullptr);
  if (hasThis(env))                  return ldThis(env);

  auto const func = curFunc(env);
  auto const clsType = func ? Type::SubCls(func->cls()) : TCls;
  return gen(env, LdFrameCls, clsType, fp(env));
}

inline SSATmp* ldCtxCls(IRGS& env) {
  auto const ctx = ldCtx(env);
  if (ctx->isA(TCls) || ctx->isA(TNullptr)) return ctx;
  assertx(ctx->isA(TObj));
  return gen(env, LdObjClass, ctx);
}

//////////////////////////////////////////////////////////////////////
// Other common helpers

inline bool classIsUnique(const Class* cls) {
  return cls && cls->isUnique();
}

inline bool classIsUniqueNormalClass(const Class* cls) {
  return classIsUnique(cls) && isNormalClass(cls);
}

inline bool classIsUniqueInterface(const Class* cls) {
  return classIsUnique(cls) && isInterface(cls);
}

inline bool classIsPersistentOrCtxParent(IRGS& env, const Class* cls) {
  if (!cls) return false;
  if (classHasPersistentRDS(cls)) return true;
  if (!curClass(env)) return false;
  return curClass(env)->classof(cls);
}

inline bool classIsUniqueOrCtxParent(IRGS& env, const Class* cls) {
  if (!cls) return false;
  if (classIsUnique(cls)) return true;
  if (!curClass(env)) return false;
  return curClass(env)->classof(cls);
}

inline const Class* lookupUniqueClass(IRGS& env,
                                      const StringData* name,
                                      bool trustUnit = false) {
  // TODO: Once top level code is entirely dead it should be safe to always
  // trust the unit.
  return Class::lookupUniqueInContext(
    name, curClass(env), trustUnit ? curUnit(env) : nullptr);
}

inline SSATmp* ldCls(IRGS& env,
                     SSATmp* lazyClassOrName,
                     Block* ctrace = nullptr) {
  auto const isLazy = lazyClassOrName->isA(TLazyCls);
  assertx(lazyClassOrName->isA(TStr) || isLazy);
  if (lazyClassOrName->hasConstVal()) {
    auto const cnameStr = isLazy ? lazyClassOrName->lclsVal().name() :
                                   lazyClassOrName->strVal();
    if (auto const cls = lookupUniqueClass(env, cnameStr)) {
      if (!classIsPersistentOrCtxParent(env, cls)) {
        auto const clsName = isLazy ? cns(env, cnameStr) : lazyClassOrName;
        gen(env, LdClsCached, ctrace, clsName);
      }
      return cns(env, cls);
    }
    auto const clsName = isLazy ? cns(env, cnameStr) : lazyClassOrName;
    return gen(env, LdClsCached, ctrace, clsName);
  }
  auto const ctxClass = curClass(env);
  auto const ctxTmp = ctxClass ? cns(env, ctxClass) : cns(env, nullptr);
  auto const clsName =
    isLazy ? gen(env, LdLazyClsName, lazyClassOrName) : lazyClassOrName;
  return gen(env, LdCls, ctrace, clsName, ctxTmp);
}

//////////////////////////////////////////////////////////////////////
// Local variables

inline SSATmp* ldLoc(IRGS& env,
                     uint32_t locId,
                     Block* exit,
                     GuardConstraint gc) {
  env.irb->constrainLocal(locId, gc, "LdLoc");
  return gen(env, LdLoc, TCell, LocalId(locId), fp(env));
}

/*
 * This is a wrapper to ldLocInner that also emits the RaiseUninitLoc if the
 * local is uninitialized. The catchBlock argument may be provided if the
 * caller requires the catch trace to be generated at a point earlier than when
 * it calls this function.
 */
inline SSATmp* ldLocWarn(IRGS& env,
                         NamedLocal loc,
                         Block* ldPMExit,
                         GuardConstraint gc) {
  auto const locVal = ldLoc(env, loc.id, ldPMExit, gc);

  auto warnUninit = [&] {
    if (loc.name == kInvalidLocalName) {
      // HHBBC incorrectly removed local name information for a local.
      gen(env, Unreachable, ASSERT_REASON);
      return cns(env, TBottom);
    }
    auto const varName = curFunc(env)->localVarName(loc.name);
    if (varName != nullptr) {
      gen(env, RaiseUninitLoc, cns(env, varName));
    }
    return cns(env, TInitNull);
  };

  env.irb->constrainLocal(loc.id, DataTypeCountnessInit, "ldLocWarn");

  if (locVal->type() <= TUninit) return warnUninit();
  if (!locVal->type().maybe(TUninit)) return locVal;

  // The local might be Uninit so we have to check at runtime.
  return cond(
    env,
    [&] (Block* taken) {
      gen(env, CheckInit, taken, locVal);
    },
    [&] { // Next: local is InitCell.
      return gen(env, AssertType, TInitCell, locVal);
    },
    [&] { // Taken: local is Uninit
      return warnUninit();
    }
  );
}

/*
 * Generate a store to a local without doing anything else.
 */
inline SSATmp* stLocRaw(IRGS& env, uint32_t id, SSATmp* fp, SSATmp* newVal) {
  return gen(
    env,
    StLoc,
    LocalId(id),
    fp,
    newVal
  );
}

/*
 * Store to a local, if it's boxed set the value on the inner cell.
 *
 * Returns the value that was stored to the local. Assumes that 'newVal' has
 * already been incremented, with this Store consuming the ref-count
 * increment. If the caller of this function needs to push the stored value on
 * stack, it should set 'incRefNew' so that 'newVal' will have its ref-count
 * incremented.
 */
inline SSATmp* stLocImpl(IRGS& env,
                         uint32_t id,
                         Block* ldPMExit,
                         SSATmp* newVal,
                         bool decRefOld,
                         bool incRefNew) {
  auto const cat = decRefOld ? DataTypeCountness : DataTypeGeneric;
  auto const oldLoc = ldLoc(env, id, ldPMExit, cat);

  stLocRaw(env, id, fp(env), newVal);
  if (incRefNew) gen(env, IncRef, newVal);
  if (decRefOld) decRef(env, oldLoc);
  return newVal;
}

inline SSATmp* stLoc(IRGS& env,
                     uint32_t id,
                     Block* ldPMExit,
                     SSATmp* newVal) {
  constexpr bool decRefOld = true;
  constexpr bool incRefNew = true;
  return stLocImpl(env, id, ldPMExit, newVal, decRefOld, incRefNew);
}

inline SSATmp* stLocNRC(IRGS& env,
                        uint32_t id,
                        Block* ldPMExit,
                        SSATmp* newVal) {
  constexpr bool decRefOld = false;
  constexpr bool incRefNew = false;
  return stLocImpl(env, id, ldPMExit, newVal, decRefOld, incRefNew);
}

inline void stLocMove(IRGS& env,
                      uint32_t id,
                      Block* ldPMExit,
                      SSATmp* newVal) {
  auto const oldLoc = ldLoc(env, id, ldPMExit, DataTypeCountness);

  stLocRaw(env, id, fp(env), newVal);
  decRef(env, oldLoc);
}

inline SSATmp* pushStLoc(IRGS& env,
                         uint32_t id,
                         Block* ldPMExit,
                         SSATmp* newVal) {
  constexpr bool decRefOld = true;
  constexpr bool incRefNew = true;
  auto const ret = stLocImpl(
    env,
    id,
    ldPMExit,
    newVal,
    decRefOld,
    incRefNew
  );

  env.irb->constrainValue(ret, DataTypeCountness);
  return push(env, ret);
}

inline SSATmp* ldLocAddr(IRGS& env, uint32_t locId) {
  env.irb->constrainLocal(locId, DataTypeSpecific, "LdLocAddr");
  return gen(env, LdLocAddr, LocalId(locId), fp(env));
}

inline SSATmp* ldStkAddr(IRGS& env, BCSPRelOffset relOffset) {
  auto const offset = offsetFromIRSP(env, relOffset);
  env.irb->constrainStack(offset, DataTypeSpecific);
  return gen(
    env,
    LdStkAddr,
    IRSPRelOffsetData { offset },
    sp(env)
  );
}

inline void decRefLocalsInline(IRGS& env) {
  for (int id = curFunc(env)->numLocals() - 1; id >= 0; --id) {
    auto const loc = ldLoc(env, id, nullptr, DataTypeGeneric);
    decRef(env, loc, id);
  }
}

inline void decRefThis(IRGS& env) {
  if (!curFunc(env)->hasThisInBody()) return;
  auto const ctx = ldCtx(env);
  decRef(env, ctx);
}

//////////////////////////////////////////////////////////////////////

/*
 * Creates a catch block and calls body immediately as the catch block begins
 */
template<class Body>
Block* create_catch_block(
    IRGS& env, Body body,
    EndCatchData::CatchMode mode = EndCatchData::CatchMode::UnwindOnly,
    int32_t offsetToAdjustSPForCall = 0) {
  auto const catchBlock = defBlock(env, Block::Hint::Unused);
  BlockPusher bp(*env.irb, env.irb->curMarker(), catchBlock);

  auto const& exnState = env.irb->exceptionStackState();
  env.irb->fs().setBCSPOff(exnState.syncedSpLevel);

  gen(env, BeginCatch);
  body();
  auto const stublogue = env.irb->fs().stublogue();
  auto const spOffset = mode != EndCatchData::CatchMode::CallCatch
    ? spOffBCFromIRSP(env)
    : spOffBCFromIRSP(env) + offsetToAdjustSPForCall;
  auto const data = EndCatchData {
    spOffset,
    mode,
    stublogue ?
      EndCatchData::FrameMode::Stublogue : EndCatchData::FrameMode::Phplogue,
    stublogue ? EndCatchData::Teardown::NA : EndCatchData::Teardown::Full
  };
  auto const marker = env.irb->curMarker();
  auto const newMarker = [&] {
    if (offsetToAdjustSPForCall == 0) return marker;
    auto const newSP = marker.spOff() - offsetToAdjustSPForCall;
    return marker.adjustSP(newSP);
  }();
  env.irb->setCurMarker(newMarker);
  gen(env, EndCatch, data, fp(env), sp(env));
  env.irb->setCurMarker(marker);
  return catchBlock;
}

//////////////////////////////////////////////////////////////////////

// If the current function doesn't have a $this, emit a fatal. Otherwise, load
// $this and return it.
SSATmp* checkAndLoadThis(IRGS& env);

//////////////////////////////////////////////////////////////////////
/*
 * clsmeth helpers.
 */
SSATmp* convertClsMethToVec(IRGS& env, SSATmp* clsMeth);

//////////////////////////////////////////////////////////////////////
/*
 * class type helpers.
 */
SSATmp* convertClassKey(IRGS& env, SSATmp* key);
}}}

