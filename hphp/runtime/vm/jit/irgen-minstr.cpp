/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#include <type_traits>
#include <sstream>

#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/base/collections.h"

#include "hphp/runtime/vm/jit/minstr-effects.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/target-profile.h"
#include "hphp/runtime/vm/jit/type-constraint.h"
#include "hphp/runtime/vm/jit/type.h"

#include "hphp/runtime/vm/jit/irgen-exit.h"
#include "hphp/runtime/vm/jit/irgen-incdec.h"
#include "hphp/runtime/vm/jit/irgen-interpone.h"
#include "hphp/runtime/vm/jit/irgen-sprop-global.h"

#include "hphp/runtime/vm/jit/irgen-internal.h"

#include "hphp/runtime/ext/collections/ext_collections-idl.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_PackedArray("PackedArray");
const StaticString s_StructArray("StructArray");

//////////////////////////////////////////////////////////////////////

bool wantPropSpecializedWarnings() {
  return !RuntimeOption::RepoAuthoritative ||
    !RuntimeOption::EvalDisableSomeRepoAuthNotices;
}

//////////////////////////////////////////////////////////////////////

enum class SimpleOp {
  None,
  Array,
  ProfiledPackedArray,
  ProfiledStructArray,
  PackedArray,
  StructArray,
  String,
  Vector, // c_Vector* or c_ImmVector*
  Map,    // c_Map*
  Pair,   // c_Pair*
};

/*
 * Minstr Translation State.  Member instructions are complex enough that we
 * need our own state environment while processing one.
 *
 * This is implicitly convertible to IRGS so you can use ht-internal functions
 * on it.  Effectively MTS <: IRGS (except the dot operator).
 */
struct MTS {
  explicit MTS(IRGS& irgs, Op effectiveOp)
    : irgs(irgs)
    , op(effectiveOp)
    , immVec(irgs.currentNormalizedInstruction->immVec)
    , immVecM(irgs.currentNormalizedInstruction->immVecM)
    , ni(*irgs.currentNormalizedInstruction)
    , irb(*irgs.irb)
    , unit(irgs.unit)
    , mii(getMInstrInfo(effectiveOp))
    , iInd(mii.valCount())
  {}
  /* implicit */ operator IRGS&() { return irgs; }
  /* implicit */ operator const IRGS&() const { return irgs; }

  IRGS& irgs;
  Op op;
  ImmVector immVec;
  jit::vector<MemberCode> immVecM;
  const NormalizedInstruction& ni;
  IRBuilder& irb;
  IRUnit& unit;
  MInstrInfo mii;

  /*
   * Member index. The current position in immVecM, which contains the list of
   * member lookup keys.
   */
  unsigned mInd;

  /*
   * Input index. The current position in ni.inputs. This travels at a
   * different rate than mInd because not all member codes correspond to a
   * NormalizedInstruction input.
   */
  unsigned iInd;

  /*
   * Cached information about which stages of the minstr need ratchet
   * operations. Filled in by computeRatchets().
   */
  unsigned numLogicalRatchets;
  bool needFirstRatchet;
  bool needFinalRatchet;

  /*
   * Old minstrs use this flag to determine whether they can elide all
   * operations to MInstrState.
   */
  bool needMIS{true};

  /*
   * The value of the base for the next member operation. Starts as the base
   * for the whole instruction and is updated as the translator makes
   * progress.
   *
   * We have a separate type in case we have more information about the type
   * than base.value->type() has (this may be the case with pointers to locals
   * or stack slots right now, for example). If base.value is not nullptr,
   * base.value->type() is always a supertype of base.type, and base.type is
   * always large enough to accommodate the type the base ends up having at
   * runtime.
   *
   * Don't change base directly; use setBase, to update base.type
   * automatically.
   */
  struct {
    SSATmp* value{nullptr};
    Type type{TBottom};
  } base;

  /* Value computed before we do anything to allow better translations for
   * common, simple operations. */
  SimpleOp simpleOp{SimpleOp::None};

  /* The result of the vector instruction. nullptr if the current instruction
   * doesn't produce a result. */
  SSATmp* result{nullptr};

  /* If set, contains a value of type CountedStr|Nullptr. If a runtime test
   * determines that the value is not Nullptr, we incorrectly predicted the
   * output type of the instruction and must side exit. */
  SSATmp* strTestResult{nullptr};

  /* If set, contains the catch target for the final set operation of this
   * instruction. The operations that set this member may need to return an
   * unexpected type, in which case they'll throw an InvalidSetMException. To
   * handle this, emitMPost adds code to the catch trace to fish the correct
   * value out of the exception and side exit. */
  Block* failedSetBlock{nullptr};
};

//////////////////////////////////////////////////////////////////////

// Make a catch block that cleans up temporary values stored in the
// MInstrState, if we have any, in addition to normal catch block behavior.
Block* makeMISCatch(MTS& env);

// Make a special catch block that deals with InvalidSetMExceptions.
Block* makeCatchSet(MTS& env);

//////////////////////////////////////////////////////////////////////
// Property information.

struct PropInfo {
  PropInfo()
    : offset{-1}
    , repoAuthType{}
    , baseClass{nullptr}
  {}

  explicit PropInfo(int offset,
                    RepoAuthType repoAuthType,
                    const Class* baseClass)
    : offset{offset}
    , repoAuthType{repoAuthType}
    , baseClass{baseClass}
  {}

  int offset;
  RepoAuthType repoAuthType;
  const Class* baseClass;
};

/*
 * Try to find a property offset for the given key in baseClass. Will return a
 * PropInfo with an offset of -1 if the mapping from baseClass's name to the
 * Class* can change (which happens in sandbox mode when the ctx class is
 * unrelated to baseClass).
 */
PropInfo getPropertyOffset(IRGS& env,
                           const Class* ctx,
                           const Class* baseClass,
                           Type keyType) {
  if (!baseClass) return PropInfo();

  if (!keyType.hasConstVal(TStr)) return PropInfo();
  auto const name = keyType.strVal();

  // If we are not in repo-authoriative mode, we need to check that baseClass
  // cannot change in between requests.
  if (!RuntimeOption::RepoAuthoritative ||
      !(baseClass->preClass()->attrs() & AttrUnique)) {
    if (!ctx) return PropInfo();
    if (!ctx->classof(baseClass)) {
      if (baseClass->classof(ctx)) {
        // baseClass can change on us in between requests, but since
        // ctx is an ancestor of baseClass we can make the weaker
        // assumption that the object is an instance of ctx
        baseClass = ctx;
      } else {
        // baseClass can change on us in between requests and it is
        // not related to ctx, so bail out
        return PropInfo();
      }
    }
  }

  // Lookup the index of the property based on ctx and baseClass
  auto const lookup = baseClass->getDeclPropIndex(ctx, name);
  auto const idx = lookup.prop;

  // If we couldn't find a property that is accessible in the current context,
  // bail out
  if (idx == kInvalidSlot || !lookup.accessible) return PropInfo();

  // If it's a declared property we're good to go: even if a subclass redefines
  // an accessible property with the same name it's guaranteed to be at the same
  // offset.
  return PropInfo(
    baseClass->declPropOffset(idx),
    baseClass->declPropRepoAuthType(idx),
    baseClass
  );
}

//////////////////////////////////////////////////////////////////////

bool constrainBase(MTS& env, TypeConstraint tc) {
  // Member operations only care about the inner type of the base if it's
  // boxed, so this handles the logic of using the inner constraint when
  // appropriate.
  if (env.base.type.maybe(TBoxedCell)) {
    tc.category = DataTypeCountness;
  }
  return env.irb.constrainValue(env.base.value, tc);
}

folly::Optional<TypeConstraint> simpleOpConstraint(SimpleOp op) {
  switch (op) {
    case SimpleOp::None:
      return folly::none;

    case SimpleOp::Array:
    case SimpleOp::ProfiledPackedArray:
    case SimpleOp::ProfiledStructArray:
    case SimpleOp::String:
      return TypeConstraint(DataTypeSpecific);

    case SimpleOp::PackedArray:
      return TypeConstraint(DataTypeSpecialized).setWantArrayKind();

    case SimpleOp::StructArray:
      return TypeConstraint(DataTypeSpecialized).setWantArrayShape();

    case SimpleOp::Vector:
      return TypeConstraint(c_Vector::classof());

    case SimpleOp::Map:
      return TypeConstraint(c_Map::classof());

    case SimpleOp::Pair:
      return TypeConstraint(c_Pair::classof());
  }

  always_assert(false);
}

void specializeBaseIfPossible(MTS& env, Type baseType) {
  if (auto tc = simpleOpConstraint(env.simpleOp)) {
    constrainBase(env, *tc);
    return;
  }

  if (baseType < TObj && baseType.clsSpec()) {
    constrainBase(env, TypeConstraint(baseType.clsSpec().cls()));
  }
}

//////////////////////////////////////////////////////////////////////

/*
 * Returns a pointer to a particular field in the MInstrState structure.
 *
 * Must not be called if !env.needMIS.
 */
SSATmp* misLea(MTS& env, ptrdiff_t offset) {
  assertx(env.needMIS);
  env.irb.fs().setNeedRatchet(true);
  auto const offvalue = cns(env, safe_cast<int32_t>(offset));
  return gen(env, LdMIStateAddr, offvalue);
}

SSATmp* tvRefPtr(MTS& env) {
  return misLea(env, offsetof(MInstrState, tvRef));
}

SSATmp* ptrToInitNull(IRGS& env) {
  // Nothing is allowed to write anything to the init null variant, so this
  // inner type is always true.
  return cns(env, Type::cns(&init_null_variant, TPtrToMembInitNull));
}

SSATmp* ptrToUninit(IRGS& env) {
  // Nothing can write to the uninit null variant either, so the inner type
  // here is also always true.
  return cns(env, Type::cns(&null_variant, TPtrToMembUninit));
}

SSATmp* getInput(IRGS& env, unsigned i, TypeConstraint tc) {
  auto const& l = env.currentNormalizedInstruction->inputs[i];

  switch (l.space) {
    case Location::Stack: {
      auto const offset = l.bcRelOffset;
      assertx(offset.offset >= 0);
      return top(env, offset, tc);
    }

    case Location::Local:
      // N.B. Exit block for LdLocPseudoMain is nullptr because we always
      // InterpOne member instructions in pseudomains
      return ldLoc(env, l.offset, nullptr, tc);

    case Location::Litstr:
      return cns(env, curUnit(env)->lookupLitstrId(l.offset));

    case Location::Litint:
      return cns(env, l.offset);

    case Location::This:
      // If we don't have a current class context, this instruction will be
      // unreachable.
      if (!curClass(env)) PUNT(Unreachable-LdThis);
      return ldThis(env);

    default: not_reached();
  }
}

bool mightCallMagicPropMethod(MInstrAttr mia, PropInfo propInfo) {
  if (!typeFromRAT(propInfo.repoAuthType).maybe(TUninit)) {
    return false;
  }
  auto const cls = propInfo.baseClass;
  if (!cls) return true;
  // NB: this function can't yet be used for unset or isset contexts.  Just get
  // and set.
  auto const relevant_attrs =
    // Magic getters can be invoked both in define contexts and non-define
    // contexts.
    AttrNoOverrideMagicGet |
    // But magic setters are only possible in define contexts.
    ((mia & MIA_define) ? AttrNoOverrideMagicSet : AttrNone);
  bool const no_override_magic =
    (cls->attrs() & relevant_attrs) == relevant_attrs;
  return !no_override_magic;
}

bool mInstrHasUnknownOffsets(IRGS& env) {
  auto const& ni = *env.currentNormalizedInstruction;
  auto const& mii = getMInstrInfo(ni.mInstrOp());
  unsigned ii = mii.valCount();

  // It's ok to use DataTypeGeneric here because our only caller will constrain
  // the base properly if we return true and it uses that information.
  auto const base = getInput(env, ii, DataTypeGeneric);
  auto baseType = base->type().unbox();
  if (!(baseType < (TObj | TInitNull)) || !baseType.clsSpec()) return true;
  ++ii;

  for (unsigned mi = 0; mi < ni.immVecM.size(); ++mi, ++ii) {
    auto const mc = ni.immVecM[mi];
    if (!mcodeIsProp(mc)) return true;

    auto const keyType = provenTypeFromLocation(env, ni.inputs[ii]);
    auto propInfo = getPropertyOffset(env,
                                      curClass(env),
                                      baseType.clsSpec().cls(),
                                      keyType);
    if (propInfo.offset == -1 ||
        mightCallMagicPropMethod(mii.getAttr(mc), propInfo)) {
      return true;
    }
    baseType = typeFromRAT(propInfo.repoAuthType);
  }

  return false;
}

// "Simple" bases are stack cells and locals, which imply that
// env.ni.inputs[env.mii.valCount()] is the actual base value. Other base types
// have things like Class references or global variable names in the first few
// inputs.
bool isSimpleBase(const IRGS& env) {
  auto const loc = env.currentNormalizedInstruction->immVec.locationCode();
  return loc == LL || loc == LC || loc == LR || loc == LH;
}

