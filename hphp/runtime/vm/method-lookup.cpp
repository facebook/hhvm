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

#include "hphp/runtime/vm/method-lookup.h"

#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/class.h"
#include "hphp/runtime/vm/named-entity.h"

#include "hphp/runtime/base/execution-context.h"
#include "hphp/runtime/base/string-data.h"
#include "hphp/runtime/base/runtime-error.h"
#include "hphp/runtime/base/object-data.h"

#include "hphp/util/assertions.h"

namespace HPHP {

namespace {

/*
 * Looks for a Func named methodName in any of the interfaces cls implements,
 * including cls if it is an interface. Returns nullptr if none was found,
 * or if its interface's vtableSlot is kInvalidSlot.
 */
const Func* lookupIfaceMethod(const Class* cls, const StringData* methodName) {

  auto checkOneInterface = [methodName](const Class* iface) -> const Func* {
    if (iface->preClass()->ifaceVtableSlot() == kInvalidSlot) return nullptr;

    const Func* func = iface->lookupMethod(methodName);
    always_assert(!func || func->cls() == iface);
    return func;
  };

  if (isInterface(cls)) {
    if (auto const func = checkOneInterface(cls)) return func;
  }

  for (auto pface : cls->allInterfaces().range()) {
    if (auto const func = checkOneInterface(pface)) return func;
  }

  return nullptr;
}

} // namespace

// Look up the method specified by methodName from the class specified by cls
// and enforce accessibility. Accessibility checks depend on the relationship
// between the class that first declared the method (baseClass) and the context
// class (ctx).
//
// If there are multiple accessible methods with the specified name declared in
// cls and ancestors of cls, the method from the most derived class will be
// returned, except if we are doing an ObjMethod call ("$obj->foo()") and there
// is an accessible private method, in which case the accessible private method
// will be returned.
//
// Accessibility rules:
//
//   | baseClass/ctx relationship | public | protected | private |
//   +----------------------------+--------+-----------+---------+
//   | anon/unrelated             | yes    | no        | no      |
//   | baseClass == ctx           | yes    | yes       | yes     |
//   | baseClass derived from ctx | yes    | yes       | no      |
//   | ctx derived from baseClass | yes    | yes       | no      |
//   +----------------------------+--------+-----------+---------+

const Func* lookupMethodCtx(const Class* cls,
                            const StringData* methodName,
                            const Class* ctx,
                            CallType callType,
                            bool raise) {
  const Func* method;
  if (callType == CallType::CtorMethod) {
    assertx(methodName == nullptr);
    method = cls->getCtor();
  } else {
    assertx(callType == CallType::ObjMethod || callType == CallType::ClsMethod);
    assertx(methodName != nullptr);
    method = cls->lookupMethod(methodName);
    if (!method) {
      // We didn't find any methods with the specified name in cls's method
      // table, handle the failure as appropriate.
      if (raise) {
        raise_call_to_undefined(methodName, cls);
      }
      return nullptr;
    }
  }
  assertx(method);
  bool accessible = true;
  // If we found a protected or private method, we need to do some
  // accessibility checks.
  if ((method->attrs() & (AttrProtected|AttrPrivate)) &&
      (g_context.isNull() || !g_context->debuggerSettings.bypassCheck)) {
    Class* baseClass = method->baseCls();
    assertx(baseClass);
    // If ctx is the class that first declared this method, then we know we
    // have the right method and we can stop here.
    if (ctx == baseClass) {
      return method;
    }
    // The invalid context cannot access protected or private methods,
    // so we can fail fast here.
    if (ctx == nullptr) {
      if (raise) {
        raise_error("Call to %s %s::%s() from invalid context",
                    (method->attrs() & AttrPrivate) ? "private" : "protected",
                    cls->name()->data(),
                    method->name()->data());
      }
      return nullptr;
    }
    assertx(ctx);
    if (method->attrs() & AttrPrivate) {
      // ctx is not the class that declared this private method, so this
      // private method is not accessible. We need to keep going because
      // ctx might define a private method with this name.
      accessible = false;
    } else {
      // If ctx is derived from the class that first declared this protected
      // method, then we know this method is accessible and thus (due to
      // semantic checks) we know ctx cannot have a private method with the
      // same name, so we're done.
      if (ctx->classof(baseClass)) {
        return method;
      }
      if (!baseClass->classof(ctx)) {
        // ctx is not related to the class that first declared this protected
        // method, so this method is not accessible. Because ctx is not the
        // same or an ancestor of the class which first declared the method,
        // we know that ctx not the same or an ancestor of cls, and therefore
        // we don't need to check if ctx declares a private method with this
        // name, so we can fail fast here.
        if (raise) {
          raise_error("Call to protected method %s::%s() from context '%s'",
                      cls->name()->data(),
                      method->name()->data(),
                      ctx->name()->data());
        }
        return nullptr;
      }
      // We now know this protected method is accessible, but we need to
      // keep going because ctx may define a private method with this name.
      assertx(accessible && baseClass->classof(ctx));
    }
  }
  // If this is an ObjMethod call ("$obj->foo()") AND there is an ancestor
  // of cls that declares a private method with this name AND ctx is an
  // ancestor of cls, we need to check if ctx declares a private method with
  // this name.
  if (method->hasPrivateAncestor() && callType == CallType::ObjMethod &&
      ctx && cls->classof(ctx)) {
    const Func* ctxMethod = ctx->lookupMethod(methodName);
    if (ctxMethod && ctxMethod->cls() == ctx &&
        (ctxMethod->attrs() & AttrPrivate)) {
      // For ObjMethod calls, a private method declared by ctx trumps
      // any other method we may have found.
      return ctxMethod;
    }
  }
  // If we found an accessible method in cls's method table, return it.
  if (accessible) {
    return method;
  }
  // If we reach here it means we've found an inaccessible private method
  // in cls's method table, handle the failure as appropriate.
  if (raise) {
    raise_error("Call to private method %s::%s() from %s'%s'",
                method->baseCls()->name()->data(),
                method->name()->data(),
                ctx ? "context " : "invalid context",
                ctx ? ctx->name()->data() : "");
  }
  return nullptr;
}

LookupResult lookupObjMethod(const Func*& f,
                             const Class* cls,
                             const StringData* methodName,
                             const Class* ctx,
                             bool raise) {
  f = lookupMethodCtx(cls, methodName, ctx, CallType::ObjMethod, false);
  if (!f) {
    f = cls->lookupMethod(s___call.get());
    if (!f) {
      if (raise) {
        // Throw a fatal error
        lookupMethodCtx(cls, methodName, ctx, CallType::ObjMethod, true);
      }
      return LookupResult::MethodNotFound;
    }
    return LookupResult::MagicCallFound;
  }
  if (f->isStaticInPrologue()) {
    return LookupResult::MethodFoundNoThis;
  }
  return LookupResult::MethodFoundWithThis;
}

ImmutableObjMethodLookup
lookupImmutableObjMethod(const Class* cls, const StringData* name,
                         const Class* ctx, bool exactClass) {
  auto constexpr notFound = ImmutableObjMethodLookup {
    ImmutableObjMethodLookup::Type::NotFound,
    nullptr
  };
  if (!cls) return notFound;
  exactClass |= cls->attrs() & AttrNoOverride;

  if (isInterface(cls)) {
    if (auto const func = lookupIfaceMethod(cls, name)) {
      return { ImmutableObjMethodLookup::Type::Interface, func };
    }
    return notFound;
  }

  const Func* func;
  LookupResult res = lookupObjMethod(func, cls, name, ctx, false);
  if (res == LookupResult::MethodNotFound) {
    if (exactClass) return notFound;
    if (auto const func = lookupIfaceMethod(cls, name)) {
      return { ImmutableObjMethodLookup::Type::Interface, func };
    }
    return notFound;
  }

  if (func->isAbstract() && exactClass) return notFound;

  assertx(res == LookupResult::MethodFoundWithThis ||
          res == LookupResult::MethodFoundNoThis ||
          res == LookupResult::MagicCallFound);

  if (res == LookupResult::MagicCallFound) {
    if (exactClass) return { ImmutableObjMethodLookup::Type::MagicFunc, func };
    return notFound;
  }

  if (exactClass || func->attrs() & AttrPrivate || func->isImmutableFrom(cls)) {
    return { ImmutableObjMethodLookup::Type::Func, func };
  }

  return { ImmutableObjMethodLookup::Type::Class, func };
}

LookupResult lookupClsMethod(const Func*& f,
                             const Class* cls,
                             const StringData* methodName,
                             ObjectData* obj,
                             const Class* ctx,
                             bool raise) {
  f = lookupMethodCtx(cls, methodName, ctx, CallType::ClsMethod, false);
  if (!f) {
    if (obj && obj->instanceof(cls)) {
      f = obj->getVMClass()->lookupMethod(s___call.get());
    }
    if (!f) {
      if (raise) {
        // Throw a fatal error
        lookupMethodCtx(cls, methodName, ctx, CallType::ClsMethod, true);
      }
      return LookupResult::MethodNotFound;
    }
    assertx(f);
    assertx(obj);
    // __call cannot be static, this should be enforced by semantic
    // checks defClass time or earlier
    assertx(!(f->attrs() & AttrStatic));
    return LookupResult::MagicCallFound;
  }
  if (obj && !(f->attrs() & AttrStatic) && obj->instanceof(cls)) {
    return LookupResult::MethodFoundWithThis;
  }
  return LookupResult::MethodFoundNoThis;
}

const Func* lookupImmutableClsMethod(const Class* cls, const StringData* name,
                                     const Class* ctx, bool exactClass) {
  if (!cls) return nullptr;
  if (cls->attrs() & AttrInterface) return nullptr;
  const Func* func;
  LookupResult res = lookupClsMethod(func, cls, name, nullptr, ctx, false);
  if (res == LookupResult::MethodNotFound) return nullptr;
  if (func->isAbstract() && exactClass) return nullptr;

  assertx(res == LookupResult::MethodFoundWithThis ||
          res == LookupResult::MethodFoundNoThis);
  return func;
}

LookupResult lookupCtorMethod(const Func*& f,
                              const Class* cls,
                              const Class* ctx,
                              bool raise) {
  f = cls->getCtor();
  if (!(f->attrs() & AttrPublic)) {
    f = lookupMethodCtx(cls, nullptr, ctx, CallType::CtorMethod, raise);
    if (!f) {
      // If raise was true than lookupMethodCtx should have thrown,
      // so we should only be able to get here if raise was false
      assertx(!raise);
      return LookupResult::MethodNotFound;
    }
  }
  return LookupResult::MethodFoundWithThis;
}

const Func* lookupImmutableCtor(const Class* cls, const Class* ctx) {
  if (!cls) return nullptr;

  auto const func = cls->getCtor();
  if (func && !(func->attrs() & AttrPublic)) {
    auto fcls = func->cls();
    if (fcls != ctx) {
      if (!ctx) return nullptr;
      if ((func->attrs() & AttrPrivate) ||
          !(ctx->classof(fcls) || fcls->classof(ctx))) {
        return nullptr;
      }
    }
  }

  return func;
}

ImmutableFuncLookup lookupImmutableFunc(const Unit* unit,
                                        const StringData* name) {
  auto const ne = NamedEntity::get(name);
  if (auto const f = ne->uniqueFunc()) {
    // We have an unique function. However, it may be conditionally defined
    // (!f->top()) or interceptable, which means we can't use it directly.
    if (!f->top() || f->isInterceptable()) return {nullptr, true};
    // We can use this function. If its persistent (which means its unit's
    // pseudo-main is trivial), its safe to use unconditionally. If its defined
    // in the same unit as the caller, its also safe to use unconditionally. By
    // virtue of the fact that we're already in the unit, we know its already
    // defined.
    if (f->isPersistent() || f->unit() == unit) {
      if (!RO::EvalJitEnableRenameFunction || f->isMethCaller()) {
        return {f, false};
      }
    } else if (RO::EvalJitEnableRenameFunction) {
      return {nullptr, true};
    }
    // Use the function, but ensure its unit is loaded.
    return {f, true};
  }

  // Trust nothing if we can rename functions
  if (RuntimeOption::EvalJitEnableRenameFunction) return {nullptr, true};

  // There's no unique function currently known for this name. However, if the
  // current unit defines a single top-level function with this name, we can use
  // it. Why? When this unit is loaded, either it successfully defined the
  // function, in which case its the correct function, or it fataled, which
  // means this code won't run anyways.

  Func* found = nullptr;
  for (auto& f : unit->funcs()) {
    if (f->isPseudoMain()) continue;
    if (!f->name()->isame(name)) continue;
    if (found) {
      // Function with duplicate name
      found = nullptr;
      break;
    }
    found = f;
  }

  if (found && found->top() && !found->isInterceptable()) {
    return {found, false};
  }
  return {nullptr, true};
}

}
