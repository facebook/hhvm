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

#include "hphp/runtime/vm/jit/irgen-create.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-incdec.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"
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

  auto knownType = TGen;
  // AttrLateInitSoft properties can have default values which can be anything,
  // so don't try to infer the type of them.
  if (!(prop.attrs & AttrLateInitSoft)) {
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
  } else if (!ignoreLateInit) {
    // If we want the AttrLateInitSoft default value behavior here, fallback to
    // the runtime helpers instead.
    return { nullptr, nullptr, 0 };
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
    slot,
  };

  static_assert(sizeof(StaticPropData) == sizeof(TypedValue),
                "StaticPropData expected to only wrap TypedValue");
}

ClsPropLookup ldClsPropAddr(IRGS& env, SSATmp* ssaCls,
                            SSATmp* ssaName, bool raise,
                            bool ignoreLateInit,
                            bool disallowConst) {
  assertx(ssaCls->isA(TCls));
  assertx(ssaName->isA(TStr));

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
    if (disallowConst && lookup.constant) return false;
    return true;
  }();

  if (sPropKnown) {
    auto const lookup = ldClsPropAddrKnown(
      env,
      ssaCls->clsVal(),
      ssaName->strVal(),
      ignoreLateInit
    );
    if (lookup.propPtr) return lookup;
  }

  auto const propAddr = gen(
    env,
    raise ? LdClsPropAddrOrRaise : LdClsPropAddrOrNull,
    ssaCls,
    ssaName,
    cns(env, curClass(env)),
    cns(env, ignoreLateInit),
    cns(env, disallowConst)
  );
  return { propAddr, nullptr, kInvalidSlot };
}

//////////////////////////////////////////////////////////////////////

void emitCGetS(IRGS& env) {
  auto const ssaCls      = topC(env);
  auto const ssaPropName = topC(env, BCSPRelOffset{1});

  if (!ssaPropName->isA(TStr)) PUNT(CGetS-PropNameNotString);
  if (!ssaCls->isA(TCls))      PUNT(EmptyS-NotClass);

  auto const propAddr  =
    ldClsPropAddr(env, ssaCls, ssaPropName, true, false, false).propPtr;
  auto const unboxed   = gen(env, UnboxPtr, propAddr);
  auto const ldMem     = gen(env, LdMem, unboxed->type().deref(), unboxed);

  discard(env);
  destroyName(env, ssaPropName);
  pushIncRef(env, ldMem);
}

void emitSetS(IRGS& env) {
  auto const ssaCls      = topC(env, BCSPRelOffset{1});
  auto const ssaPropName = topC(env, BCSPRelOffset{2});

  if (!ssaPropName->isA(TStr)) PUNT(SetS-PropNameNotString);
  if (!ssaCls->isA(TCls))      PUNT(SetS-NotClass);

  auto const value  = popC(env, DataTypeCountness);
  auto const lookup = ldClsPropAddr(env, ssaCls, ssaPropName, true, true, true);

  if (lookup.tc) {
    verifyPropType(
      env,
      ssaCls,
      lookup.tc,
      lookup.slot,
      value,
      ssaPropName,
      true
    );
  } else if (RuntimeOption::EvalCheckPropTypeHints > 0) {
    auto const slot = gen(env, LookupSPropSlot, ssaCls, ssaPropName);
    gen(env, VerifyProp, ssaCls, slot, value, cns(env, true));
  }

  auto const ptr = gen(env, UnboxPtr, lookup.propPtr);

  discard(env);
  destroyName(env, ssaPropName);
  bindMem(env, ptr, value);
}

void emitIssetS(IRGS& env) {
  auto const ssaCls      = topC(env);
  auto const ssaPropName = topC(env, BCSPRelOffset{1});

  if (!ssaPropName->isA(TStr)) PUNT(IssetS-PropNameNotString);
  if (!ssaCls->isA(TCls))      PUNT(IssetS-NotClass);

  auto const ret = cond(
    env,
    [&] (Block* taken) {
      auto const propAddr =
        ldClsPropAddr(env, ssaCls, ssaPropName, false, true, false).propPtr;
      return gen(env, CheckNonNull, taken, propAddr);
    },
    [&] (SSATmp* ptr) { // Next: property or global exists
      return gen(env, IsNTypeMem, TNull, gen(env, UnboxPtr, ptr));
    },
    [&] { // Taken: LdClsPropAddr* returned Nullptr because it isn't defined
      return cns(env, false);
    }
  );

  discard(env);
  destroyName(env, ssaPropName);
  push(env, ret);
}

void emitEmptyS(IRGS& env) {
  auto const ssaCls      = topC(env);
  auto const ssaPropName = topC(env, BCSPRelOffset{1});

  if (!ssaPropName->isA(TStr)) PUNT(EmptyS-PropNameNotString);
  if (!ssaCls->isA(TCls))      PUNT(EmptyS-NotClass);

  auto const ret = cond(
    env,
    [&] (Block* taken) {
      auto const propAddr =
        ldClsPropAddr(env, ssaCls, ssaPropName, false, true, false).propPtr;
      return gen(env, CheckNonNull, taken, propAddr);
    },
    [&] (SSATmp* ptr) {
      auto const unbox = gen(env, UnboxPtr, ptr);
      auto const val   = gen(env, LdMem, unbox->type().deref(), unbox);
      return gen(env, XorBool, gen(env, ConvCellToBool, val), cns(env, true));
    },
    [&] { // Taken: LdClsPropAddr* returned Nullptr because it isn't defined
      return cns(env, true);
    });

  discard(env);
  destroyName(env, ssaPropName);
  push(env, ret);
}

