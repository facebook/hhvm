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
#ifndef incl_HPHP_JIT_IRGEN_INTERNAL_H_
#define incl_HPHP_JIT_IRGEN_INTERNAL_H_

#include "hphp/runtime/vm/jit/irgen.h"

#include <vector>
#include <algorithm>
#include <utility>

#include "hphp/util/trace.h"

#include "hphp/runtime/vm/srckey.h"
#include "hphp/runtime/vm/jit/punt.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/simplify.h"

namespace HPHP { namespace jit { namespace irgen {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////
// Convenient short-hand state accessors

inline SSATmp* fp(const HTS& env) { return env.irb->fp(); }
inline SSATmp* sp(const HTS& env) { return env.irb->sp(); }

inline Offset bcOff(const HTS& env) {
  return env.bcStateStack.back().offset();
}

inline bool resumed(const HTS& env) {
  return env.bcStateStack.back().resumed();
}

inline const Func* curFunc(const HTS& env) {
  return env.bcStateStack.back().func();
}

inline const Unit* curUnit(const HTS& env) {
  return curFunc(env)->unit();
}

inline const Class* curClass(const HTS& env) {
  return curFunc(env)->cls();
}

inline SrcKey curSrcKey(const HTS& env) {
  return env.bcStateStack.back();
}

inline SrcKey nextSrcKey(const HTS& env) {
  auto srcKey = curSrcKey(env);
  srcKey.advance(curUnit(env));
  return srcKey;
}

inline Offset nextBcOff(const HTS& env) {
  return nextSrcKey(env).offset();
}

//////////////////////////////////////////////////////////////////////
// Control-flow helpers.

inline void hint(HTS& env, Block::Hint h) {
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

/*
 * cond() generates if-then-else blocks within a trace.  The caller supplies
 * lambdas to create the branch, next-body, and taken-body.  The next and
 * taken lambdas must return one SSATmp* value; cond() returns the SSATmp for
 * the merged value.
 *
 * If branch returns void, next must take zero arguments. If branch returns
 * SSATmp*, next must take one SSATmp* argument. This allows branch to return
 * an SSATmp* that is only defined in the next branch, without letting it
 * escape into the caller's scope (most commonly used with things like
 * LdMem).
 *
 * The producedRefs argument is needed for the refcount optimizations in
 * refcount-opts.cpp. It should be the number of unconsumed references
 * forwarded from each Jmp src to the DefLabel's dst (for a description of
 * reference producers and consumers, read the "Refcount Optimizations"
 * section in hphp/doc/hackers-guide/jit-optimizations.md). As an example,
 * code that looks like the following should pass 1 for producedRefs, since
 * LdCns and LookupCns each produce a reference that should then be forwarded
 * to t2, the dest of the DefLabel:
 *
 * B0:
 *   t0:FramePtr = DefFP
 *   t1:Cell = LdCns "foo"        // produce reference to t1
 *   CheckInit t1:Cell -> B3<Unlikely>
 *  -> B1
 *
 * B1 (preds B0):
 *   Jmp t1:Cell -> B2            // forward t1's unconsumed ref to t2
 *
 * B2 (preds B1, B3):
 *   t2:Cell = DefLabel           // produce reference to t2, from t1 and t4
 *   StLoc<1> t0:FramePtr t2:Cell // consume reference to t2
 *   Halt
 *
 * B3<Unlikely> (preds B0):
 *   t3:Uninit = AssertType<Uninit> t1:Cell // consume reference to t1
 *   t4:Cell = LookupCns "foo"    // produce reference to t4
 *   Jmp t4:Cell -> B2            // forward t4's unconsumed ref to t2
 *
 * A sufficiently advanced analysis pass could deduce this value from the
 * structure of the IR, but it would require traversing all possible control
 * flow paths, causing an explosion of required CPU time and/or memory.
 */
template<class Branch, class Next, class Taken>
SSATmp* cond(HTS& env,
             unsigned producedRefs,
             Branch branch,
             Next next,
             Taken taken) {
  auto const taken_block = env.unit.defBlock();
  auto const done_block = env.unit.defBlock();

  using T = decltype(branch(taken_block));
  auto const v1 = BranchImpl<T>::go(branch, taken_block, next);
  gen(env, Jmp, done_block, v1);
  env.irb->appendBlock(taken_block);
  auto const v2 = taken();
  gen(env, Jmp, done_block, v2);

  env.irb->appendBlock(done_block);
  auto const label = env.unit.defLabel(
    1,
    env.irb->curMarker(),
    {producedRefs}
  );
  done_block->push_back(label);
  auto const result = label->dst(0);
  result->setType(Type::unionOf(v1->type(), v2->type()));
  return result;
}

/*
 * ifThenElse() generates if-then-else blocks within a trace that do not
 * produce values. Code emitted in the {next,taken} lambda will be executed iff
 * the branch emitted in the branch lambda is {not,} taken.
 */
template<class Branch, class Next, class Taken>
void ifThenElse(HTS& env, Branch branch, Next next, Taken taken) {
  auto const taken_block = env.unit.defBlock();
  auto const done_block = env.unit.defBlock();
  branch(taken_block);
  next();
  // patch the last block added by the Next lambda to jump to
  // the done block.  Note that last might not be taken_block.
  auto const cur = env.irb->curBlock();
  if (cur->empty() || !cur->back().isBlockEnd()) {
    gen(env, Jmp, done_block);
  } else if (!cur->back().isTerminal()) {
    cur->back().setNext(done_block);
  }
  env.irb->appendBlock(taken_block);
  taken();
  // patch the last block added by the Taken lambda to jump to
  // the done block.  Note that last might not be taken_block.
  auto const last = env.irb->curBlock();
  if (last->empty() || !last->back().isBlockEnd()) {
    gen(env, Jmp, done_block);
  } else if (!last->back().isTerminal()) {
    last->back().setNext(done_block);
  }
  env.irb->appendBlock(done_block);
}

/*
 * ifThen generates if-then blocks within a trace that do not produce
 * values. Code emitted in the taken lambda will be executed iff the branch
 * emitted in the branch lambda is taken.
 */
template<class Branch, class Taken>
void ifThen(HTS& env, Branch branch, Taken taken) {
  auto const taken_block = env.unit.defBlock();
  auto const done_block = env.unit.defBlock();
  branch(taken_block);
  auto const cur = env.irb->curBlock();
  if (cur->empty() || !cur->back().isBlockEnd()) {
    gen(env, Jmp, done_block);
  } else if (!cur->back().isTerminal()) {
    cur->back().setNext(done_block);
  }
  env.irb->appendBlock(taken_block);
  taken();
  // patch the last block added by the Taken lambda to jump to
  // the done block.  Note that last might not be taken_block.
  auto const last = env.irb->curBlock();
  if (last->empty() || !last->back().isBlockEnd()) {
    gen(env, Jmp, done_block);
  } else if (!last->back().isTerminal()) {
    last->back().setNext(done_block);
  }
  env.irb->appendBlock(done_block);
}

/*
 * ifElse generates if-then-else blocks with an empty 'then' block
 * that do not produce values. Code emitted in the next lambda will
 * be executed iff the branch emitted in the branch lambda is not
 * taken.
 */
template<class Branch, class Next>
void ifElse(HTS& env, Branch branch, Next next) {
  auto const done_block = env.unit.defBlock();
  branch(done_block);
  next();
  // patch the last block added by the Next lambda to jump to
  // the done block.
  auto last = env.irb->curBlock();
  if (last->empty() || !last->back().isBlockEnd()) {
    gen(env, Jmp, done_block);
  } else if (!last->back().isTerminal()) {
    last->back().setNext(done_block);
  }
  env.irb->appendBlock(done_block);
}

//////////////////////////////////////////////////////////////////////

inline BCMarker makeMarker(HTS& env, Offset bcOff) {
  int32_t stackOff = env.irb->syncedSpLevel() +
    env.irb->evalStack().size() - env.irb->stackDeficit();

  FTRACE(2, "makeMarker: bc {} sp {} fn {}\n",
         bcOff, stackOff, curFunc(env)->fullName()->data());

  return BCMarker {
    SrcKey { curFunc(env), bcOff, resumed(env) },
    stackOff,
    env.profTransID
  };
}

inline void updateMarker(HTS& env) {
  env.irb->setCurMarker(makeMarker(env, bcOff(env)));
}

//////////////////////////////////////////////////////////////////////
// Eval stack manipulation

// Convert stack offsets that are relative to the current HHBC instruction
// (positive is higher on the stack) to offsets that are relative to the
// currently defined StkPtr.
inline int32_t offsetFromSP(const HTS& env, int32_t offsetFromInstr) {
  int32_t const virtDelta = env.irb->evalStack().size() -
    env.irb->stackDeficit();
  auto const curSPTop = env.irb->syncedSpLevel() + virtDelta;
  auto const absSPOff = curSPTop - offsetFromInstr;
  auto const ret = -static_cast<int32_t>(absSPOff - env.irb->spOffset());
  FTRACE(1,
    "offsetFromSP({}) --> spOff: {}, virtDelta: {}, curTop: {}, abs: {}, "
      "ret: {}\n",
    offsetFromInstr,
    env.irb->spOffset(),
    virtDelta,
    curSPTop,
    absSPOff,
    ret
  );
  return ret;
}

inline SSATmp* pop(HTS& env, Type type, TypeConstraint tc = DataTypeSpecific) {
  auto const opnd = env.irb->evalStack().pop();
  env.irb->constrainValue(opnd, tc);

  if (opnd == nullptr) {
    env.irb->constrainStack(offsetFromSP(env, 0), tc);
    auto value = gen(
      env,
      LdStk,
      type,
      StackOffset { offsetFromSP(env, 0) },
      sp(env)
    );
    env.irb->incStackDeficit();
    FTRACE(2, "popping {}\n", *value->inst());
    return value;
  }

  FTRACE(2, "popping {}\n", *opnd->inst());
  return opnd;
}

inline SSATmp* popC(HTS& env, TypeConstraint tc = DataTypeSpecific) {
  return pop(env, Type::Cell, tc);
}

inline SSATmp* popA(HTS& env) { return pop(env, Type::Cls); }
inline SSATmp* popV(HTS& env) { return pop(env, Type::BoxedInitCell); }
inline SSATmp* popR(HTS& env) { return pop(env, Type::Gen); }
inline SSATmp* popF(HTS& env) { return pop(env, Type::Gen); }

inline void discard(HTS& env, uint32_t n) {
  for (auto i = uint32_t{0}; i < n; ++i) {
    pop(env, Type::StkElem, DataTypeGeneric); // don't care about the values
  }
}

inline void popDecRef(HTS& env,
                      Type type,
                      TypeConstraint tc = DataTypeCountness) {
  if (auto const src = env.irb->evalStack().pop()) {
    env.irb->constrainValue(src, tc);
    gen(env, DecRef, src);
    return;
  }

  auto const offset = offsetFromSP(env, 0);
  env.irb->constrainStack(offset, tc);
  gen(
    env,
    DecRefStk,
    StackOffset { offset },
    type,
    sp(env)
  );
  env.irb->incStackDeficit();
}

inline SSATmp* push(HTS& env, SSATmp* tmp) {
  FTRACE(2, "pushing {}\n", *tmp->inst());
  env.irb->evalStack().push(tmp);
  return tmp;
}

inline SSATmp* pushIncRef(HTS& env,
                          SSATmp* tmp,
                          TypeConstraint tc = DataTypeCountness) {
  env.irb->constrainValue(tmp, tc);
  gen(env, IncRef, tmp);
  return push(env, tmp);
}

inline void extendStack(HTS& env, uint32_t index, Type type) {
  // DataTypeGeneric is used in here because nobody's actually looking at the
  // values, we're just inserting LdStks into the eval stack to be consumed
  // elsewhere.
  if (index == 0) {
    push(env, pop(env, type, DataTypeGeneric));
    return;
  }

  auto const tmp = pop(env, Type::StkElem, DataTypeGeneric);
  extendStack(env, index - 1, type);
  push(env, tmp);
}

inline SSATmp* top(HTS& env,
                   Type type,
                   uint32_t index = 0,
                   TypeConstraint tc = DataTypeSpecific) {
  auto tmp = env.irb->evalStack().top(index);
  if (!tmp) {
    extendStack(env, index, type);
    tmp = env.irb->evalStack().top(index);
  }
  assert(tmp);
  env.irb->constrainValue(tmp, tc);
  return tmp;
}

inline SSATmp* topC(HTS& env,
                    uint32_t i = 0,
                    TypeConstraint tc = DataTypeSpecific) {
  return top(env, Type::Cell, i, tc);
}

inline SSATmp* topF(HTS& env,
                    uint32_t i = 0,
                    TypeConstraint tc = DataTypeSpecific) {
  return top(env, Type::Gen, i, tc);
}

inline SSATmp* topV(HTS& env, uint32_t i = 0) {
  return top(env, Type::BoxedInitCell, i);
}

inline SSATmp* topR(HTS& env, uint32_t i = 0) {
  return top(env, Type::Gen, i);
}

inline Type topType(HTS& env,
                    uint32_t idx,
                    TypeConstraint constraint = DataTypeSpecific) {
  FTRACE(5, "Asking for type of stack elem {}\n", idx);
  if (idx < env.irb->evalStack().size()) {
    auto const tmp = env.irb->evalStack().top(idx);
    env.irb->constrainValue(tmp, constraint);
    return tmp->type();
  }
  return env.irb->stackType(offsetFromSP(env, idx), constraint);
}

//////////////////////////////////////////////////////////////////////
// Eval stack---SpillStack machinery

inline void spillStack(HTS& env) {
  auto const toSpill = env.irb->evalStack();
  for (auto idx = toSpill.size(); idx-- > 0;) {
    gen(env,
        StStk,
        StackOffset { offsetFromSP(env, idx) },
        sp(env),
        toSpill.top(idx));
  }
  env.irb->syncEvalStack();
}

//////////////////////////////////////////////////////////////////////
// Frame

inline SSATmp* ldThis(HTS& env) {
  auto const ctx = gen(env, LdCtx, fp(env));
  return gen(env, CastCtxThis, ctx);
}

inline SSATmp* ldCtx(HTS& env) {
  if (env.irb->thisAvailable()) return ldThis(env);
  return gen(env, LdCtx, fp(env));
}

inline SSATmp* unbox(HTS& env, SSATmp* val, Block* exit) {
  auto const type = val->type();
  // If we don't have an exit the LdRef can't be a guard.
  auto const inner = exit ? (type & Type::BoxedCell).innerType() : Type::Cell;

  if (type.isBoxed() || type.notBoxed()) {
    env.irb->constrainValue(val, DataTypeCountness);
    if (type.isBoxed()) {
      gen(env, CheckRefInner, inner, exit, val);
      return gen(env, LdRef, inner, val);
    }
    return val;
  }

  return cond(
    env,
    0,
    [&](Block* taken) {
      return gen(env, CheckType, Type::BoxedCell, taken, val);
    },
    [&](SSATmp* box) { // Next: val is a ref
      env.irb->constrainValue(box, DataTypeCountness);
      gen(env, CheckRefInner, inner, exit, box);
      return gen(env, LdRef, inner, box);
    },
    [&] { // Taken: val is unboxed
      return gen(env, AssertType, Type::Cell, val);
    }
  );
}

//////////////////////////////////////////////////////////////////////
// Other common helpers

inline bool classIsUnique(const Class* cls) {
  return RuntimeOption::RepoAuthoritative && cls && (cls->attrs() & AttrUnique);
}

inline bool classIsUniqueNormalClass(const Class* cls) {
  return classIsUnique(cls) && isNormalClass(cls);
}

inline bool classIsUniqueInterface(const Class* cls) {
  return classIsUnique(cls) && isInterface(cls);
}

inline bool classIsPersistentOrCtxParent(HTS& env, const Class* cls) {
  if (!cls) return false;
  if (classHasPersistentRDS(cls)) return true;
  if (!curClass(env)) return false;
  return curClass(env)->classof(cls);
}

inline bool classIsUniqueOrCtxParent(HTS& env, const Class* cls) {
  if (!cls) return false;
  if (classIsUnique(cls)) return true;
  if (!curClass(env)) return false;
  return curClass(env)->classof(cls);
}

inline SSATmp* ldCls(HTS& env, SSATmp* className) {
  assert(className->isA(Type::Str));
  if (className->isConst()) {
    if (auto const cls = Unit::lookupClass(className->strVal())) {
      if (classIsPersistentOrCtxParent(env, cls)) return cns(env, cls);
    }
    return gen(env, LdClsCached, className);
  }
  return gen(env, LdCls, className, cns(env, curClass(env)));
}

inline void decRefLocalsInline(HTS& env) {
  for (int id = curFunc(env)->numLocals() - 1; id >= 0; --id) {
    gen(env, DecRefLoc, Type::Gen, LocalId(id), fp(env));
  }
}

//////////////////////////////////////////////////////////////////////
// Local variables

inline SSATmp* ldLoc(HTS& env,
                     uint32_t locId,
                     Block* exit,
                     TypeConstraint tc) {
  assert(IMPLIES(exit == nullptr, !curFunc(env)->isPseudoMain()));

  auto const opStr = curFunc(env)->isPseudoMain()
    ? "LdLocPseudoMain"
    : "LdLoc";
  env.irb->constrainLocal(locId, tc, opStr);

  if (curFunc(env)->isPseudoMain()) {
    auto const type = env.irb->predictedLocalType(locId).relaxToGuardable();
    assert(!type.isSpecialized());
    assert(type == type.dropConstVal());

    // We don't support locals being type Gen, so if we ever get into such a
    // case, we need to punt.
    if (type == Type::Gen) PUNT(LdGbl-Gen);
    return gen(env, LdLocPseudoMain, type, exit, LocalId(locId), fp(env));
  }

  return gen(env, LdLoc, Type::Gen, LocalId(locId), fp(env));
}

/*
 * Load a local, and if it's boxed dereference to get the inner cell.
 *
 * Note: For boxed values, this will generate a LdRef instruction which takes
 *       the given exit trace in case the inner type doesn't match the tracked
 *       type for this local.  This check may be optimized away if we can
 *       determine that the inner type must match the tracked type.
 */
inline SSATmp* ldLocInner(HTS& env,
                          uint32_t locId,
                          Block* ldrefExit,
                          Block* ldPMExit,
                          TypeConstraint constraint) {
  // We only care if the local is KindOfRef or not. DataTypeCountness
  // gets us that.
  auto const loc = ldLoc(env, locId, ldPMExit, DataTypeCountness);
  assert((loc->type().isBoxed() || loc->type().notBoxed()) &&
         "Currently we don't handle traces where locals are maybeBoxed");

  if (!loc->type().isBoxed()) {
    env.irb->constrainValue(loc, constraint);
    return loc;
  }

  auto const predTy = env.irb->predictedInnerType(locId);
  gen(env, CheckRefInner, predTy, ldrefExit, loc);
  return gen(env, LdRef, predTy, loc);
}

/*
 * This is a wrapper to ldLocInner that also emits the RaiseUninitLoc if the
 * local is uninitialized. The catchBlock argument may be provided if the
 * caller requires the catch trace to be generated at a point earlier than when
 * it calls this function.
 */
inline SSATmp* ldLocInnerWarn(HTS& env,
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
    return cns(env, Type::InitNull);
  };

  env.irb->constrainLocal(id, DataTypeCountnessInit, "ldLocInnerWarn");
  if (locVal->type() <= Type::Uninit) {
    return warnUninit();
  }

  if (locVal->type().maybe(Type::Uninit)) {
    // The local might be Uninit so we have to check at runtime.
    return cond(
      env,
      0,
      [&](Block* taken) {
        gen(env, CheckInit, taken, locVal);
      },
      [&] { // Next: local is Init
        return locVal;
      },
      [&] { // Taken: local is Uninit
        return warnUninit();
      }
    );
  }

  return locVal;
}

/*
 * Generate a store to a local without doing anything else.  This function just
 * handles using StLocPseudoMain if we're in a pseudomain.
 */
inline SSATmp* stLocRaw(HTS& env, uint32_t id, SSATmp* fp, SSATmp* newVal) {
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
 * Pre: !newVal->type().isBoxed() && !newVal->type().maybeBoxed()
 * Pre: exit != nullptr if the local may be boxed
 */
inline SSATmp* stLocImpl(HTS& env,
                         uint32_t id,
                         Block* ldrefExit,
                         Block* ldPMExit,
                         SSATmp* newVal,
                         bool decRefOld,
                         bool incRefNew) {
  assert(!newVal->type().maybeBoxed());

  auto const cat = decRefOld ? DataTypeCountness : DataTypeGeneric;
  auto const oldLoc = ldLoc(env, id, ldPMExit, cat);
  assert(oldLoc->type().isBoxed() || oldLoc->type().notBoxed());

  if (oldLoc->type().notBoxed()) {
    stLocRaw(env, id, fp(env), newVal);
    if (incRefNew) gen(env, IncRef, newVal);
    if (decRefOld) gen(env, DecRef, oldLoc);
    return newVal;
  }

  // It's important that the IncRef happens after the guard on the inner type
  // of the ref, since it may side-exit.
  auto const predTy = env.irb->predictedInnerType(id);

  // We may not have a ldrefExit, but if so we better not be loading the inner
  // ref.
  if (ldrefExit == nullptr) always_assert(!decRefOld);
  if (ldrefExit != nullptr) {
    gen(env, CheckRefInner, predTy, ldrefExit, oldLoc);
  }
  auto const innerCell = decRefOld ? gen(env, LdRef, predTy, oldLoc) : nullptr;
  gen(env, StRef, oldLoc, newVal);
  if (incRefNew) gen(env, IncRef, newVal);
  if (decRefOld) {
    gen(env, DecRef, innerCell);
    env.irb->constrainValue(oldLoc, DataTypeCountness);
  }

  return newVal;
}

inline SSATmp* stLoc(HTS& env,
                     uint32_t id,
                     Block* ldrefExit,
                     Block* ldPMExit,
                     SSATmp* newVal) {
  constexpr bool decRefOld = true;
  constexpr bool incRefNew = false;
  return stLocImpl(env, id, ldrefExit, ldPMExit, newVal, decRefOld, incRefNew);
}

inline SSATmp* stLocNRC(HTS& env,
                        uint32_t id,
                        Block* ldrefExit,
                        Block* ldPMExit,
                        SSATmp* newVal) {
  constexpr bool decRefOld = false;
  constexpr bool incRefNew = false;
  return stLocImpl(env, id, ldrefExit, ldPMExit, newVal, decRefOld, incRefNew);
}

inline SSATmp* pushStLoc(HTS& env,
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

  env.irb->constrainValue(ret, DataTypeCountness);
  return push(env, ret);
}

inline SSATmp* ldLocAddr(HTS& env, uint32_t locId) {
  env.irb->constrainLocal(locId, DataTypeSpecific, "LdLocAddr");
  return gen(env, LdLocAddr, Type::PtrToFrameGen, LocalId(locId), fp(env));
}

inline SSATmp* ldStkAddr(HTS& env, int32_t relOffset) {
  // You're almost certainly doing it wrong if you want to get the address of a
  // stack cell that's in irb->evalStack().
  assert(relOffset >= static_cast<int32_t>(env.irb->evalStack().size()));
  auto const offset = offsetFromSP(env, relOffset);
  env.irb->constrainStack(offset, DataTypeSpecific);
  return gen(
    env,
    LdStkAddr,
    Type::PtrToStkGen,
    StackOffset { offset },
    sp(env)
  );
}

//////////////////////////////////////////////////////////////////////

}}}

#endif
