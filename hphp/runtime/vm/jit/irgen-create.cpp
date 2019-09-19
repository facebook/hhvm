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
#include "hphp/runtime/vm/jit/irgen-types.h"

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
      gen(env, CheckRDSInitialized, taken, RDSHandleData { cls->propHandle() });
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

  auto const propAddr = [&] (Slot slot) {
    return gen(
      env,
      LdPropAddr,
      ByteOffsetData { (ptrdiff_t)rootCls->declPropOffset(slot) },
      TUncounted.lval(Ptr::Prop),
      throwable
    );
  };

  // Load Exception::$traceOpts
  auto const lookup = ldClsPropAddrKnown(
    env,
    SystemLib::s_ExceptionClass,
    s_traceOpts.get(),
    false
  );
  assertx(!lookup.tc->isCheckable());
  auto const sprop = lookup.propPtr;
  assertx(sprop);

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
  auto const traceSlot = rootCls->lookupDeclProp(s_trace.get());
  assertx(traceSlot != kInvalidSlot);
  assertx(!rootCls->declPropTypeConstraint(traceSlot).isCheckable());
  gen(env, StMem, propAddr(traceSlot), trace);

  // Populate $throwable->{file,line}
  if (UNLIKELY(curFunc(env)->isBuiltin())) {
    gen(env, InitThrowableFileAndLine, throwable);
  } else {
    auto const fileSlot = rootCls->lookupDeclProp(s_file.get());
    auto const lineSlot = rootCls->lookupDeclProp(s_line.get());
    auto const unit = curFunc(env)->unit();
    auto const line = unit->getLineNumber(bcOff(env));
    assertx(rootCls->declPropTypeConstraint(fileSlot).isString());
    assertx(rootCls->declPropTypeConstraint(lineSlot).isInt());
    gen(env, StMem, propAddr(fileSlot), cns(env, unit->filepath()));
    gen(env, StMem, propAddr(lineSlot), cns(env, line));
  }
}

void checkPropTypeRedefs(IRGS& env, const Class* cls) {
  assertx(cls->maybeRedefinesPropTypes());
  assertx(cls->parent());
  assertx(RuntimeOption::EvalCheckPropTypeHints > 0);

  ifThen(
    env,
    [&] (Block* taken) {
      gen(
        env,
        CheckRDSInitialized,
        taken,
        RDSHandleData { cls->checkedPropTypeRedefinesHandle() }
      );
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      IRUnit::Hinter h(env.irb->unit(), Block::Hint::Unlikely);

      auto const parent = cls->parent();
      if (parent->maybeRedefinesPropTypes()) checkPropTypeRedefs(env, parent);
      for (auto const& prop : cls->declProperties()) {
        if (prop.attrs & AttrNoBadRedeclare) continue;
        auto const slot = parent->lookupDeclProp(prop.name);
        assertx(slot != kInvalidSlot);
        gen(env, PropTypeRedefineCheck, cns(env, cls), cns(env, slot));
      }

      gen(
        env,
        MarkRDSInitialized,
        RDSHandleData { cls->checkedPropTypeRedefinesHandle() }
      );
    }
  );
}

void checkPropInitialValues(IRGS& env, const Class* cls) {
  assertx(cls->needsPropInitialValueCheck());
  assertx(RuntimeOption::EvalCheckPropTypeHints > 0);

  ifThen(
    env,
    [&] (Block* taken) {
      gen(
        env,
        CheckRDSInitialized,
        taken,
        RDSHandleData { cls->checkedPropInitialValuesHandle() }
      );
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      IRUnit::Hinter h(env.irb->unit(), Block::Hint::Unlikely);

      auto const& props = cls->declProperties();
      for (Slot slot = 0; slot < props.size(); ++slot) {
        auto const& prop = props[slot];
        if (prop.attrs & AttrInitialSatisfiesTC) continue;
        auto const& tc = prop.typeConstraint;
        if (!tc.isCheckable()) continue;
        auto index = cls->propSlotToIndex(slot);
        const TypedValue& tv = cls->declPropInit()[index];
        if (tv.m_type == KindOfUninit) continue;
        verifyPropType(
          env,
          cns(env, cls),
          &tc,
          slot,
          cns(env, tv),
          cns(env, makeStaticString(prop.name)),
          false
        );
      }

      gen(
        env,
        MarkRDSInitialized,
        RDSHandleData { cls->checkedPropInitialValuesHandle() }
      );
    }
  );
}

