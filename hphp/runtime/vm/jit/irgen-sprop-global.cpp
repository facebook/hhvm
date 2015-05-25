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
#include "hphp/runtime/vm/jit/irgen-sprop-global.h"

#include "hphp/runtime/vm/jit/irgen-create.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

void bindMem(IRGS& env, SSATmp* ptr, SSATmp* src) {
  auto const prevValue = gen(env, LdMem, ptr->type().deref(), ptr);
  pushIncRef(env, src);
  gen(env, StMem, ptr, src);
  gen(env, DecRef, prevValue);
}

void destroyName(IRGS& env, SSATmp* name) {
  assertx(name == topC(env));
  popDecRef(env);
}

//////////////////////////////////////////////////////////////////////

}

//////////////////////////////////////////////////////////////////////

SSATmp* ldClsPropAddrKnown(IRGS& env,
                           const Class* cls,
                           const StringData* name) {
  initSProps(env, cls); // calls init; must be above sPropHandle()
  auto const slot = cls->lookupSProp(name);
  auto const handle = cls->sPropHandle(slot);
  auto const repoTy =
    !RuntimeOption::RepoAuthoritative
      ? RepoAuthType{}
      : cls->staticPropRepoAuthType(slot);
  auto const ptrTy = typeFromRAT(repoTy).ptr(Ptr::SProp);
  return gen(env, LdRDSAddr, RDSHandleData { handle }, ptrTy);
}

SSATmp* ldClsPropAddr(IRGS& env, SSATmp* ssaCls, SSATmp* ssaName, bool raise) {
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
    if (!classIsPersistentOrCtxParent(env, cls)) return false;

    auto const lookup = cls->findSProp(curClass(env), propName);
    return lookup.prop != kInvalidSlot && lookup.accessible;
  }();

  if (sPropKnown) {
    return ldClsPropAddrKnown(env, ssaCls->clsVal(), ssaName->strVal());
  }

  return gen(
    env,
    raise ? LdClsPropAddrOrRaise : LdClsPropAddrOrNull,
    ssaCls,
    ssaName,
    cns(env, curClass(env))
  );
}

//////////////////////////////////////////////////////////////////////

void emitCGetS(IRGS& env) {
  auto const ssaPropName = topC(env, BCSPOffset{1});

  if (!ssaPropName->isA(TStr)) {
    PUNT(CGetS-PropNameNotString);
  }

  auto const ssaCls   = popA(env);
  auto const propAddr = ldClsPropAddr(env, ssaCls, ssaPropName, true);
  auto const unboxed  = gen(env, UnboxPtr, propAddr);
  auto const ldMem    = gen(env, LdMem, unboxed->type().deref(), unboxed);

  destroyName(env, ssaPropName);
  pushIncRef(env, ldMem);
}

void emitSetS(IRGS& env) {
  auto const ssaPropName = topC(env, BCSPOffset{2});

  if (!ssaPropName->isA(TStr)) {
    PUNT(SetS-PropNameNotString);
  }

  auto const value    = popC(env, DataTypeCountness);
  auto const ssaCls   = popA(env);
  auto const propAddr = ldClsPropAddr(env, ssaCls, ssaPropName, true);
  auto const ptr      = gen(env, UnboxPtr, propAddr);

  destroyName(env, ssaPropName);
  bindMem(env, ptr, value);
}

void emitVGetS(IRGS& env) {
  auto const ssaPropName = topC(env, BCSPOffset{1});

  if (!ssaPropName->isA(TStr)) {
    PUNT(VGetS-PropNameNotString);
  }

  auto const ssaCls   = popA(env);
  auto const propAddr = ldClsPropAddr(env, ssaCls, ssaPropName, true);

  destroyName(env, ssaPropName);
  auto const val = gen(
    env,
    LdMem,
    TBoxedInitCell,
    gen(env, BoxPtr, propAddr)
  );
  pushIncRef(env, val);
}

void emitBindS(IRGS& env) {
  auto const ssaPropName = topC(env, BCSPOffset{2});

  if (!ssaPropName->isA(TStr)) {
    PUNT(BindS-PropNameNotString);
  }

  auto const value    = popV(env);
  auto const ssaCls   = popA(env);
  auto const propAddr = ldClsPropAddr(env, ssaCls, ssaPropName, true);

  destroyName(env, ssaPropName);
  bindMem(env, propAddr, value);
}

