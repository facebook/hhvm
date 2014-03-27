/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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
#ifndef incl_HPHP_INTERP_INTERNAL_H_
#define incl_HPHP_INTERP_INTERNAL_H_

#include <algorithm>

#include "folly/Optional.h"

#include "hphp/hhbbc/interp-state.h"
#include "hphp/hhbbc/interp.h"
#include "hphp/hhbbc/representation.h"
#include "hphp/hhbbc/type-system.h"

namespace HPHP { namespace HHBBC {

//////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(hhbbc);

const StaticString s_extract("extract");
const StaticString s_compact("compact");
const StaticString s_get_defined_vars("get_defined_vars");

//////////////////////////////////////////////////////////////////////

/*
 * Interpreter Step State.
 *
 * This struct is gives interpreter functions access to shared state.
 * It's not in interp-state.h because it's part of the internal
 * implementation of interpreter routines.  The publicized state as
 * results of interpretation are in that header and interp.h.
 */
struct ISS {
  explicit ISS(Interp& bag, StepFlags& flags, PropagateFn propagate)
    : index(bag.index)
    , ctx(bag.ctx)
    , props(bag.props)
    , blk(*bag.blk)
    , state(bag.state)
    , flags(flags)
    , propagate(propagate)
  {}

  const Index& index;
  const Context ctx;
  PropertiesInfo& props;
  const php::Block& blk;
  State& state;
  StepFlags& flags;
  PropagateFn propagate;
};

//////////////////////////////////////////////////////////////////////

namespace {

void nothrow(ISS& env) {
  FTRACE(2, "    nothrow\n");
  env.flags.wasPEI = false;
}

void calledNoReturn(ISS& env)    { env.flags.calledNoReturn = true; }
void constprop(ISS& env)         { env.flags.canConstProp = true; }
void nofallthrough(ISS& env)     { env.flags.tookBranch = true; }
void readUnknownLocals(ISS& env) { env.flags.mayReadLocalSet.set(); }
void readAllLocals(ISS& env)     { env.flags.mayReadLocalSet.set(); }

void doRet(ISS& env, Type t) {
  readAllLocals(env);
  assert(env.state.stack.empty());
  env.flags.returned = t;
}

void killLocals(ISS& env) {
  FTRACE(2, "    killLocals\n");
  readUnknownLocals(env);
  for (auto& l : env.state.locals) l = TGen;
}

// Force non-ref locals to TCell.  Used when something modifies an
// unknown local's value, without changing reffiness.
void loseNonRefLocalTypes(ISS& env) {
  readUnknownLocals(env);
  FTRACE(2, "    loseNonRefLocalTypes\n");
  for (auto& l : env.state.locals) {
    if (l.subtypeOf(TCell)) l = TCell;
  }
}

void boxUnknownLocal(ISS& env) {
  readUnknownLocals(env);
  FTRACE(2, "   boxUnknownLocal\n");
  for (auto& l : env.state.locals) {
    if (!l.subtypeOf(TRef)) l = TGen;
  }
}

void unsetUnknownLocal(ISS& env) {
  readUnknownLocals(env);
  FTRACE(2, "  unsetUnknownLocal\n");
  for (auto& l : env.state.locals) l = union_of(l, TUninit);
}

void unsetLocals(ISS& env) {
  for (auto i = size_t{0}; i < env.state.locals.size(); ++i) {
    env.state.locals[i] = TUninit;
  }
}

void specialFunctionEffects(ISS& env, SString name) {
  // extract() trashes the local variable environment.
  if (name->isame(s_extract.get())) {
    readUnknownLocals(env);
    killLocals(env);
    return;
  }
  // compact() and get_defined_vars() read the local variable
  // environment.  We could check which locals for compact, but for
  // now we just include them all.
  if (name->isame(s_compact.get()) ||
      name->isame(s_get_defined_vars.get())) {
    readUnknownLocals(env);
    return;
  }
}

void specialFunctionEffects(ISS& env, ActRec ar) {
  switch (ar.kind) {
  case FPIKind::Unknown:
    // fallthrough
  case FPIKind::Func:
    // Could be a dynamic call to extract, get_defined_vars, etc:
    if (!ar.func) {
      readUnknownLocals(env);
      killLocals(env);
      return;
    }
    specialFunctionEffects(env, ar.func->name());
    break;
  case FPIKind::Ctor:
  case FPIKind::ObjMeth:
  case FPIKind::ClsMeth:
  case FPIKind::ObjInvoke:
  case FPIKind::CallableArr:
    break;
  }
}

//////////////////////////////////////////////////////////////////////
// eval stack

Type popT(ISS& env) {
  assert(!env.state.stack.empty());
  auto const ret = env.state.stack.back();
  FTRACE(2, "    pop:  {}\n", show(ret));
  env.state.stack.pop_back();
  return ret;
}

Type popC(ISS& env) {
  auto const v = popT(env);
  assert(v.subtypeOf(TInitCell)); // or it would be popU, which doesn't exist
  return v;
}

Type popV(ISS& env) {
  auto const v = popT(env);
  assert(v.subtypeOf(TRef));
  return v;
}

Type popA(ISS& env) {
  auto const v = popT(env);
  assert(v.subtypeOf(TCls));
  return v;
}

Type popR(ISS& env)  { return popT(env); }
Type popF(ISS& env)  { return popT(env); }
Type popCV(ISS& env) { return popT(env); }
Type popU(ISS& env)  { return popT(env); }

void popFlav(ISS& env, Flavor flav) {
  switch (flav) {
  case Flavor::C: popC(env); break;
  case Flavor::V: popV(env); break;
  case Flavor::U: popU(env); break;
  case Flavor::F: popF(env); break;
  case Flavor::R: popR(env); break;
  case Flavor::A: popA(env); break;
  }
}

Type topT(ISS& env, uint32_t idx = 0) {
  assert(idx < env.state.stack.size());
  return env.state.stack[env.state.stack.size() - idx - 1];
}

Type topA(ISS& env, uint32_t i = 0) {
  assert(topT(env, i).subtypeOf(TCls));
  return topT(env, i);
}

Type topC(ISS& env, uint32_t i = 0) {
  assert(topT(env, i).subtypeOf(TInitCell));
  return topT(env, i);
}

Type topR(ISS& env, uint32_t i = 0) { return topT(env, i); }

Type topV(ISS& env, uint32_t i = 0) {
  assert(topT(env, i).subtypeOf(TRef));
  return topT(env, i);
}

void push(ISS& env, Type t) {
  FTRACE(2, "    push: {}\n", show(t));
  env.state.stack.push_back(t);
}

//////////////////////////////////////////////////////////////////////
// fpi

void fpiPush(ISS& env, ActRec ar) {
  FTRACE(2, "    fpi+: {}\n", show(ar));
  env.state.fpiStack.push_back(ar);
}

ActRec fpiPop(ISS& env) {
  assert(!env.state.fpiStack.empty());
  auto const ret = env.state.fpiStack.back();
  FTRACE(2, "    fpi-: {}\n", show(ret));
  env.state.fpiStack.pop_back();
  return ret;
}

ActRec fpiTop(ISS& env) {
  assert(!env.state.fpiStack.empty());
  return env.state.fpiStack.back();
}

PrepKind prepKind(ISS& env, uint32_t paramId) {
  auto ar = fpiTop(env);
  if (ar.func) return env.index.lookup_param_prep(env.ctx, *ar.func, paramId);
  return PrepKind::Unknown;
}

//////////////////////////////////////////////////////////////////////
// locals

void mayReadLocal(ISS& env, uint32_t id) {
  if (id < env.flags.mayReadLocalSet.size()) {
    env.flags.mayReadLocalSet.set(id);
  }
}

Type locRaw(ISS& env, borrowed_ptr<const php::Local> l) {
  mayReadLocal(env, l->id);
  return env.state.locals[l->id];
}

void setLocRaw(ISS& env, borrowed_ptr<const php::Local> l, Type t) {
  mayReadLocal(env, l->id);
  env.state.locals[l->id] = t;
}

// Read a local type in the sense of CGetL.  (TUninits turn into
// TInitNull, and potentially reffy types return the "inner" type,
// which is always a subtype of InitCell.)
Type locAsCell(ISS& env, borrowed_ptr<const php::Local> l) {
  auto t = locRaw(env, l);
  return !t.subtypeOf(TCell) ? TInitCell :
          t.subtypeOf(TUninit) ? TInitNull :
          remove_uninit(t);
}

// Read a local type, dereferencing refs, but without converting
// potential TUninits to TInitNull.
Type derefLoc(ISS& env, borrowed_ptr<const php::Local> l) {
  auto v = locRaw(env, l);
  if (v.subtypeOf(TCell)) return v;
  return v.couldBe(TUninit) ? TCell : TInitCell;
}

void ensureInit(ISS& env, borrowed_ptr<const php::Local> l) {
  auto t = locRaw(env, l);
  if (t.couldBe(TUninit)) {
    if (t.subtypeOf(TUninit)) return setLocRaw(env, l, TInitNull);
    if (t.subtypeOf(TCell))   return setLocRaw(env, l, remove_uninit(t));
    setLocRaw(env, l, TInitGen);
  }
}

bool locCouldBeUninit(ISS& env, borrowed_ptr<const php::Local> l) {
  return locRaw(env, l).couldBe(TUninit);
}

/*
 * Set a local type in the sense of tvSet.  If the local is boxed or
 * not known to be not boxed, we can't change the type.  May be used
 * to set locals to types that include Uninit.
 */
void setLoc(ISS& env, borrowed_ptr<const php::Local> l, Type t) {
  auto v = locRaw(env, l);
  if (v.subtypeOf(TCell)) env.state.locals[l->id] = t;
}

borrowed_ptr<php::Local> findLocal(ISS& env, SString name) {
  for (auto& l : env.ctx.func->locals) {
    if (l->name->same(name)) {
      mayReadLocal(env, l->id);
      return borrow(l);
    }
  }
  return nullptr;
}

borrowed_ptr<php::Local> findLocalById(ISS& env, int32_t id) {
  assert(id < env.ctx.func->locals.size());
  mayReadLocal(env, id);
  return borrow(env.ctx.func->locals[id]);
}

//////////////////////////////////////////////////////////////////////
// iterators

void setIter(ISS& env, borrowed_ptr<php::Iter> iter, Iter iterState) {
  env.state.iters[iter->id] = std::move(iterState);
}
void freeIter(ISS& env, borrowed_ptr<php::Iter> iter) {
  env.state.iters[iter->id] = UnknownIter {};
}

//////////////////////////////////////////////////////////////////////
// $this

void setThisAvailable(ISS& env) {
  FTRACE(2, "    setThisAvailable\n");
  env.state.thisAvailable = true;
}

bool thisAvailable(ISS& env) { return env.state.thisAvailable; }

// Returns the type $this would have if it's not null.  Generally
// you have to check thisIsAvailable() before assuming it can't be
// null.
folly::Optional<Type> thisType(ISS& env) {
  if (!env.ctx.cls) return folly::none;
  if (auto const rcls = env.index.resolve_class(env.ctx, env.ctx.cls->name)) {
    return subObj(*rcls);
  }
  return folly::none;
}

folly::Optional<Type> selfCls(ISS& env) {
  if (!env.ctx.cls) return folly::none;
  if (auto const rcls = env.index.resolve_class(env.ctx, env.ctx.cls->name)) {
    return subCls(*rcls);
  }
  return folly::none;
}

//////////////////////////////////////////////////////////////////////
// properties on $this

/*
 * Note: we are only tracking control-flow insensitive types for
 * object properties, because it can be pretty rough to try to track
 * all cases that could re-enter the VM, run arbitrary code, and
 * potentially change the type of a property.
 *
 * Because of this, the various "setter" functions for thisProps
 * here actually just union the new type into what we already had.
 */

Type* thisPropRaw(ISS& env, SString name) {
  auto& privateProperties = env.props.privateProperties();
  auto const it = privateProperties.find(name);
  if (it != end(privateProperties)) {
    return &it->second;
  }
  return nullptr;
}

bool isTrackedThisProp(ISS& env, SString name) {
  return thisPropRaw(env, name);
}

void killThisProps(ISS& env) {
  FTRACE(2, "    killThisProps\n");
  for (auto& kv : env.props.privateProperties()) kv.second = TGen;
}

/*
 * This function returns a type that includes all the possible types
 * that could result from reading a property $this->name.
 *
 * Note that this may include types that the property itself cannot
 * actually contain, due to the effects of a possible __get
 * function.  For now we handle that case by just returning
 * InitCell, rather than detecting if $this could have a magic
 * getter.  TODO(#3669480).
 */
folly::Optional<Type> thisPropAsCell(ISS& env, SString name) {
  auto const t = thisPropRaw(env, name);
  if (!t) return folly::none;

  if (t->couldBe(TUninit)) {
    // Could come out of __get.
    return TInitCell;
  }
  if (t->subtypeOf(TCell)) return *t;
  return TInitCell;
}

/*
 * Merge a type into the track property types on $this, in the sense
 * of tvSet (i.e. setting the inner type on possible refs).
 *
 * Note that all types we see that could go into an object property
 * have to loosen_statics and loosen_values.  This is because the
 * object could be serialized and then deserialized, losing the
 * static-ness of a string or array member, and we don't guarantee
 * deserialization would preserve a constant value object property
 * type.
 */
void mergeThisProp(ISS& env, SString name, Type type) {
  auto const t = thisPropRaw(env, name);
  if (!t) return;
  *t = union_of(*t, loosen_statics(loosen_values(type)));
}

/*
 * Merge something into each this prop.  Usually MapFn will be a
 * predicate that returns TBottom when some condition doesn't hold.
 *
 * The types given to the map function are the raw tracked types
 * (i.e. could be TRef or TUninit).
 */
template<class MapFn>
void mergeEachThisPropRaw(ISS& env, MapFn fn) {
  for (auto& kv : env.props.privateProperties()) {
    mergeThisProp(env, kv.first, fn(kv.second));
  }
}

void unsetThisProp(ISS& env, SString name) {
  mergeThisProp(env, name, TUninit);
}

void unsetUnknownThisProp(ISS& env) {
  for (auto& kv : env.props.privateProperties()) {
    mergeThisProp(env, kv.first, TUninit);
  }
}

void boxThisProp(ISS& env, SString name) {
  auto const t = thisPropRaw(env, name);
  if (!t) return;
  *t = union_of(*t, TRef);
}

/*
 * Forces non-ref property types up to TCell.  This is used when an
 * operation affects an unknown property on $this, but can't change
 * its reffiness.  This could only do TInitCell, but we're just
 * going to gradually get rid of the callsites of this.
 */
void loseNonRefThisPropTypes(ISS& env) {
  FTRACE(2, "    loseNonRefThisPropTypes\n");
  for (auto& kv : env.props.privateProperties()) {
    if (kv.second.subtypeOf(TCell)) kv.second = TCell;
  }
}

//////////////////////////////////////////////////////////////////////
// properties on self::

// Similar to $this properties above, we only track control-flow
// insensitive types for these.

Type* selfPropRaw(ISS& env, SString name) {
  auto& privateStatics = env.props.privateStatics();
  auto it = privateStatics.find(name);
  if (it != end(privateStatics)) {
    return &it->second;
  }
  return nullptr;
}

void killSelfProps(ISS& env) {
  FTRACE(2, "    killSelfProps\n");
  for (auto& kv : env.props.privateStatics()) kv.second = TGen;
}

void killSelfProp(ISS& env, SString name) {
  FTRACE(2, "    killSelfProp {}\n", name->data());
  if (auto t = selfPropRaw(env, name)) *t = TGen;
}

// TODO(#3684136): self::$foo can't actually ever be uninit.  Right
// now uninits may find their way into here though.
folly::Optional<Type> selfPropAsCell(ISS& env, SString name) {
  auto const t = selfPropRaw(env, name);
  if (!t) return folly::none;
  return !t->subtypeOf(TCell) ? TInitCell :
          t->subtypeOf(TUninit) ? TInitNull :
          remove_uninit(*t);
}

/*
 * Merges a type into tracked static properties on self, in the
 * sense of tvSet (i.e. setting the inner type on possible refs).
 */
void mergeSelfProp(ISS& env, SString name, Type type) {
  auto const t = selfPropRaw(env, name);
  if (!t) return;
  *t = union_of(*t, type);
}

/*
 * Similar to mergeEachThisPropRaw, but for self props.
 */
template<class MapFn>
void mergeEachSelfPropRaw(ISS& env, MapFn fn) {
  for (auto& kv : env.props.privateStatics()) {
    mergeSelfProp(env, kv.first, fn(kv.second));
  }
}

void boxSelfProp(ISS& env, SString name) {
  mergeSelfProp(env, name, TRef);
}

/*
 * Forces non-ref static properties up to TCell.  This is used when
 * an operation affects an unknown static property on self::, but
 * can't change its reffiness.
 *
 * This could only do TInitCell because static properties can never
 * be unset.  We're just going to get rid of the callers of this
 * function over a few more changes, though.
 */
void loseNonRefSelfPropTypes(ISS& env) {
  FTRACE(2, "    loseNonRefSelfPropTypes\n");
  for (auto& kv : env.props.privateStatics()) {
    if (kv.second.subtypeOf(TInitCell)) kv.second = TCell;
  }
}

}

//////////////////////////////////////////////////////////////////////

}}

#endif
