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
#ifndef incl_HPHP_JIT_IRGEN_INTERNAL_H_
#define incl_HPHP_JIT_IRGEN_INTERNAL_H_

#include "hphp/runtime/vm/jit/irgen.h"

#include <vector>
#include <algorithm>
#include <utility>

#include "hphp/util/trace.h"

#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/punt.h"
#include "hphp/runtime/vm/jit/simplify.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/stack-offsets.h"
#include "hphp/runtime/vm/jit/type-constraint.h"
#include "hphp/runtime/vm/jit/type.h"
#include "hphp/runtime/vm/srckey.h"

namespace HPHP { namespace jit { namespace irgen {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////
// Convenient short-hand state accessors

inline SSATmp* fp(const IRGS& env) { return env.irb->fs().fp(); }
inline SSATmp* sp(const IRGS& env) { return env.irb->fs().sp(); }

inline Offset bcOff(const IRGS& env) {
  return env.bcStateStack.back().offset();
}

inline bool hasThis(const IRGS& env) {
  return env.bcStateStack.back().hasThis();
}

inline bool resumed(const IRGS& env) {
  return env.bcStateStack.back().resumed();
}

inline const Func* curFunc(const IRGS& env) {
  return env.bcStateStack.back().func();
}

inline const Unit* curUnit(const IRGS& env) {
  return curFunc(env)->unit();
}

inline const Class* curClass(const IRGS& env) {
  return curFunc(env)->cls();
}

inline SrcKey curSrcKey(const IRGS& env) {
  return env.bcStateStack.back();
}

inline SrcKey nextSrcKey(const IRGS& env) {
  auto srcKey = curSrcKey(env);
  srcKey.advance(curUnit(env));
  return srcKey;
}

inline Offset nextBcOff(const IRGS& env) {
  return nextSrcKey(env).offset();
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
    auto const label = env.unit.defLabel(1, env.irb->nextBCContext());
    done_block->push_back(label);
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
  auto const label = env.unit.defLabel(2, env.irb->nextBCContext());
  done_block->push_back(label);
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
  env.irb->appendBlock(taken_block, branch_block);

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
  env.irb->appendBlock(done_block, branch_block);
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
// Eval stack manipulation

inline SSATmp* assertType(SSATmp* tmp, Type type) {
  assert(!tmp || tmp->isA(type));
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

inline SSATmp* pop(IRGS& env, TypeConstraint tc = DataTypeSpecific) {
  auto const offset = offsetFromIRSP(env, BCSPRelOffset{0});
  auto const knownType = env.irb->stack(offset, tc).type;
  auto value = gen(env, LdStk, knownType, IRSPRelOffsetData{offset}, sp(env));
  env.irb->fs().decBCSPDepth();
  FTRACE(2, "popping {}\n", *value->inst());
  return value;
}

inline SSATmp* popC(IRGS& env, TypeConstraint tc = DataTypeSpecific) {
  return assertType(pop(env, tc), TCell);
}

inline SSATmp* popV(IRGS& env) { return assertType(pop(env), TBoxedInitCell); }
inline SSATmp* popR(IRGS& env) { return assertType(pop(env), TGen); }
inline SSATmp* popF(IRGS& env) { return assertType(pop(env), TGen); }
inline SSATmp* popU(IRGS& env) { return assertType(pop(env), TUninit); }

inline void discard(IRGS& env, uint32_t n) {
  env.irb->fs().decBCSPDepth(n);
}

inline void decRef(IRGS& env, SSATmp* tmp, int locId=-1) {
  gen(env, DecRef, DecRefData(locId), tmp);
}

inline void popDecRef(IRGS& env,
                      TypeConstraint tc = DataTypeCountness) {
  auto const val = pop(env, tc);
  decRef(env, val);
}

inline SSATmp* push(IRGS& env, SSATmp* tmp) {
  FTRACE(2, "pushing {}\n", *tmp->inst());
  env.irb->fs().incBCSPDepth();
  auto const offset = offsetFromIRSP(env, BCSPRelOffset{0});
  gen(env, StStk, IRSPRelOffsetData{offset}, sp(env), tmp);
  return tmp;
}

inline SSATmp* pushIncRef(IRGS& env,
                          SSATmp* tmp,
                          TypeConstraint tc = DataTypeCountness) {
  env.irb->constrainValue(tmp, tc);
  gen(env, IncRef, tmp);
  return push(env, tmp);
}

inline Type topType(IRGS& env,
                    BCSPRelOffset idx = BCSPRelOffset{0},
                    TypeConstraint constraint = DataTypeSpecific) {
  FTRACE(5, "Asking for type of stack elem {}\n", idx.offset);
  return env.irb->stack(offsetFromIRSP(env, idx), constraint).type;
}

inline SSATmp* top(IRGS& env,
                   BCSPRelOffset index = BCSPRelOffset{0},
                   TypeConstraint tc = DataTypeSpecific) {
  auto const offset = offsetFromIRSP(env, index);
  auto const knownType = env.irb->stack(offset, tc).type;
  return gen(env, LdStk, IRSPRelOffsetData{offset}, knownType, sp(env));
}

inline SSATmp* topC(IRGS& env,
                    BCSPRelOffset i = BCSPRelOffset{0},
                    TypeConstraint tc = DataTypeSpecific) {
  return assertType(top(env, i, tc), TCell);
}

inline SSATmp* topF(IRGS& env,
                    BCSPRelOffset i = BCSPRelOffset{0},
                    TypeConstraint tc = DataTypeSpecific) {
  return assertType(top(env, i, tc), TGen);
}

inline SSATmp* topV(IRGS& env, BCSPRelOffset i = BCSPRelOffset{0}) {
  return assertType(top(env, i), TBoxedCell);
}

inline SSATmp* topR(IRGS& env, BCSPRelOffset i = BCSPRelOffset{0}) {
  return assertType(top(env, i), TGen);
}

//////////////////////////////////////////////////////////////////////

inline BCMarker makeMarker(IRGS& env, Offset bcOff) {
  auto const stackOff = spOffBCFromFP(env);

  FTRACE(2, "makeMarker: bc {} sp {} fn {}\n",
         bcOff, stackOff.offset, curFunc(env)->fullName()->data());

  return BCMarker {
    SrcKey(curSrcKey(env), bcOff),
    stackOff,
    env.profTransID,
    env.irb->fs().fp()
  };
}

inline void updateMarker(IRGS& env) {
  env.irb->setCurMarker(makeMarker(env, bcOff(env)));
}

//////////////////////////////////////////////////////////////////////
// Frame

inline SSATmp* castCtxThis(IRGS& env, SSATmp* val) {
  assertx(val->isA(TCtx));
  auto const func = curFunc(env);
  auto const thisType = func ? thisTypeFromFunc(func) : TObj;
  return gen(env, AssertType, thisType, val);
}

inline SSATmp* ldThis(IRGS& env) {
  auto const ctx = gen(env, LdCtx, fp(env));
  return castCtxThis(env, ctx);
}

inline SSATmp* ldCtx(IRGS& env) {
  if (!curClass(env))                return cns(env, nullptr);
  if (hasThis(env))                  return ldThis(env);
  return gen(env, LdCctx, fp(env));
}

inline SSATmp* unbox(IRGS& env, SSATmp* val, Block* exit) {
  auto const type = val->type();
  // If we don't have an exit the LdRef can't be a guard.
  auto const inner = exit ? (type & TBoxedCell).inner() : TInitCell;

  if (type <= TCell) {
    env.irb->constrainValue(val, DataTypeBoxAndCountness);
    return val;
  }
  if (type <= TBoxedCell) {
    gen(env, CheckRefInner, inner, exit, val);
    return gen(env, LdRef, inner, val);
  }

  return cond(
    env,
    [&](Block* taken) {
      return gen(env, CheckType, TBoxedCell, taken, val);
    },
    [&](SSATmp* box) { // Next: val is a ref
      env.irb->constrainValue(box, DataTypeBoxAndCountness);
      gen(env, CheckRefInner, inner, exit, box);
      return gen(env, LdRef, inner, box);
    },
    [&] { // Taken: val is unboxed
      return gen(env, AssertType, TCell, val);
    }
  );
}

//////////////////////////////////////////////////////////////////////
// Other common helpers

inline bool classIsUnique(const Class* cls) {
  return cls && (cls->attrs() & AttrUnique);
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

inline SSATmp* ldCls(IRGS& env, SSATmp* className) {
  assertx(className->isA(TStr));
  if (className->hasConstVal()) {
    if (auto const cls =
        Unit::lookupUniqueClassInContext(className->strVal(), curClass(env))) {
      if (!classIsPersistentOrCtxParent(env, cls)) {
        gen(env, LdClsCached, className);
      }
      return cns(env, cls);
    }
    return gen(env, LdClsCached, className);
  }
  return gen(env, LdCls, className, cns(env, curClass(env)));
}

//////////////////////////////////////////////////////////////////////
// Local variables

inline SSATmp* ldLoc(IRGS& env,
                     uint32_t locId,
                     Block* exit,
                     TypeConstraint tc) {
  assertx(IMPLIES(exit == nullptr, !curFunc(env)->isPseudoMain()));

  auto const opStr = curFunc(env)->isPseudoMain()
    ? "LdLocPseudoMain"
    : "LdLoc";
  env.irb->constrainLocal(locId, tc, opStr);

  if (curFunc(env)->isPseudoMain()) {
    auto const pred = env.irb->fs().local(locId).predictedType;
    auto const type = relaxToGuardable(pred);
    assertx(!type.isSpecialized());
    assertx(type == type.dropConstVal());

    // We don't support locals being type Gen, so if we ever get into such a
    // case, we need to punt.
    if (type == TGen) PUNT(LdGbl-Gen);
    return gen(env, LdLocPseudoMain, type, exit, LocalId(locId), fp(env));
  }

  return gen(env, LdLoc, TGen, LocalId(locId), fp(env));
}

/*
 * Load a local, and if it's boxed dereference to get the inner cell.
 *
 * Note: For boxed values, this will generate a LdRef instruction which takes
 *       the given exit trace in case the inner type doesn't match the tracked
 *       type for this local.  This check may be optimized away if we can
 *       determine that the inner type must match the tracked type.
 */
inline SSATmp* ldLocInner(IRGS& env,
                          uint32_t locId,
                          Block* ldrefExit,
                          Block* ldPMExit,
                          TypeConstraint constraint) {
  // We only care if the local is KindOfRef or not. DataTypeBoxAndCountness
  // gets us that.
  auto const loc = ldLoc(env, locId, ldPMExit, DataTypeBoxAndCountness);

  if (loc->type() <= TCell) {
    env.irb->constrainValue(loc, constraint);
    return loc;
  }

  // Handle the Boxed case manually outside of unbox() so we can use the
  // local's predicted type.
  if (loc->type() <= TBoxedCell) {
    auto const predTy = env.irb->predictedLocalInnerType(locId);
    gen(env, CheckRefInner, predTy, ldrefExit, loc);
    return gen(env, LdRef, predTy, loc);
  };

  return unbox(env, loc, ldrefExit);
}

/*
 * This is a wrapper to ldLocInner that also emits the RaiseUninitLoc if the
 * local is uninitialized. The catchBlock argument may be provided if the
 * caller requires the catch trace to be generated at a point earlier than when
 * it calls this function.
 */
inline SSATmp* ldLocInnerWarn(IRGS& env,
                              uint32_t id,
                              Block* ldrefExit,
                              Block* ldPMExit,
                              TypeConstraint constraint) {
  auto const locVal = ldLocInner(env, id, ldrefExit, ldPMExit, constraint);
  auto const varName = curFunc(env)->localVarName(id);

  auto warnUninit = [&] {
    if (varName != nullptr) {
      gen(env, RaiseUninitLoc, cns(env, varName));
    }
    return cns(env, TInitNull);
  };

  env.irb->constrainLocal(id, DataTypeBoxAndCountnessInit, "ldLocInnerWarn");

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
 * Generate a store to a local without doing anything else.  This function just
 * handles using StLocPseudoMain if we're in a pseudomain.
 */
inline SSATmp* stLocRaw(IRGS& env, uint32_t id, SSATmp* fp, SSATmp* newVal) {
  return gen(
    env,
    curFunc(env)->isPseudoMain() ? StLocPseudoMain : StLoc,
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
 *
 * Pre: !newVal->type().maybe(TBoxedCell)
 * Pre: exit != nullptr if the local may be boxed
 */
inline SSATmp* stLocImpl(IRGS& env,
                         uint32_t id,
                         Block* ldrefExit,
                         Block* ldPMExit,
                         SSATmp* newVal,
                         bool decRefOld,
                         bool incRefNew) {
  assertx(!newVal->type().maybe(TBoxedCell));

  auto const cat = decRefOld ? DataTypeBoxAndCountness : DataTypeGeneric;
  auto const oldLoc = ldLoc(env, id, ldPMExit, cat);

  auto unboxed_case = [&] {
    stLocRaw(env, id, fp(env), newVal);
    if (incRefNew) gen(env, IncRef, newVal);
    if (decRefOld) decRef(env, oldLoc);
    return newVal;
  };

  auto boxed_case = [&] (SSATmp* box) {
    // It's important that the IncRef happens after the guard on the inner type
    // of the ref, since it may side-exit.
    auto const predTy = env.irb->predictedLocalInnerType(id);

    // We may not have a ldrefExit, but if so we better not be loading the inner
    // ref.
    if (ldrefExit == nullptr) always_assert(!decRefOld);
    if (ldrefExit != nullptr) gen(env, CheckRefInner, predTy, ldrefExit, box);

    auto const innerCell = decRefOld ? gen(env, LdRef, predTy, box) : nullptr;
    gen(env, StRef, box, newVal);
    if (incRefNew) gen(env, IncRef, newVal);
    if (decRefOld) {
      decRef(env, innerCell);
      env.irb->constrainValue(box, DataTypeBoxAndCountness);
    }
    return newVal;
  };

  if (oldLoc->type() <= TCell) return unboxed_case();
  if (oldLoc->type() <= TBoxedCell) return boxed_case(oldLoc);

  return cond(
    env,
    [&] (Block* taken) {
      return gen(env, CheckType, TBoxedCell, taken, oldLoc);
    },
    boxed_case,
    unboxed_case
  );
}

inline SSATmp* stLoc(IRGS& env,
                     uint32_t id,
                     Block* ldrefExit,
                     Block* ldPMExit,
                     SSATmp* newVal) {
  constexpr bool decRefOld = true;
  constexpr bool incRefNew = true;
  return stLocImpl(env, id, ldrefExit, ldPMExit, newVal, decRefOld, incRefNew);
}

inline SSATmp* stLocNRC(IRGS& env,
                        uint32_t id,
                        Block* ldrefExit,
                        Block* ldPMExit,
                        SSATmp* newVal) {
  constexpr bool decRefOld = false;
  constexpr bool incRefNew = false;
  return stLocImpl(env, id, ldrefExit, ldPMExit, newVal, decRefOld, incRefNew);
}

inline void stLocMove(IRGS& env,
                      uint32_t id,
                      Block* ldrefExit,
                      Block* ldPMExit,
                      SSATmp* newVal) {
  assertx(!newVal->type().maybe(TBoxedCell));

  auto const oldLoc = ldLoc(env, id, ldPMExit, DataTypeBoxAndCountness);

  // If the local isn't a ref and we're not in a pseudo-main, we can just move
  // newValue into the local without manipulating its ref-count.
  auto unboxed_case = [&] {
    if (curFunc(env)->isPseudoMain()) gen(env, IncRef, newVal);
    stLocRaw(env, id, fp(env), newVal);
    decRef(env, oldLoc);
    if (curFunc(env)->isPseudoMain()) decRef(env, newVal);
    return nullptr;
  };

  // However, if the local is a ref, we'll manipulate the ref-counts as if this
  // was a SetL, PopC pair. Otherwise, overwriting the ref's inner value can
  // trigger a destructor, which can then overwrite the ref's inner value
  // again. If we don't increment newVal's ref-count, this second overwrite
  // might trigger newVal's destructor, where it wouldn't otherwise. So, to keep
  // destructor ordering consistent with SetL, PopC, we'll emulate its ref-count
  // behavior.
  auto boxed_case = [&] (SSATmp* box) {
    // It's important that the IncRef happens after the guard on the inner type
    // of the ref, since it may side-exit.
    auto const predTy = env.irb->predictedLocalInnerType(id);

    assert(ldrefExit);
    gen(env, CheckRefInner, predTy, ldrefExit, box);

    auto const innerCell = gen(env, LdRef, predTy, box);
    gen(env, StRef, box, newVal);
    gen(env, IncRef, newVal);
    decRef(env, innerCell);
    decRef(env, newVal);
    env.irb->constrainValue(box, DataTypeBoxAndCountness);
    return nullptr;
  };

  if (oldLoc->type() <= TCell) {
    unboxed_case();
    return;
  }
  if (oldLoc->type() <= TBoxedCell) {
    boxed_case(oldLoc);
    return;
  }

  cond(
    env,
    [&] (Block* taken) {
      return gen(env, CheckType, TBoxedCell, taken, oldLoc);
    },
    boxed_case,
    unboxed_case
  );
}

inline SSATmp* pushStLoc(IRGS& env,
                         uint32_t id,
                         Block* ldrefExit,
                         Block* ldPMExit,
                         SSATmp* newVal) {
  constexpr bool decRefOld = true;
  constexpr bool incRefNew = true;
  auto const ret = stLocImpl(
    env,
    id,
    ldrefExit,
    ldPMExit,
    newVal,
    decRefOld,
    incRefNew
  );

  env.irb->constrainValue(ret, DataTypeBoxAndCountness);
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
  assertx(!curFunc(env)->isPseudoMain());
  for (int id = curFunc(env)->numLocals() - 1; id >= 0; --id) {
    auto const loc = ldLoc(env, id, nullptr, DataTypeGeneric);
    decRef(env, loc, id);
  }
}

inline void decRefThis(IRGS& env) {
  if (!curFunc(env)->mayHaveThis()) return;
  auto const ctx = ldCtx(env);
  decRef(env, ctx);
}

//////////////////////////////////////////////////////////////////////
// Class-ref slots

inline void killClsRef(IRGS& env, uint32_t slot) {
  gen(env, KillClsRef, ClsRefSlotData{slot}, fp(env));
}

inline SSATmp* peekClsRef(IRGS& env, uint32_t slot) {
  auto const knownType = env.irb->clsRefSlot(slot).type;
  return gen(env, LdClsRef, knownType, ClsRefSlotData{slot}, fp(env));
}

inline SSATmp* takeClsRef(IRGS& env, uint32_t slot) {
  auto const cls = peekClsRef(env, slot);
  killClsRef(env, slot);
  return cls;
}

inline void putClsRef(IRGS& env, uint32_t slot, SSATmp* cls) {
  gen(env, StClsRef, ClsRefSlotData{slot}, fp(env), cls);
}

//////////////////////////////////////////////////////////////////////

}}}

#endif
