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
#include "hphp/runtime/vm/jit/irgen-sprop-global.h"

#include "hphp/runtime/base/datatype.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/irgen-call.h"
#include "hphp/runtime/vm/jit/irgen-create.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-incdec.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/irgen-minstr.h"
#include "hphp/runtime/vm/jit/irgen-types.h"

namespace HPHP::jit::irgen {

namespace {

//////////////////////////////////////////////////////////////////////

void bindMem(IRGS& env, SSATmp* ptr, SSATmp* src, Type prevTy) {
  auto const prevValue = gen(env, LdMem, prevTy, ptr);
  pushIncRef(env, src);
  gen(env, StMem, ptr, src);
  decRef(env, prevValue);
}

void destroyName(IRGS& env, SSATmp* name) {
  if (env.irb->inUnreachableState()) return;
  assertx(name == topC(env));
  popDecRef(env);
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

ClsPropLookup ldClsPropAddrKnown(IRGS& env,
                                 const Class* cls,
                                 const StringData* name,
                                 bool ignoreLateInit,
                                 bool writeMode,
                                 ReadonlyOp readonlyOp) {
  initSProps(env, cls); // calls init; must be above sPropHandle()
  auto const slot = cls->lookupSProp(name);
  auto const handle = cls->sPropHandle(slot);
  assertx(!rds::isNormalHandle(handle));

  auto const ctx = curClass(env);
  auto const& prop = cls->staticProperties()[slot];

  auto knownType = TCell;
  if (RuntimeOption::EvalCheckPropTypeHints >= 3) {
    knownType = typeFromPropTC(prop.typeConstraint, cls, ctx, true);
    if (!(prop.attrs & AttrNoImplicitNullable)) knownType |= TInitNull;
  }
  knownType &= typeFromRAT(prop.repoAuthType, ctx);
  // Repo-auth-type doesn't include uninit for AttrLateInit props, so we need
  // to add it after intersecting with it.
  if (prop.attrs & AttrLateInit) {
    // If we're ignoring AttrLateInit, the prop might be uninit, but if we're
    // validating it, we'll never see uninit, so remove it.
    if (ignoreLateInit) {
      knownType |= TUninit;
    } else {
      knownType -= TUninit;
    }
  }

  if (prop.attrs & AttrInternal) {
    emitModuleBoundaryCheckKnown(env, &prop);
  }

  profileRDSAccess(env, handle);

  auto data = ClassData { cls };
  auto const readonly = prop.attrs & AttrIsReadonly;

  auto const checkReadonly = [&](SSATmp* addr) {
    auto const copyOnWriteCheck = [&]() {
      gen(env, StMROProp, cns(env, true));
      ifElse(
          env,
          [&] (Block* taken) {
            gen(env, CheckTypeMem, TObj, taken, addr);
          },
          [&] {
            gen(env, ThrowMustBeValueTypeException, data, cns(env, name));
            return cns(env, TBottom);
          }
        );
      return addr;
    };

    if (readonly && readonlyOp == ReadonlyOp::Mutable) {
      if (writeMode) {
        gen(env, ThrowMustBeMutableException, data, cns(env, name));
      } else {
        gen(env, ThrowMustBeEnclosedInReadonly, data, cns(env, name));
      }
      return cns(env, TBottom);
    } else if (readonly && readonlyOp == ReadonlyOp::CheckMutROCOW) {
      return copyOnWriteCheck();
    } else if (readonlyOp == ReadonlyOp::CheckROCOW) {
      if (!readonly) {
        gen(env, ThrowMustBeReadonlyException, data, cns(env, name));
        return cns(env, TBottom);
      }
      return copyOnWriteCheck();
    }
    return addr;
  };

  auto const addr = [&]{
    if (!(prop.attrs & AttrLateInit) || ignoreLateInit) {
      return gen(
        env,
        LdRDSAddr,
        RDSHandleAndType { handle, knownType },
        TPtrToSProp
      );
    }

    return cond(
      env,
      [&] (Block* taken) {
        return gen(
          env,
          LdInitRDSAddr,
          RDSHandleAndType { handle, knownType },
          taken,
          TPtrToSProp
        );
      },
      [&] (SSATmp* addr) { return addr; },
      [&] {
        hint(env, Block::Hint::Unlikely);
        gen(
          env,
          ThrowLateInitPropError,
          cns(env, prop.cls.get()),
          cns(env, name),
          cns(env, true)
        );
        return cns(env, TBottom);
      }
    );
  }();

  auto const checkedAddr = checkReadonly(addr);

  return {
    checkedAddr,
    knownType,
    &prop.typeConstraint,
    &prop.ubs,
    slot,
  };

  static_assert(sizeof(StaticPropData) == sizeof(TypedValue),
                "StaticPropData expected to only wrap TypedValue");
}

ClsPropLookup ldClsPropAddr(IRGS& env, SSATmp* ssaCls, SSATmp* ssaName,
                            const LdClsPropOptions& opts) {
  assertx(ssaCls->isA(TCls));
  assertx(ssaName->isA(TStr));

  /*
   * We can use ldClsPropAddrKnown if either we know which property it is and
   * that it is visible && accessible, or we know it is a property on this
   * class itself.
   */
  auto const sPropKnown = [&] {
    if (!ssaName->hasConstVal()) return false;
    auto const propName = ssaName->strVal();

    if (!ssaCls->hasConstVal()) return false;
    auto const cls = ssaCls->clsVal();

    auto const lookup = cls->findSProp(MemberLookupContext(curClass(env), curUnit(env)->moduleName()), propName);

    if (lookup.slot == kInvalidSlot) return false;
    if (!lookup.accessible) return false;
    if (opts.writeMode && lookup.constant) return false;
    return true;
  }();

  if (sPropKnown) {
    auto const lookup = ldClsPropAddrKnown(
      env,
      ssaCls->clsVal(),
      ssaName->strVal(),
      opts.ignoreLateInit,
      opts.writeMode,
      opts.readOnlyCheck
    );
    if (lookup.propPtr) return lookup;
  }

  auto const ctxFunc = cns(env, curFunc(env));
  auto const data = ReadonlyData{ opts.readOnlyCheck };
  auto const knownType = opts.ignoreLateInit ? TCell : TInitCell;
  auto const propAddr = gen(
    env,
    opts.raise ? LdClsPropAddrOrRaise : LdClsPropAddrOrNull,
    data,
    knownType,
    ssaCls,
    ssaName,
    ctxFunc,
    cns(env, opts.ignoreLateInit),
    cns(env, opts.writeMode)
  );
  return {
    propAddr,
    knownType,
    nullptr,
    nullptr,
    kInvalidSlot
  };
}

//////////////////////////////////////////////////////////////////////

void emitCGetS(IRGS& env, ReadonlyOp op) {
  auto const ssaCls      = topC(env);
  auto const ssaPropName = topC(env, BCSPRelOffset{1});

  if (!ssaPropName->isA(TStr)) PUNT(CGetS-PropNameNotString);
  if (!ssaCls->isA(TCls))      PUNT(CGetS-NotClass);

  const LdClsPropOptions opts { op, true, false, false };
  auto const lookup = ldClsPropAddr(env, ssaCls, ssaPropName, opts);
  auto const ldMem  = gen(env, LdMem, lookup.knownType, lookup.propPtr);

  discard(env);
  destroyName(env, ssaPropName);
  pushIncRef(env, ldMem);
}

void emitSetS(IRGS& env, ReadonlyOp op) {
  auto const ssaCls      = topC(env, BCSPRelOffset{1});
  auto const ssaPropName = topC(env, BCSPRelOffset{2});

  if (!ssaPropName->isA(TStr)) PUNT(SetS-PropNameNotString);
  if (!ssaCls->isA(TCls))      PUNT(SetS-NotClass);

  auto value  = popC(env, DataTypeGeneric);
  const LdClsPropOptions opts { op, true, true, true };
  auto const lookup = ldClsPropAddr(env, ssaCls, ssaPropName, opts);

  if (lookup.tc) {
    verifyPropType(
      env,
      ssaCls,
      lookup.tc,
      lookup.ubs,
      lookup.slot,
      value,
      ssaPropName,
      true,
      &value
    );
  } else if (RuntimeOption::EvalCheckPropTypeHints > 0) {
    auto const slot = gen(env, LookupSPropSlot, ssaCls, ssaPropName);
    value = gen(env, VerifyPropCoerceAll, ssaCls, slot, value, cns(env, true));
  }

  if (lookup.slot != kInvalidSlot && ssaCls->hasConstVal()) {
    if (!ssaCls->clsVal()->sPropLink(lookup.slot).isLocal()) {
      gen(env, Unreachable, ASSERT_REASON);
      return;
    }
  }

  discard(env);
  destroyName(env, ssaPropName);
  bindMem(env, lookup.propPtr, value, lookup.knownType);
}

void emitSetOpS(IRGS& env, SetOpOp op) {
  auto const ssaCls      = topC(env, BCSPRelOffset{1});
  auto const ssaPropName = topC(env, BCSPRelOffset{2});

  if (!ssaPropName->isA(TStr)) PUNT(SetOpS-PropNameNotString);
  if (!ssaCls->isA(TCls))      PUNT(SetOpS-NotClass);

  auto const rhs = popC(env);
  const LdClsPropOptions opts { ReadonlyOp::Any, true, false, true };
  auto const lookup = ldClsPropAddr(env, ssaCls, ssaPropName, opts);

  auto const lhs = gen(env, LdMem, lookup.knownType, lookup.propPtr);

  auto const finish = [&] (SSATmp* value) {
    if (lookup.tc) {
      verifyPropType(
        env,
        ssaCls,
        lookup.tc,
        lookup.ubs,
        lookup.slot,
        value,
        ssaPropName,
        true,
        &value
      );
    } else if (RuntimeOption::EvalCheckPropTypeHints > 0) {
      auto const slot = gen(env, LookupSPropSlot, ssaCls, ssaPropName);
      value = gen(env, VerifyPropCoerceAll, ssaCls, slot, value,
                  cns(env, true));
    }

    if (lookup.slot != kInvalidSlot && ssaCls->hasConstVal()) {
      if (!ssaCls->clsVal()->sPropLink(lookup.slot).isLocal()) {
        gen(env, Unreachable, ASSERT_REASON);
        return;
      }
    }

    discard(env);
    destroyName(env, ssaPropName);
    pushIncRef(env, value);
    gen(env, StMem, lookup.propPtr, value);
    decRef(env, lhs, DecRefProfileId::SetOpSLhs);
    decRef(env, rhs, DecRefProfileId::SetOpSRhs);
  };

  if (auto value = inlineSetOp(env, op, lhs, rhs)) {
    finish(value);
  } else {
    // Handle cases not performed inline.
    finish(gen(env, OutlineSetOp, SetOpData{op}, lhs, rhs));
  }
}

void emitIssetS(IRGS& env) {
  auto const ssaCls      = topC(env);
  auto const ssaPropName = topC(env, BCSPRelOffset{1});

  if (!ssaPropName->isA(TStr)) PUNT(IssetS-PropNameNotString);
  if (!ssaCls->isA(TCls))      PUNT(IssetS-NotClass);

  auto const ret = cond(
    env,
    [&] (Block* taken) {
      const LdClsPropOptions opts { ReadonlyOp::Any, false, true, false };
      auto const propAddr =
        ldClsPropAddr(env, ssaCls, ssaPropName, opts).propPtr;
      return gen(env, CheckNonNull, taken, propAddr);
    },
    [&] (SSATmp* ptr) { // Next: property or global exists
      return gen(env, IsNTypeMem, TNull, ptr);
    },
    [&] { // Taken: LdClsPropAddr* returned Nullptr because it isn't defined
      return cns(env, false);
    }
  );

  discard(env);
  destroyName(env, ssaPropName);
  push(env, ret);
}

void emitIncDecS(IRGS& env, IncDecOp subop) {
  auto const ssaCls      = topC(env);
  auto const ssaPropName = topC(env, BCSPRelOffset{1});

  if (!ssaPropName->isA(TStr)) PUNT(IncDecS-PropNameNotString);
  if (!ssaCls->isA(TCls))      PUNT(IncDecS-NotClass);
  const LdClsPropOptions opts { ReadonlyOp::Any, true, false, true };
  auto const lookup = ldClsPropAddr(env, ssaCls, ssaPropName, opts);
  auto const oldVal = gen(env, LdMem, lookup.knownType, lookup.propPtr);

  auto const result = incDec(env, subop, oldVal);
  if (!result) PUNT(IncDecS);
  assertx(result->isA(TUncounted));
  assertx(!result->type().maybe(TClsMeth));

  if (lookup.tc) {
    verifyPropType(
      env,
      ssaCls,
      lookup.tc,
      lookup.ubs,
      lookup.slot,
      result,
      ssaPropName,
      true
    );
  } else if (RuntimeOption::EvalCheckPropTypeHints > 0) {
    auto const slot = gen(env, LookupSPropSlot, ssaCls, ssaPropName);
    gen(env, VerifyPropAll, ssaCls, slot, result, cns(env, true));
  }

  if (lookup.slot != kInvalidSlot && ssaCls->hasConstVal()) {
    if (!ssaCls->clsVal()->sPropLink(lookup.slot).isLocal()) {
      gen(env, Unreachable, ASSERT_REASON);
      return;
    }
  }

  discard(env);
  destroyName(env, ssaPropName);
  pushIncRef(env, isPre(subop) ? result : oldVal);

  // Update marker to ensure newly-pushed value isn't clobbered by DecRef.
  updateMarker(env);

  gen(env, StMem, lookup.propPtr, result);
  gen(env, IncRef, result);
  decRef(env, oldVal);
}

//////////////////////////////////////////////////////////////////////

void emitCGetG(IRGS& env) {
  auto const name = topC(env);
  if (!name->isA(TStr)) PUNT(CGetG-NonStrName);

  auto const ret = profiledGlobalAccess(
    env,
    name,
    [&] (Block* taken) {
      auto const addr = gen(env, LdGblAddr, name);
      return gen(env, CheckNonNull, taken, addr);
    },
    [&] (SSATmp* ptr, Type type) {
      auto const tmp = gen(env, LdMem, type, ptr);
      gen(env, IncRef, tmp);
      return tmp;
    },
    [&] { return cns(env, TInitNull); },
    true
  );

  destroyName(env, name);
  push(env, ret);
}

void emitSetG(IRGS& env) {
  auto const name = topC(env, BCSPRelOffset{1});
  if (!name->isA(TStr)) PUNT(SetG-NameNotStr);
  auto const value = topC(env, BCSPRelOffset{0}, DataTypeGeneric);

  auto const ptr = profiledGlobalAccess(
    env,
    name,
    [&] (Block*) { return gen(env, LdGblAddrDef, name); },
    [&] (SSATmp* ptr, Type) { return ptr; },
    [&] { return gen(env, LdGblAddrDef, name); },
    false
  );

  discard(env);
  destroyName(env, name);
  bindMem(env, ptr, value, TCell);
}

void emitIssetG(IRGS& env) {
  auto const name = topC(env, BCSPRelOffset{0});
  if (!name->isA(TStr)) PUNT(IssetG-NameNotStr);

  auto const ret = profiledGlobalAccess(
    env,
    name,
    [&] (Block* taken) {
      auto const addr = gen(env, LdGblAddr, name);
      return gen(env, CheckNonNull, taken, addr);
    },
    [&] (SSATmp* ptr, Type) {
      return gen(env, IsNTypeMem, TNull, ptr);
    },
    [&] { return cns(env, false); },
    false
  );

  destroyName(env, name);
  push(env, ret);
}

//////////////////////////////////////////////////////////////////////

void emitCheckProp(IRGS& env, const StringData* propName) {
  auto const cls = ldCtxCls(env);
  auto const propInitVec = gen(env, LdClsInitData, cls);

  auto const ctx = curClass(env);
  auto const slot = ctx->lookupDeclProp(propName);
  auto const idx = ctx->propSlotToIndex(slot);

  auto const curVal = gen(env, LdClsInitElem, IndexData{idx}, propInitVec);
  push(env, gen(env, IsNType, TUninit, curVal));
}

void emitInitProp(IRGS& env, const StringData* propName, InitPropOp op) {
  auto val = popC(env);
  auto const ctx = curClass(env);

  switch (op) {
  case InitPropOp::Static:
    {
      // For sinit, the context class is always the same as the late-bound
      // class, so we can just use curClass().
      auto const slot = ctx->lookupSProp(propName);
      assertx(slot != kInvalidSlot);
      auto const handle = ctx->sPropHandle(slot);
      assertx(!rds::isNormalHandle(handle));

      auto const& prop = ctx->staticProperties()[slot];
      assertx(!(prop.attrs & AttrSystemInitialValue));
      if (!(prop.attrs & AttrInitialSatisfiesTC)) {
        verifyPropType(
          env,
          cns(env, ctx),
          &prop.typeConstraint,
          &prop.ubs,
          slot,
          val,
          cns(env, propName),
          true,
          &val
        );
      }

      profileRDSAccess(env, handle);
      auto const base = gen(
        env,
        LdRDSAddr,
        RDSHandleAndType { handle, TCell },
        TPtrToSProp
      );
      gen(env, StMem, base, val);
    }
    break;

  case InitPropOp::NonStatic:
    {
      // The above is not the case for pinit, so we need to load.
      auto const cls = ldCtxCls(env);

      const auto slot = ctx->lookupDeclProp(propName);
      auto const idx = ctx->propSlotToIndex(slot);
      auto const& prop = ctx->declProperties()[slot];
      assertx(!(prop.attrs & AttrSystemInitialValue));
      if (!(prop.attrs & AttrInitialSatisfiesTC)) {
        verifyPropType(
          env,
          cls,
          &prop.typeConstraint,
          &prop.ubs,
          slot,
          val,
          cns(env, propName),
          false,
          &val
        );
      }

      auto const base = gen(env, LdClsInitData, cls);
      gen(env, StClsInitElem, IndexData{idx}, base, val);
    }
    break;
  }
}

//////////////////////////////////////////////////////////////////////

}