bool isSingleMember(const IRGS& env) {
  return env.currentNormalizedInstruction->immVecM.size() == 1;
}

bool isOptimizableCollectionClass(const Class* klass) {
  return collections::isType(klass, CollectionType::Vector,
                                    CollectionType::Map,
                                    CollectionType::Pair);
}

/*
 * Inspect the instruction we're about to translate and determine if it can be
 * executed without using an MInstrState struct.
 */
void checkMIState(MTS& env) {
  // We're definitely going to punt in emitBaseN, so we might not have guarded
  // the base's type.
  if (env.immVec.locationCode() == LNL || env.immVec.locationCode() == LNC) {
    return;
  }

  Type baseType       = env.base.type.derefIfPtr();
  auto const isCGetM  = env.op == Op::CGetM;
  auto const isSetM   = env.op == Op::SetM;
  auto const isIssetM = env.op == Op::IssetM;
  auto const isUnsetM = env.op == Op::UnsetM;
  auto const isSingle = env.immVecM.size() == 1;

  // We don't need to bother with weird base types.
  if (baseType.maybe(TCell) && baseType.maybe(TBoxedCell)) {
    return;
  }

  if (baseType <= TBoxedCell) {
    baseType = ldRefReturn(baseType.unbox());
  }

  // CGetM or SetM with no unknown property offsets.
  auto const simpleProp = [&]() {
    if (!isCGetM && !isSetM) return false;
    if (mInstrHasUnknownOffsets(env)) return false;
    auto const cls = baseType.clsSpec().cls();
    if (cls == nullptr) return false;
    return !constrainBase(env, TypeConstraint(cls).setWeak());
  }();

  // Final operations that don't use MIState.
  auto const simpleFinal = isCGetM || isSetM || isIssetM || isUnsetM;

  if (simpleProp || (isSingle && simpleFinal)) {
    env.needMIS = false;

    // Element access with one element in the vector.
    auto const singleElem = isSingle && mcodeIsElem(env.immVecM[0]);

    // CGetM or IssetM with one vector array element and a collection type.
    auto const simpleCollection =
      (isIssetM || isCGetM) &&
      singleElem &&
      baseType < TObj &&
      isOptimizableCollectionClass(baseType.clsSpec().cls());

    if (simpleCollection) {
      constrainBase(env, TypeConstraint(baseType.clsSpec().cls()));
    } else {
      constrainBase(env, DataTypeSpecific);
    }
  }
}

void emitMTrace(MTS& env) {
  auto rttStr = [&](int i) {
    return predictedTypeFromLocation(env, env.ni.inputs[i]).unbox().toString();
  };
  std::ostringstream shape;
  int iInd = env.mii.valCount();
  const char* separator = "";

  shape << opcodeToName(env.op) << " <";
  auto baseLoc = env.immVec.locationCode();
  shape << folly::format("{}:{} ", locationCodeString(baseLoc), rttStr(iInd));
  ++iInd;

  for (int mInd = 0; mInd < env.immVecM.size(); ++mInd) {
    auto mcode = env.immVecM[mInd];
    shape << separator;
    if (mcode == MW) {
      shape << "MW";
    } else if (mcodeIsElem(mcode)) {
      shape << "ME:" << rttStr(iInd);
    } else if (mcodeIsProp(mcode)) {
      shape << "MP:" << rttStr(iInd);
    } else {
      not_reached();
    }
    if (mcode != MW) ++iInd;
    separator = " ";
  }
  shape << '>';
  gen(env,
      IncStatGrouped,
      cns(env, makeStaticString("vector instructions")),
      cns(env, makeStaticString(shape.str())),
      cns(env, 1));
}

void setBase(MTS& env,
             SSATmp* tmp,
             folly::Optional<Type> baseType = folly::none) {
  env.base.value = tmp;
  env.base.type = baseType ? *baseType : env.base.value->type();
  always_assert(env.base.type <= env.base.value->type());
}

SSATmp* getUnconstrainedKey(MTS& env) {
  return getInput(env, env.iInd, DataTypeGeneric);
}

SSATmp* getKey(MTS& env) {
  auto key = getInput(env, env.iInd, DataTypeSpecific);
  auto const keyType = key->type();

  assertx(keyType <= TCell || keyType <= TBoxedCell);
  if (keyType <= TBoxedCell) {
    key = gen(env, LdRef, TInitCell, key);
  }
  return key;
}

SSATmp* getValue(MTS& env) {
  // If an instruction takes an rhs, it's always input 0.
  assertx(env.mii.valCount() == 1);
  const int kValIdx = 0;
  return getInput(env, kValIdx, DataTypeGeneric);
}

//////////////////////////////////////////////////////////////////////

// Compute whether the current instruction a 1-element simple collection
// (includes Array) operation.
SimpleOp computeSimpleCollectionOp(
  const IRGS& env,
  Type(*getType)(const IRGS&, const Location&)
) {
  // DataTypeGeneric is used in here to avoid constraining the base in case we
  // end up not caring about the type. Consumers of the return value must
  // constrain the base as appropriate.
  if (!isSimpleBase(env)) return SimpleOp::None;

  auto const& ni = *env.currentNormalizedInstruction;
  auto const op = ni.mInstrOp();
  auto const& mii = getMInstrInfo(ni.mInstrOp());
  auto baseType = getType(env, ni.inputs[mii.valCount()]);
  if (baseType.maybe(TCell) && baseType.maybe(TBoxedCell)) {
    // We might be doing a Base NL or something similar.  Either way we can't
    // do a simple op if we have a mixed boxed/unboxed type.
    return SimpleOp::None;
  }

  auto const& baseL = ni.inputs[mii.valCount()];
  // Before we do any simpleCollectionOp on a local base, we will always emit
  // the appropriate CheckRefInner guard to allow us to use a predicted inner
  // type.  So when calculating the SimpleOp assume that type.
  if (baseType.maybe(TBoxedCell) && baseL.isLocal()) {
    baseType = env.irb->predictedInnerType(baseL.offset);
  }

  bool const readInst = (op == Op::CGetM || op == Op::IssetM);
  if ((op == OpSetM || readInst) && isSimpleBase(env) &&
      isSingleMember(env)) {
    if (baseType <= TArr) {
      auto isPacked = false;
      auto isStruct = false;
      if (auto arrSpec = baseType.arrSpec()) {
        isPacked = arrSpec.kind() == ArrayData::kPackedKind;
        isStruct = arrSpec.kind() == ArrayData::kStructKind &&
                   arrSpec.shape() != nullptr;
      }
      if (mcodeIsElem(ni.immVecM[0])) {
        auto const keyType = getType(env, ni.inputs[mii.valCount() + 1]);
        if (keyType <=TInt || keyType <= TStr) {
          if (readInst) {
            if (keyType <= TInt) {
              return isPacked ? SimpleOp::PackedArray
                              : SimpleOp::ProfiledPackedArray;
            } else if (keyType.hasConstVal(TStaticStr)) {
              if (!isStruct || !baseType.arrSpec().shape()) {
                return SimpleOp::ProfiledStructArray;
              }
              return SimpleOp::StructArray;
            }
          }
          return SimpleOp::Array;
        }
      }
    } else if (baseType <= TStr && mcodeMaybeArrayIntKey(ni.immVecM[0])) {
      auto const keyType = getType(env, ni.inputs[mii.valCount() + 1]);
      if (keyType <= TInt) {
        // Don't bother with SetM on strings, because profile data
        // shows it basically never happens.
        if (readInst) return SimpleOp::String;
      }
    } else if (baseType < TObj) {
      const Class* klass = baseType.clsSpec().cls();
      auto const isVector = collections::isType(klass, CollectionType::Vector);
      auto const isImmVector =
        collections::isType(klass, CollectionType::ImmVector);
      auto const isPair   = collections::isType(klass, CollectionType::Pair);
      auto const isMap    = collections::isType(klass, CollectionType::Map);
      auto const isImmMap = collections::isType(klass, CollectionType::ImmMap);

      if (isVector || isPair || (isImmVector && readInst)) {
        if (mcodeMaybeVectorKey(ni.immVecM[0])) {
          auto const keyType = getType(env, ni.inputs[mii.valCount() + 1]);
          if (keyType <= TInt) {
            // We don't specialize setting pair elements.
            if (isPair && op == Op::SetM) return SimpleOp::None;
            return (isImmVector || isVector)
              ? SimpleOp::Vector : SimpleOp::Pair;
          }
        }
      } else if (isMap || (isImmMap && readInst)) {
        if (mcodeIsElem(ni.immVecM[0])) {
          auto const keyType = getType(env, ni.inputs[mii.valCount() + 1]);
          if (keyType <= TInt || keyType <= TStr) {
            return SimpleOp::Map;
          }
        }
      }
    }
  }

  return SimpleOp::None;
}

/*
 * Punt if the given base type isn't known to be boxed or unboxed.
 */
void checkGenBase(Type baseType) {
  if (baseType.maybe(TCell) && baseType.maybe(TBoxedCell)) {
    PUNT(MInstr-GenBase);
  }
}

//////////////////////////////////////////////////////////////////////
// Base ops

void emitBaseLCR(MTS& env) {
  auto const& mia = env.mii.getAttr(env.immVec.locationCode());
  auto const& baseL = env.ni.inputs[env.iInd];
  // We use DataTypeGeneric here because we might not care about the type. If
  // we do, it's constrained further.
  auto base = getInput(env, env.iInd, DataTypeGeneric);
  auto baseType = base->type();

  checkGenBase(baseType);

  // Check for Uninit and warn if needed.
  if (baseL.isLocal() && (mia & MIA_warn) && baseType <= TUninit) {
    env.irb.constrainLocal(baseL.offset, DataTypeSpecific,
                           "emitBaseLCR: Uninit base local");
    gen(env,
        RaiseUninitLoc,
        cns(env, curFunc(env)->localVarName(baseL.offset)));
  }

  /*
   * If the base is boxed, and from a local, we can do a better translation
   * using the inner type after guarding.  If we're going to do a generic
   * translation that uses a pointer to the local we still want this
   * LdRef---some of the translations will be smarter if they know the inner
   * type.  This is the first code emitted for the minstr so it's ok to
   * side-exit here.
   */
  Block* failedRef = baseType <= TBoxedCell ? makeExit(env) : nullptr;
  if (baseType <= TBoxedCell && baseL.isLocal()) {
    auto const predTy = env.irb.predictedInnerType(baseL.offset);
    gen(env, CheckRefInner, predTy, failedRef, base);
    base = gen(env, LdRef, predTy, base);
    baseType = base->type();
  }

  // Check for common cases where we can pass the base by value, we unboxed
  // above if it was needed.
  if ((baseType <= TObj && mcodeIsProp(env.immVecM[0])) ||
      env.simpleOp != SimpleOp::None) {
    // Register that we care about the specific type of the base, though, and
    // might care about its specialized type.
    setBase(env, base, baseType);
    constrainBase(env, DataTypeSpecific);
    specializeBaseIfPossible(env, baseType);
    return;
  }

  // Everything else is passed by pointer. We don't have to worry about
  // unboxing, since all the generic helpers understand boxed bases. They still
  // may rely on the CheckRefInner guard above, though; the various emit*
  // functions may do smarter things based on the guarded type.
  if (baseL.space == Location::Local) {
    setBase(
      env,
      ldLocAddr(env, baseL.offset),
      env.irb.localType(
        baseL.offset,
        DataTypeSpecific
      ).ptr(Ptr::Frame)
    );
  } else {
    assertx(baseL.space == Location::Stack);
    auto const stkType = env.irb.stackType(
      offsetFromIRSP(env, baseL.bcRelOffset),
      DataTypeGeneric
    );
    setBase(
      env,
      ldStkAddr(env, baseL.bcRelOffset),
      stkType.ptr(Ptr::Stk)
    );
  }
  assertx(env.base.value->type() <= TPtrToGen);
  assertx(env.base.type <= TPtrToGen);

  // TODO(t2598894): We do this for consistency with the old guard relaxation
  // code, but may change it in the future.
  constrainBase(env, DataTypeSpecific);
}

void emitBaseH(MTS& env) {
  setBase(env, getInput(env, env.iInd, DataTypeSpecific));
}

void emitBaseN(MTS& env) {
  // If this is ever implemented, the check at the beginning of
  // checkMIState must be removed/adjusted as appropriate.
  PUNT(emitBaseN);
}

void emitBaseG(MTS& env) {
  auto const& mia = env.mii.getAttr(env.immVec.locationCode());
  auto const gblName = getInput(env, env.iInd, DataTypeSpecific);
  if (!gblName->isA(TStr)) PUNT(BaseG-non-string-name);
  setBase(
    env,
    gen(env, BaseG, MInstrAttrData { mia }, gblName)
  );
}

void emitBaseS(MTS& env) {
  const int kClassIdx = env.ni.inputs.size() - 1;
  auto const key = getKey(env);
  auto const clsRef = getInput(env, kClassIdx,
                               DataTypeGeneric /* will be a Cls */);

  /*
   * Note, the base may be a pointer to a boxed type after this.  We don't
   * unbox here, because we never are going to generate a special translation
   * unless we know it's not boxed, and the C++ helpers for generic dims
   * currently always conditionally unbox.
   */
  setBase(env, ldClsPropAddr(env, clsRef, key, true));
}