void initObjProps(IRGS& env, const Class* cls, SSATmp* obj) {
  auto const nprops = cls->numDeclProperties();

  if (nprops <= RuntimeOption::EvalHHIRInliningMaxInitObjProps &&
      cls->pinitVec().size() == 0) {
    if (cls->hasMemoSlots()) {
      gen(env, InitObjMemoSlots, ClassData(cls), obj);
    }
    for (int slot = 0; slot < nprops; ++slot) {
      auto index = cls->propSlotToIndex(slot);
      const TypedValue& tv = cls->declPropInit()[index];
      auto const val = cns(env, tv);
      auto const addr = gen(
        env,
        LdPropAddr,
        ByteOffsetData { (ptrdiff_t)(cls->declPropOffset(slot)) },
        TLvalToPropGen,
        obj
      );
      gen(env, StMem, addr, val);
    }
  } else {
    gen(env, InitObjProps, ClassData(cls), obj);
  }
}

//////////////////////////////////////////////////////////////////////

}

void initSProps(IRGS& env, const Class* cls) {
  cls->initSPropHandles();
  if (rds::isPersistentHandle(cls->sPropInitHandle())) return;
  ifThen(
    env,
    [&] (Block* taken) {
      gen(
        env,
        CheckRDSInitialized,
        taken,
        RDSHandleData { cls->sPropInitHandle() }
      );
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      gen(env, InitSProps, ClassData(cls));
    }
  );
}

//////////////////////////////////////////////////////////////////////

