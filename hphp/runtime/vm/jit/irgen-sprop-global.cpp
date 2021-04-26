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
#include "hphp/runtime/vm/jit/irgen-create.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-incdec.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
#include "hphp/runtime/vm/jit/irgen-minstr.h"
#include "hphp/runtime/vm/jit/irgen-types.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

void bindMem(IRGS& env, SSATmp* ptr, SSATmp* src) {
  auto const prevValue = gen(env, LdMem, ptr->type().deref(), ptr);
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
                                 bool ignoreLateInit) {
  initSProps(env, cls); // calls init; must be above sPropHandle()
  auto const slot = cls->lookupSProp(name);
  auto const handle = cls->sPropHandle(slot);
  assertx(!rds::isNormalHandle(handle));

  auto const ctx = curClass(env);
  auto const& prop = cls->staticProperties()[slot];

  auto knownType = TCell;
  if (RuntimeOption::EvalCheckPropTypeHints >= 3 &&
      (!prop.typeConstraint.isUpperBound() ||
       RuntimeOption::EvalEnforceGenericsUB >= 2)) {
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

  auto const ptrTy = knownType.ptr(Ptr::SProp);

  auto const addr = [&]{
    if (!(prop.attrs & AttrLateInit) || ignoreLateInit) {
      return gen(env, LdRDSAddr, RDSHandleData { handle }, ptrTy);
    }

    return cond(
      env,
      [&] (Block* taken) {
        return gen(env, LdInitRDSAddr, RDSHandleData { handle }, taken, ptrTy);
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

  return {
    addr,
    &prop.typeConstraint,
    &prop.ubs,
    slot,
  };

  static_assert(sizeof(StaticPropData) == sizeof(TypedValue),
                "StaticPropData expected to only wrap TypedValue");
}

ClsPropLookup ldClsPropAddr(IRGS& env, SSATmp* ssaCls, SSATmp* ssaName, SSATmp* roProp,
                            const LdClsPropOptions& opts) {
  assertx(ssaCls->isA(TCls));
  assertx(ssaName->isA(TStr));

  auto const mustBeMutable = opts.readOnlyCheck == ReadOnlyOp::Mutable;
  auto const mustBeReadOnly = opts.readOnlyCheck == ReadOnlyOp::ReadOnly;
  auto const checkROCOW = opts.readOnlyCheck == ReadOnlyOp::CheckROCOW;
  /*
   * We can use ldClsPropAddrKnown if either we know which property it is and
   * that it is visible && accessible, or we know it is a property on this
   * class itself.
   */
  bool const sPropKnown = [&] {
    if (!ssaName->hasConstVal()) return false;
    auto const propName = ssaName->strVal();

    if (!ssaCls->hasConstVal()) return false;
    auto const cls = ssaCls->clsVal();

    auto const lookup = cls->findSProp(curClass(env), propName);

    if (lookup.slot == kInvalidSlot) return false;
    if (!lookup.accessible) return false;
    if (opts.writeMode && lookup.constant) return false;
    if ((mustBeMutable || checkROCOW) && lookup.readonly) return false;
    if (mustBeReadOnly && !lookup.readonly) return false;
    return true;
  }();

  if (sPropKnown) {
    auto const lookup = ldClsPropAddrKnown(
      env,
      ssaCls->clsVal(),
      ssaName->strVal(),
      opts.ignoreLateInit
    );
    if (lookup.propPtr) return lookup;
  }

  auto const ctxClass = curClass(env);
  auto const ctxTmp = ctxClass ? cns(env, ctxClass) : cns(env, nullptr);
  auto const propAddr = gen(
    env,
    opts.raise ? LdClsPropAddrOrRaise : LdClsPropAddrOrNull,
    ssaCls,
    ssaName,
    ctxTmp,
    roProp,
    cns(env, opts.ignoreLateInit),
    cns(env, opts.writeMode),
    cns(env, mustBeMutable),
    cns(env, mustBeReadOnly),
    cns(env, checkROCOW)
  );
  return { propAddr, nullptr, nullptr, kInvalidSlot };
}

//////////////////////////////////////////////////////////////////////

void emitCGetS(IRGS& env, ReadOnlyOp op) {
  auto const ssaCls      = topC(env);
  auto const ssaPropName = topC(env, BCSPRelOffset{1});

  if (!ssaPropName->isA(TStr)) PUNT(CGetS-PropNameNotString);
  if (!ssaCls->isA(TCls))      PUNT(CGetS-NotClass);

  const LdClsPropOptions opts { op, true, false, false };
  auto const propAddr = ldClsPropAddr(env, ssaCls, ssaPropName, cns(env, nullptr), opts).propPtr;
  auto const ldMem    = gen(env, LdMem, propAddr->type().deref(), propAddr);

  discard(env);
  destroyName(env, ssaPropName);
  pushIncRef(env, ldMem);
}

void emitSetS(IRGS& env, ReadOnlyOp op) {
  auto const ssaCls      = topC(env, BCSPRelOffset{1});
  auto const ssaPropName = topC(env, BCSPRelOffset{2});

  if (!ssaPropName->isA(TStr)) PUNT(SetS-PropNameNotString);
  if (!ssaCls->isA(TCls))      PUNT(SetS-NotClass);

  auto value  = popC(env, DataTypeGeneric);
  const LdClsPropOptions opts { op, true, true, true };
  auto const lookup = ldClsPropAddr(env, ssaCls, ssaPropName, cns(env, nullptr), opts);

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

  discard(env);
  destroyName(env, ssaPropName);
  bindMem(env, lookup.propPtr, value);
}

void emitSetOpS(IRGS& env, SetOpOp op) {
  auto const ssaCls      = topC(env, BCSPRelOffset{1});
  auto const ssaPropName = topC(env, BCSPRelOffset{2});

  if (!ssaPropName->isA(TStr)) PUNT(SetOpS-PropNameNotString);
  if (!ssaCls->isA(TCls))      PUNT(SetOpS-NotClass);

  auto const rhs = popC(env);
  const LdClsPropOptions opts { ReadOnlyOp::Any, true, false, true };
  auto const lookup = ldClsPropAddr(env, ssaCls, ssaPropName, cns(env, nullptr), opts);

  auto const lhs = gen(env, LdMem, lookup.propPtr->type().deref(),
                       lookup.propPtr);

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

    discard(env);
    destroyName(env, ssaPropName);
    pushIncRef(env, value);
    gen(env, StMem, lookup.propPtr, value);
    decRef(env, lhs);
    decRef(env, rhs);
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
      const LdClsPropOptions opts { ReadOnlyOp::Any, false, true, false };
      auto const propAddr =
        ldClsPropAddr(env, ssaCls, ssaPropName, cns(env, nullptr), opts).propPtr;
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
  const LdClsPropOptions opts { ReadOnlyOp::Any, true, false, true };
  auto const lookup = ldClsPropAddr(env, ssaCls, ssaPropName, cns(env, nullptr), opts);
  auto const oldVal =
    gen(env, LdMem, lookup.propPtr->type().deref(), lookup.propPtr);

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

  auto ret = cond(
    env,
    [&] (Block* taken) {
      auto const ptr = gen(env, LdGblAddr, name);
      return gen(env, CheckNonNull, taken, ptr);
    },
    [&] (SSATmp* ptr) {
      auto tmp = gen(env, LdMem, TCell, ptr);
      gen(env, IncRef, tmp);
      return tmp;
    },
    // Taken: LdGblAddr branched here because no global variable exists with
    // that name.
    [&] { return cns(env, TInitNull); }
  );

  destroyName(env, name);
  push(env, ret);
}

void emitSetG(IRGS& env) {
  auto const name = topC(env, BCSPRelOffset{1});
  if (!name->isA(TStr)) PUNT(SetG-NameNotStr);
  auto const value   = popC(env, DataTypeGeneric);
  auto const ptr = gen(env, LdGblAddrDef, name);
  destroyName(env, name);
  bindMem(env, ptr, value);
}

void emitIssetG(IRGS& env) {
  auto const name = topC(env, BCSPRelOffset{0});
  if (!name->isA(TStr)) PUNT(IssetG-NameNotStr);

  auto const ret = cond(
    env,
    [&] (Block* taken) {
      auto const ptr = gen(env, LdGblAddr, name);
      return gen(env, CheckNonNull, taken, ptr);
    },
    [&] (SSATmp* ptr) { // Next: global exists
      return gen(env, IsNTypeMem, TNull, ptr);
    },
    [&] { // Taken: global doesn't exist
      return cns(env, false);
    }
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

      auto const base = gen(
        env,
        LdRDSAddr,
        RDSHandleData { handle },
        TPtrToSPropCell
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

}}}