void emitBaseOp(MTS& env) {
  switch (env.immVec.locationCode()) {
  case LL: case LC: case LR: emitBaseLCR(env); break;
  case LH:                   emitBaseH(env);   break;
  case LGL: case LGC:        emitBaseG(env);   break;
  case LNL: case LNC:        emitBaseN(env);   break;
  case LSL: case LSC:        emitBaseS(env);   break;
  case InvalidLocationCode:
    not_reached();
  }
}

//////////////////////////////////////////////////////////////////////
// Intermediate ops

PropInfo getCurrentPropertyOffset(IRGS& env, SSATmp* base,
                                  Type baseType, Type keyType) {
  // We allow the use of clases from nullable objects because
  // emitPropSpecialized() explicitly checks for null (when needed) before
  // doing the property access.
  baseType = baseType.derefIfPtr();
  if (!(baseType < (TObj | TInitNull) && baseType.clsSpec())) return PropInfo{};

  auto const baseCls = baseType.clsSpec().cls();
  auto const info = getPropertyOffset(env, curClass(env), baseCls, keyType);
  if (info.offset == -1) return info;

  if (env.irb->constrainValue(base,
                              TypeConstraint(baseCls).setWeak())) {
    // We can't use this specialized class without making a guard more
    // expensive, so don't do it.
    return PropInfo{};
  }

  return info;
}

PropInfo getCurrentPropertyOffset(MTS& env) {
  return getCurrentPropertyOffset(
    env, env.base.value, env.base.type,
    provenTypeFromLocation(env, env.ni.inputs[env.iInd])
  );
}

/*
 * Helper for emitPropSpecialized to check if a property is Uninit. It returns
 * a pointer to the property's address, or init_null_variant if the property
 * was Uninit and doWarn is true.
 *
 * We can omit the uninit check for properties that we know may not be uninit
 * due to the frontend's type inference.
 */
SSATmp* checkInitProp(IRGS& env,
                      SSATmp* baseAsObj,
                      SSATmp* propAddr,
                      SSATmp* key,
                      bool doWarn,
                      bool doDefine) {
  assertx(key->isA(TStaticStr));
  assertx(baseAsObj->isA(TObj));
  assertx(propAddr->type() <= TPtrToGen);
  assertx(!doWarn || !doDefine);

  auto const needsCheck = doWarn && propAddr->type().deref().maybe(TUninit);
  if (!needsCheck) return propAddr;

  return cond(
    env,
    [&] (Block* taken) {
      gen(env, CheckInitMem, taken, propAddr);
    },
    [&] { // Next: Property isn't Uninit. Do nothing.
      return propAddr;
    },
    [&] { // Taken: Property is Uninit. Raise a warning and return a pointer to
          // init_null_variant.
      hint(env, Block::Hint::Unlikely);
      if (wantPropSpecializedWarnings()) {
        gen(env, RaiseUndefProp, baseAsObj, key);
      }
      return ptrToInitNull(env);
    }
  );
}

SSATmp* emitPropSpecialized(
  IRGS& env,
  SSATmp* base,
  Type baseType,
  SSATmp* key,
  bool nullsafe,
  const MInstrAttr mia,
  PropInfo propInfo
) {
  assertx(!(mia & MIA_warn) || !(mia & MIA_unset));
  const bool doWarn   = mia & MIA_warn;
  const bool doDefine = mia & MIA_define || mia & MIA_unset;

  auto const initNull = ptrToInitNull(env);

  /*
   * Normal case, where the base is an object (and not a pointer to
   * something)---just do a lea with the type information we got from static
   * analysis.  The caller of this function will use it to know whether it can
   * avoid a generic incref, unbox, etc.
   */
  if (baseType <= TObj) {
    auto const propAddr = gen(
      env,
      LdPropAddr,
      PropOffset { propInfo.offset },
      typeFromRAT(propInfo.repoAuthType).ptr(Ptr::Prop),
      base
    );
    return checkInitProp(env, base, propAddr, key, doWarn, doDefine);
  }

  /*
   * We also support nullable objects for the base.  This is a frequent result
   * of static analysis on multi-dim property accesses ($foo->bar->baz), since
   * hhbbc doesn't try to prove __construct must be run or that sort of thing
   * (so every object-holding object property can also be null).
   *
   * After a null check, if it's actually an object we can just do LdPropAddr,
   * otherwise we just give out a pointer to the init_null_variant (after
   * raising the appropriate warnings).
   */
  return cond(
    env,
    [&] (Block* taken) {
      gen(env, CheckTypeMem, TObj, taken, base);
    },
    [&] {
      // Next: Base is an object. Load property and check for uninit.
      auto const obj = gen(env, LdMem, baseType.deref() & TObj, base);
      auto const propAddr = gen(
        env,
        LdPropAddr,
        PropOffset { propInfo.offset },
        typeFromRAT(propInfo.repoAuthType).ptr(Ptr::Prop),
        obj
      );
      return checkInitProp(env, obj, propAddr, key, doWarn, doDefine);
    },
    [&] { // Taken: Base is Null. Raise warnings/errors and return InitNull.
      hint(env, Block::Hint::Unlikely);
      if (!nullsafe && doWarn) {
        auto const msg = makeStaticString(
            "Cannot access property on non-object");
        gen(env, RaiseNotice, cns(env, msg));
      }
      if (doDefine) {
        /*
         * This case is where we're logically supposed to do stdClass
         * promotion.  However, it's impossible that we're going to be asked to
         * do this with the way type inference works ahead of time right now:
         *
         *   o In defining minstrs, the only way hhbbc will know that an object
         *     type is nullable and also specialized is if the only type it can
         *     be is ?Obj=stdClass.  (This is because it does object property
         *     inference in a control flow insensitive way, so if null is
         *     possible stdClass must be added to the type, and there are no
         *     unions of multiple specialized object types.)
         *
         *   o On the other hand, if the type was really ?Obj=stdClass, we
         *     wouldn't have gotten a known property offset for any properties,
         *     because stdClass has no declared properties, so we can't be
         *     here.
         *
         * We could punt, but it's better to assert for now because if we
         * change this in hhbbc it will be on-purpose...
         */
        always_assert_flog(
          false,
          "Static analysis error: we would've needed to generate "
          "stdClass-promotion code in the JIT, which is unexpected."
        );
      }
      return initNull;
    }
  );
}

SSATmp* emitPropSpecialized(MTS& env, const MInstrAttr mia,
                            PropInfo propInfo) {
  return emitPropSpecialized(
    env, env.base.value, env.base.type, getKey(env),
    env.immVecM[env.mInd] == MQT, mia, propInfo
  );
}

void emitPropGeneric(MTS& env) {
  auto const mCode = env.immVecM[env.mInd];
  auto const mia = MInstrAttr(env.mii.getAttr(mCode) & MIA_intermediate_prop);
  auto const nullsafe = (mCode == MQT);

  if ((mia & MIA_unset) && !env.base.type.strip().maybe(TObj)) {
    constrainBase(env, DataTypeSpecific);
    setBase(env, ptrToInitNull(env));
    return;
  }

  auto const key = getKey(env);

  if (mia & MIA_define) {
    if (nullsafe) {
      gen(
        env,
        RaiseError,
        cns(env, makeStaticString(Strings::NULLSAFE_PROP_WRITE_ERROR))
      );
      setBase(env, ptrToInitNull(env));
      return;
    }
    setBase(
      env,
      gen(
        env,
        PropDX,
        MInstrAttrData { mia },
        env.base.value,
        key,
        tvRefPtr(env)
      )
    );
  } else {
    setBase(
      env,
      nullsafe
        ? gen(
            env,
            PropQ,
            env.base.value,
            key,
            tvRefPtr(env)
          )
        : gen(
            env,
            PropX,
            MInstrAttrData { mia },
            env.base.value,
            key,
            tvRefPtr(env)
          )
    );
  }
}

void emitProp(MTS& env) {
  const auto propInfo   = getCurrentPropertyOffset(env);
  auto mia = env.mii.getAttr(env.immVecM[env.mInd]);
  if (propInfo.offset == -1 || (mia & MIA_unset) ||
      mightCallMagicPropMethod(mia, propInfo)) {
    emitPropGeneric(env);
  } else {
    setBase(env, emitPropSpecialized(env, mia, propInfo));
  }
}

void emitElem(MTS& env) {
  auto const mCode = env.immVecM[env.mInd];
  auto const mia = MInstrAttr(env.mii.getAttr(mCode) & MIA_intermediate);
  auto const key = getKey(env);

  auto const warn   = mia & MIA_warn;
  auto const unset  = mia & MIA_unset;
  auto const define = mia & MIA_define;

  assertx(!define || !unset);
  assertx(!define || !warn);

  // Fast path for the common/easy case.
  if (env.base.type <= TPtrToArr && !unset && !define &&
      key->type().subtypeOfAny(TInt, TStr)) {
    setBase(
      env,
      gen(env,
          warn ? ElemArrayW : ElemArray,
          gen(env, LdMem, TArr, env.base.value),
          key)
    );
    return;
  }

  if (env.base.type <= TPtrToArr && (define || unset) &&
      key->type().subtypeOfAny(TInt, TStr)) {
    setBase(
      env,
      gen(env, unset ? ElemArrayU : ElemArrayD, env.base.value, key)
    );
    return;
  }

  if (unset) {
    auto const uninit = ptrToUninit(env);
    auto const baseType = env.base.type.strip();
    constrainBase(env, DataTypeSpecific);
    if (baseType <= TStr) {
      gen(
        env,
        RaiseError,
        cns(env, makeStaticString(Strings::OP_NOT_SUPPORTED_STRING))
      );
      setBase(env, uninit);
      return;
    }
    if (!baseType.maybe(TArr | TObj)) {
      setBase(env, uninit);
      return;
    }
  }

  if (define || unset) {
    setBase(
      env,
      gen(env,
          define ? ElemDX : ElemUX,
          MInstrAttrData { mia },
          env.base.value,
          key,
          tvRefPtr(env))
    );
    return;
  }
  setBase(
    env,
    gen(env,
        ElemX,
        MInstrAttrData { mia },
        env.base.value,
        key,
        tvRefPtr(env))
  );
}

void emitNewElem(MTS& env) { PUNT(emitNewElem); }

void emitIntermediateOp(MTS& env) {
  switch (env.immVecM[env.mInd]) {
    case MEC: case MEL: case MET: case MEI: {
      emitElem(env);
      ++env.iInd;
      break;
    }
    case MQT:
    case MPC: case MPL: case MPT:
      emitProp(env);
      ++env.iInd;
      break;
    case MW:
      assertx(env.mii.newElem());
      emitNewElem(env);
      break;
    case InvalidMemberCode:
      not_reached();
  }
}

//////////////////////////////////////////////////////////////////////

bool needFirstRatchet(MTS& env) {
  if (!isSimpleBase(env)) return true;

  auto const firstVal = getInput(env, env.mii.valCount(), DataTypeSpecific);
  auto const firstTy = firstVal->type().unbox();
  if (firstTy <= TArr) {
    if (mcodeIsElem(env.immVecM[0])) return false;
    return true;
  }

  // Using the specialized type here is safe because we only elide the first
  // ratchet if the first member instruction is a property access, in which
  // case we have constrained the specialized type of the base in emitBaseLCR
  if (!(firstTy < TObj && firstTy.clsSpec())) return true;
  auto const firstCls = firstTy.clsSpec().cls();

  auto const no_overrides = AttrNoOverrideMagicGet|
    AttrNoOverrideMagicSet|
    AttrNoOverrideMagicIsset|
    AttrNoOverrideMagicUnset;
  if ((firstCls->attrs() & no_overrides) != no_overrides) {
    // Note: we could also add a check here on whether the first property RAT
    // contains Uninit---if not we can still return false.  See
    // mightCallMagicPropMethod.
    return true;
  }

  if (firstCls->hasNativePropHandler()) {
    auto propInfo = getPropertyOffset(
      env, curClass(env), firstCls,
      provenTypeFromLocation(env, env.ni.inputs[env.mii.valCount() + 1])
    );
    // For native properties if the property is declared then we know don't
    // call the native handler
    if (propInfo.offset == -1) return true;
  }

  return !mcodeIsProp(env.immVecM[0]);
}

bool needFinalRatchet(const MTS& env) { return env.mii.finalGet(); }

