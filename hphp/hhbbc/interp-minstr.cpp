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
#include "hphp/hhbbc/interp.h"

#include <vector>
#include <algorithm>
#include <string>
#include <utility>

#include <folly/Optional.h>
#include <folly/Format.h>

#include "hphp/util/trace.h"

#include "hphp/hhbbc/interp-internal.h"
#include "hphp/hhbbc/type-ops.h"

namespace HPHP { namespace HHBBC {

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_stdClass("stdClass");

//////////////////////////////////////////////////////////////////////

/*
 * Note: the couldBe comparisons here with sempty() are asking "can this string
 * be a non-reference counted empty string".  What actually matters is whether
 * it can be an empty string at all.  Currently, all reference counted strings
 * are TStr, which has no values and may also be non-reference
 * counted---emptiness isn't separately tracked like it is for arrays, so if
 * anything happened that could make it reference counted this check will
 * return true.
 *
 * This means this code is fine for now, but if we implement #3837503
 * (non-static strings with values in the type system) it will need to change.
 */

bool couldBeEmptyish(Type ty) {
  return ty.couldBe(TNull) ||
         ty.couldBe(sempty()) ||
         ty.couldBe(TFalse);
}

bool mustBeEmptyish(Type ty) {
  return ty.subtypeOf(TNull) ||
         ty.subtypeOf(sempty()) ||
         ty.subtypeOf(TFalse);
}

bool elemCouldPromoteToArr(Type ty) { return couldBeEmptyish(ty); }
bool propCouldPromoteToObj(Type ty) { return couldBeEmptyish(ty); }
bool elemMustPromoteToArr(Type ty)  { return mustBeEmptyish(ty); }
bool propMustPromoteToObj(Type ty)  { return mustBeEmptyish(ty); }

bool keyCouldBeWeird(Type key) {
  return key.couldBe(TObj) || key.couldBe(TArr);
}

//////////////////////////////////////////////////////////////////////

/*
 * Tag indicating what sort of thing contains the current base.
 *
 * The base is always the unboxed version of the type, and its
 * location could be inside of a Ref.  So, for example, a base with
 * BaseLoc::Frame could be located inside of a Ref that is pointed
 * to by the Frame.  (We may want to distinguish these two cases at
 * some point if we start trying to track real information about
 * Refs, but not yet.)
 */
enum class BaseLoc {
  /*
   * Base is in a number of possible places after an Elem op.  It
   * cannot possibly be in an object property (although it certainly
   * may alias one).  See miElem for details.  Not all post-elem ops
   * use this location (see LocalArrChain).
   *
   * If it is definitely in an array, the locTy in the Base will be
   * a subtype of TArr.
   */
  PostElem,

  /*
   * Base is in possible locations after a Prop op.  This means it
   * possibly lives in a property on an object, but possibly not
   * (e.g. it could be a null in tvScratch).  See miProp for
   * details.
   *
   * If it is definitely known to be a property in an object, the
   * locTy in the Base will be a subtype of TObj.
   */
  PostProp,

  /*
   * Known to be a static property on an object.  This is only
   * possible as an initial base.
   */
  StaticObjProp,

  /*
   * The base is inside of a local that contains a specialized array
   * type, and the arrayChain is non-empty.
   *
   * When the location is set to this, the chain will continue as
   * long as we keep staying inside specialized array types.  If it
   * moves to something like a ?Arr type, we must leave the chain
   * when the base moves.
   */
  LocalArrChain,

  /*
   * Known to be contained in the current frame as a local, as the
   * frame $this, by the evaluation stack, or inside $GLOBALS.  Only
   * possible as initial bases.
   */
  Frame,
  FrameThis,
  EvalStack,
  Global,

  /*
   * If we've execute an operation that's known to fatal, we use
   * this BaseLoc.
   */
  Fataled,
};

struct Base {
  Type type;
  BaseLoc loc;

  /*
   * We also need to track effects of intermediate dims on the type
   * of the base.  So we have a type, name, and possibly associated
   * local for the base's container.
   *
   * For StaticObjProp, locName this is the name of the property if
   * known, or nullptr, and locTy is the type of the class
   * containing the static property.
   *
   * Similarly, if loc is PostProp, locName is the name of the
   * property if it was known, and locTy gives as much information
   * about the object type it is in.  (If we actually *know* it is
   * in an object, locTy will be a subtype of TObj.)
   */
  Type locTy;
  SString locName;
  borrowed_ptr<php::Local> local;
};

std::string base_string(const Base& b) {
  auto const locStr = [&]() -> const char* {
    switch (b.loc) {
    case BaseLoc::PostElem:      return "PostElem";
    case BaseLoc::PostProp:      return "PostProp";
    case BaseLoc::StaticObjProp: return "StaticObjProp";
    case BaseLoc::LocalArrChain: return "ArrChain";
    case BaseLoc::Frame:         return "Frame";
    case BaseLoc::FrameThis:     return "FrameThis";
    case BaseLoc::EvalStack:     return "EvalStack";
    case BaseLoc::Global:        return "Global";
    case BaseLoc::Fataled:       return "Fataled";
    }
    not_reached();
  }();
  return folly::format(
    "{: <32}  ({: <14} {: <8} @ {})",
    show(b.type),
    locStr,
    show(b.locTy),
    b.locName ? b.locName->data() : "?"
  ).str();
}

Type baseLocNameType(const Base& b) {
  return b.locName ? sval(b.locName) : TInitGen;
}

struct MIS : ISS {
  explicit MIS(ISS& env,
               const MInstrInfo* info,
               const MVector& mvec)
    : ISS(env)
    , info(*info)
    , mvec(mvec)
    , stackIdx(info->valCount() + numVecPops(mvec))
  {}

  /*
   * Return the current MElem.  Only valid after the first base has
   * been processed.
   */
  const MElem& melem() const {
    assert(mInd < mvec.mcodes.size());
    return mvec.mcodes[mInd];
  }

  /*
   * Return the current MemberCode.  Only valid after the first base
   * has been processed.
   */
  MemberCode mcode() const { return melem().mcode; }

  const MInstrInfo& info;
  const MVector& mvec;