void emitIssetS(IRGS& env) {
  auto const ssaPropName = topC(env, BCSPOffset{1});
  if (!ssaPropName->isA(TStr)) {
    PUNT(IssetS-PropNameNotString);
  }
  auto const ssaCls = popA(env);

  auto const ret = cond(
    env,
    [&] (Block* taken) {
      auto propAddr = ldClsPropAddr(env, ssaCls, ssaPropName, false);
      return gen(env, CheckNonNull, taken, propAddr);
    },
    [&] (SSATmp* ptr) { // Next: property or global exists
      return gen(env, IsNTypeMem, TNull, gen(env, UnboxPtr, ptr));
    },
    [&] { // Taken: LdClsPropAddr* returned Nullptr because it isn't defined
      return cns(env, false);
    }
  );

  destroyName(env, ssaPropName);
  push(env, ret);
}

void emitEmptyS(IRGS& env) {
  auto const ssaPropName = topC(env, BCSPOffset{1});
  if (!ssaPropName->isA(TStr)) {
    PUNT(EmptyS-PropNameNotString);
  }

  auto const ssaCls = popA(env);
  auto const ret = cond(
    env,
    [&] (Block* taken) {
      auto propAddr = ldClsPropAddr(env, ssaCls, ssaPropName, false);
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

  destroyName(env, ssaPropName);
  push(env, ret);
}

//////////////////////////////////////////////////////////////////////

void emitCGetG(IRGS& env) {
  auto const exit = makeExitSlow(env);
  auto const name = topC(env);
  if (!name->isA(TStr)) PUNT(CGetG-NonStrName);
  auto const ptr = gen(env, LdGblAddr, exit, name);
  destroyName(env, name);
  pushIncRef(
    env,
    gen(env, LdMem, TCell, gen(env, UnboxPtr, ptr))
  );
}

void emitVGetG(IRGS& env) {
  auto const name = topC(env);
  if (!name->isA(TStr)) PUNT(VGetG-NonStrName);
  auto const ptr = gen(env, LdGblAddrDef, name);
  destroyName(env, name);
  pushIncRef(
    env,
    gen(env, LdMem, TBoxedInitCell, gen(env, BoxPtr, ptr))
  );
}

void emitBindG(IRGS& env) {
  auto const name = topC(env, BCSPOffset{1});
  if (!name->isA(TStr)) PUNT(BindG-NameNotStr);
  auto const box = popV(env);
  auto const ptr = gen(env, LdGblAddrDef, name);
  destroyName(env, name);
  bindMem(env, ptr, box);
}

void emitSetG(IRGS& env) {
  auto const name = topC(env, BCSPOffset{1});
  if (!name->isA(TStr)) PUNT(SetG-NameNotStr);
  auto const value   = popC(env, DataTypeCountness);
  auto const unboxed = gen(env, UnboxPtr, gen(env, LdGblAddrDef, name));
  destroyName(env, name);
  bindMem(env, unboxed, value);
}

void emitIssetG(IRGS& env) {
  auto const name = topC(env, BCSPOffset{0});
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
  auto const idx = ctx->lookupDeclProp(propName);

  auto const curVal = gen(env, LdElem, propInitVec,
    cns(env, idx * sizeof(TypedValue)));
  push(env, gen(env, IsNType, TUninit, curVal));
}

void emitInitProp(IRGS& env, const StringData* propName, InitPropOp op) {
  auto const val = popC(env);
  auto const ctx = curClass(env);

  SSATmp* base;
  Slot idx = 0;
  switch (op) {
  case InitPropOp::Static:
    {
      // For sinit, the context class is always the same as the late-bound
      // class, so we can just use curClass().
      auto const handle = ctx->sPropHandle(ctx->lookupSProp(propName));
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

      base = gen(env, LdClsInitData, cls);
      idx = ctx->lookupDeclProp(propName);
    }
    break;
  }

  gen(env, StElem, base, cns(env, idx * sizeof(TypedValue)), val);
}

//////////////////////////////////////////////////////////////////////

}}}