// Ratchet operations occur after each intermediate operation, except
// possibly the first and last (see need{First,Final}Ratchet()).  No actual
// ratchet occurs after the final operation, but this means that both tvRef
// and tvRef2 can contain references just after the final operation.  Here we
// pretend that a ratchet occurs after the final operation, i.e. a "logical"
// ratchet.  The reason for counting logical ratchets as part of the total is
// the following case, in which the logical count is 0:
//
//   (base is array)
//   BaseL
//   IssetElemL
//     no logical ratchet
//
// Following are a few more examples to make the algorithm clear:
//
//   (base is array)      (base is object*)  (base is object*)
//   BaseL                BaseL              BaseL
//   ElemL                ElemL              CGetPropL
//     no ratchet           ratchet            logical ratchet
//   ElemL                PropL
//     ratchet              ratchet
//   ElemL                CGetElemL
//     ratchet              logical ratchet
//   IssetElemL
//     logical ratchet
//
//   (base is array)        * If the base is a known (specialized) object type,
//   BaseL                    we can also avoid the first rachet if we can
//   ElemL                    prove it can't possibly invoke magic methods.
//     no ratchet
//   ElemL
//     ratchet
//   ElemL
//     logical ratchet
//   SetElemL
//     no ratchet
unsigned nLogicalRatchets(MTS& env) {
  // If we've proven elsewhere that we don't need an MInstrState struct, we know
  // this translation won't need any ratchets.
  if (!env.needMIS) return 0;

  unsigned ratchets = env.immVecM.size();
  if (!env.needFirstRatchet) --ratchets;
  if (!env.needFinalRatchet) --ratchets;
  return ratchets;
}

int ratchetInd(MTS& env) {
  return env.needFirstRatchet ? int(env.mInd) : int(env.mInd) - 1;
}

/*
 * Compute and store ratchet-related fields in env, because their values can
 * depending on state that will change during translation of the minstr.
 */
void computeRatchets(MTS& env) {
  env.needFirstRatchet = needFirstRatchet(env);
  env.needFinalRatchet = needFinalRatchet(env);
  env.numLogicalRatchets = nLogicalRatchets(env);
}

void emitRatchetRefs(MTS& env) {
  if (ratchetInd(env) < 0 || ratchetInd(env) >= int(env.numLogicalRatchets)) {
    return;
  }

  if (!env.irb.fs().needRatchet()) {
    return;
  }

  auto const misRefAddr = misLea(env, offsetof(MInstrState, tvRef));

  setBase(env, cond(
    env,
    [&] (Block* taken) {
      gen(env, CheckInitMem, taken, misRefAddr);
    },
    [&] { // Next: tvRef isn't Uninit. Ratchet the refs
      auto const misRef2Addr = misLea(env, offsetof(MInstrState, tvRef2));
      // Clean up tvRef2 before overwriting it.
      if (ratchetInd(env) > 0) {
        auto const val = gen(env, LdMem, TGen, misRef2Addr);
        gen(env, DecRef, val);
      }
      // Copy tvRef to tvRef2.
      auto const tvRef = gen(env, LdMem, TGen, misRefAddr);
      gen(env, StMem, misRef2Addr, tvRef);
      // Reset tvRef.
      gen(env, StMem, misRefAddr, cns(env, TUninit));

      // Adjust base pointer.
      assertx(env.base.type <= TPtrToGen);

      // See the comment in ratchetRefs().
      return misLea(env, offsetof(MInstrState, tvRef2));
    },
    [&] { // Taken: tvRef is Uninit. Do nothing.
      return env.base.value;
    }
  ));
}

void emitMPre(MTS& env) {
  if (HPHP::Trace::moduleEnabled(HPHP::Trace::minstr, 1)) {
    emitMTrace(env);
  }

  env.simpleOp = computeSimpleCollectionOp(env, provenTypeFromLocation);
  emitBaseOp(env);
  ++env.iInd;

  checkMIState(env);
  computeRatchets(env);
  if (env.needMIS) {
    auto const uninit = cns(env, TUninit);
    if (env.numLogicalRatchets > 0) {
      gen(env, StMem, misLea(env, offsetof(MInstrState, tvRef)), uninit);
      gen(env, StMem, misLea(env, offsetof(MInstrState, tvRef2)), uninit);
    }

    // If we're using an MInstrState, all the default-created catch blocks for
    // exception paths from here out will need to clean up the tvRef{,2}
    // storage, so install a custom catch creator.
    auto const penv = &env;
    env.irgs.catchCreator = [penv] { return makeMISCatch(*penv); };
  }

  /*
   * Iterate over all but the last member, which is consumed by a final
   * operation.
   *
   * Intermediate operations (and the base op) can define new StkPtrs, even
   * though the stack depth won't be changing, so we need to have a stack
   * boundary in-between each one.
   */
  for (env.mInd = 0; env.mInd < env.immVecM.size() - 1; ++env.mInd) {
    env.irb.exceptionStackBoundary();
    emitIntermediateOp(env);
    emitRatchetRefs(env);
  }
  env.irb.exceptionStackBoundary();
}

//////////////////////////////////////////////////////////////////////
// "Simple op" handlers.

SSATmp* emitPackedArrayGet(IRGS& env, SSATmp* base, SSATmp* key) {
  assertx(base->isA(TArr) &&
          base->type().arrSpec().kind() == ArrayData::kPackedKind &&
          key->isA(TInt));

  auto doLdElem = [&] {
    auto const type = packedArrayElemType(base, key).ptr(Ptr::Arr);
    auto addr = gen(env, LdPackedArrayElemAddr, type, base, key);
    auto res = gen(env, LdMem, type.deref(), addr);
    auto unboxed = unbox(env, res, nullptr);
    gen(env, IncRef, unboxed);
    return unboxed;
  };

  if (key->hasConstVal()) {
    int64_t idx = key->intVal();
    if (base->hasConstVal()) {
      const ArrayData* arr = base->arrVal();
      if (idx < 0 || idx >= arr->size()) {
        gen(env, RaiseArrayIndexNotice, key);
        return cns(env, TInitNull);
      }
      auto const value = arr->nvGet(idx);
      return cns(env, *value);
    }

    switch (packedArrayBoundsStaticCheck(base->type(), idx)) {
    case PackedBounds::In:
      return doLdElem();
    case PackedBounds::Out:
      gen(env, RaiseArrayIndexNotice, key);
      return cns(env, TInitNull);
    case PackedBounds::Unknown:
      break;
    }
  }

  return cond(
    env,
    [&] (Block* taken) {
      gen(env, CheckPackedArrayBounds, taken, base, key);
    },
    [&] { // Next:
      return doLdElem();
    },
    [&] { // Taken:
      hint(env, Block::Hint::Unlikely);
      gen(env, RaiseArrayIndexNotice, key);
      return cns(env, TInitNull);
    }
  );
}

SSATmp* emitStructArrayGet(IRGS& env, SSATmp* base, SSATmp* key) {
  assertx(base->isA(TArr));
  assertx(base->type().arrSpec().kind() == ArrayData::kStructKind);
  assertx(base->type().arrSpec().shape());
  assertx(key->hasConstVal(TStr));
  assertx(key->strVal()->isStatic());

  const auto keyStr = key->strVal();
  const auto shape = base->type().arrSpec().shape();
  auto offset = shape->offsetFor(keyStr);

  if (offset == PropertyTable::kInvalidOffset) {
    gen(env, RaiseArrayKeyNotice, key);
    return cns(env, TInitNull);
  }

  auto res = gen(env, LdStructArrayElem, base, key);
  auto unboxed = unbox(env, res, nullptr);
  gen(env, IncRef, unboxed);
  return unboxed;
}

SSATmp* emitArrayGet(IRGS& env, SSATmp* base, SSATmp* key) {
  auto elem = unbox(env, gen(env, ArrayGet, base, key), nullptr);
  gen(env, IncRef, elem);
  return elem;
}

SSATmp* emitProfiledPackedArrayGet(IRGS& env, SSATmp* base, SSATmp* key) {
  TargetProfile<NonPackedArrayProfile> prof(env.context,
                                            env.irb->curMarker(),
                                            s_PackedArray.get());
  if (prof.profiling()) {
    gen(env, ProfilePackedArray, RDSHandleData{prof.handle()}, base);
    return emitArrayGet(env, base, key);
  }

  if (prof.optimizing()) {
    auto const data = prof.data(NonPackedArrayProfile::reduce);
    // NonPackedArrayProfile data counts how many times a non-packed array was
    // observed.  Zero means it was monomorphic (or never executed).
    auto const typePackedArr = Type::Array(ArrayData::kPackedKind);
    if (base->type().maybe(typePackedArr) &&
        (data.count == 0 || RuntimeOption::EvalJitPGOArrayGetStress)) {
      // It's safe to side-exit still because we only do these profiled array
      // gets on the first element, with simple bases and single-element dims.
      // See computeSimpleCollectionOp.
      auto const exit = makeExit(env);
      base = gen(env, CheckType, typePackedArr, exit, base);
      env.irb->constrainValue(
        base,
        TypeConstraint(DataTypeSpecialized).setWantArrayKind()
      );
      return emitPackedArrayGet(env, base, key);
    }
  }

  // Fall back to a generic array get.
  return emitArrayGet(env, base, key);
}

SSATmp* emitProfiledStructArrayGet(IRGS& env, SSATmp* base, SSATmp* key) {
  TargetProfile<StructArrayProfile> prof(env.context,
                                         env.irb->curMarker(),
                                         s_StructArray.get());
  if (prof.profiling()) {
    gen(env, ProfileStructArray, RDSHandleData{prof.handle()}, base);
    return emitArrayGet(env, base, key);
  }

  if (prof.optimizing()) {
    auto const data = prof.data(StructArrayProfile::reduce);
    // StructArrayProfile data counts how many times a non-struct array was
    // observed.  Zero means it was monomorphic (or never executed).
    //
    // It also records how many Shapes it saw. The possible values are:
    //  0 (never executed)
    //  1 (monomorphic)
    //  many (polymorphic)
    //
    // If we never executed then we fall back to generic get. If we're
    // monomorphic, we'll emit a check for that specific Shape. If we're
    // polymorphic, we'll also fall back to generic get. Eventually we'd like
    // to emit an inline cache, which should be faster than calling out of line.
    if (base->type().maybe(Type::Array(ArrayData::kStructKind))) {
      if (data.nonStructCount == 0 && data.isMonomorphic()) {
        // It's safe to side-exit still because we only do these profiled array
        // gets on the first element, with simple bases and single-element dims.
        // See computeSimpleCollectionOp.
        auto const exit = makeExit(env);
        base = gen(env, CheckType, Type::Array(data.getShape()), exit, base);
        env.irb->constrainValue(
          base,
          TypeConstraint(DataTypeSpecialized).setWantArrayShape()
        );
        return emitStructArrayGet(env, base, key);
      }
    }
  }

  // Fall back to a generic array get.
  return emitArrayGet(env, base, key);
}

void checkBounds(IRGS& env, SSATmp* idx, SSATmp* limit) {
  ifThen(
    env,
    [&](Block* taken) {
      auto ok = gen(env, CheckRange, idx, limit);
      gen(env, JmpZero, taken, ok);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      gen(env, ThrowOutOfBounds, idx);
    }
  );
}

SSATmp* emitVectorGet(IRGS& env, SSATmp* base, SSATmp* key) {
  auto const size = gen(env, LdVectorSize, base);
  checkBounds(env, key, size);
  base = gen(env, LdVectorBase, base);
  static_assert(sizeof(TypedValue) == 16,
                "TypedValue size expected to be 16 bytes");
  auto idx = gen(env, Shl, key, cns(env, 4));
  auto result = gen(env, LdElem, base, idx);
  gen(env, IncRef, result);
  return result;
}

SSATmp* emitPairGet(IRGS& env, SSATmp* base, SSATmp* key) {
  assertx(key->isA(TInt));

  auto const idx = [&] {
    if (key->hasConstVal()) {
      auto keyVal = key->intVal();
      if (keyVal < 0 || keyVal > 1) PUNT(emitPairGet);

      // no reason to check bounds
      return cns(env, keyVal * sizeof(TypedValue));
    }

    static_assert(sizeof(TypedValue) == 16,
                  "TypedValue size expected to be 16 bytes");
    checkBounds(env, key, cns(env, 2));
    return gen(env, Shl, key, cns(env, 4));
  }();

  auto const pairBase = gen(env, LdPairBase, base);
  auto const result = gen(env, LdElem, pairBase, idx);
  gen(env, IncRef, result);
  return result;
}

SSATmp* emitPackedArrayIsset(IRGS& env, SSATmp* base, SSATmp* key) {
  assertx(base->type().arrSpec().kind() == ArrayData::kPackedKind);

  auto const type = packedArrayElemType(base, key);
  if (type <= TNull) return cns(env, false);

  if (key->hasConstVal()) {
    auto const idx = key->intVal();
    switch (packedArrayBoundsStaticCheck(base->type(), idx)) {
    case PackedBounds::In: {
      if (!type.maybe(TNull)) return cns(env, true);

      auto const elemAddr = gen(env, LdPackedArrayElemAddr,
                                type.ptr(Ptr::Arr), base, key);
      return gen(env, IsNTypeMem, TNull, elemAddr);
    }
    case PackedBounds::Out:
      return cns(env, false);
    case PackedBounds::Unknown:
      break;
    }
  }

  return cond(
    env,
    [&] (Block* taken) {
      gen(env, CheckPackedArrayBounds, taken, base, key);
    },
    [&] { // Next:
      auto const elemAddr = gen(env, LdPackedArrayElemAddr,
                                type.ptr(Ptr::Arr), base, key);
      return gen(env, IsNTypeMem, TNull, elemAddr);
    },
    [&] { // Taken:
      return cns(env, false);
    }
  );
}