SSATmp* allocObjFast(IRGS& env, const Class* cls) {
  // Make sure our property init vectors are all set up.
  auto const props = cls->pinitVec().size() > 0;
  auto const sprops = cls->numStaticProperties() > 0;
  auto const redefine = cls->maybeRedefinesPropTypes();
  auto const propVal = cls->needsPropInitialValueCheck();
  assertx(
    (props || sprops || redefine || propVal) == cls->needInitialization()
  );
  if (cls->needInitialization()) {
    if (redefine) checkPropTypeRedefs(env, cls);
    if (propVal) checkPropInitialValues(env, cls);
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
    initObjProps(env, cls, obj);
  }

  // Initialize Throwable.
  if (cls->needsInitThrowable()) {
    initThrowable(env, cls, obj);
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
  assertx(!cls->needInitialization());

  auto const func = cls->getCachedInvoke();

  auto const closure = gen(env, ConstructClosure, ClassData(cls));

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
    args[numParams - i - 1] = popCU(env);
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

  assertx(cls->numDeclProperties() == numParams);

  push(env, closure);
}

void emitNewArray(IRGS& env, uint32_t capacity) {
  if (capacity == 0) {
    push(env, cns(env, ArrayData::Create()));
  } else {
    push(env, gen(env, NewArray, cns(env, capacity)));
  }
}

void emitNewMixedArray(IRGS& env, uint32_t capacity) {
  if (capacity == 0) {
    push(env, cns(env, ArrayData::Create()));
  } else {
    push(env, gen(env, NewMixedArray, cns(env, capacity)));
  }
}

void emitNewDArray(IRGS& env, uint32_t capacity) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  if (capacity == 0) {
    push(env, cns(env, ArrayData::CreateDArray()));
  } else {
    push(env, gen(env, NewDArray, cns(env, capacity)));
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

void emitNewVArray(IRGS& env, uint32_t numArgs) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  emitNewPackedLayoutArray(env, numArgs, AllocVArray);
}

namespace {

void newStructImpl(IRGS& env, const ImmVector& immVec, Opcode op) {
  auto const numArgs = immVec.size();
  auto const ids = immVec.vec32();

  NewStructData extra;
  extra.offset  = spOffBCFromIRSP(env);
  extra.numKeys = numArgs;
  extra.keys    = new (env.unit.arena()) StringData*[numArgs];
  for (auto i = size_t{0}; i < numArgs; ++i) {
    extra.keys[i] = curUnit(env)->lookupLitstrId(ids[i]);
  }

  auto const structData = gen(env, op, extra, sp(env));
  discard(env, numArgs);
  push(env, structData);
}

}

void emitNewStructArray(IRGS& env, const ImmVector& immVec) {
  newStructImpl(env, immVec, NewStructArray);
}

void emitNewStructDArray(IRGS& env, const ImmVector& immVec) {
  assertx(!RuntimeOption::EvalHackArrDVArrs);
  newStructImpl(env, immVec, NewStructDArray);
}

void emitNewStructDict(IRGS& env, const ImmVector& immVec) {
  newStructImpl(env, immVec, NewStructDict);
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
  auto const arrType = topC(env, BCSPRelOffset{1})->type();
  if (!arrType.subtypeOfAny(TArr, TKeyset, TVec)) {
    return interpOne(env);
  }
  auto const val = popC(env, DataTypeCountness);
  auto const arr = popC(env);
  push(
    env,
    gen(
      env,
      [&]{
        if (arr->isA(TArr))    return AddNewElem;
        if (arr->isA(TKeyset)) return AddNewElemKeyset;
        if (arr->isA(TVec))    return AddNewElemVec;
        always_assert(false);
      }(),
      arr,
      val
    )
  );
  decRef(env, val);
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

void emitNewRecordImpl(IRGS& env, const StringData* name,
                       const ImmVector& immVec,
                       Opcode newRecordOp) {
  auto const recDesc = Unit::lookupUniqueRecDesc(name);
  auto const isPersistent = recordHasPersistentRDS(recDesc);
  auto const cachedRec = isPersistent ?
    cns(env, recDesc) : gen(env, LdRecDescCached, RecNameData{name});
  auto const numArgs = immVec.size();
  auto const ids = immVec.vec32();
  NewStructData extra;
  extra.offset = spOffBCFromIRSP(env);
  extra.numKeys = numArgs;
  extra.keys = new (env.unit.arena()) StringData*[numArgs];
  for (auto i = size_t{0}; i < numArgs; ++i) {
    extra.keys[i] = curUnit(env)->lookupLitstrId(ids[i]);
  }
  auto const recData = gen(env, newRecordOp, extra, cachedRec, sp(env));
  discard(env, numArgs);
  push(env, recData);
}

void emitNewRecord(IRGS& env, const StringData* name, const ImmVector& immVec) {
  emitNewRecordImpl(env, name, immVec, NewRecord);
}

void emitNewRecordArray(IRGS& env, const StringData* name,
                        const ImmVector& immVec) {
  emitNewRecordImpl(env, name, immVec, NewRecordArray);
}

void emitColFromArray(IRGS& env, CollectionType type) {
  assertx(type != CollectionType::Pair);
  auto const arr = popC(env);
  if (UNLIKELY(!arr->isA(TVec) && !arr->isA(TDict))) {
    PUNT(BadColType);
  }
  if (UNLIKELY(arr->isA(TVec) && type != CollectionType::Vector &&
               type != CollectionType::ImmVector)) {
      PUNT(ColTypeMismatch);
  }
  if (UNLIKELY(arr->isA(TDict) && (type == CollectionType::Vector ||
               type == CollectionType::ImmVector))) {
      PUNT(ColTypeMismatch);
  }
  push(env, gen(env, NewColFromArray, NewColData{type}, arr));
}

void emitCheckReifiedGenericMismatch(IRGS& env) {
  auto const cls = curClass(env);
  if (!cls) {
    // no static context class, so this will raise an error
    interpOne(env);
    return;
  }
  auto const reified = popC(env);
  if (!reified->isA(RuntimeOption::EvalHackArrDVArrs ? TVec : TArr)) {
    PUNT(CheckReifiedGenericMismatch-InvalidTS);
  }
  gen(env, CheckClsReifiedGenericMismatch, ClassData{cls}, reified);
}

//////////////////////////////////////////////////////////////////////

}}}
