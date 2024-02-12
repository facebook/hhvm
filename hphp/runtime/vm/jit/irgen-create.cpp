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

#include "hphp/runtime/base/configs/hhir.h"
#include "hphp/runtime/base/vanilla-vec.h"
#include "hphp/runtime/ext/std/ext_std_errorfunc.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/irgen.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-interpone.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/irgen-sprop-global.h"
#include "hphp/runtime/vm/jit/irgen-types.h"

namespace HPHP::jit::irgen {

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
  assertx(cls->classof(SystemLib::getErrorClass()) ||
          cls->classof(SystemLib::getExceptionClass()));
  assertx(throwable->type() <= TObj);

  // Root of the class hierarchy.
  auto const rootCls = cls->classof(SystemLib::getExceptionClass())
    ? SystemLib::getExceptionClass() : SystemLib::getErrorClass();

  auto const propAddr = [&] (Slot slot) {
    return gen(
      env,
      LdPropAddr,
      IndexData { rootCls->propSlotToIndex(slot) },
      TCell,
      throwable
    );
  };

  // Load Exception::$traceOpts
  auto const lookup = ldClsPropAddrKnown(
    env,
    SystemLib::getExceptionClass(),
    s_traceOpts.get(),
    false,
    false,
    ReadonlyOp::Any
  );
  assertx(lookup.tc->isInt());
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
      return gen(env, DebugBacktrace, opts);
    },
    [&] {
      // sprop is a garbage, use default traceOpts value (0)
      return gen(env, DebugBacktrace, cns(env, 0));
    }
  );

  // $throwable->trace = $trace
  auto const traceSlot = rootCls->lookupDeclProp(s_trace.get());
  assertx(traceSlot != kInvalidSlot);
  assertx(rootCls->declPropTypeConstraint(traceSlot).isVec());
  // `DebugBacktrace` unconditionally returns a `Vec`, so we shouldn't need to
  // check that it's such before assigning it into `$trace`.
  gen(env, StMem, propAddr(traceSlot), trace);

  // Populate $throwable->{file,line}
  if (UNLIKELY(curFunc(env)->isBuiltin())) {
    gen(env, InitThrowableFileAndLine, throwable);
  } else {
    auto const fileSlot = rootCls->lookupDeclProp(s_file.get());
    auto const lineSlot = rootCls->lookupDeclProp(s_line.get());
    auto const line = curSrcKey(env).lineNumber();
    assertx(rootCls->declPropTypeConstraint(fileSlot).isString());
    assertx(rootCls->declPropTypeConstraint(lineSlot).isInt());
    gen(env, StMem, propAddr(fileSlot), cns(env, curFunc(env)->filename()));
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

      auto const anyUnresolved = [&] {
        for (Slot slot = 0; slot < props.size(); ++slot) {
          auto const& prop = props[slot];
          if (prop.typeConstraint.isUnresolved()) return true;
          for (auto const& ub : prop.ubs.m_constraints) {
            if (ub.isUnresolved()) return true;
          }
        }
        return false;
      }();
      if (anyUnresolved) {
        // Will do all the checking for us
        gen(env, PropTypeValid, cns(env, cls));
        return;
      }
      assertx(RO::EvalCheckPropTypeHints > 0);

      for (Slot slot = 0; slot < props.size(); ++slot) {
        auto const& prop = props[slot];

        if (prop.attrs & (AttrInitialSatisfiesTC|AttrSystemInitialValue)) {
          continue;
        }
        auto const isAnyCheckable = [&] {
          if (prop.typeConstraint.isCheckable()) return true;
          for (auto const& ub : prop.ubs.m_constraints) {
            if (ub.isCheckable()) return true;
          }
          return false;
        }();
        if (!isAnyCheckable) continue;
        auto index = cls->propSlotToIndex(slot);
        auto const tv = cls->declPropInit()[index].val.tv();
        if (tv.m_type == KindOfUninit) continue;
        assertx(!isClsMethType(tv.m_type));
        verifyPropType(
          env,
          cns(env, cls),
          &prop.typeConstraint,
          &prop.ubs,
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

  if (nprops <= Cfg::HHIR::InliningMaxInitObjProps &&
      cls->pinitVec().size() == 0) {
    if (cls->hasMemoSlots()) {
      gen(env, InitObjMemoSlots, ClassData(cls), obj);
    }
    for (int slot = 0; slot < nprops; ++slot) {
      auto index = cls->propSlotToIndex(slot);
      auto const tv = cls->declPropInit()[index].val.tv();
      auto const val = cns(env, tv);
      auto const addr = gen(
        env,
        LdPropAddr,
        IndexData { cls->propSlotToIndex(slot) },
        TCell,
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
void emitCreateCl(IRGS& env, uint32_t numParams, const StringData* name) {
  auto const preCls = curFunc(env)->unit()->lookupPreClass(name);
  assertx(preCls);
  assertx(preCls->attrs() & AttrIsClosureClass);
  auto const templateCls = Class::defClosure(preCls, false);
  assertx(templateCls);

  auto const cls = templateCls->rescope(const_cast<Class*>(curClass(env)));

  assertx(!cls->needInitialization());
  // In repo mode, rescoping should always use the template class
  // (except if we're in a trait).
  assertx(
    IMPLIES(
      RO::RepoAuthoritative &&
        !(curFunc(env)->preClass() &&
          curFunc(env)->preClass()->attrs() & AttrTrait),
      cls == templateCls
    )
  );

  auto const func = cls->getRegularInvoke();

  auto const live_ctx = [&] {
    if (func->isStatic()) return ldCtxCls(env);
    assertx(hasThis(env));
    auto const ldctx = ldThis(env);
    gen(env, IncRef, ldctx);
    return ldctx;
  }();

  auto const closure = gen(env, ConstructClosure, ClassData(cls), live_ctx);

  auto const hasCoeffectsProp = cls->hasClosureCoeffectsProp();
  if (hasCoeffectsProp) numParams++;

  assertx(cls->numDeclProperties() == numParams);

  SSATmp** args = (SSATmp**)alloca(sizeof(SSATmp*) * numParams);
  for (int32_t i = 0; i < numParams; ++i) {
    auto const slot = numParams - i - 1;
    if (hasCoeffectsProp && slot == cls->getCoeffectsProp()) {
      assertx(cls->getCoeffectsProp() == 0);
      assertx(cls->propSlotToIndex(slot) == slot);
      args[slot] = curRequiredCoeffects(env);
    } else {
      args[slot] = popCU(env);
    }
  }

  for (int32_t propId = 0; propId < numParams; ++propId) {
    gen(
      env,
      StClosureArg,
      IndexData { cls->propSlotToIndex(propId) },
      closure,
      args[propId]
    );
  }

  push(env, closure);
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

void emitNewVec(IRGS& env, uint32_t numArgs) {
  if (numArgs == 0) {
    push(env, cns(env, ArrayData::CreateVec()));
    return;
  }

  jit::vector<RepoAuthType> types(numArgs, RepoAuthType(RepoAuthType::Tag::InitCell));
  auto const array = gen(env, AllocVec, VanillaVecData { numArgs });

  auto genAssert = [&]() {
    using A = RepoAuthType::Array;
    auto rat = A::tuple(A::Empty::No, types);
    gen(env, AssertType, Type::Vec(rat), array);
  };

  if (numArgs > Cfg::HHIR::MaxInlineInitPackedElements) {
    gen(
      env,
      InitVecElemLoop,
      InitVanillaVecLoopData {
        spOffBCFromIRSP(env),
        numArgs
      },
      array,
      sp(env)
    );
    discard(env, numArgs);
    push(env, array);
    genAssert();
    return;
  }

  for (int i = 0; i < numArgs; ++i) {
    auto src = popC(env, DataTypeGeneric);
    gen(
      env,
      InitVecElem,
      IndexData { static_cast<uint32_t>(numArgs - i - 1) },
      array,
      src
    );
  }
  push(env, array);
  genAssert();
}

void emitNewStructDict(IRGS& env, const ImmVector& immVec) {
  auto const numArgs = immVec.size();
  auto const ids = immVec.vec32();

  NewStructData extra;
  extra.offset  = spOffBCFromIRSP(env);
  extra.numKeys = numArgs;
  extra.keys    = new (env.unit.arena()) StringData*[numArgs];
  for (auto i = size_t{0}; i < numArgs; ++i) {
    extra.keys[i] = curUnit(env)->lookupLitstrId(ids[i]);
  }

  if (numArgs > Cfg::HHIR::MaxInlineInitMixedElements) {
    auto const arr = gen(env, NewStructDict, extra, sp(env));
    discard(env, numArgs);
    push(env, arr);
    return;
  }

  auto const arr = gen(env, AllocStructDict, extra);
  for (int i = 0; i < numArgs; ++i) {
    auto const index = numArgs - i - 1;
    auto const data = KeyedIndexData(index, extra.keys[index]);
    gen(env, InitDictElem, data, arr, popC(env, DataTypeGeneric));
  }
  push(env, arr);
}

void emitAddElemC(IRGS& env) {
  // This is just to peek at the types; they'll be consumed for real down below
  // and we don't want to constrain it if we're just going to InterpOne.
  auto const kt = topC(env, BCSPRelOffset{1}, DataTypeGeneric)->type();
  auto const at = topC(env, BCSPRelOffset{2}, DataTypeGeneric)->type();

  // This operation is only defined for dicts.
  if (!(at <= TDict)) {
    PUNT(AddElemC-BadArr);
  } else if (!kt.subtypeOfAny(TInt, TStr, TCls, TLazyCls)) {
    interpOne(env, at.unspecialize(), 3);
    return;
  }

  // DictSet teleports the value from the stack into the base, and dec-refs the
  // old base on copy / escalation, so we only need to to do refcount ops on
  // the key.
  auto const val = popC(env, DataTypeGeneric);
  auto const key = convertClassKey(env, popC(env));
  auto const arr = popC(env);
  push(env, gen(env, DictSet, arr, key, val));
  decRef(env, key);
}

void emitAddNewElemC(IRGS& env) {
  auto const arrType = topC(env, BCSPRelOffset{1})->type();
  // This operation is only defined for vecs and keysets.
  if (!arrType.subtypeOfAny(TKeyset, TVec)) {
    return interpOne(env);
  }
  auto const val = popC(env, DataTypeGeneric);
  auto const arr = popC(env);
  auto const op = arr->isA(TVec) ? AddNewElemVec : AddNewElemKeyset;
  push(env, gen(env, op, arr, val));
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

void emitCheckClsReifiedGenericMismatch(IRGS& env) {
  auto const cls = curClass(env);
  if (!cls) {
    // no static context class, so this will raise an error
    interpOne(env);
    return;
  }
  auto const reified = topC(env);
  if (!reified->isA(TVec)) {
    PUNT(CheckClsReifiedGenericMismatch-InvalidTS);
  }
  gen(env, CheckClsReifiedGenericMismatch, cns(env, cls), reified);
  popDecRef(env);
}

void emitClassHasReifiedGenerics(IRGS& env) {
  auto const cls_ = topC(env);
  if (!cls_->isA(TCls) && !cls_->isA(TLazyCls)) return interpOne(env);
  auto const cls = cls_->isA(TLazyCls) ? ldCls(env, cls_) : cls_;
  auto const result = gen(env, ClassHasReifiedGenerics, cls);
  popDecRef(env);
  push(env, result);
}

void emitGetClsRGProp(IRGS& env) {
  auto const cls_ = topC(env);
  if (!cls_->isA(TCls) && !cls_->isA(TLazyCls)) return interpOne(env);
  auto const cls = cls_->isA(TLazyCls) ? ldCls(env, cls_) : cls_;
  auto const thiz = checkAndLoadThis(env);
  auto const result = gen(env, GetClsRGProp, cls, thiz);
  popDecRef(env);
  push(env, result);
}

void emitHasReifiedParent(IRGS& env) {
  auto const cls_ = topC(env);
  if (!cls_->isA(TCls) && !cls_->isA(TLazyCls)) return interpOne(env);
  auto const cls = cls_->isA(TLazyCls) ? ldCls(env, cls_) : cls_;
  auto const result = gen(env, HasReifiedParent, cls);
  popDecRef(env);
  push(env, result);
}

void emitCheckClsRGSoft(IRGS& env) {
  auto const cls_ = topC(env);
  if (!cls_->isA(TCls) && !cls_->isA(TLazyCls)) return interpOne(env);
  auto const cls = cls_->isA(TLazyCls) ? ldCls(env, cls_) : cls_;
  gen(env, CheckClsRGSoft, cls);
  popDecRef(env);
}

//////////////////////////////////////////////////////////////////////

}