void emitArraySet(MTS& env, SSATmp* key, SSATmp* value) {
  assertx(env.iInd == env.mii.valCount() + 1);
  const int baseStkIdx = env.mii.valCount();
  assertx(key->type() <= TCell);
  assertx(value->type() <= TCell);

  auto const& baseLoc = env.ni.inputs[env.mii.valCount()];
  auto const setRef =
    getInput(env, env.mii.valCount(), DataTypeSpecific)->type() <= TBoxedCell;

  // No catch trace below because the helper can't throw. It may reenter to
  // call destructors so it has a sync point in nativecalls.cpp, but exceptions
  // are swallowed at destructor boundaries right now: #2182869.
  if (setRef) {
    assertx(baseLoc.space == Location::Local ||
           baseLoc.space == Location::Stack);
    auto const box = getInput(env, baseStkIdx, DataTypeSpecific);
    gen(env, ArraySetRef, env.base.value, key, value, box);
    // Unlike the non-ref case, we don't need to do anything to the stack
    // because any load of the box will be guarded.
    env.result = value;
    return;
  }

  auto const newArr = gen(
    env,
    ArraySet,
    env.base.value,
    key,
    value
  );

  // Update the base's value with the new array
  if (baseLoc.space == Location::Local) {
    // We know it's not boxed (setRef above handles that), and
    // newArr has already been incref'd in the helper.
    gen(env, StLoc, LocalId(baseLoc.offset), fp(env), newArr);
  } else if (baseLoc.space == Location::Stack) {
    auto const offset = offsetFromIRSP(env, BCSPOffset{baseStkIdx});
    gen(env, StStk, IRSPOffsetData{offset}, sp(env), newArr);
  } else {
    always_assert(false);
  }

  env.result = value;
}

void emitVectorSet(IRGS& env, SSATmp* base, SSATmp* key, SSATmp* value) {
  auto const size = gen(env, LdVectorSize, base);
  checkBounds(env, key, size);

  ifThen(
    env,
    [&] (Block* taken) {
      gen(env, VectorHasImmCopy, taken, base);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      gen(env, VectorDoCow, base);
    }
  );

  gen(env, IncRef, value);
  auto const vecBase = gen(env, LdVectorBase, base);
  static_assert(sizeof(TypedValue) == 16,
                "TypedValue size expected to be 16 bytes");
  auto const idx = gen(env, Shl, key, cns(env, 4));
  auto const oldVal = gen(env, LdElem, vecBase, idx);
  gen(env, StElem, vecBase, idx, value);
  gen(env, DecRef, oldVal);
}

//////////////////////////////////////////////////////////////////////

void emitCGetProp(MTS& env) {
  const auto propInfo = getCurrentPropertyOffset(env);

  if (propInfo.offset != -1 &&
      !mightCallMagicPropMethod(MIA_none, propInfo)) {
    setBase(env, emitPropSpecialized(env, MIA_warn, propInfo));

    if (!RuntimeOption::RepoAuthoritative) {
      auto const cellPtr = gen(env, UnboxPtr, env.base.value);
      env.result = gen(env, LdMem, TCell, cellPtr);
      gen(env, IncRef, env.result);
      return;
    }

    auto const ty = env.base.type.deref();
    auto const cellPtr = ty.maybe(TBoxedCell)
      ? gen(env, UnboxPtr, env.base.value)
      : env.base.value;

    env.result = gen(env, LdMem, ty.unbox(), cellPtr);
    gen(env, IncRef, env.result);
    return;
  }

  auto const nullsafe = (env.immVecM[env.mInd] == MQT);
  auto const key = getKey(env);

  if (nullsafe) {
    env.result = gen(env, CGetPropQ, env.base.value, key);
  } else {
    env.result =
      gen(env, CGetProp, MInstrAttrData{MIA_warn}, env.base.value, key);
  }
}

void emitVGetProp(MTS& env) {
  auto const key = getKey(env);
  if (env.immVecM[env.mInd] == MQT) {
    gen(
      env,
      RaiseError,
      cns(env, makeStaticString(Strings::NULLSAFE_PROP_WRITE_ERROR))
    );
  }
  env.result = gen(env, VGetProp, env.base.value, key, tvRefPtr(env));
}

void emitIssetProp(MTS& env) {
  auto const key = getKey(env);
  env.result = gen(env, IssetProp, env.base.value, key);
}

void emitEmptyProp(MTS& env) {
  auto const key = getKey(env);
  env.result = gen(env, EmptyProp, env.base.value, key);
}

void emitSetProp(MTS& env) {
  auto const value = getValue(env);

  /* If we know the class for the current base, emit a direct property set. */
  auto const propInfo = getCurrentPropertyOffset(env);

  if (propInfo.offset != -1 &&
      !mightCallMagicPropMethod(MIA_define, propInfo)) {
    setBase(env, emitPropSpecialized(env, MIA_define, propInfo));

    auto cellTy = env.base.type.deref();
    auto cellPtr = env.base.value;

    if (cellTy.maybe(TBoxedCell)) {
      cellTy = cellTy.unbox();
      cellPtr = gen(env, UnboxPtr, cellPtr);
    }
    auto const oldVal  = gen(env, LdMem, cellTy, cellPtr);

    env.irb.constrainValue(value, DataTypeCountness);
    gen(env, IncRef, value);
    gen(env, StMem, cellPtr, value);
    gen(env, DecRef, oldVal);
    env.result = value;
    return;
  }

  // Emit the appropriate helper call.
  auto const key = getKey(env);
  gen(env, SetProp, makeCatchSet(env), env.base.value, key, value);
  env.result = value;
}

void emitSetOpProp(MTS& env) {
  SetOpOp op = SetOpOp(env.ni.imm[0].u_OA);
  auto const key = getKey(env);
  auto const value = getValue(env);
  env.result = gen(env, SetOpProp, SetOpData { op },
                   env.base.value, key, value, tvRefPtr(env));
}

void emitIncDecProp(MTS& env) {
  auto const op = static_cast<IncDecOp>(env.ni.imm[0].u_OA);
  auto const key = getKey(env);
  auto const propInfo = getCurrentPropertyOffset(env);

  if (RuntimeOption::RepoAuthoritative &&
      propInfo.offset != -1 &&
      !mightCallMagicPropMethod(MIA_none, propInfo) &&
      !mightCallMagicPropMethod(MIA_define, propInfo)) {

    // Special case for when the property is known to be an int.
    if (env.base.type <= TObj &&
        propInfo.repoAuthType.tag() == RepoAuthType::Tag::Int) {
      DEBUG_ONLY auto const propIntTy = TInt.ptr(Ptr::Prop);
      setBase(env, emitPropSpecialized(env, MIA_define, propInfo));
      assertx(env.base.value->type() <= propIntTy);
      auto const prop = gen(env, LdMem, TInt, env.base.value);
      auto const result = incDec(env, op, prop);
      assertx(result != nullptr);
      gen(env, StMem, env.base.value, result);
      env.result = isPre(op) ? result : prop;
      return;
    }
  }

  env.result = gen(env, IncDecProp, IncDecData { op }, env.base.value, key);
}

void emitBindProp(MTS& env) {
  auto const key = getKey(env);
  auto const box = getValue(env);
  gen(env, BindProp, env.base.value, key, box, tvRefPtr(env));
  env.result = box;
}

void emitUnsetProp(MTS& env) {
  auto const key = getKey(env);
  if (!env.base.type.strip().maybe(TObj)) {
    // Noop
    constrainBase(env, DataTypeSpecific);
    return;
  }
  gen(env, UnsetProp, env.base.value, key);
}

SSATmp* emitCGetElem(IRGS& env, SSATmp* base, SSATmp* key,
                     MOpFlags flags, SimpleOp simpleOp) {
  switch (simpleOp) {
    case SimpleOp::Array:
      return emitArrayGet(env, base, key);
    case SimpleOp::PackedArray:
      return emitPackedArrayGet(env, base, key);
    case SimpleOp::StructArray:
      return emitStructArrayGet(env, base, key);
    case SimpleOp::ProfiledPackedArray:
      return emitProfiledPackedArrayGet(env, base, key);
    case SimpleOp::ProfiledStructArray:
      return emitProfiledStructArrayGet(env, base, key);
    case SimpleOp::String:
      return gen(env, StringGet, base, key);
    case SimpleOp::Vector:
      return emitVectorGet(env, base, key);
    case SimpleOp::Pair:
      return emitPairGet(env, base, key);
    case SimpleOp::Map:
      return gen(env, MapGet, base, key);
    case SimpleOp::None:
      return gen(env, CGetElem, MInstrAttrData{mOpFlagsToAttr(flags)},
                 base, key);
  }
  always_assert(false);
}

void emitCGetElem(MTS& env) {
  env.result = emitCGetElem(env, env.base.value, getKey(env),
                            MOpFlags::Warn, env.simpleOp);
}

void emitVGetElem(MTS& env) {
  auto const key = getKey(env);
  env.result = gen(env, VGetElem, env.base.value, key, tvRefPtr(env));
}

SSATmp* emitIssetElem(IRGS& env, SSATmp* base, SSATmp* key, SimpleOp simpleOp) {
  switch (simpleOp) {
  case SimpleOp::Array:
  case SimpleOp::StructArray:
  case SimpleOp::ProfiledPackedArray:
  case SimpleOp::ProfiledStructArray:
    return gen(env, ArrayIsset, base, key);
  case SimpleOp::PackedArray:
    return emitPackedArrayIsset(env, base, key);
  case SimpleOp::String:
    return gen(env, StringIsset, base, key);
  case SimpleOp::Vector:
    return gen(env, VectorIsset, base, key);
  case SimpleOp::Pair:
    return gen(env, PairIsset, base, key);
  case SimpleOp::Map:
    return gen(env, MapIsset, base, key);
  case SimpleOp::None:
    return gen(env, IssetElem, base, key);
  }

  always_assert(false);
}

void emitIssetElem(MTS& env) {
  env.result = emitIssetElem(env, env.base.value, getKey(env), env.simpleOp);
}

void emitEmptyElem(MTS& env) {
  auto const key = getKey(env);
  env.result = gen(env, EmptyElem, env.base.value, key);
}

void emitSetNewElem(MTS& env) {
  auto const value = getValue(env);
  if (env.base.type <= TPtrToArr) {
    constrainBase(env, DataTypeSpecific);
    gen(env, SetNewElemArray, makeCatchSet(env), env.base.value, value);
  } else {
    gen(env, SetNewElem, makeCatchSet(env), env.base.value, value);
  }
  env.result = value;
}

void emitSetWithRefLProp(MTS& env) { SPUNT(__func__); }
void emitSetWithRefRProp(MTS& env) { emitSetWithRefLProp(env); }

void emitSetWithRefNewElem(MTS& env) {
  auto const val = getValue(env);
  gen(env, SetWithRefNewElem, env.base.value, val, tvRefPtr(env));
  env.result = nullptr;
}

void emitSetElem(MTS& env) {
  auto const value = getValue(env);
  auto const key = getKey(env);

  switch (env.simpleOp) {
  case SimpleOp::Array:
  case SimpleOp::ProfiledPackedArray:
  case SimpleOp::ProfiledStructArray:
    emitArraySet(env, key, value);
    break;
  case SimpleOp::PackedArray:
  case SimpleOp::StructArray:
  case SimpleOp::String:
    always_assert(false && "Bad SimpleOp in emitSetElem");
    break;
  case SimpleOp::Vector:
    emitVectorSet(env, env.base.value, key, value);
    env.result = value;
    break;
  case SimpleOp::Map:
    gen(env, MapSet, env.base.value, key, value);
    env.result = value;
    break;
  case SimpleOp::Pair:
  case SimpleOp::None:
    constrainBase(env, DataTypeSpecific);
    auto const result = gen(env, SetElem, makeCatchSet(env),
                            env.base.value, key, value);
    auto const t = result->type();
    if (t == TNullptr) {
      // Base is not a string. Result is always value.
      env.result = value;
    } else if (t == TCountedStr) {
      // Base is a string. Stack result is a new string so we're responsible for
      // decreffing value.
      env.irb.constrainValue(value, DataTypeCountness);
      env.result = result;
      gen(env, DecRef, value);
    } else {
      assertx(t == (TCountedStr | TNullptr));
      // Base might be a string. Assume the result is value, then inform
      // emitMPost that it needs to test the actual result.
      env.result = value;
      env.strTestResult = result;
    }
    break;
  }
}

void emitSetWithRefElem(MTS& env) {
  auto const key = getUnconstrainedKey(env);
  auto const val = getValue(env);
  gen(env, SetWithRefElem, env.base.value, key, val, tvRefPtr(env));
  env.result = nullptr;
}