  /*
   * The current base.  Updated as we move through the vector
   * instruction.
   */
  Base base;

  /*
   * Chains of operations on array elements will all affect the type
   * of something further back in the member instruction.  Currently
   * this is just used for locals.  This vector tracks the base,key
   * type pair that was used at each stage.  See resolveArrayChain.
   */
  std::vector<std::pair<Type,Type>> arrayChain;

  /*
   * Current index in mcodes vector (or 0 if we still haven't done
   * the base).
   */
  uint32_t mInd = 0;

  /*
   * One above next stack slot to read, going forward in the mvec
   * (deeper to higher on stack).
   */
  int32_t stackIdx;
};

//////////////////////////////////////////////////////////////////////

/*
 * A note about bases.
 *
 * Generally type inference needs to know two kinds of things about
 * the base to handle effects on tracked locations:
 *
 *   - Could the base be a location we're tracking deeper structure
 *     on, so the next operation actually affects something inside
 *     of it.  For example, could the base be an object with the
 *     same type as $this, or an array in a local variable.
 *
 *   - Could the base be something (regardless of type) that is
 *     inside one of the things we're tracking.  I.e., the base
 *     might be whatever (an array or a bool or something), but
 *     living inside a property inside an object with the same type
 *     as $this, or living inside of an array in the local frame.
 *
 * The first cases apply because final operations are going to
 * directly affect the type of these elements.  The second case is
 * because vector operations may change the base at each step if it
 * is a defining instruction.
 *
 * Note that both of these cases can apply to the same base in some
 * cases: you might have an object property on $this that could be
 * an object of the type of $this.
 *
 * The functions below with names "couldBeIn*" detect the second
 * case.  The effects on the tracked location in the second case are
 * handled in the functions with names "handleIn*{Prop,Elem,..}".
 * The effects for the first case are generally handled in the
 * miFinal op functions.
 *
 * Control flow insensitive vs. control flow sensitive types:
 *
 * Things are also slightly complicated by the fact that we are
 * analyzing some control flow insensitve types along side precisely
 * tracked types.  For effects on locals, we perform the type
 * effects of each operation on base.type, and then allow moveBase()
 * to make the updates to the local when we know what its final type
 * will be.
 *
 * This approach doesn't do as well for possible properties in $this
 * or self::, because we may see situations where the base could be
 * one of these properties but we're not sure---perhaps because it
 * came off a property with the same name on an object with an
 * unknown type (i.e. base.type is InitCell but couldBeInThis is
 * true).  In these situations, we can get away with just merging
 * Obj=stdClass into the thisProp (because it 'could' promote)
 * instead of merging the whole InitCell, which possibly lets us
 * leave the type at ?Obj in some cases.
 *
 * This is why there's two fairly different mechanisms for handling
 * the effects of defining ops on base types.
 */

//////////////////////////////////////////////////////////////////////

bool couldBeThisObj(ISS& env, const Base& b) {
  if (b.loc == BaseLoc::Fataled) return false;
  auto const thisTy = thisType(env);
  return b.type.couldBe(thisTy ? *thisTy : TObj);
}

bool mustBeThisObj(ISS& env, const Base& b) {
  if (b.loc == BaseLoc::FrameThis) return true;
  if (auto const ty = thisType(env)) return b.type.subtypeOf(*ty);
  return false;
}

bool mustBeInFrame(const Base& b) {
  return b.loc == BaseLoc::Frame;
}

bool couldBeInThis(ISS& env, const Base& b) {
  if (b.loc != BaseLoc::PostProp) return false;
  auto const thisTy = thisType(env);
  if (!thisTy) return true;
  if (!b.locTy.couldBe(*thisTy)) return false;
  if (b.locName) {
    return isTrackedThisProp(env, b.locName);
  }
  return true;
}

bool couldBeInSelf(ISS& env, const Base& b) {
  if (b.loc != BaseLoc::StaticObjProp) return false;
  auto const selfTy = selfCls(env);
  return !selfTy || b.locTy.couldBe(*selfTy);
}

bool couldBeInPublicStatic(ISS& env, const Base& b) {
  return b.loc == BaseLoc::StaticObjProp;
}

//////////////////////////////////////////////////////////////////////

void handleInThisPropD(MIS& env) {
  // NullSafe (Q) props do not promote an emptyish base to stdClass instance.
  if (env.mvec.mcodes[env.mInd].mcode == MQT) {
    return;
  }
  if (!couldBeInThis(env, env.base)) return;

  if (auto const name = env.base.locName) {
    auto const ty = thisPropAsCell(env, name);
    if (ty && propCouldPromoteToObj(*ty)) {
      mergeThisProp(env, name,
        objExact(env.index.builtin_class(s_stdClass.get())));
    }
    return;
  }

  mergeEachThisPropRaw(env, [&] (Type t) {
    return propCouldPromoteToObj(t) ? TObj : TBottom;
  });
}

void handleInSelfPropD(MIS& env) {
  // NullSafe (Q) props do not promote an emptyish base to stdClass instance.
  if (env.mvec.mcodes[env.mInd].mcode == MQT) {
    return;
  }

  if (!couldBeInSelf(env, env.base)) return;

  if (auto const name = env.base.locName) {
    auto const ty = selfPropAsCell(env, name);
    if (ty && propCouldPromoteToObj(*ty)) {
      mergeSelfProp(env, name,
        objExact(env.index.builtin_class(s_stdClass.get())));
    }
    return;
  }

  loseNonRefSelfPropTypes(env);
}

void handleInPublicStaticPropD(MIS& env) {
  // NullSafe (Q) props do not promote an emptyish base to stdClass instance.
  if (env.mvec.mcodes[env.mInd].mcode == MQT) {
    return;
  }

  if (!couldBeInPublicStatic(env, env.base)) return;

  auto const indexer = env.collect.publicStatics;
  if (!indexer) return;

  auto const name = baseLocNameType(env.base);
  auto const ty = env.index.lookup_public_static(env.base.locTy, name);
  if (propCouldPromoteToObj(ty)) {
    indexer->merge(env.ctx, env.base.locTy, name,
      objExact(env.index.builtin_class(s_stdClass.get())));
  }
}

void handleInThisElemD(MIS& env) {
  if (!couldBeInThis(env, env.base)) return;

  if (auto const name = env.base.locName) {
    auto const ty = thisPropAsCell(env, name);
    if (ty && elemCouldPromoteToArr(*ty)) {
      mergeThisProp(env, name, TArr);
    }
    return;
  }

  mergeEachThisPropRaw(env, [&] (Type t) {
    return elemCouldPromoteToArr(t) ? TArr : TBottom;
  });
}

void handleInSelfElemD(MIS& env) {
  if (!couldBeInSelf(env, env.base)) return;

  if (auto const name = env.base.locName) {
    if (auto const ty = selfPropAsCell(env, name)) {
      if (elemCouldPromoteToArr(*ty)) {
        mergeSelfProp(env, name, TArr);
      }
      mergeSelfProp(env, name, loosen_statics(*ty));
    }
    return;
  }
  loseNonRefSelfPropTypes(env);
}

void handleInPublicStaticElemD(MIS& env) {
  if (!couldBeInPublicStatic(env, env.base)) return;

  auto const indexer = env.collect.publicStatics;
  if (!indexer) return;

  auto const name = baseLocNameType(env.base);
  auto const ty = env.index.lookup_public_static(env.base.locTy, name);
  if (elemCouldPromoteToArr(ty)) {
    // Might be possible to only merge a TArrE, but for now this is ok.
    indexer->merge(env.ctx, env.base.locTy, name, TArr);
  }
}

// Currently NewElem and Elem InFoo effects don't need to do
// anything different from each other.
void handleInThisNewElem(MIS& env) { handleInThisElemD(env); }
void handleInSelfNewElem(MIS& env) { handleInSelfElemD(env); }
void handleInPublicStaticNewElem(MIS& env) { handleInPublicStaticElemD(env); }

void handleInSelfElemU(MIS& env) {
  if (!couldBeInSelf(env, env.base)) return;

  if (auto const name = env.base.locName) {
    auto const ty = selfPropAsCell(env, name);
    if (ty) mergeSelfProp(env, name, loosen_statics(*ty));
  } else {
    mergeEachSelfPropRaw(env, loosen_statics);
  }
}

void handleInPublicStaticElemU(MIS& env) {
  if (!couldBeInPublicStatic(env, env.base)) return;

  auto const indexer = env.collect.publicStatics;
  if (!indexer) return;

  /*
   * We need to ensure that the type could become non-static, but since we're
   * never going to see anything specialized from lookup_public_static the
   * first time we're running with collect.publicStatics, we can't do much
   * right now since we don't have a type for the union of all counted types.
   *
   * Merging InitCell is correct, but very conservative, for now.
   */
  auto const name = baseLocNameType(env.base);
  indexer->merge(env.ctx, env.base.locTy, name, TInitCell);
}

//////////////////////////////////////////////////////////////////////

// MInstrs can throw in between each op, so the states of locals
// need to be propagated across factored exit edges.
void miThrow(MIS& env) {
  for (auto& factored : env.blk.factoredExits) {
    env.propagate(*factored, without_stacks(env.state));
  }
}

//////////////////////////////////////////////////////////////////////

void setLocalForBase(MIS& env, Type ty) {
  assert(mustBeInFrame(env.base) ||
         env.base.loc == BaseLoc::LocalArrChain);
  if (!env.base.local) return loseNonRefLocalTypes(env);
  setLoc(env, env.base.local, ty);
  FTRACE(4, "      ${} := {}\n",
    env.base.locName ? env.base.locName->data() : "$<unnamed>",
    show(ty)
  );
}

// Run backwards through an array chain doing array_set operations
// to produce the array type that incorporates the effects of any
// intermediate defining dims.
Type currentChainType(MIS& env, Type val) {
  auto it = env.arrayChain.rbegin();
  for (; it != env.arrayChain.rend(); ++it) {
    val = array_set(it->first, it->second, val);
  }
  return val;
}

Type resolveArrayChain(MIS& env, Type val) {
  static UNUSED const char prefix[] = "              ";
  FTRACE(5, "{}chain\n", prefix, show(val));
  do {
    auto arr = std::move(env.arrayChain.back().first);
    auto key = std::move(env.arrayChain.back().second);
    assert(arr.subtypeOf(TArr));
    env.arrayChain.pop_back();
    FTRACE(5, "{}  | {} := {} in {}\n", prefix,
      show(key), show(val), show(arr));
    val = array_set(std::move(arr), key, val);
  } while (!env.arrayChain.empty());
  FTRACE(5, "{}  = {}\n", prefix, show(val));
  return val;
}

void moveBase(MIS& env, folly::Optional<Base> base) {
  SCOPE_EXIT { if (base) env.base = *base; };

  // Note: these miThrows probably can be left out if base is
  // folly::none (i.e. we're on the last dim).

  auto const& ty = env.base.type;
  if (mustBeInFrame(env.base)) {
    setLocalForBase(env, ty);
    miThrow(env);
    return;
  }

  if (!env.arrayChain.empty()) {
    auto const continueChain = base && base->loc == BaseLoc::LocalArrChain;
    if (!continueChain) {
      setLocalForBase(env, resolveArrayChain(env, env.base.type));
      return;
    }

    /*
     * We have a chain in progress, but it's not done.  But we still
     * need to throw any type effects on the local across factored
     * edges.
     */
    setLocalForBase(env, currentChainType(env, env.base.type));
    miThrow(env);
  }
}

//////////////////////////////////////////////////////////////////////

/*
 * The following handleBase{Elem,Prop}* functions are used to implement the
 * 'normal' portion of the effects on base types, which are mostly what are
 * done by intermediate dims.  (Contrast with the handleInXXX{Elem,Prop}
 * functions, which handle the effects on the type of the thing that's
 * /containing/ the base.)
 *
 * The contract with these functions is that they should handle all the effects
 * on the base type /except/ for the case of the base being a subtype of
 * TArr---the caller is responsible for that.  The reason for this is that for
 * tracking effects on specialized array types (e.g. LocalArrChain), the final
 * ops generally need to do completely different things to the array, so this
 * allows reuse of this shared part of the type transitions.  The
 * miIntermediate routines must handle subtypes of TArr outside of calls to
 * this as well.
 */

void handleBaseElemU(MIS& env) {
  auto& ty = env.base.type;
  if (ty.couldBe(TArr)) {
    // We're conservative with unsets on array types for now.
    ty = union_of(ty, TArr);
  }
  if (ty.couldBe(TSStr)) {
    ty = loosen_statics(env.base.type);
  }
}

void handleBasePropD(MIS& env) {
  // NullSafe (Q) props do not promote an emptyish base to stdClass instance.
  if (env.mvec.mcodes[env.mInd].mcode == MQT) {
    return;
  }
  auto& ty = env.base.type;
  if (ty.subtypeOf(TObj)) return;
  if (propMustPromoteToObj(ty)) {
    ty = objExact(env.index.builtin_class(s_stdClass.get()));
    return;
  }
  if (propCouldPromoteToObj(ty)) {
    ty = promote_emptyish(ty, TObj);
    return;
  }
}

void handleBaseElemD(MIS& env) {
  auto& ty = env.base.type;

  // When the base is actually a subtype of array, we handle it in the callers
  // of these functions.
  if (ty.subtypeOf(TArr)) return;

  if (elemMustPromoteToArr(ty)) {
    ty = counted_aempty();
    return;
  }
  // Intermediate ElemD operations on strings fatal, unless the
  // string is empty, which promotes to array.  So for any string
  // here we can assume it promoted to an empty array.
  if (ty.subtypeOf(TStr)) {
    ty = counted_aempty();
    return;
  }
  if (elemCouldPromoteToArr(ty)) {
    ty = promote_emptyish(ty, counted_aempty());
  }

  /*
   * If the base still couldBe some kind of array (but isn't a subtype of TArr,
   * which would be handled outside this routine), we need to give up on any
   * information better than TArr here (or track the effects, but we're not
   * doing that yet).
   */
  if (ty.couldBe(TArr)) {
    ty = union_of(ty, TArr);
  }
}

void handleBaseNewElem(MIS& env) {
  handleBaseElemD(env);
  // Technically we don't need to do TStr case.
}

//////////////////////////////////////////////////////////////////////

Type mcodeKey(MIS& env) {
  auto const melem = env.mvec.mcodes[env.mInd];
  switch (melem.mcode) {
  case MPC:  return topC(env, --env.stackIdx);
  case MPL:  return locAsCell(env, melem.immLoc);
  case MPT:  return sval(melem.immStr);
  case MQT:  return sval(melem.immStr);
  case MEC:  return topC(env, --env.stackIdx);
  case MET:  return sval(melem.immStr);
  case MEL:  return locAsCell(env, melem.immLoc);
  case MEI:  return ival(melem.immInt);

  case MW:
  case InvalidMemberCode:
    always_assert(0);
    break;
  }
  not_reached();
}

// Returns nullptr if it's an unknown key or not a string.
SString mcodeStringKey(MIS& env) {
  auto const v = tv(mcodeKey(env));
  return v && v->m_type == KindOfStaticString ? v->m_data.pstr : nullptr;
}

void miPop(MIS& env) {
  auto& mvec = env.mvec;
  if (mvec.lcode == LSL || mvec.lcode == LSC) {
    assert(env.stackIdx == env.info.valCount() + 1 /* clsref */);
  } else {
    assert(env.stackIdx == env.info.valCount());
  }
  for (auto i = uint32_t{0}; i < numVecPops(mvec); ++i) popT(env);
}

//////////////////////////////////////////////////////////////////////
// base ops

Base miBaseLoc(MIS& env) {
  bool const isDefine = env.info.getAttr(env.mvec.lcode) & MIA_define;
  auto const locBase  = env.mvec.locBase;

  if (!isDefine) {
    return Base { derefLoc(env, locBase),
                  BaseLoc::Frame,
                  TBottom,
                  locBase->name,
                  locBase };
  }

  // We're changing the local to define it, but we don't need to do
  // an miThrow yet---the promotions (to array or stdClass) on
  // previously uninitialized locals happen before raising warnings
  // that could throw, so we can wait until the first moveBase.
  ensureInit(env, locBase);
  return Base { locAsCell(env, locBase),
                BaseLoc::Frame,
                TBottom,
                locBase->name,
                locBase };
}

Base miBaseSProp(MIS& env, Type cls, Type tprop) {
  auto const self = selfCls(env);
  auto const prop = tv(tprop);
  auto const name = prop && prop->m_type == KindOfStaticString
                      ? prop->m_data.pstr : nullptr;
  if (self && cls.subtypeOf(*self) && name) {
    if (auto const ty = selfPropAsCell(env, prop->m_data.pstr)) {
      return Base { *ty, BaseLoc::StaticObjProp, cls, name };
    }
  }
  auto const indexTy = env.index.lookup_public_static(cls, tprop);
  if (indexTy.subtypeOf(TInitCell)) {
    return Base { indexTy, BaseLoc::StaticObjProp, cls, name };
  }
  return Base { TInitCell, BaseLoc::StaticObjProp, cls, name };
}

Base miBase(MIS& env) {
  auto& mvec = env.mvec;
  switch (mvec.lcode) {
  case LL:
    return miBaseLoc(env);
  case LC:
    return Base { topC(env, --env.stackIdx), BaseLoc::EvalStack };
  case LR:
    {
      auto const t = topR(env, --env.stackIdx);
      return Base { t.subtypeOf(TInitCell) ? t : TInitCell,
                    BaseLoc::EvalStack };
    }
  case LH:
    {
      auto const ty = thisType(env);
      return Base { ty ? *ty : TObj, BaseLoc::FrameThis };
    }
  case LGL:
    locAsCell(env, env.mvec.locBase);
    return Base { TInitCell, BaseLoc::Global };
  case LGC:
    topC(env, --env.stackIdx);
    return Base { TInitCell, BaseLoc::Global };

    // The first moveBase() on these unknown local bases will cause
    // a env.loseNonRefLocalTypes.
  case LNL:
    return Base { TInitCell, BaseLoc::Frame };
  case LNC:
    topC(env, --env.stackIdx);
    return Base { TInitCell, BaseLoc::Frame };

  case LSL:
    {
      auto const cls  = topA(env, env.info.valCount());
      auto const prop = locAsCell(env, env.mvec.locBase);
      return miBaseSProp(env, cls, prop);
    }
  case LSC:
    {
      auto const cls  = topA(env, env.info.valCount());
      auto const prop = topC(env, --env.stackIdx);
      return miBaseSProp(env, cls, prop);
    }

  case InvalidLocationCode:
    break;
  }
  not_reached();
}

//////////////////////////////////////////////////////////////////////
// intermediate ops

void miProp(MIS& env) {
  auto const name     = mcodeStringKey(env);
  auto const isDefine = env.info.getAttr(env.mcode()) & MIA_define;
  auto const isUnset  = env.info.getAttr(env.mcode()) & MIA_unset;
  auto const isNullsafe = (env.mvec.mcodes[env.mInd].mcode == MQT);

  /*
   * MIA_unset Props doesn't promote "emptyish" things to stdClass,
   * or affect arrays, however it can define a property on an object
   * base.  This means we don't need any couldBeInFoo logic, but if
   * the base could actually be $this, and a declared property could
   * be Uninit, we need to merge InitNull.
   *
   * We're trying to handle this case correctly as far as the type
   * inference here is concerned, but the runtime doesn't actually
   * behave this way right now for declared properties.  Note that
   * it never hurts to merge more types than a thisProp could
   * actually be, so this is fine.
   *
   * See TODO(#3602740): unset with intermediate dims on previously
   * declared properties doesn't define them to null.
   */
  if (isUnset && couldBeThisObj(env, env.base)) {
    if (name) {
      auto const ty = thisPropRaw(env, name);
      if (ty && ty->couldBe(TUninit)) {
        mergeThisProp(env, name, TInitNull);
      }
    } else {
      mergeEachThisPropRaw(env, [&] (Type ty) {
        return ty.couldBe(TUninit) ? TInitNull : TBottom;
      });
    }
  }

  if (isDefine) {
    handleInThisPropD(env);
    handleInSelfPropD(env);
    handleInPublicStaticPropD(env);
    handleBasePropD(env);
  }

  if (mustBeThisObj(env, env.base)) {
    auto const optThisTy = thisType(env);
    auto const thisTy    = optThisTy ? *optThisTy : TObj;
    if (name) {
      auto const propTy = thisPropAsCell(env, name);
      moveBase(env, Base { propTy ? *propTy : TInitCell,
                           BaseLoc::PostProp,
                           thisTy,
                           name });
    } else {
      moveBase(env, Base { TInitCell, BaseLoc::PostProp, thisTy });
    }
    return;
  }

  // We know for sure we're going to be in an object property.
  if (env.base.type.subtypeOf(TObj)) {
    moveBase(env, Base { TInitCell,
                         BaseLoc::PostProp,
                         env.base.type,
                         name });
    return;
  }

  /*
   * Otherwise, intermediate props with define can promote a null, false, or ""
   * to stdClass.  Those cases, and others, if it's not MIA_define, will set
   * the base to a null value in tvScratch.  The base may also legitimately be
   * an object and our next base is in an object property.
   *
   * If we know for sure we're promoting to stdClass, we can put the locType
   * pointing at that.  Otherwise we conservatively treat all these cases as
   * "possibly" being inside of an object property with "PostProp" with locType
   * TTop.
   */
  auto const newBaseLocTy =
    isDefine && !isNullsafe && propMustPromoteToObj(env.base.type)
      ? objExact(env.index.builtin_class(s_stdClass.get()))
      : TTop;

  moveBase(env, Base { TInitCell, BaseLoc::PostProp, newBaseLocTy, name });
}

void miElem(MIS& env) {
  auto const key      = mcodeKey(env);
  bool const isDefine = env.info.getAttr(env.mcode()) & MIA_define;
  bool const isUnset  = env.info.getAttr(env.mcode()) & MIA_unset;

  /*
   * Elem dims with MIA_unset can change a base from a static array
   * into a reference counted array.  It never promotes emptyish
   * types, however.
   *
   * We only need to handle this for self props, because we don't
   * track static-ness on this props.  The similar effect on local
   * bases is handled in miBase.
   */
  if (isUnset) {
    handleInSelfElemU(env);
    handleInPublicStaticElemU(env);
    handleBaseElemU(env);
  }

  if (isDefine) {
    handleInThisElemD(env);
    handleInSelfElemD(env);
    handleInPublicStaticElemD(env);
    handleBaseElemD(env);

    auto const couldDoChain =
      mustBeInFrame(env.base) || env.base.loc == BaseLoc::LocalArrChain;
    if (couldDoChain && env.base.type.subtypeOf(TArr)) {
      env.arrayChain.emplace_back(env.base.type, key);
      moveBase(env, Base { array_elem(env.base.type, key),
                           BaseLoc::LocalArrChain,
                           TBottom,
                           env.base.locName,
                           env.base.local });
      return;
    }
  }

  if (env.base.type.subtypeOf(TArr)) {
    moveBase(env, Base { array_elem(env.base.type, key),
                         BaseLoc::PostElem,
                         env.base.type });
    return;
  }
  if (env.base.type.subtypeOf(TStr)) {
    moveBase(env, Base { TStr, BaseLoc::PostElem });
    return;
  }

  /*
   * Other cases could leave the base as anything (if nothing else,
   * via ArrayAccess on an object).
   *
   * The resulting BaseLoc is either inside an array, is the global
   * init_null_variant, or inside tvScratch.  We represent this with
   * the PostElem base location with locType TTop.
   */
  moveBase(env, Base { TInitCell, BaseLoc::PostElem, TTop });
}

void miNewElem(MIS& env) {
  if (!env.info.newElem()) {
    moveBase(env, Base { TInitCell, BaseLoc::Fataled });
    return;
  }

  handleInThisNewElem(env);
  handleInSelfNewElem(env);
  handleInPublicStaticNewElem(env);
  handleBaseNewElem(env);

  auto const couldDoChain =
    mustBeInFrame(env.base) || env.base.loc == BaseLoc::LocalArrChain;
  if (couldDoChain && env.base.type.subtypeOf(TArr)) {
    env.arrayChain.push_back(
      array_newelem_key(env.base.type, TInitNull));
    moveBase(env, Base { TInitNull,
                         BaseLoc::LocalArrChain,
                         TBottom,
                         env.base.locName,
                         env.base.local });
    return;
  }

  if (env.base.type.subtypeOf(TArr)) {
    moveBase(env, Base { TInitNull, BaseLoc::PostElem, env.base.type });
    return;
  }

  moveBase(env, Base { TInitCell, BaseLoc::PostElem, TTop });
}

void miIntermediate(MIS& env) {
  if (mcodeIsProp(env.mcode())) return miProp(env);
  if (mcodeIsElem(env.mcode())) return miElem(env);
  return miNewElem(env);
}

//////////////////////////////////////////////////////////////////////
// final prop ops

void miFinalIssetProp(MIS& env) {
  auto const name = mcodeStringKey(env);
  miPop(env);
  if (name && mustBeThisObj(env, env.base)) {
    if (auto const pt = thisPropAsCell(env, name)) {
      if (pt->subtypeOf(TNull))  return push(env, TFalse);
      if (!pt->couldBe(TNull))   return push(env, TTrue);
    }
  }
  push(env, TBool);
}

void miFinalCGetProp(MIS& env) {
  auto const name = mcodeStringKey(env);
  miPop(env);
  if (name && mustBeThisObj(env, env.base)) {
    if (auto const t = thisPropAsCell(env, name)) {
      return push(env, *t);
    }
  }
  push(env, TInitCell);
}

void miFinalVGetProp(MIS& env) {
  auto const name = mcodeStringKey(env);
  miPop(env);
  handleInThisPropD(env);
  handleInSelfPropD(env);
  handleInPublicStaticPropD(env);
  handleBasePropD(env);
  if (couldBeThisObj(env, env.base)) {
    if (name) {
      boxThisProp(env, name);
    } else {
      killThisProps(env);
    }
  }
  push(env, TRef);
}

void miFinalSetProp(MIS& env) {
  auto const name = mcodeStringKey(env);
  auto const t1 = popC(env);

  miPop(env);
  handleInThisPropD(env);
  handleInSelfPropD(env);
  handleInPublicStaticPropD(env);
  handleBasePropD(env);

  auto const resultTy = env.base.type.subtypeOf(TObj) ? t1 : TInitCell;

  if (couldBeThisObj(env, env.base)) {
    if (!name) {
      mergeEachThisPropRaw(env, [&] (Type propTy) -> Type {
        if (propTy.couldBe(TInitCell)) {
          return union_of(std::move(propTy), t1);
        }
        return TBottom;
      });
      push(env, resultTy);
      return;
    }
    mergeThisProp(env, name, t1);
    push(env, resultTy);
    return;
  }

  push(env, resultTy);
}

void miFinalSetOpProp(MIS& env, SetOpOp subop) {
  auto const name = mcodeStringKey(env);
  auto const rhsTy = popC(env);

  miPop(env);
  handleInThisPropD(env);
  handleInSelfPropD(env);
  handleInPublicStaticPropD(env);
  handleBasePropD(env);

  auto resultTy = TInitCell;

  if (couldBeThisObj(env, env.base)) {
    if (name && mustBeThisObj(env, env.base)) {
      if (auto const lhsTy = thisPropAsCell(env, name)) {
        resultTy = typeSetOp(subop, *lhsTy, rhsTy);
      }
    }

    if (name) {
      mergeThisProp(env, name, resultTy);
    } else {
      loseNonRefThisPropTypes(env);
    }
  }

  push(env, resultTy);
}

void miFinalIncDecProp(MIS& env, IncDecOp subop) {
  auto const name = mcodeStringKey(env);
  miPop(env);
  handleInThisPropD(env);
  handleInSelfPropD(env);
  handleInPublicStaticPropD(env);
  handleBasePropD(env);

  auto prePropTy  = TInitCell;
  auto postPropTy = TInitCell;

  if (couldBeThisObj(env, env.base)) {
    if (name && mustBeThisObj(env, env.base)) {
      if (auto const propTy = thisPropAsCell(env, name)) {
        prePropTy  = typeIncDec(subop, *propTy);
        postPropTy = *propTy;
      }
    }

    if (name) {
      mergeThisProp(env, name, prePropTy);
    } else {
      loseNonRefThisPropTypes(env);
    }
  }
  push(env, isPre(subop) ? prePropTy : postPropTy);
}

void miFinalBindProp(MIS& env) {
  auto const name = mcodeStringKey(env);
  popV(env);
  miPop(env);
  handleInThisPropD(env);
  handleInSelfPropD(env);
  handleInPublicStaticPropD(env);
  handleBasePropD(env);
  if (couldBeThisObj(env, env.base)) {
    if (name) {
      boxThisProp(env, name);
    } else {
      killThisProps(env);
    }
  }
  push(env, TRef);
}

void miFinalUnsetProp(MIS& env) {
  auto const name = mcodeStringKey(env);
  miPop(env);

  /*
   * Unset does define intermediate dims but with slightly different
   * rules than sets.  It only applies to object properties.
   *
   * Note that this can't affect self props, because static
   * properties can never be unset.  It also can't change anything
   * about an inner array type.
   */
  handleInThisPropD(env);

  if (couldBeThisObj(env, env.base)) {
    if (name) {
      unsetThisProp(env, name);
    } else {
      unsetUnknownThisProp(env);
    }
  }
}

//////////////////////////////////////////////////////////////////////
// Final elem ops

// This is a helper for final defining Elem operations that need to
// handle array chains and frame effects, but don't yet do anything
// better than supplying a single type.
void pessimisticFinalElemD(MIS& env, Type key, Type ty) {
  if (mustBeInFrame(env.base) && env.base.type.subtypeOf(TArr)) {
    env.base.type = array_set(env.base.type, key, ty);
    return;
  }
  if (env.base.loc == BaseLoc::LocalArrChain) {
    if (env.base.type.subtypeOf(TArr)) {
      env.arrayChain.emplace_back(env.base.type, key);
      env.base.type = ty;
    }
  }
}

void miFinalCGetElem(MIS& env) {
  auto const key = mcodeKey(env);
  auto const ty =
    env.base.type.subtypeOf(TArr)
      ? array_elem(env.base.type, key)
      : TInitCell;
  miPop(env);
  push(env, ty);
}

void miFinalVGetElem(MIS& env) {
  auto const key = mcodeKey(env);
  miPop(env);
  handleInThisElemD(env);
  handleInSelfElemD(env);
  handleInPublicStaticElemD(env);
  handleBaseElemD(env);
  pessimisticFinalElemD(env, key, TInitGen);
  push(env, TRef);
}

void miFinalSetElem(MIS& env) {
  auto const key = mcodeKey(env);
  auto const t1  = popC(env);
  miPop(env);

  handleInThisElemD(env);
  handleInSelfElemD(env);
  handleInPublicStaticElemD(env);

  // Note: we must handle the string-related cases before doing the
  // general handleBaseElemD, since operates on strings as if this
  // was an intermediate ElemD.
  if (env.base.type.subtypeOf(sempty())) {
    env.base.type = counted_aempty();
  } else {
    auto& ty = env.base.type;
    if (ty.couldBe(TStr)) {
      // Note here that a string type stays a string (with a changed character,
      // and loss of staticness), unless it was the empty string, where it
      // becomes an array.  Do it conservatively for now:
      ty = union_of(loosen_statics(ty), counted_aempty());
    }
    if (!ty.subtypeOf(TStr)) {
      handleBaseElemD(env);
    }
  }

  /*
   * In some unusual cases with illegal keys, SetM pushes null
   * instead of the right hand side.
   *
   * There are also some special cases for SetM for different base types:
   * 1. If the base is a string, SetM pushes a new string with the
   * value of the first character of the right hand side converted
   * to a string (or something like that).
   * 2. If the base is a primitive type, SetM pushes null.
   * 3. If the base is an object, and it does not implement ArrayAccess,
   * it is still ok to push the right hand side, because it is a
   * fatal.
   *
   * We push the right hand side on the stack only if the base is an
   * array, object or emptyish.
   */
  auto const isWeird = keyCouldBeWeird(key) ||
                       (!env.base.type.subtypeOf(TArr) &&
                        !env.base.type.subtypeOf(TObj) &&
                        !mustBeEmptyish(env.base.type));

  if (mustBeInFrame(env.base) && env.base.type.subtypeOf(TArr)) {
    env.base.type = array_set(env.base.type, key, t1);
    push(env, isWeird ? TInitCell : t1);
    return;
  }
  if (env.base.loc == BaseLoc::LocalArrChain) {
    if (env.base.type.subtypeOf(TArr)) {
      env.arrayChain.emplace_back(env.base.type, key);
      env.base.type = t1;
      push(env, isWeird ? TInitCell : t1);
      return;
    }
  }

  // ArrayAccess on $this will always push the rhs, even if things
  // were weird.
  if (mustBeThisObj(env, env.base)) return push(env, t1);

  push(env, isWeird ? TInitCell : t1);
}

void miFinalSetOpElem(MIS& env, SetOpOp subop) {
  auto const key   = mcodeKey(env);
  auto const rhsTy = popC(env);
  miPop(env);
  handleInThisElemD(env);
  handleInSelfElemD(env);
  handleInPublicStaticElemD(env);
  handleBaseElemD(env);
  auto const lhsTy = env.base.type.subtypeOf(TArr) && !keyCouldBeWeird(key)
    ? array_elem(env.base.type, key)
    : TInitCell;
  auto const resultTy = typeSetOp(subop, lhsTy, rhsTy);
  pessimisticFinalElemD(env, key, resultTy);
  push(env, resultTy);
}

void miFinalIncDecElem(MIS& env, IncDecOp subop) {
  auto const key = mcodeKey(env);
  miPop(env);
  handleInThisElemD(env);
  handleInSelfElemD(env);
  handleInPublicStaticElemD(env);
  handleBaseElemD(env);
  auto const postTy = env.base.type.subtypeOf(TArr) && !keyCouldBeWeird(key)
    ? array_elem(env.base.type, key)
    : TInitCell;
  auto const preTy = typeIncDec(subop, postTy);
  pessimisticFinalElemD(env, key, typeIncDec(subop, preTy));
  push(env, isPre(subop) ? preTy : postTy);
}

void miFinalBindElem(MIS& env) {
  auto const key = mcodeKey(env);
  popV(env);
  miPop(env);
  handleInThisElemD(env);
  handleInSelfElemD(env);
  handleInPublicStaticElemD(env);
  handleBaseElemD(env);
  pessimisticFinalElemD(env, key, TInitGen);
  push(env, TRef);
}

void miFinalUnsetElem(MIS& env) {
  mcodeKey(env);
  miPop(env);
  handleInSelfElemU(env);
  handleInPublicStaticElemU(env);
  handleBaseElemU(env);
  // We don't handle inner-array types with unset yet.
  always_assert(env.base.loc != BaseLoc::LocalArrChain);
  if (mustBeInFrame(env.base)) {
    always_assert(!env.base.type.strictSubtypeOf(TArr));
  }
}

//////////////////////////////////////////////////////////////////////
// Final new elem ops

// This is a helper for final defining Elem operations that need to
// handle array chains and frame effects, but don't yet do anything
// better than supplying a single type.
void pessimisticFinalNewElem(MIS& env, Type ty) {
  if (mustBeInFrame(env.base) && env.base.type.subtypeOf(TArr)) {
    env.base.type = array_newelem(env.base.type, ty);
    return;
  }
  if (env.base.loc == BaseLoc::LocalArrChain &&
      env.base.type.subtypeOf(TArr)) {
    env.base.type = array_newelem(env.base.type, ty);
    return;
  }
}

void miFinalVGetNewElem(MIS& env) {
  miPop(env);
  handleInThisNewElem(env);
  handleInSelfNewElem(env);
  handleInPublicStaticNewElem(env);
  handleBaseNewElem(env);
  pessimisticFinalNewElem(env, TInitGen);
  push(env, TRef);
}

void miFinalSetNewElem(MIS& env) {
  auto const t1 = popC(env);
  miPop(env);
  handleInThisNewElem(env);
  handleInSelfNewElem(env);
  handleInPublicStaticNewElem(env);
  handleBaseNewElem(env);

  if (mustBeInFrame(env.base) && env.base.type.subtypeOf(TArr)) {
    env.base.type = array_newelem(env.base.type, t1);
    push(env, t1);
    return;
  }
  if (env.base.loc == BaseLoc::LocalArrChain &&
      env.base.type.subtypeOf(TArr)) {
    env.base.type = array_newelem(env.base.type, t1);
    push(env, t1);
    return;
  }

  // ArrayAccess on $this will always push the rhs.
  if (mustBeThisObj(env, env.base)) return push(env, t1);

  // TODO(#3343813): we should push the type of the rhs when we can;
  // SetM for a new elem still has some weird cases where it pushes
  // null instead to handle.  (E.g. if the base is a number.)
  push(env, TInitCell);
}

void miFinalSetOpNewElem(MIS& env) {
  popC(env);
  miPop(env);
  handleInThisNewElem(env);
  handleInSelfNewElem(env);
  handleInPublicStaticNewElem(env);
  handleBaseNewElem(env);
  pessimisticFinalNewElem(env, TInitCell);
  push(env, TInitCell);
}

void miFinalIncDecNewElem(MIS& env) {
  miPop(env);
  handleInThisNewElem(env);
  handleInSelfNewElem(env);
  handleInPublicStaticNewElem(env);
  handleBaseNewElem(env);
  pessimisticFinalNewElem(env, TInitCell);
  push(env, TInitCell);
}

void miFinalBindNewElem(MIS& env) {
  popV(env);
  miPop(env);
  handleInThisNewElem(env);
  handleInSelfNewElem(env);
  handleInPublicStaticNewElem(env);
  handleBaseNewElem(env);
  pessimisticFinalNewElem(env, TInitGen);
  push(env, TRef);
}

//////////////////////////////////////////////////////////////////////

void miFinal(MIS& env, const bc::EmptyM& op) {
  // Same thing for now for props and elems, regardless of the base.
  // MW would be a fatal.
  mcodeKey(env);
  miPop(env);
  push(env, TBool);
}

void miFinal(MIS& env, const bc::IssetM& op) {
  if (mcodeIsProp(env.mcode())) return miFinalIssetProp(env);
  // Elem case (MW would be a fatal):
  if (env.mcode() != MemberCode::MW) mcodeKey(env);
  miPop(env);
  push(env, TBool);
}

void miFinal(MIS& env, const bc::CGetM& op) {
  if (mcodeIsProp(env.mcode())) return miFinalCGetProp(env);
  if (env.mcode() == MemberCode::MW) {
    // MW is a fatal.
    miPop(env);
    push(env, TInitCell);
    return;
  }
  return miFinalCGetElem(env);
}

void miFinal(MIS& env, const bc::VGetM& op) {
  if (mcodeIsElem(env.mcode())) return miFinalVGetElem(env);
  if (mcodeIsProp(env.mcode())) return miFinalVGetProp(env);
  return miFinalVGetNewElem(env);
}

void miFinal(MIS& env, const bc::SetM& op) {
  if (mcodeIsElem(env.mcode())) return miFinalSetElem(env);
  if (mcodeIsProp(env.mcode())) return miFinalSetProp(env);
  return miFinalSetNewElem(env);
}

void miFinal(MIS& env, const bc::SetOpM& op) {
  if (mcodeIsElem(env.mcode())) return miFinalSetOpElem(env, op.subop);
  if (mcodeIsProp(env.mcode())) return miFinalSetOpProp(env, op.subop);
  return miFinalSetOpNewElem(env);
}

void miFinal(MIS& env, const bc::IncDecM& op) {
  if (mcodeIsElem(env.mcode())) return miFinalIncDecElem(env, op.subop);
  if (mcodeIsProp(env.mcode())) return miFinalIncDecProp(env, op.subop);
  return miFinalIncDecNewElem(env);
}

void miFinal(MIS& env, const bc::BindM& op) {
  if (mcodeIsElem(env.mcode())) return miFinalBindElem(env);
  if (mcodeIsProp(env.mcode())) return miFinalBindProp(env);
  return miFinalBindNewElem(env);
}

void miFinal(MIS& env, const bc::UnsetM& op) {
  if (mcodeIsElem(env.mcode())) return miFinalUnsetElem(env);
  if (mcodeIsProp(env.mcode())) return miFinalUnsetProp(env);
  // The MW case is a fatal.
  miPop(env);
}

void miFinal(MIS& env, const bc::SetWithRefLM& op) {
  moveBase(env, folly::none);
  killLocals(env);
  killThisProps(env);
  killSelfProps(env);
  if (env.mcode() != MemberCode::MW) mcodeKey(env);
  miPop(env);
}

void miFinal(MIS& env, const bc::SetWithRefRM& op) {
  moveBase(env, folly::none);
  killLocals(env);
  killThisProps(env);
  killSelfProps(env);
  if (env.mcode() != MemberCode::MW) mcodeKey(env);
  popR(env);
  miPop(env);
}

//////////////////////////////////////////////////////////////////////

template<class Op>
void miImpl(ISS& baseEnv, const Op& op,
    const MInstrInfo& info, const MVector& mvec) {
  MIS env { baseEnv, &info, mvec };
  env.base = miBase(env);
  FTRACE(3, "    base: {}\n", base_string(env.base));
  miThrow(env);
  for (; env.mInd < mvec.mcodes.size() - 1; ++env.mInd) {
    miIntermediate(env);
    FTRACE(3, "    base: {}\n", base_string(env.base));
    miThrow(env);
  }
  miFinal(env, op);

  // If the final operation was either a define, a NewElem, or an
  // unset, there may be pending effects in local array chains or on
  // local types, so we need to move the base one last time.  The
  // other cases should have no effects here.
  moveBase(env, folly::none);
}

//////////////////////////////////////////////////////////////////////

}

#define MII(m, ...)                                   \
void minstr(ISS& env, const bc::m##M& op) {           \
  miImpl(env, op, getMInstrInfo(Op::m##M), op.mvec);  \
}
MINSTRS
#undef MII

//////////////////////////////////////////////////////////////////////

}}
