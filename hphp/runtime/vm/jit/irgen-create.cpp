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
#include "hphp/runtime/vm/jit/irgen-create.h"

#include "hphp/runtime/base/packed-array.h"
#include "hphp/runtime/ext/std/ext_std_errorfunc.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/irgen.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-interpone.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/irgen-sprop-global.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_uuinvoke("__invoke");
const StaticString s_traceOpts("traceOpts");
const StaticString s_trace("trace");
const StaticString s_file("file");
const StaticString s_line("line");

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

void initThrowable(IRGS& env, const Class* cls, SSATmp* throwable) {
  assertx(cls->classof(SystemLib::s_ErrorClass) ||
          cls->classof(SystemLib::s_ExceptionClass));
  assertx(throwable->type() <= TObj);

  // Root of the class hierarchy.
  auto const rootCls = cls->classof(SystemLib::s_ExceptionClass)
    ? SystemLib::s_ExceptionClass : SystemLib::s_ErrorClass;

  auto const propAddr = [&] (Slot idx) {
    return gen(
      env,
      LdPropAddr,
      ByteOffsetData { (ptrdiff_t)rootCls->declPropOffset(idx) },
      TInitNull.ptr(Ptr::Prop),
      throwable
    );
  };

  // Load Exception::$traceOpts
  auto const sprop = ldClsPropAddrKnown(
    env, SystemLib::s_ExceptionClass, s_traceOpts.get());

  auto const trace = cond(
    env,
    [&] (Block* taken) {
      gen(env, CheckTypeMem, TInt, taken, sprop);
    },
    [&] {
      // sprop is an integer, load it
      auto const opts = gen(env, LdMem, TInt, sprop);
      return cond(
        env,
        [&] (Block* taken) {
          if (!RuntimeOption::EnableArgsInBacktraces) {
            auto const filterOpts =
              gen(env, AndInt, opts, cns(env, ~k_DEBUG_BACKTRACE_IGNORE_ARGS));

            gen(env, JmpNZero, taken, filterOpts);
          } else {
            auto const safe =
              gen(env, EqInt, opts, cns(env, k_DEBUG_BACKTRACE_IGNORE_ARGS));
            gen(env, JmpZero, taken, safe);
          }
        },
        [&] {
          // traceOpts is default value, use fast lazy construction
          if (RuntimeOption::EvalEnableCompactBacktrace) {
            return gen(env, DebugBacktraceFast);
          }
          return gen(env, DebugBacktrace, cns(env, 0));
        },
        [&] {
          // Call debug_backtrace(traceOpts)
          return gen(env, DebugBacktrace, opts);
        });
    },
    [&] {
      // sprop is a garbage, use default traceOpts value (0)
      if (!RuntimeOption::EnableArgsInBacktraces &&
          RuntimeOption::EvalEnableCompactBacktrace) {
        return gen(env, DebugBacktraceFast);
      }
      return gen(env, DebugBacktrace, cns(env, 0));
    }
  );

  // $throwable->trace = $trace
  auto const traceIdx = rootCls->lookupDeclProp(s_trace.get());
  assertx(traceIdx != kInvalidSlot);
  gen(env, StMem, propAddr(traceIdx), trace);

  // Populate $throwable->{file,line}
  if (UNLIKELY(curFunc(env)->isBuiltin())) {
    gen(env, InitThrowableFileAndLine, throwable);
  } else {
    auto const fileIdx = rootCls->lookupDeclProp(s_file.get());
    auto const lineIdx = rootCls->lookupDeclProp(s_line.get());
    auto const unit = curFunc(env)->unit();
    auto const line = unit->getLineNumber(bcOff(env));
    gen(env, StMem, propAddr(fileIdx), cns(env, unit->filepath()));
    gen(env, StMem, propAddr(lineIdx), cns(env, line));
  }
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
  SSATmp* obj;
  if (cls->instanceCtor()) {
    // If it's an extension class with a custom instance initializer,
    // use it to construct the object.
    obj = gen(env, ConstructInstance, ClassData(cls));
  } else {
    // Construct a new instance of PHP class.
    obj = gen(env, NewInstanceRaw, ClassData(cls));

    // Initialize the properties.
    gen(env, InitObjProps, ClassData(cls), obj);
  }

  // Initialize Throwable.
  if (cls->needsInitThrowable()) {
    initThrowable(env, cls, obj);
  }

  if (RuntimeOption::EnableObjDestructCall && cls->getDtor()) {
    gen(env, RegisterLiveObj, obj);
  }

  return obj;
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
void emitCreateCl(IRGS& env, uint32_t numParams, uint32_t clsIx) {
  auto const preCls = curFunc(env)->unit()->lookupPreClassId(clsIx);
  auto cls = Unit::defClosure(preCls);

  assertx(cls);
  assertx(cls->attrs() & AttrUnique);

  cls = cls->rescope(const_cast<Class*>(curClass(env)));

  auto const func = cls->getCachedInvoke();

  auto const closure = allocObjFast(env, cls);

  auto const live_ctx = [&] {
    auto const ldctx = ldCtx(env);
    if (!ldctx->type().maybe(TObj)) {
      return ldctx;
    }
    if (func->isStatic()) {
      return gen(env, FwdCtxStaticCall, ldctx);
    }
    gen(env, IncRef, ldctx);
    return ldctx;
  }();

  gen(env, StClosureCtx, closure, live_ctx);

  SSATmp** args = (SSATmp**)alloca(sizeof(SSATmp*) * numParams);
  for (int32_t i = 0; i < numParams; ++i) {
    args[numParams - i - 1] = popF(env);
  }

  int32_t propId = 0;
  for (; propId < numParams; ++propId) {
    gen(
      env,
      StClosureArg,
      ByteOffsetData { safe_cast<ptrdiff_t>(cls->declPropOffset(propId)) },
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
      ByteOffsetData { safe_cast<ptrdiff_t>(cls->declPropOffset(propId)) },
      closure,
      cns(env, TUninit)
    );
  }

  push(env, closure);
}

void emitNewArray(IRGS& env, uint32_t capacity) {
  if (capacity == 0) {
    push(env, cns(env, staticEmptyArray()));
  } else {
    push(env, gen(env, NewArray, cns(env, capacity)));
  }
}

void emitNewMixedArray(IRGS& env, uint32_t capacity) {
  if (capacity == 0) {
    push(env, cns(env, staticEmptyArray()));
  } else {
    push(env, gen(env, NewMixedArray, cns(env, capacity)));
  }
}

void emitNewDictArray(IRGS& env, uint32_t capacity) {
  push(env, gen(env, NewDictArray, cns(env, capacity)));
}

void emitNewKeysetArray(IRGS& env, uint32_t numArgs) {
  auto const array = gen(
    env,
    NewKeysetArray,
    NewKeysetArrayData {
      spOffBCFromIRSP(env),
      static_cast<uint32_t>(numArgs)
    },
    sp(env)
  );
  discard(env, numArgs);
  push(env, array);
}

void emitNewLikeArrayL(IRGS& env, int32_t id, uint32_t capacity) {
  auto const ldrefExit = makeExit(env);
  auto const ldPMExit = makePseudoMainExit(env);
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

namespace {

ALWAYS_INLINE
void emitNewPackedLayoutArray(IRGS& env, uint32_t numArgs, Opcode op) {
  auto const array = gen(
    env,
    op,
    PackedArrayData { numArgs }
  );
  static constexpr auto kMaxUnrolledInitArray = 8;
  if (numArgs > kMaxUnrolledInitArray) {
    gen(
      env,
      InitPackedLayoutArrayLoop,
      InitPackedArrayLoopData {
        spOffBCFromIRSP(env),
        numArgs
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
      InitPackedLayoutArray,
      IndexData { static_cast<uint32_t>(numArgs - i - 1) },
      array,
      popC(env, DataTypeGeneric)
    );
  }
  push(env, array);
}

}

void emitNewPackedArray(IRGS& env, uint32_t numArgs) {
  emitNewPackedLayoutArray(env, numArgs, AllocPackedArray);
}

void emitNewVecArray(IRGS& env, uint32_t numArgs) {
  emitNewPackedLayoutArray(env, numArgs, AllocVecArray);
}

void emitNewStructArray(IRGS& env, const ImmVector& immVec) {
  auto const numArgs = immVec.size();
  auto const ids = immVec.vec32();

  NewStructData extra;
  extra.offset  = spOffBCFromIRSP(env);
  extra.numKeys = numArgs;
  extra.keys    = new (env.unit.arena()) StringData*[numArgs];
  for (auto i = size_t{0}; i < numArgs; ++i) {
    extra.keys[i] = curUnit(env)->lookupLitstrId(ids[i]);
  }

  discard(env, numArgs);
  push(env, gen(env, NewStructArray, extra, sp(env)));
}

void emitAddElemC(IRGS& env) {
  // This is just to peek at the types; they'll be consumed for real down below
  // and we don't want to constrain it if we're just going to InterpOne.
  auto const kt = topC(env, BCSPRelOffset{1}, DataTypeGeneric)->type();
  auto const at = topC(env, BCSPRelOffset{2}, DataTypeGeneric)->type();
  Opcode op;
  if (at <= TArr) {
    if (kt <= TInt) {
      op = AddElemIntKey;
    } else if (kt <= TStr) {
      op = AddElemStrKey;
    } else {
      interpOne(env, TArr, 3);
      return;
    }
  } else if (at <= TDict) {
    if (kt <= TInt) {
      op = DictAddElemIntKey;
    } else if (kt <= TStr) {
      op = DictAddElemStrKey;
    } else {
      interpOne(env, TDict, 3);
      return;
    }
  } else {
    PUNT(AddElemC-BadArr);
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
  if (!topC(env, BCSPRelOffset{1})->isA(TArr)) {
    return interpOne(env, TArr, 2);
  }

  auto const val = popC(env);
  auto const arr = popC(env);
  // The AddNewElem helper decrefs its args, so don't decref pop'ed values.
  push(env, gen(env, AddNewElem, arr, val));
}

void emitNewCol(IRGS& env, CollectionType type) {
  assertx(type != CollectionType::Pair);
  push(env, gen(env, NewCol, NewColData{type}));
}

void emitNewPair(IRGS& env) {
  auto const c1 = popC(env, DataTypeGeneric);
  auto const c2 = popC(env, DataTypeGeneric);
  // elements were pushed onto the stack in the order they should appear
  // in the pair, so the top of the stack should become the second element
  push(env, gen(env, NewPair, c2, c1));
}

void emitColFromArray(IRGS& env, CollectionType type) {
  assertx(type != CollectionType::Pair);
  auto const arr = popC(env);
  push(env, gen(env, NewColFromArray, NewColData{type}, arr));
}

void emitStaticLocInit(IRGS& env, int32_t locId, const StringData* name) {
  auto const func = curFunc(env);
  if (func->isPseudoMain()) PUNT(StaticLocInit);

  auto const value = popC(env);

  // Closures and generators from closures don't satisfy the "one static per
  // source location" rule that the inline fastpath requires
  auto const box = [&]{
    if (func->isClosureBody()) {
      assertx(func->isClosureBody());
      assertx(!func->hasVariadicCaptureParam());
      auto const obj = gen(
        env, LdLoc, TObj, LocalId(func->numParams()), fp(env));

      auto const theStatic = gen(env,
                                 LdClosureStaticLoc,
                                 StaticLocName { func, name },
                                 obj);
      ifThen(
        env,
        [&] (Block* taken) {
          gen(env, CheckTypeMem, TBoxedCell, taken, theStatic);
        },
        [&] {
          hint(env, Block::Hint::Unlikely);
          gen(env, StMem, theStatic, value);
          gen(env, BoxPtr, theStatic);
        }
      );
      return gen(env, LdMem, TBoxedCell, theStatic);
    }

    ifThen(
      env,
      [&] (Block* taken) {
        gen(
          env,
          CheckStaticLoc,
          StaticLocName { func, name },
          taken
        );
      },
      [&] {
        hint(env, Block::Hint::Unlikely);
        gen(
          env,
          InitStaticLoc,
          StaticLocName { func, name },
          value
        );
      }
    );
    return gen(env, LdStaticLoc, StaticLocName { func, name });
  }();

  gen(env, IncRef, box);
  auto const oldValue = ldLoc(env, locId, nullptr, DataTypeSpecific);
  stLocRaw(env, locId, fp(env), box);
  decRef(env, oldValue);
  // We don't need to decref value---it's a bytecode invariant that
  // our Cell was not ref-counted.
}

void emitStaticLocCheck(IRGS& env, int32_t locId, const StringData* name) {
  auto const func = curFunc(env);
  if (func->isPseudoMain()) PUNT(StaticLocCheck);

  auto bindLocal = [&] (SSATmp* box) {
    gen(env, IncRef, box);
    auto const oldValue = ldLoc(env, locId, nullptr, DataTypeGeneric);
    stLocRaw(env, locId, fp(env), box);
    decRef(env, oldValue);
    return cns(env, true);
  };

  auto const inited = [&] {
    if (func->isClosureBody()) {
      auto const obj = gen(
        env, LdLoc, TObj, LocalId(func->numParams()), fp(env));
      auto const theStatic = gen(env,
                                 LdClosureStaticLoc,
                                 StaticLocName { func, name },
                                 obj);
      return cond(
        env,
        [&] (Block* taken) {
          gen(env, CheckTypeMem, TBoxedCell, taken, theStatic);
        },
        [&] {
          return bindLocal(gen(env, LdMem, TInitCell, theStatic));
        },
        [&] {
          hint(env, Block::Hint::Unlikely);
          return cns(env, false);
        }
      );
    }

    return cond(
      env,
      [&] (Block* taken) {
        gen(
          env,
          CheckStaticLoc,
          StaticLocName { func, name },
          taken
        );
      },
      [&] {
        // Next: the static local is already initialized
        return bindLocal(gen(env, LdStaticLoc, StaticLocName { func, name }));
      },
      [&] { // Taken: need to initialize the static local
        return cns(env, false);
      }
    );
  }();

  push(env, inited);
}

void emitStaticLocDef(IRGS& env, int32_t locId, const StringData* name) {
  auto const func = curFunc(env);
  if (func->isPseudoMain()) PUNT(StaticLocDef);

  auto const value = popC(env);

  auto const box = [&] {
    if (func->isClosureBody()) {
      auto const obj = gen(
        env, LdLoc, TObj, LocalId(func->numParams()), fp(env));
      auto const theStatic = gen(env,
                                 LdClosureStaticLoc,
                                 StaticLocName { func, name },
                                 obj);
      gen(env, StMem, theStatic, value);
      auto const boxedStatic = gen(env, BoxPtr, theStatic);
      return gen(env, LdMem, TBoxedInitCell, boxedStatic);
    }

    auto init = [&] {
      gen(
        env,
        InitStaticLoc,
        StaticLocName { func, name },
        value
      );
    };

    if (func->isMemoizeWrapper() && !func->numParams()) {
      ifThenElse(
        env,
        [&] (Block* taken) {
          gen(
            env,
            CheckStaticLoc,
            StaticLocName { func, name },
            taken
          );
        },
        [&] {
          hint(env, Block::Hint::Unlikely);
          auto oldBox = gen(env, LdStaticLoc, StaticLocName { func, name });
          auto oldVal = gen(env, LdRef, TInitCell, oldBox);
          init();
          decRef(env, oldVal);
        },
        [&] {
          init();
        }
      );
    } else {
      init();
    }

    return gen(
      env,
      LdStaticLoc,
      StaticLocName { func, name }
    );
  }();

  gen(env, IncRef, box);
  auto const oldValue = ldLoc(env, locId, nullptr, DataTypeGeneric);
  stLocRaw(env, locId, fp(env), box);
  decRef(env, oldValue);
}

//////////////////////////////////////////////////////////////////////

}}}