void emitSetWithRefLElem(MTS& env) { emitSetWithRefElem(env); }
void emitSetWithRefRElem(MTS& env) { emitSetWithRefElem(env); }

void emitSetOpElem(MTS& env) {
  auto const op = static_cast<SetOpOp>(env.ni.imm[0].u_OA);
  env.result = gen(env, SetOpElem, SetOpData{op},
                   env.base.value, getKey(env), getValue(env),
                   tvRefPtr(env));
}

void emitIncDecElem(MTS& env) {
  auto const op = static_cast<IncDecOp>(env.ni.imm[0].u_OA);
  env.result = gen(env, IncDecElem, IncDecData { op },
                   env.base.value, getKey(env), tvRefPtr(env));
}

void emitBindElem(MTS& env) {
  auto const key = getKey(env);
  auto const box = getValue(env);
  gen(env, BindElem, env.base.value, key, box, tvRefPtr(env));
  env.result = box;
}

void emitUnsetElem(MTS& env) {
  auto const key = getKey(env);

  auto const baseType = env.base.type.strip();
  constrainBase(env, DataTypeSpecific);
  if (baseType <= TStr) {
    gen(env,
        RaiseError,
        cns(env, makeStaticString(Strings::CANT_UNSET_STRING)));
    return;
  }
  if (!baseType.maybe(TArr | TObj)) {
    // Noop
    return;
  }

  gen(env, UnsetElem, env.base.value, key);
}

void emitNotSuppNewElem(MTS& env) {
  PUNT(NotSuppNewElem);
}

void emitVGetNewElem(MTS& env) {
  SPUNT(__func__);
}

void emitSetOpNewElem(MTS& env) {
  SPUNT(__func__);
}

void emitIncDecNewElem(MTS& env) {
  SPUNT(__func__);
}

void emitBindNewElem(MTS& env) {
  auto const box = getValue(env);
  gen(env, BindNewElem, env.base.value, box, tvRefPtr(env));
  env.result = box;
}

void emitFinalMOp(MTS& env) {
  using MemFun = void (*)(MTS&);

  switch (env.immVecM[env.mInd]) {
  case MEC: case MEL: case MET: case MEI:
    static constexpr MemFun elemOps[] = {
#   define MII(instr, ...) &emit##instr##Elem,
    MINSTRS
#   undef MII
    };
    elemOps[env.mii.instr()](env);
    break;

  case MQT:
  case MPC: case MPL: case MPT:
    static constexpr MemFun propOps[] = {
#   define MII(instr, ...) &emit##instr##Prop,
    MINSTRS
#   undef MII
    };
    propOps[env.mii.instr()](env);
    break;

  case MW:
    assertx(env.mii.getAttr(MW) & MIA_final);
    static constexpr MemFun newOps[] = {
#   define MII(instr, attrs, bS, iS, vC, fN) \
      &emit##fN,
    MINSTRS
#   undef MII
    };
    newOps[env.mii.instr()](env);
    break;

  case InvalidMemberCode:
    not_reached();
  }
}

//////////////////////////////////////////////////////////////////////

/*
 * Helper to generate decrefs for tvRef(2): during exception handling any
 * objects required only during vector expansion need to be DecRef'd, and we
 * also need to clean them on the main code path before exiting.
 *
 * There may be either one or two such scratch objects, in the case of a Set
 * the first of which will always be tvRef2, in all other cases if only one
 * scratch value is present it will be stored in tvRef.  TODO: that's a bit
 * weird right?
 */
void cleanTvRefs(MTS& env) {
  constexpr ptrdiff_t refOffs[] = {
    offsetof(MInstrState, tvRef),
    offsetof(MInstrState, tvRef2)
  };
  for (unsigned i = 0; i < std::min(env.numLogicalRatchets, 2U); ++i) {
    auto const addr = misLea(env, refOffs[env.failedSetBlock ? 1 - i : i]);
    auto const val  = gen(env, LdMem, TGen, addr);
    gen(env, DecRef, val);
  }
}

enum class DecRefStyle { FromCatch, FromMain };
uint32_t decRefStackInputs(MTS& env, DecRefStyle why) {
  auto const startOff = BCSPOffset{
    env.op == Op::SetM || env.op == Op::BindM ? 1 : 0
  };
  auto const hasStackRHS =
    env.mii.valCount() && env.ni.inputs[0].space == Location::Stack;
  auto const stackCnt = env.immVec.numStackValues() + hasStackRHS;
  for (auto i = startOff; i < stackCnt; ++i) {
    auto const type = topType(env, i, DataTypeGeneric);
    if (type <= TCls) continue;

    auto const tc = why == DecRefStyle::FromMain ? DataTypeSpecific
                                                 : DataTypeGeneric;
    auto const input = top(env, i, tc);
    gen(env, DecRef, input);
  }
  return stackCnt;
}

void handleStrTestResult(MTS& env) {
  if (!env.strTestResult) return;
  // We expected SetElem's base to not be a Str but might be wrong. Make an
  // exit trace to side exit to the next instruction, replacing our guess
  // with the correct stack output.
  ifThen(
    env,
    [&] (Block* taken) {
      gen(env, CheckNullptr, taken, env.strTestResult);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      auto const str = gen(env, AssertNonNull, env.strTestResult);
      gen(env, DecRef, env.result);
      auto const stackCnt = decRefStackInputs(env, DecRefStyle::FromMain);
      discard(env, stackCnt);
      cleanTvRefs(env);
      push(env, str);
      gen(env, Jmp, makeExit(env, nextBcOff(env)));
    }
  );
}

Block* makeMISCatch(MTS& env) {
  auto const exit = env.unit.defBlock(Block::Hint::Unused);
  BlockPusher bp(env.irb, makeMarker(env, bcOff(env)), exit);
  gen(env, BeginCatch);
  cleanTvRefs(env);
  gen(env, EndCatch, IRSPOffsetData { offsetFromIRSP(env, BCSPOffset{0}) },
    fp(env), sp(env));
  return exit;
}

Block* makeCatchSet(MTS& env) {
  env.failedSetBlock = env.unit.defBlock(Block::Hint::Unused);

  const bool isSetWithRef = env.op == Op::SetWithRefLM ||
                            env.op == Op::SetWithRefRM;

  BlockPusher bp(env.irb, makeMarker(env, bcOff(env)), env.failedSetBlock);
  gen(env, BeginCatch);

  ifThen(
    env,
    [&] (Block* taken) {
      gen(env, UnwindCheckSideExit, taken, fp(env), sp(env));
    },
    [&] {
      hint(env, Block::Hint::Unused);
      cleanTvRefs(env);
      gen(env, EndCatch, IRSPOffsetData { offsetFromIRSP(env, BCSPOffset{0}) },
        fp(env), sp(env));
    }
  );
  hint(env, Block::Hint::Unused);

  /*
   * Fallthrough from here on is side-exiting due to an InvalidSetMException.
   */

  // For consistency with the interpreter, decref the rhs before we decref the
  // stack inputs, and decref the ratchet storage after the stack inputs.
  if (!isSetWithRef) {
    auto const val = top(env, BCSPOffset{0}, DataTypeGeneric);
    gen(env, DecRef, val);
  }
  auto const stackCnt = decRefStackInputs(env, DecRefStyle::FromCatch);
  discard(env, stackCnt);
  cleanTvRefs(env);
  if (!isSetWithRef) {
    auto const val = gen(env, LdUnwinderValue, TCell);
    push(env, val);
  }
  gen(env, Jmp, makeExit(env, nextBcOff(env)));
  return env.failedSetBlock;
}

void emitMPost(MTS& env) {
  handleStrTestResult(env);

  auto const stackCnt = decRefStackInputs(env, DecRefStyle::FromMain);
  discard(env, stackCnt);
  cleanTvRefs(env);

  // Push result, if one was produced. If we had a predicted result
  // (strTestResult case), it was already guarded on above.
  if (env.result) {
    push(env, env.result);
  } else {
    assertx(env.op == Op::UnsetM ||
            env.op == Op::SetWithRefLM ||
            env.op == Op::SetWithRefRM);
  }
}

//////////////////////////////////////////////////////////////////////

void implMInstr(IRGS& irgs, Op effectiveOp) {
  if (curFunc(irgs)->isPseudoMain()) {
    interpOne(irgs, *irgs.currentNormalizedInstruction);
    return;
  }

  auto env = MTS { irgs, effectiveOp };
  emitMPre(env);          // Emit the base and every intermediate op
  emitFinalMOp(env);      // Emit the final operation
  emitMPost(env);         // Cleanup: decref inputs and scratch values
}

//////////////////////////////////////////////////////////////////////

bool isClassSpecializedTypeReliable(Type input) {
  assert(input.isSpecialized());
  assert(input.clsSpec());
  auto baseClass = input.clsSpec().cls();
  return RuntimeOption::RepoAuthoritative &&
      (baseClass->preClass()->attrs() & AttrUnique);
}

}

TypeConstraint mInstrBaseConstraint(const IRGS& env, Type predictedType) {
  if (!isSimpleBase(env)) return DataTypeSpecific;

  if (predictedType.isSpecialized()) {
    if (predictedType.clsSpec() &&
        isClassSpecializedTypeReliable(predictedType)) {
      return TypeConstraint(predictedType.clsSpec().cls());
    }

    auto const simpleOp =
      computeSimpleCollectionOp(env, predictedTypeFromLocation);
    if (auto tc = simpleOpConstraint(simpleOp)) return *tc;
  }

  return DataTypeSpecific;
}

//////////////////////////////////////////////////////////////////////

