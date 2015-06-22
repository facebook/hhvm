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
#include "hphp/runtime/vm/jit/irgen-create.h"

#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-interpone.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_uuinvoke("__invoke");

//////////////////////////////////////////////////////////////////////

void initProps(IRGS& env, const Class* cls) {
  cls->initPropHandle();
  ifThen(
    env,
    [&] (Block* taken) {
      gen(env, CheckInitProps, taken, ClassData(cls));
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      gen(env, InitProps, ClassData(cls));
    }
  );
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

void initSProps(IRGS& env, const Class* cls) {
  cls->initSPropHandles();
  if (rds::isPersistentHandle(cls->sPropInitHandle())) return;
  ifThen(
    env,
    [&] (Block* taken) {
      gen(env, CheckInitSProps, taken, ClassData(cls));
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      gen(env, InitSProps, ClassData(cls));
    }
  );
}

SSATmp* allocObjFast(IRGS& env, const Class* cls) {
  // CustomInstance classes always go through IR:AllocObj and
  // ObjectData::newInstance()
  assert(!cls->callsCustomInstanceInit());

  auto registerObj = [&] (SSATmp* obj) {
    if (RuntimeOption::EnableObjDestructCall && cls->getDtor()) {
      gen(env, RegisterLiveObj, obj);
    }
    return obj;
  };

  // If it's an extension class with a custom instance initializer,
  // that init function does all the work.
  if (cls->instanceCtor()) {
    auto const obj = gen(env, ConstructInstance, ClassData(cls));
    return registerObj(obj);
  }

  // Make sure our property init vectors are all set up.
  const bool props = cls->pinitVec().size() > 0;
  const bool sprops = cls->numStaticProperties() > 0;
  assertx((props || sprops) == cls->needInitialization());
  if (cls->needInitialization()) {
    if (props) initProps(env, cls);
    if (sprops) initSProps(env, cls);
  }

  /*
   * Allocate the object.  This must happen after we do sinits for consistency
   * with the interpreter about o_id assignments.  Also, the prop
   * initialization above can throw, so we don't want to have the object
   * allocated already.
   */
  auto const ssaObj = gen(env, NewInstanceRaw, ClassData(cls));

  // Initialize the properties
  gen(env, InitObjProps, ClassData(cls), ssaObj);
  return registerObj(ssaObj);
}

//////////////////////////////////////////////////////////////////////

/*
 * The CreateCl opcode is specified as not being allowed before the
 * class it creates exists, and closure classes are always unique.
 *
 * This means even if we're not in RepoAuthoritative mode, as long as
 * this code is reachable it will always use the same closure Class*,
 * so we can just burn it into the TC without using RDS.
 */
void emitCreateCl(IRGS& env, int32_t numParams, const StringData* clsName) {
  auto cls = Unit::lookupClassOrUniqueClass(clsName)->rescope(
    const_cast<Class*>(curClass(env))
  );
  assertx(cls && (cls->attrs() & AttrUnique));

  auto const func = cls->getCachedInvoke();

  auto const closure = allocObjFast(env, cls);

  auto const ctx = [&]{
    if (!curClass(env)) return cns(env, nullptr);
    auto const ldctx = gen(env, LdCtx, fp(env));
    if (func->isStatic()) {
      return gen(env, ConvClsToCctx, gen(env, LdClsCtx, ldctx));
    }
    gen(env, IncRefCtx, ldctx);
    return ldctx;
  }();

  gen(env, StClosureCtx, closure, ctx);

  SSATmp* args[numParams];
  for (int32_t i = 0; i < numParams; ++i) {
    args[numParams - i - 1] = popF(env);
  }

  int32_t propId = 0;
  for (; propId < numParams; ++propId) {
    gen(
      env,
      StClosureArg,
      PropByteOffset(cls->declPropOffset(propId)),
      closure,
      args[propId]
    );
  }

  assertx(cls->numDeclProperties() == func->numStaticLocals() + numParams);

  // Closure static variables are per instance, and need to start
  // uninitialized.  After numParams use vars, the remaining instance
  // properties hold any static locals.
  for (int32_t numDeclProperties = cls->numDeclProperties();
      propId < numDeclProperties;
      ++propId) {
    gen(
      env,
      StClosureArg,
      PropByteOffset(cls->declPropOffset(propId)),
      closure,
      cns(env, TUninit)
    );
  }

  push(env, closure);
}

void emitNewArray(IRGS& env, int32_t capacity) {
  if (capacity == 0) {
    push(env, cns(env, staticEmptyArray()));
  } else {
    if (auto newCap = PackedArray::getMaxCapInPlaceFast(capacity)) {
      assertx(newCap > static_cast<uint32_t>(capacity));
      capacity = newCap;
    }
    push(env, gen(env, NewArray, cns(env, capacity)));
  }
}

void emitNewMixedArray(IRGS& env, int32_t capacity) {
  if (capacity == 0) {
    push(env, cns(env, staticEmptyArray()));
  } else {
    push(env, gen(env, NewMixedArray, cns(env, capacity)));
  }
}

void emitNewLikeArrayL(IRGS& env, int32_t id, int32_t capacity) {
  auto const ldrefExit = makeExit(env);
  auto const ldPMExit = makeExit(env);
  auto const ld = ldLocInner(env, id, ldrefExit, ldPMExit, DataTypeSpecific);

  SSATmp* arr;
  if (ld->isA(TArr)) {
    arr = gen(env, NewLikeArray, ld, cns(env, capacity));
  } else {
    capacity = (capacity ? capacity : MixedArray::SmallSize);
    arr = gen(env, NewArray, cns(env, capacity));
  }
  push(env, arr);
}

void emitNewPackedArray(IRGS& env, int32_t numArgs) {
  if (numArgs > CapCode::Threshold) {
    PUNT(NewPackedArray-UnrealisticallyHuge);
  }

  auto const array = gen(
    env,
    AllocPackedArray,
    PackedArrayData { static_cast<uint32_t>(numArgs) }
  );
  static constexpr auto kMaxUnrolledInitArray = 8;
  if (numArgs > kMaxUnrolledInitArray) {
    spillStack(env);
    gen(
      env,
      InitPackedArrayLoop,
      InitPackedArrayLoopData {
        offsetFromIRSP(env, BCSPOffset{0}),
        static_cast<uint32_t>(numArgs)
      },
      array,
      sp(env)
    );
    discard(env, numArgs);
    push(env, array);
    return;
  }

  for (int i = 0; i < numArgs; ++i) {
    gen(
      env,
      InitPackedArray,
      IndexData { static_cast<uint32_t>(numArgs - i - 1) },
      array,
      popC(env, DataTypeGeneric)
    );
  }
  push(env, array);
}

void emitNewStructArray(IRGS& env, const ImmVector& immVec) {
  auto const numArgs = immVec.size();
  auto const ids = immVec.vec32();

  // The NewPackedArray opcode's helper needs array values passed to it
  // via the stack.  We use spillStack() to flush the eval stack and
  // obtain a pointer to the topmost item; if over-flushing becomes
  // a problem then we should refactor the NewPackedArray opcode to
  // take its values directly as SSA operands.
  spillStack(env);

  NewStructData extra;
  extra.offset  = offsetFromIRSP(env, BCSPOffset{0});
  extra.numKeys = numArgs;
  extra.keys    = new (env.unit.arena()) StringData*[numArgs];
  for (auto i = size_t{0}; i < numArgs; ++i) {
    extra.keys[i] = curUnit(env)->lookupLitstrId(ids[i]);
  }

  discard(env, numArgs);
  push(env, gen(env, NewStructArray, extra, sp(env)));
}

void emitAddElemC(IRGS& env) {
  // This is just to peek at the type; it'll be consumed for real down below and
  // we don't want to constrain it if we're just going to InterpOne.
  auto const kt = topC(env, BCSPOffset{1}, DataTypeGeneric)->type();
  Opcode op;
  if (kt <= TInt) {
    op = AddElemIntKey;
  } else if (kt <= TStr) {
    op = AddElemStrKey;
  } else {
    interpOne(env, TArr, 3);
    return;
  }

  // val is teleported from the stack to the array, so we don't have to do any
  // refcounting.
  auto const val = popC(env, DataTypeGeneric);
  auto const key = popC(env);
  auto const arr = popC(env);
  // The AddElem* instructions decref their args, so don't decref pop'ed
  // values.
  push(env, gen(env, op, arr, key, val));
}

void emitAddNewElemC(IRGS& env) {
  if (!topC(env, BCSPOffset{1})->isA(TArr)) {
    return interpOne(env, TArr, 2);
  }

  auto const val = popC(env);
  auto const arr = popC(env);
  // The AddNewElem helper decrefs its args, so don't decref pop'ed values.
  push(env, gen(env, AddNewElem, arr, val));
}

void emitNewCol(IRGS& env, int type) {
  push(env, gen(env, NewCol, NewColData{type}));
}

void emitColFromArray(IRGS& env, int type) {
  auto const arr = popC(env);
  push(env, gen(env, NewColFromArray, NewColData{type}, arr));
}

void emitMapAddElemC(IRGS& env) {
  if (!topC(env, BCSPOffset{2})->isA(TObj)) {
    return interpOne(env, TObj, 3);
  }
  if (!topC(env, BCSPOffset{1}, DataTypeGeneric)->type().
      subtypeOfAny(TInt, TStr)) {
    interpOne(env, TObj, 3);
    return;
  }

  auto const val = popC(env);
  auto const key = popC(env);
  auto const coll = popC(env);
  push(env, gen(env, MapAddElemC, coll, key, val));
  gen(env, DecRef, key);
}

void emitColAddNewElemC(IRGS& env) {
  if (!topC(env, BCSPOffset{1})->isA(TObj)) {
    return interpOne(env, TObj, 2);
  }

  auto const val = popC(env);
  auto const coll = popC(env);
  // The AddNewElem helper decrefs its args, so don't decref pop'ed values.
  push(env, gen(env, ColAddNewElemC, coll, val));
}

void emitStaticLocInit(IRGS& env, int32_t locId, const StringData* name) {
  if (curFunc(env)->isPseudoMain()) PUNT(StaticLocInit);

  auto const ldPMExit = makePseudoMainExit(env);
  auto const value = popC(env);

  // Closures and generators from closures don't satisfy the "one static per
  // source location" rule that the inline fastpath requires
  auto const box = [&]{
    if (curFunc(env)->isClosureBody()) {
      return gen(env, ClosureStaticLocInit, cns(env, name), fp(env), value);
    }

    auto const cachedBox =
      gen(env, LdStaticLocCached, StaticLocName { curFunc(env), name });
    ifThen(
      env,
      [&] (Block* taken) {
        gen(env, CheckStaticLocInit, taken, cachedBox);
      },
      [&] {
        hint(env, Block::Hint::Unlikely);
        gen(env, StaticLocInitCached, cachedBox, value);
      }
    );
    return cachedBox;
  }();
  gen(env, IncRef, box);
  auto const oldValue = ldLoc(env, locId, ldPMExit, DataTypeSpecific);
  stLocRaw(env, locId, fp(env), box);
  gen(env, DecRef, oldValue);
  // We don't need to decref value---it's a bytecode invariant that
  // our Cell was not ref-counted.
}

void emitStaticLoc(IRGS& env, int32_t locId, const StringData* name) {
  if (curFunc(env)->isPseudoMain()) PUNT(StaticLoc);

  auto const ldPMExit = makePseudoMainExit(env);

  auto const box = curFunc(env)->isClosureBody() ?
    gen(env, ClosureStaticLocInit,
             cns(env, name), fp(env), cns(env, TUninit)) :
    gen(env, LdStaticLocCached, StaticLocName { curFunc(env), name });

  auto const res = cond(
    env,
    [&] (Block* taken) {
      gen(env, CheckStaticLocInit, taken, box);
    },
    [&] { // Next: the static local is already initialized
      return cns(env, true);
    },
    [&] { // Taken: need to initialize the static local
      /*
       * Even though this path is "cold", we're not marking it
       * unlikely because the size of the instructions this will
       * generate is about 10 bytes, which is not much larger than the
       * 5 byte jump to acold would be.
       *
       * One note about StaticLoc: we're literally always going to
       * generate a fallthrough trace here that is cold (the code that
       * initializes the static local).  TODO(#2894612).
       */
      gen(env, StaticLocInitCached, box, cns(env, TInitNull));
      return cns(env, false);
    });
  gen(env, IncRef, box);
  auto const oldValue = ldLoc(env, locId, ldPMExit, DataTypeGeneric);
  stLocRaw(env, locId, fp(env), box);
  gen(env, DecRef, oldValue);
  push(env, res);
}

//////////////////////////////////////////////////////////////////////

}}}
