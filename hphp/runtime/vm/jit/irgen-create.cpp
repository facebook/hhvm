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

void initProps(HTS& env, const Class* cls) {
  cls->initPropHandle();
  env.irb->ifThen(
    [&](Block* taken) {
      gen(env, CheckInitProps, taken, ClassData(cls));
    },
    [&] {
      env.irb->hint(Block::Hint::Unlikely);
      gen(env, InitProps, ClassData(cls));
    }
  );
}

SSATmp* touchArgsSpillStackAndPopArgs(HTS& env, int numArgs) {
  // Before the spillStack() we touch all of the incoming stack
  // arguments so that they are available to later optimizations via
  // getStackValue().
  for (int i = 0; i < numArgs; i++) topC(env, i, DataTypeGeneric);
  auto const stack = spillStack(env );
  for (int i = 0; i < numArgs; i++) popC(env, DataTypeGeneric);
  return stack;
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

void initSProps(HTS& env, const Class* cls) {
  cls->initSPropHandles();
  if (RDS::isPersistentHandle(cls->sPropInitHandle())) return;
  env.irb->ifThen(
    [&](Block* taken) {
      gen(env, CheckInitSProps, taken, ClassData(cls));
    },
    [&] {
      env.irb->hint(Block::Hint::Unlikely);
      gen(env, InitSProps, ClassData(cls));
    }
  );
}

SSATmp* allocObjFast(HTS& env, const Class* cls) {
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
  assert((props || sprops) == cls->needInitialization());
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

  // Call a custom initializer if one exists
  if (cls->callsCustomInstanceInit()) {
    return registerObj(gen(env, CustomInstanceInit, ssaObj));
  }

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
void emitCreateCl(HTS& env, int32_t numParams, const StringData* clsName) {
  auto const cls = Unit::lookupUniqueClass(clsName);
  auto const invokeFunc = cls->lookupMethod(s_uuinvoke.get());
  auto const clonedFunc = invokeFunc->cloneAndSetClass(
    const_cast<Class*>(curClass(env))
  );
  assert(cls && (cls->attrs() & AttrUnique));

  auto const closure = allocObjFast(env, cls);
  gen(env, IncRef, closure);

  auto const ctx = [&]{
    if (!curClass(env)) return cns(env, nullptr);
    auto const ldctx = gen(env, LdCtx, fp(env));
    if (invokeFunc->attrs() & AttrStatic) {
      return gen(env, ConvClsToCctx, gen(env, LdClsCtx, ldctx));
    }
    gen(env, IncRefCtx, ldctx);
    return ldctx;
  }();
  gen(env, StClosureCtx, closure, ctx);
  gen(env, StClosureFunc, FuncData(clonedFunc), closure);

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

  // Closure static variables are per instance, and need to start
  // uninitialized.  After numParams use vars, the remaining instance
  // properties hold any static locals.
  assert(cls->numDeclProperties() ==
         clonedFunc->numStaticLocals() + numParams);
  for (int32_t numDeclProperties = cls->numDeclProperties();
      propId < numDeclProperties;
      ++propId) {
    gen(
      env,
      StClosureArg,
      PropByteOffset(cls->declPropOffset(propId)),
      closure,
      cns(env, Type::Uninit)
    );
  }

  push(env, closure);
}

void emitNewArray(HTS& env, int32_t capacity) {
  if (capacity == 0) {
    push(env, cns(env, staticEmptyArray()));
  } else {
    if (auto newCap = PackedArray::getMaxCapInPlaceFast(capacity)) {
      assert(newCap > static_cast<uint32_t>(capacity));
      capacity = newCap;
    }
    push(env, gen(env, NewArray, cns(env, capacity)));
  }
}

void emitNewMixedArray(HTS& env, int32_t capacity) {
  if (capacity == 0) {
    push(env, cns(env, staticEmptyArray()));
  } else {
    push(env, gen(env, NewMixedArray, cns(env, capacity)));
  }
}

void emitNewVArray(HTS& env, int32_t capacity) {
  // TODO(t4757263) staticEmptyArray() for VArray
  if (auto newCap = PackedArray::getMaxCapInPlaceFast(capacity)) {
    assert(newCap > static_cast<uint32_t>(capacity));
    capacity = newCap;
  }
  push(env, gen(env, NewVArray, cns(env, capacity)));
}

void emitNewMIArray(HTS& env, int32_t capacity) {
  // TODO(t4757263) staticEmptyArray() for IntMap
  push(env, gen(env, NewMIArray, cns(env, capacity)));
}

void emitNewMSArray(HTS& env, int capacity) {
  // TODO(t4757263) staticEmptyArray() for StrMap
  push(env, gen(env, NewMSArray, cns(env, capacity)));
}

void emitNewLikeArrayL(HTS& env, int32_t id, int32_t capacity) {
  auto const ldrefExit = makeExit(env);
  auto const ldPMExit = makeExit(env);
  auto const ld = ldLocInner(env, id, ldrefExit, ldPMExit, DataTypeSpecific);

  SSATmp* arr;
  if (ld->isA(Type::Arr)) {
    arr = gen(env, NewLikeArray, ld, cns(env, capacity));
  } else {
    capacity = (capacity ? capacity : MixedArray::SmallSize);
    arr = gen(env, NewArray, cns(env, capacity));
  }
  push(env, arr);
}

void emitNewPackedArray(HTS& env, int32_t numArgs) {
  if (numArgs > kPackedCapCodeThreshold) {
    PUNT(NewPackedArray-UnrealisticallyHuge);
  }

  auto const extra = PackedArrayData { static_cast<uint32_t>(numArgs) };
  auto const array = gen(env, AllocPackedArray, extra);
  static constexpr auto kMaxUnrolledInitArray = 8;
  if (numArgs > kMaxUnrolledInitArray) {
    auto const stack = touchArgsSpillStackAndPopArgs(env, numArgs);
    gen(env, InitPackedArrayLoop, extra, array, stack);
    push(env, array);
    return;
  }

  for (int i = 0; i < numArgs; ++i) {
    gen(
      env,
      InitPackedArray,
      IndexData { static_cast<uint32_t>(numArgs - i - 1) },
      array,
      popC(env)
    );
  }
  push(env, array);
}

void emitNewStructArray(HTS& env, const ImmVector& immVec) {
  auto const numArgs = immVec.size();
  auto const ids = immVec.vec32();

  // The NewPackedArray opcode's helper needs array values passed to it
  // via the stack.  We use spillStack() to flush the eval stack and
  // obtain a pointer to the topmost item; if over-flushing becomes
  // a problem then we should refactor the NewPackedArray opcode to
  // take its values directly as SSA operands.
  auto const stack = spillStack(env);
  for (int i = 0; i < numArgs; i++) popC(env, DataTypeGeneric);
  NewStructData extra;
  extra.numKeys = numArgs;
  extra.keys = new (env.unit.arena()) StringData*[numArgs];
  for (auto i = size_t{0}; i < numArgs; ++i) {
    extra.keys[i] = curUnit(env)->lookupLitstrId(ids[i]);
  }
  push(env, gen(env, NewStructArray, extra, stack));
}

void emitAddElemC(HTS& env) {
  // This is just to peek at the type; it'll be consumed for real down below and
  // we don't want to constrain it if we're just going to InterpOne.
  auto const kt = topC(env, 1, DataTypeGeneric)->type();
  Opcode op;
  if (kt <= Type::Int) {
    op = AddElemIntKey;
  } else if (kt <= Type::Str) {
    op = AddElemStrKey;
  } else {
    interpOne(env, Type::Arr, 3);
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

void emitAddNewElemC(HTS& env) {
  if (!topC(env, 1)->isA(Type::Arr)) {
    return interpOne(env, Type::Arr, 2);
  }

  auto const val = popC(env);
  auto const arr = popC(env);
  // The AddNewElem helper decrefs its args, so don't decref pop'ed values.
  push(env, gen(env, AddNewElem, arr, val));
}

void emitNewCol(HTS& env, int type, int size) {
  push(env,
    gen(env, NewCol, cns(env, type), cns(env, size)));
}

void emitColAddElemC(HTS& env) {
  if (!topC(env, 2)->isA(Type::Obj)) {
    return interpOne(env, Type::Obj, 3);
  }
  if (!topC(env, 1, DataTypeGeneric)->type().
      subtypeOfAny(Type::Int, Type::Str)) {
    interpOne(env, Type::Obj, 3);
    return;
  }

  auto const val = popC(env);
  auto const key = popC(env);
  auto const coll = popC(env);
  push(env, gen(env, ColAddElemC, coll, key, val));
  gen(env, DecRef, key);
}

void emitColAddNewElemC(HTS& env) {
  if (!topC(env, 1)->isA(Type::Obj)) {
    return interpOne(env, Type::Obj, 2);
  }

  auto const val = popC(env);
  auto const coll = popC(env);
  // The AddNewElem helper decrefs its args, so don't decref pop'ed values.
  push(env, gen(env, ColAddNewElemC, coll, val));
}

void emitStaticLocInit(HTS& env, int32_t locId, const StringData* name) {
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
    env.irb->ifThen(
      [&] (Block* taken) {
        gen(env, CheckStaticLocInit, taken, cachedBox);
      },
      [&] {
        env.irb->hint(Block::Hint::Unlikely);
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

void emitStaticLoc(HTS& env, int32_t locId, const StringData* name) {
  if (curFunc(env)->isPseudoMain()) PUNT(StaticLoc);

  auto const ldPMExit = makePseudoMainExit(env);

  auto const box = curFunc(env)->isClosureBody() ?
    gen(env, ClosureStaticLocInit,
             cns(env, name), fp(env), cns(env, Type::Uninit)) :
    gen(env, LdStaticLocCached, StaticLocName { curFunc(env), name });

  auto const res = env.irb->cond(
    0,
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
      gen(env, StaticLocInitCached, box, cns(env, Type::InitNull));
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