namespace {

/*
 * Determine which simple collection op to use for the given base and key
 * types.
 */
SimpleOp simpleCollectionOp(Type baseType, Type keyType, bool readInst) {
  if (baseType <= TArr) {
    auto isPacked = false;
    auto isStruct = false;
    if (auto arrSpec = baseType.arrSpec()) {
      isPacked = arrSpec.kind() == ArrayData::kPackedKind;
      isStruct = arrSpec.kind() == ArrayData::kStructKind &&
                 arrSpec.shape() != nullptr;
    }
    if (keyType <= TInt || keyType <= TStr) {
      if (readInst) {
        if (keyType <= TInt) {
          return isPacked ? SimpleOp::PackedArray
                          : SimpleOp::ProfiledPackedArray;
        } else if (keyType.hasConstVal(TStaticStr)) {
          if (!isStruct || !baseType.arrSpec().shape()) {
            return SimpleOp::ProfiledStructArray;
          }
          return SimpleOp::StructArray;
        }
      }
      return SimpleOp::Array;
    }
  } else if (baseType <= TStr) {
    if (keyType <= TInt) {
      // Don't bother with SetM on strings, because profile data shows it
      // basically never happens.
      if (readInst) return SimpleOp::String;
    }
  } else if (baseType < TObj && baseType.clsSpec()) {
    const Class* klass = baseType.clsSpec().cls();
    auto const isVector = collections::isType(klass, CollectionType::Vector);
    auto const isImmVector =
      collections::isType(klass, CollectionType::ImmVector);
    auto const isPair   = collections::isType(klass, CollectionType::Pair);
    auto const isMap    = collections::isType(klass, CollectionType::Map);
    auto const isImmMap = collections::isType(klass, CollectionType::ImmMap);

    if (isVector || isPair || (isImmVector && readInst)) {
      if (keyType <= TInt) {
        // We don't specialize setting pair elements.
        if (isPair && !readInst) return SimpleOp::None;

        return (isImmVector || isVector) ? SimpleOp::Vector : SimpleOp::Pair;
      }
    } else if ((isMap || (isImmMap && readInst)) &&
               (keyType <= TInt || keyType <= TStr)) {
      return SimpleOp::Map;
    }
  }

  return SimpleOp::None;
}

/*
 * Returns a pointer to a specific value in MInstrState.
 */
SSATmp* misLea(IRGS& env, int32_t offset) {
  env.irb->fs().setNeedRatchet(true);
  return gen(env, LdMIStateAddr, cns(env, offset));
}

SSATmp* tvRefPtr(IRGS& env) {
  return misLea(env, offsetof(MInstrState, tvRef));
}

SSATmp* tvRef2Ptr(IRGS& env) {
  return misLea(env, offsetof(MInstrState, tvRef2));
}

/*
 * Store Uninit to tvRef and tvRef2.
 */
void initTvRefs(IRGS& env) {
  gen(env, StMem, tvRefPtr(env), cns(env, TUninit));
  gen(env, StMem, tvRef2Ptr(env), cns(env, TUninit));
}

/*
 * DecRef tvRef and tvRef2.
 */
void cleanTvRefs(IRGS& env) {
  for (auto ptr : {tvRefPtr(env), tvRef2Ptr(env)}) {
    gen(env, DecRef, gen(env, LdMem, TGen, ptr));
  }
}

/*
 * If tvRef contains a value, DecRef tvRef2 and move tvRef's value to tvRef2,
 * storing Uninit to tvRef. Returns the adjusted base, which may point to
 * tvRef2.
 */
SSATmp* ratchetRefs(IRGS& env, SSATmp* base) {
  if (!env.irb->fs().needRatchet()) {
    return base;
  }

  auto tvRef = tvRefPtr(env);

  return cond(
    env,
    [&](Block* taken) {
      gen(env, CheckTypeMem, taken, TUninit, tvRef);
    },
    [&] { // Next: tvRef is Uninit. Do nothing.
      return base;
    },
    [&] { // Taken: tvRef isn't Uninit. Ratchet the refs
      auto tvRef2 = tvRef2Ptr(env);
      // Clean up tvRef2 before overwriting it.
      auto const oldRef2 = gen(env, LdMem, TGen, tvRef2);
      gen(env, DecRef, oldRef2);

      // Copy tvRef to tvRef2.
      auto const tvRefVal = gen(env, LdMem, TGen, tvRef);
      gen(env, StMem, tvRef2, tvRefVal);
      // Reset tvRef.
      gen(env, StMem, tvRef, cns(env, TUninit));

      // Adjust base pointer.  Don't use 'tvRef2' here so that we don't reuse
      // the temp.  This will let us elide uses of the register for 'tvRef2',
      // until the Jmp we're going to emit here.
      return tvRef2Ptr(env);
    }
  );
}

void baseGImpl(IRGS& env, SSATmp* name, MOpFlags flags) {
  if (!name->isA(TStr)) PUNT(BaseG-non-string-name);
  auto gblPtr = gen(env, BaseG, MInstrAttrData{mOpFlagsToAttr(flags)}, name);
  gen(env, StMBase, gblPtr);
}

void baseSImpl(IRGS& env, SSATmp* name, int32_t clsIdx) {
  auto cls = topA(env, BCSPOffset{clsIdx});
  auto spropPtr = ldClsPropAddr(env, cls, name, true);
  gen(env, StMBase, spropPtr);

  if (clsIdx == 1) {
    auto rhs = popC(env, DataTypeGeneric);
    popA(env);
    push(env, rhs);
  } else {
    popA(env);
  }
}

void simpleBaseImpl(IRGS& env, SSATmp* base, Type innerTy) {
  checkGenBase(base->type());

  if (base->isA(TBoxedCell)) {
    env.irb->constrainValue(base, DataTypeSpecific);
    gen(env, CheckRefInner, innerTy, makeExit(env), base);
    base = gen(env, LdRef, innerTy, base);
  }

  env.irb->fs().setMemberBaseValue(base);

  // TODO(t2598894): We do this for consistency with the old guard relaxation
  // code, but may change it in the future.
  env.irb->constrainValue(base, DataTypeSpecific);
}

SSATmp* propGenericImpl(IRGS& env, MOpFlags flags, SSATmp* base, SSATmp* key,
                        bool nullsafe) {
  auto const miaData = MInstrAttrData{mOpFlagsToAttr(flags)};
  if (flags & MOpFlags::Define) {
    if (nullsafe) {
      gen(env, RaiseError,
          cns(env, makeStaticString(Strings::NULLSAFE_PROP_WRITE_ERROR)));
      return ptrToInitNull(env);
    }

    return gen(env, PropDX, miaData, base, key, tvRefPtr(env));
  }

  if (nullsafe) {
    return gen(env, PropQ, base, key, tvRefPtr(env));
  }
  return gen(env, PropX, miaData, base, key, tvRefPtr(env));
}

SSATmp* propImpl(IRGS& env, MOpFlags flags, SSATmp* key, bool nullsafe) {
  auto base = env.irb->fs().memberBaseValue();
  auto const basePtr = gen(env, LdMBase, TPtrToGen);
  auto const baseType = base ? base->type() : basePtr->type().deref();

  if ((flags & MOpFlags::Unset) && !baseType.maybe(TObj)) {
    env.irb->constrainValue(base, DataTypeSpecific);
    return ptrToInitNull(env);
  }

  if (base && base->isA(TObj)) {
    env.irb->constrainValue(base, DataTypeSpecific);
  } else {
    base = basePtr;
  }

  auto const mia = mOpFlagsToAttr(flags);
  auto const propInfo =
    getCurrentPropertyOffset(env, base, base->type(), key->type());
  if (propInfo.offset == -1 || (flags & MOpFlags::Unset) ||
      mightCallMagicPropMethod(mia, propInfo)) {
    return propGenericImpl(env, flags, base, key, nullsafe);
  }

  return emitPropSpecialized(env, base, base->type(), key,
                             nullsafe, mia, propInfo);
}

SSATmp* elemImpl(IRGS& env, MOpFlags flags, SSATmp* key) {
  auto const warn = flags & MOpFlags::Warn;
  auto const unset = flags & MOpFlags::Unset;
  auto const define = flags & MOpFlags::Define;
  auto const base = env.irb->fs().memberBaseValue();
  auto const basePtr = gen(env, LdMBase, TPtrToGen);
  auto const baseType = base ? base->type() : basePtr->type().deref();

  assertx(!define || !unset);
  assertx(!define || !warn);

  if (base && base->isA(TArr) && key->type().subtypeOfAny(TInt, TStr)) {
    env.irb->constrainValue(base, DataTypeSpecific);
    if (define || unset) {
      return gen(env, unset ? ElemArrayU : ElemArrayD, basePtr, key);
    }
    return gen(env, warn ? ElemArrayW : ElemArray, base, key);
  }

  if (unset) {
    env.irb->constrainValue(base, DataTypeSpecific);
    if (baseType <= TStr) {
      gen(env, RaiseError,
          cns(env, makeStaticString(Strings::OP_NOT_SUPPORTED_STRING)));
      return ptrToUninit(env);
    }

    if (!baseType.maybe(TArr | TObj)) {
      return ptrToUninit(env);
    }
  }

  auto const miaData = MInstrAttrData{mOpFlagsToAttr(flags)};
  auto const op = define ? ElemDX : unset ? ElemUX : ElemX;
  return gen(env, op, miaData, basePtr, key, tvRefPtr(env));
}

void dimImpl(IRGS& env, PropElemOp propElem, MOpFlags flags, SSATmp* key) {
  // Eagerly mark us as not needing ratchets.  If the intermediate operation
  // ends up calling misLea(), this will be set to true.
  env.irb->fs().setNeedRatchet(false);

  auto newBase = [&] {
    switch (propElem) {
      case PropElemOp::Prop:
        return propImpl(env, flags, key, false);
      case PropElemOp::PropQ:
        return propImpl(env, flags, key, true);
      case PropElemOp::Elem:
        return elemImpl(env, flags, key);
    }
    always_assert(false);
  }();

  newBase = ratchetRefs(env, newBase);
  gen(env, StMBase, newBase);
}

/*
 * Pop nDiscard elements from the stack, push the result (if present), DecRef
 * tvRef(2), and mark the member operation as complete.
 */
void mFinalImpl(IRGS& env, int32_t nDiscard, SSATmp* result) {
  for (auto i = 0; i < nDiscard; ++i) popDecRef(env);
  cleanTvRefs(env);
  if (result) push(env, result);

  gen(env, FinishMemberOp);
}

SSATmp* cGetPropImpl(IRGS& env, SSATmp* base, SSATmp* key,
                     MOpFlags flags, bool nullsafe) {
  auto const propInfo =
    getCurrentPropertyOffset(env, base, base->type(), key->type());
  auto const mia = mOpFlagsToAttr(flags);

  if (propInfo.offset != -1 &&
      !mightCallMagicPropMethod(MIA_none, propInfo)) {
    auto propAddr = emitPropSpecialized(
      env, base, base->type(), key,
      nullsafe, mia, propInfo
    );

    if (!RuntimeOption::RepoAuthoritative) {
      auto const cellPtr = gen(env, UnboxPtr, base);
      auto const result = gen(env, LdMem, TCell, cellPtr);
      gen(env, IncRef, result);
      return result;
    }

    auto const ty = propAddr->type().deref();
    auto const cellPtr = ty.maybe(TBoxedCell)
      ? gen(env, UnboxPtr, propAddr)
      : propAddr;

    auto const result = gen(env, LdMem, ty.unbox(), cellPtr);
    gen(env, IncRef, result);
    return result;
  }

  // No warning takes precedence over nullsafe
  if (!nullsafe || !(mia & MIA_warn)) {
    return gen(env, CGetProp, MInstrAttrData{mia}, base, key);
  }
  return gen(env, CGetPropQ, base, key);
}

void queryMImpl(IRGS& env, int32_t nDiscard, QueryMOp query,
                PropElemOp propElem, SSATmp* key) {
  auto basePtr = gen(env, LdMBase, TPtrToGen);
  auto base = env.irb->fs().memberBaseValue();
  auto objBase = base && base->isA(TObj) ? base : basePtr;
  auto simpleOp = SimpleOp::None;

  if (base && propElem == PropElemOp::Elem &&
      query != QueryMOp::Empty && query != QueryMOp::CGetQuiet) {
    simpleOp = simpleCollectionOp(base->type(), key->type(), true);

    if (auto tc = simpleOpConstraint(simpleOp)) {
      env.irb->constrainValue(base, *tc);
    }
  }

  auto result = [&] {
    switch (query) {
      case QueryMOp::CGet:
      case QueryMOp::CGetQuiet: {
        auto const flags = getQueryMOpFlags(query);
        switch (propElem) {
          case PropElemOp::Prop:
          case PropElemOp::PropQ:
            return cGetPropImpl(env, objBase, key, flags,
                                propElem == PropElemOp::PropQ);
          case PropElemOp::Elem:
            auto const realBase = simpleOp == SimpleOp::None ? basePtr : base;
            return emitCGetElem(env, realBase, key, flags, simpleOp);
        }
        always_assert(false);
      }

      case QueryMOp::Isset:
        switch (propElem) {
          case PropElemOp::Prop:
          case PropElemOp::PropQ:
            return gen(env, IssetProp, objBase, key);
          case PropElemOp::Elem:
            auto const realBase = simpleOp == SimpleOp::None ? basePtr : base;
            return emitIssetElem(env, realBase, key, simpleOp);
        }

      case QueryMOp::Empty:
        switch (propElem) {
          case PropElemOp::Prop:
          case PropElemOp::PropQ:
            return gen(env, EmptyProp, objBase, key);
          case PropElemOp::Elem:
            return gen(env, EmptyElem, basePtr, key);
        }
    }

    always_assert(false);
  }();

  mFinalImpl(env, nDiscard, result);
}

Block* makeCatchSet(IRGS& env, bool isSetWithRef = false) {
  auto block = env.unit.defBlock(Block::Hint::Unused);

  BlockPusher bp(*env.irb, makeMarker(env, bcOff(env)), block);
  gen(env, BeginCatch);

  ifThen(
    env,
    [&] (Block* taken) {
      gen(env, UnwindCheckSideExit, taken, fp(env), sp(env));
    },
    [&] {
      hint(env, Block::Hint::Unused);
      gen(env, EndCatch, IRSPOffsetData { offsetFromIRSP(env, BCSPOffset{0}) },
        fp(env), sp(env));
    }
  );

  // Fallthrough from here on is side-exiting due to an InvalidSetMException.
  hint(env, Block::Hint::Unused);

  // For consistency with the interpreter, decref the rhs before we decref the
  // stack inputs, and decref the ratchet storage after the stack inputs.
  if (!isSetWithRef) {
    popDecRef(env, DataTypeGeneric);
  }
  auto const nDiscard = env.currentNormalizedInstruction->imm[0].u_IVA;
  for (int i = 0; i < nDiscard; ++i) {
    popDecRef(env, DataTypeGeneric);
  }
  cleanTvRefs(env);
  if (!isSetWithRef) {
    auto const val = gen(env, LdUnwinderValue, TCell);
    push(env, val);
  }

  // The minstr is done here, so we want to drop a FinishMemberOp to kill off
  // stores to MIState.
  gen(env, FinishMemberOp);

  gen(env, Jmp, makeExit(env, nextBcOff(env)));
  return block;
}

SSATmp* setPropImpl(IRGS& env, SSATmp* key) {
  auto const value = topC(env, BCSPOffset{0}, DataTypeGeneric);
  auto base = env.irb->fs().memberBaseValue();
  auto const basePtr = gen(env, LdMBase, TPtrToGen);

  if (base && base->isA(TObj)) {
    env.irb->constrainValue(base, DataTypeSpecific);
  } else {
    base = basePtr;
  }

  auto const mia = MIA_define;
  auto const propInfo =
    getCurrentPropertyOffset(env, base, base->type(), key->type());

  if (propInfo.offset != -1 &&
      !mightCallMagicPropMethod(MIA_define, propInfo)) {
    auto propPtr =
      emitPropSpecialized(env, base, base->type(), key, false, mia, propInfo);
    auto propTy = propPtr->type().deref();

    if (propTy.maybe(TBoxedCell)) {
      propTy = propTy.unbox();
      propPtr = gen(env, UnboxPtr, propPtr);
    }

    env.irb->constrainValue(value, DataTypeCountness);
    auto const oldVal = gen(env, LdMem, propTy, propPtr);
    gen(env, IncRef, value);
    gen(env, StMem, propPtr, value);
    gen(env, DecRef, oldVal);
  } else {
    gen(env, SetProp, makeCatchSet(env), base, key, value);
  }

  return value;
}

void handleStrTestResult(IRGS& env, SSATmp* strTestResult) {
  // We expected SetElem's base to not be a Str but might be wrong. Make an
  // exit trace to side exit to the next instruction, replacing our guess with
  // the correct stack output.
  ifThen(
    env,
    [&] (Block* taken) {
      gen(env, CheckNullptr, taken, strTestResult);
    },
    [&] {
      hint(env, Block::Hint::Unlikely);
      auto const str = gen(env, AssertNonNull, strTestResult);
      popDecRef(env, DataTypeSpecific);
      auto const nDiscard = env.currentNormalizedInstruction->imm[0].u_IVA;
      for (int i = 0; i < nDiscard; ++i) {
        popDecRef(env, DataTypeSpecific);
      }
      cleanTvRefs(env);
      push(env, str);
      gen(env, Jmp, makeExit(env, nextBcOff(env)));
    }
  );
}

SSATmp* emitArraySet(IRGS& env, SSATmp* key, SSATmp* value) {
  // We need to store to a local after doing some user-visible operations, so
  // don't go down this path for pseudomains.
  if (curFunc(env)->isPseudoMain()) return nullptr;

  auto const base = env.irb->fs().memberBaseValue();
  auto const basePtr = gen(env, LdMBase, TPtrToGen);
  auto const ptrInst = basePtr->inst();
  Location baseLoc;
  if (ptrInst->is(LdLocAddr)) {
    auto const id = ptrInst->extra<LocalId>()->locId;
    baseLoc = Location{Location::Local, id};
  } else if (ptrInst->is(LdStkAddr)) {
    auto const irOff = ptrInst->extra<IRSPOffsetData>()->offset;
    baseLoc = Location{offsetFromBCSP(env, irOff)};
  } else {
    return nullptr;
  }

  // base may be from inside a RefData inside a stack/local, so to determine
  // setRef we must check the actual value of the stack/local.
  auto const rawBaseType = provenTypeFromLocation(env, baseLoc);
  auto const setRef = rawBaseType <= TBoxedCell;

  if (setRef) {
    auto const box = baseLoc.space == Location::Local ?
      ldLoc(env, baseLoc.offset, nullptr, DataTypeSpecific) :
      top(env, baseLoc.bcRelOffset, DataTypeSpecific);
    gen(env, ArraySetRef, base, key, value, box);
    // Unlike the non-ref case, we don't need to do anything to the stack/local
    // because any load of the box will be guarded.
    return value;
  }

  auto const newArr = gen(env, ArraySet, base, key, value);

  // Update the base's location with the new array
  if (baseLoc.space == Location::Local) {
    // We know it's not boxed (setRef above handles that), and the helper has
    // already decref'd the old array and incref'd newArr.
    gen(env, StLoc, LocalId(baseLoc.offset), fp(env), newArr);
  } else if (baseLoc.space == Location::Stack) {
    auto const offset = offsetFromIRSP(env, baseLoc.bcRelOffset);
    gen(env, StStk, IRSPOffsetData{offset}, sp(env), newArr);
  } else {
    always_assert(false);
  }

  return value;
}

SSATmp* setElemImpl(IRGS& env, SSATmp* key) {
  auto value = topC(env, BCSPOffset{0}, DataTypeGeneric);
  auto const base = env.irb->fs().memberBaseValue();
  auto const simpleOp =
    base ? simpleCollectionOp(base->type(), key->type(), false)
         : SimpleOp::None;

  if (auto tc = simpleOpConstraint(simpleOp)) {
    env.irb->constrainValue(base, *tc);
  }

  switch (simpleOp) {
    case SimpleOp::PackedArray:
    case SimpleOp::StructArray:
    case SimpleOp::String:
      always_assert(false && "Bad SimpleOp in setElemImpl");
      break;

    case SimpleOp::Vector:
      emitVectorSet(env, base, key, value);
      break;

    case SimpleOp::Map:
      gen(env, MapSet, base, key, value);
      break;

    case SimpleOp::Array:
    case SimpleOp::ProfiledPackedArray:
    case SimpleOp::ProfiledStructArray:
      if (auto result = emitArraySet(env, key, value)) {
        return result;
      }
      // If we couldn't emit ArraySet, fall through to the generic path.

    case SimpleOp::Pair:
    case SimpleOp::None:
      auto const result =
        gen(env, SetElem, makeCatchSet(env), gen(env, LdMBase, TPtrToGen),
            key, value);
      auto const t = result->type();
      if (t == TNullptr) {
        // Base is not a string. Result is always value.
      } else if (t == TCountedStr) {
        // Base is a string. Stack result is a new string so we're responsible
        // for decreffing value.
        env.irb->constrainValue(value, DataTypeCountness);
        gen(env, DecRef, value);
        value = result;
      } else {
        assertx(t == (TCountedStr | TNullptr));
        // Base might be a string. Emit a check to verify the result before
        // returning the optimistic result.
        handleStrTestResult(env, result);
      }
      break;
  }

  return value;
}

void setMImpl(IRGS& env, int32_t nDiscard, PropElemOp propElem, SSATmp* key) {
  auto const result = [&] {
    if (propElem == PropElemOp::Prop) return setPropImpl(env, key);
    if (propElem == PropElemOp::Elem) return setElemImpl(env, key);
    always_assert(false);
  }();

  popC(env, DataTypeGeneric);
  mFinalImpl(env, nDiscard, result);
}

}

