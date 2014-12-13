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

inline BCMarker makeMarker(HTS& env, Offset bcOff) {
  int32_t stackOff = env.irb->spOffset() +
    env.irb->evalStack().numCells() - env.irb->stackDeficit();

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
// IR instruction creation functions

template<class... Args>
SSATmp* cns(HTS& env, Args&&... args) {
  return env.unit.cns(std::forward<Args>(args)...);
}

template<class... Args>
SSATmp* gen(HTS& env, Args&&... args) {
  return env.irb->gen(std::forward<Args>(args)...);
}

//////////////////////////////////////////////////////////////////////
// Eval stack manipulation

inline SSATmp* pop(HTS& env, Type type, TypeConstraint tc = DataTypeSpecific) {
  auto const opnd = env.irb->evalStack().pop();
  env.irb->constrainValue(opnd, tc);

  if (opnd == nullptr) {
    auto const stackOff = env.irb->stackDeficit();
    env.irb->incStackDeficit();
    env.irb->constrainStack(stackOff, tc);

    // pop() is usually called with Cell or Gen. Don't rely on the simplifier
    // to get a better type for the LdStack.
    auto const info = getStackValue(sp(env), stackOff);
    type = std::min(type, info.knownType);

    auto value = gen(env, LdStack, type, StackOffset(stackOff), sp(env));
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
inline SSATmp* popV(HTS& env) { return pop(env, Type::BoxedCell); }
inline SSATmp* popR(HTS& env) { return pop(env, Type::Gen); }
inline SSATmp* popF(HTS& env) { return pop(env, Type::Gen); }

inline void discard(HTS& env, uint32_t n) {
  for (auto i = uint32_t{0}; i < n; ++i) {
    pop(env, Type::StackElem, DataTypeGeneric); // don't care about the values
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

  env.irb->constrainStack(env.irb->stackDeficit(), tc);
  gen(env, DecRefStack, StackOffset(env.irb->stackDeficit()), type, sp(env));
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

inline SSATmp* top(HTS& env,
                   uint32_t offset = 0,
                   TypeConstraint tc = DataTypeSpecific) {
  auto const tmp = env.irb->evalStack().top(offset);
  if (!tmp) return nullptr;
  env.irb->constrainValue(tmp, tc);
  return tmp;
}

/*
 * We don't know what type description to expect for the stack locations before
 * index, so we use a generic type when popping the intermediate values.  If it
 * ends up creating a new LdStack, refineType during a later pop() or top()
 * will fix up the type to the known type.
 *
 * TODO(#4810319): the above comment is definitely not true anymore.
 */
inline void extendStack(HTS& env, uint32_t index, Type type) {
  // DataTypeGeneric is used in here because nobody's actually looking at the
  // values, we're just inserting LdStacks into the eval stack to be consumed
  // elsewhere.
  if (index == 0) {
    push(env, pop(env, type, DataTypeGeneric));
    return;
  }

  auto const tmp = pop(env, Type::StackElem, DataTypeGeneric);
  extendStack(env, index - 1, type);
  push(env, tmp);
}

inline SSATmp* top(HTS& env,
                   Type type,
                   uint32_t index = 0,
                   TypeConstraint tc = DataTypeSpecific) {
  auto tmp = top(env, index, tc);
  if (!tmp) {
    extendStack(env, index, type);
    tmp = top(env, index, tc);
    assert(tmp);
  }
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
  return top(env, Type::BoxedCell, i);
}

inline SSATmp* topR(HTS& env, uint32_t i = 0) {
  return top(env, Type::Gen, i);
}

inline Type topType(HTS& env,
                    uint32_t idx,
                    TypeConstraint constraint = DataTypeSpecific) {
  FTRACE(5, "Asking for type of stack elem {}\n", idx);
  if (idx < env.irb->evalStack().size()) {
    return top(env, idx, constraint)->type();
  }
  auto const absIdx = idx - env.irb->evalStack().size() + env.irb->stackDeficit();
  auto const stkVal = getStackValue(sp(env), absIdx);
  env.irb->constrainStack(absIdx, constraint);
  return stkVal.knownType;
}

//////////////////////////////////////////////////////////////////////
// Eval stack---SpillStack machinery

inline SSATmp* spillStack(HTS& env) {
  auto vals = std::vector<SSATmp*>{};
  vals.reserve(env.irb->evalStack().size() + 2);
  vals.push_back(sp(env));
  vals.push_back(cns(env, int64_t{env.irb->stackDeficit()}));
  for (auto i = uint32_t{0}; i < env.irb->evalStack().size(); ++i) {
    // DataTypeGeneric is used here because SpillStack just teleports the
    // values to memory.
    vals.push_back(top(env, i, DataTypeGeneric));
  }

  auto const newSp = gen(
    env,
    SpillStack,
    std::make_pair(vals.size(), &vals[0])
  );
  env.irb->evalStack().clear();
  env.irb->clearStackDeficit();
  return newSp;
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

  return env.irb->cond(
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
    return env.irb->cond(
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

inline SSATmp* ldStackAddr(HTS& env, int32_t offset) {
  env.irb->constrainStack(offset, DataTypeSpecific);
  // You're almost certainly doing it wrong if you want to get the address of a
  // stack cell that's in m_irb->evalStack().
  assert(offset >= static_cast<int32_t>(env.irb->evalStack().numCells()));
  return gen(
    env,
    LdStackAddr,
    Type::PtrToStkGen,
    StackOffset(offset + env.irb->stackDeficit() -
      env.irb->evalStack().numCells()),
    sp(env)
  );
}

//////////////////////////////////////////////////////////////////////

}}}

#endif