void emitIncDecS(IRGS& env, IncDecOp subop) {
  auto const ssaCls      = topC(env);
  auto const ssaPropName = topC(env, BCSPRelOffset{1});

  if (!ssaPropName->isA(TStr)) PUNT(IncDecS-PropNameNotString);
  if (!ssaCls->isA(TCls))      PUNT(IncDecS-NotClass);

  auto const lookup  =
    ldClsPropAddr(env, ssaCls, ssaPropName, true, false, true);
  auto const unboxed = gen(env, UnboxPtr, lookup.propPtr);
  auto const oldVal  = gen(env, LdMem, unboxed->type().deref(), unboxed);

  auto const result = incDec(env, subop, oldVal);
  if (!result) PUNT(IncDecS);
  assertx(result->isA(TUncounted));

  if (lookup.tc) {
    verifyPropType(
      env,
      ssaCls,
      lookup.tc,
      lookup.slot,
      result,
      ssaPropName,
      true
    );
  } else if (RuntimeOption::EvalCheckPropTypeHints > 0) {
    auto const slot = gen(env, LookupSPropSlot, ssaCls, ssaPropName);
    gen(env, VerifyProp, ssaCls, slot, result, cns(env, true));
  }

  discard(env);
  destroyName(env, ssaPropName);
  pushIncRef(env, isPre(subop) ? result : oldVal);

  // Update marker to ensure newly-pushed value isn't clobbered by DecRef.
  updateMarker(env);

  gen(env, StMem, unboxed, result);
  gen(env, IncRef, result);
  decRef(env, oldVal);
}

//////////////////////////////////////////////////////////////////////

void emitCGetG(IRGS& env) {
  auto const name = topC(env);
  if (!name->isA(TStr)) PUNT(CGetG-NonStrName);

  auto ret = cond(
    env,
    [&] (Block* taken) { return gen(env, LdGblAddr, taken, name); },
    [&] (SSATmp* ptr) {
      auto tmp = gen(env, LdMem, TCell, gen(env, UnboxPtr, ptr));
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
  auto const value   = popC(env, DataTypeCountness);
  auto const unboxed = gen(env, UnboxPtr, gen(env, LdGblAddrDef, name));
  destroyName(env, name);
  bindMem(env, unboxed, value);
}

void emitIssetG(IRGS& env) {
  auto const name = topC(env, BCSPRelOffset{0});
  if (!name->isA(TStr)) PUNT(IssetG-NameNotStr);

  auto const ret = cond(
    env,
    [&] (Block* taken) {
      return gen(env, LdGblAddr, taken, name);
    },
    [&] (SSATmp* ptr) { // Next: global exists
      return gen(env, IsNTypeMem, TNull, gen(env, UnboxPtr, ptr));
    },
    [&] { // Taken: global doesn't exist
      return cns(env, false);
    }
  );
  destroyName(env, name);
  push(env, ret);
}

void emitEmptyG(IRGS& env) {
  auto const name = topC(env);
  if (!name->isA(TStr)) PUNT(EmptyG-NameNotStr);

  auto const ret = cond(
    env,
    [&] (Block* taken) {
      return gen(env, LdGblAddr, taken, name);
    },
    [&] (SSATmp* ptr) { // Next: global exists
      auto const unboxed = gen(env, UnboxPtr, ptr);
      auto const val     = gen(env, LdMem, TCell, unboxed);
      return gen(env, XorBool, gen(env, ConvCellToBool, val), cns(env, true));
    },
    [&] { // Taken: global doesn't exist
      return cns(env, true);
    });
  destroyName(env, name);
  push(env, ret);
}

//////////////////////////////////////////////////////////////////////

void emitCheckProp(IRGS& env, const StringData* propName) {
  auto const cctx = gen(env, LdCctx, fp(env));
  auto const cls = gen(env, LdClsCtx, cctx);
  auto const propInitVec = gen(env, LdClsInitData, cls);

  auto const ctx = curClass(env);
  auto const slot = ctx->lookupDeclProp(propName);
  auto const idx = ctx->propSlotToIndex(slot);

  auto const curVal = gen(env, LdElem, propInitVec,
    cns(env, idx * sizeof(TypedValue)));
  push(env, gen(env, IsNType, TUninit, curVal));
}

void emitInitProp(IRGS& env, const StringData* propName, InitPropOp op) {
  auto const val = popC(env);
  auto const ctx = curClass(env);

  SSATmp* base;
  uint32_t idx = 0;
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
          slot,
          val,
          cns(env, propName),
          true
        );
      }

      base = gen(
        env,
        LdRDSAddr,
        RDSHandleData { handle },
        TPtrToSPropCell
      );
    }
    break;

  case InitPropOp::NonStatic:
    {
      // The above is not the case for pinit, so we need to load.
      auto const cctx = gen(env, LdCctx, fp(env));
      auto const cls = gen(env, LdClsCtx, cctx);

      const auto slot = ctx->lookupDeclProp(propName);
      idx = ctx->propSlotToIndex(slot);
      auto const& prop = ctx->declProperties()[slot];
      assertx(!(prop.attrs & AttrSystemInitialValue));
      if (!(prop.attrs & AttrInitialSatisfiesTC)) {
        verifyPropType(
          env,
          cls,
          &prop.typeConstraint,
          slot,
          val,
          cns(env, propName),
          false
        );
      }

      base = gen(env, LdClsInitData, cls);
    }
    break;
  }

  gen(env, StElem, base, cns(env, idx * sizeof(TypedValue)), val);
}

//////////////////////////////////////////////////////////////////////

}}}