void emitBaseNC(IRGS& env, int32_t idx, MOpFlags flags) {
  interpOne(env, *env.currentNormalizedInstruction);
}

void emitBaseNL(IRGS& env, int32_t locId, MOpFlags flags) {
  interpOne(env, *env.currentNormalizedInstruction);
}

void emitBaseGC(IRGS& env, int32_t idx, MOpFlags flags) {
  initTvRefs(env);
  auto name = top(env, BCSPOffset{idx});
  baseGImpl(env, name, flags);
}

void emitBaseGL(IRGS& env, int32_t locId, MOpFlags flags) {
  initTvRefs(env);
  auto name = ldLocInner(env, locId, makeExit(env), makePseudoMainExit(env),
                         DataTypeSpecific);
  baseGImpl(env, name, flags);
}

void emitBaseSC(IRGS& env, int32_t propIdx, int32_t clsIdx) {
  initTvRefs(env);
  auto name = top(env, BCSPOffset{propIdx});
  baseSImpl(env, name, clsIdx);
}

void emitBaseSL(IRGS& env, int32_t locId, int32_t clsIdx) {
  initTvRefs(env);
  auto name = ldLocInner(env, locId, makeExit(env), makePseudoMainExit(env),
                         DataTypeSpecific);
  baseSImpl(env, name, clsIdx);
}

void emitBaseL(IRGS& env, int32_t locId, MOpFlags flags) {
  initTvRefs(env);
  gen(env, StMBase, ldLocAddr(env, locId));

  auto base = ldLoc(env, locId, makePseudoMainExit(env), DataTypeGeneric);

  if (base->isA(TUninit) && (flags & MOpFlags::Warn)) {
    env.irb->constrainLocal(locId, DataTypeSpecific,
                            "emitBaseL: Uninit base local");
    gen(env, RaiseUninitLoc, cns(env, curFunc(env)->localVarName(locId)));
  }

  auto innerTy = base->isA(TBoxedCell) ? env.irb->predictedInnerType(locId)
                                       : TTop;
  simpleBaseImpl(env, base, innerTy);
}

void emitBaseC(IRGS& env, int32_t idx) {
  initTvRefs(env);

  auto const bcOff = BCSPOffset{idx};
  auto const irOff = offsetFromIRSP(env, bcOff);
  gen(env, StMBase, ldStkAddr(env, bcOff));

  auto base = top(env, bcOff);
  auto innerTy = base->isA(TBoxedCell) ? env.irb->predictedStackInnerType(irOff)
                                       : TTop;
  simpleBaseImpl(env, top(env, bcOff), innerTy);
}

void emitBaseR(IRGS& env, int32_t idx) {
  emitBaseC(env, idx);
}

void emitBaseH(IRGS& env) {
  if (!curClass(env)) return interpOne(env, *env.currentNormalizedInstruction);

  initTvRefs(env);
  auto base = ldThis(env);
  auto scratchPtr = misLea(env, offsetof(MInstrState, tvTempBase));
  gen(env, StMem, scratchPtr, base);
  gen(env, StMBase, scratchPtr);
  env.irb->fs().setMemberBaseValue(base);
}

void emitDimL(IRGS& env, int32_t locId, PropElemOp propElem, MOpFlags flags) {
  auto key = ldLocInner(env, locId, makeExit(env), makePseudoMainExit(env),
                        DataTypeSpecific);
  dimImpl(env, propElem, flags, key);
}

void emitDimC(IRGS& env, int32_t idx, PropElemOp propElem, MOpFlags flags) {
  dimImpl(env, propElem, flags, topC(env, BCSPOffset{idx}));
}

void emitDimInt(IRGS& env, int64_t key, PropElemOp propElem, MOpFlags flags) {
  dimImpl(env, propElem, flags, cns(env, key));
}

void emitDimStr(IRGS& env, const StringData* key,
                PropElemOp propElem, MOpFlags flags) {
  dimImpl(env, propElem, flags, cns(env, key));
}

void emitDimNewElem(IRGS& env, MOpFlags flags) {
  interpOne(env, *env.currentNormalizedInstruction);
}

void emitQueryML(IRGS& env, int32_t nDiscard, QueryMOp query,
                 PropElemOp propElem, int32_t locId) {
  auto key = ldLocInner(env, locId, makeExit(env), makePseudoMainExit(env),
                        DataTypeSpecific);
  queryMImpl(env, nDiscard, query, propElem, key);
}

void emitQueryMC(IRGS& env, int32_t nDiscard, QueryMOp query,
                 PropElemOp propElem) {
  queryMImpl(env, nDiscard, query, propElem, topC(env));
}

void emitQueryMInt(IRGS& env, int32_t nDiscard, QueryMOp query,
                   PropElemOp propElem, int64_t key) {
  queryMImpl(env, nDiscard, query, propElem, cns(env, key));
}

void emitQueryMStr(IRGS& env, int32_t nDiscard, QueryMOp query,
                   PropElemOp propElem, const StringData* key) {
  queryMImpl(env, nDiscard, query, propElem, cns(env, key));
}

void emitSetML(IRGS& env, int32_t nDiscard, PropElemOp propElem,
               int32_t locId) {
  auto key = ldLocInner(env, locId, makeExit(env), makePseudoMainExit(env),
                        DataTypeSpecific);
  setMImpl(env, nDiscard, propElem, key);
}

void emitSetMC(IRGS& env, int32_t nDiscard, PropElemOp propElem) {
  setMImpl(env, nDiscard, propElem, topC(env, BCSPOffset{1}));
}

void emitSetMInt(IRGS& env, int32_t nDiscard, PropElemOp propElem,
                 int64_t key) {
  setMImpl(env, nDiscard, propElem, cns(env, key));
}

void emitSetMStr(IRGS& env, int32_t nDiscard, PropElemOp propElem,
                 const StringData* key) {
  setMImpl(env, nDiscard, propElem, cns(env, key));
}

void emitSetMNewElem(IRGS& env, int32_t nDiscard) {
  auto const value = topC(env);
  auto const basePtr = gen(env, LdMBase, TPtrToGen);
  auto const base = env.irb->fs().memberBaseValue();

  if (base && base->isA(TArr)) {
    env.irb->constrainValue(base, DataTypeSpecific);
    gen(env, SetNewElemArray, makeCatchSet(env), basePtr, value);
  } else {
    gen(env, SetNewElem, makeCatchSet(env), basePtr, value);
  }

  popC(env, DataTypeGeneric);
  mFinalImpl(env, nDiscard, value);
}

//////////////////////////////////////////////////////////////////////

void emitBindM(IRGS& env, int)                 { implMInstr(env, Op::BindM); }
void emitCGetM(IRGS& env, int)                 { implMInstr(env, Op::CGetM); }
void emitEmptyM(IRGS& env, int)                { implMInstr(env, Op::EmptyM); }
void emitIncDecM(IRGS& env, IncDecOp, int)     { implMInstr(env, Op::IncDecM); }
void emitIssetM(IRGS& env, int)                { implMInstr(env, Op::IssetM); }
void emitSetM(IRGS& env, int)                  { implMInstr(env, Op::SetM); }
void emitSetOpM(IRGS& env, SetOpOp, int)       { implMInstr(env, Op::SetOpM); }
void emitUnsetM(IRGS& env, int)                { implMInstr(env, Op::UnsetM); }
void emitVGetM(IRGS& env, int)                 { implMInstr(env, Op::VGetM); }

void emitSetWithRefLM(IRGS& env, int, int32_t) {
  implMInstr(env, Op::SetWithRefLM);
}

void emitSetWithRefRM(IRGS& env, int) {
  implMInstr(env, Op::SetWithRefRM);
}

//////////////////////////////////////////////////////////////////////

}}}
