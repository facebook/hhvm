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

#include <type_traits>
#include <sstream>

#include "hphp/runtime/base/strings.h"

#include "hphp/runtime/ext/ext_collections.h"
#include "hphp/runtime/vm/jit/minstr-effects.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/target-profile.h"

#include "hphp/runtime/vm/jit/irgen-sprop-global.h"
#include "hphp/runtime/vm/jit/irgen-exit.h"

#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { namespace jit { namespace irgen {

namespace {

//////////////////////////////////////////////////////////////////////

const StaticString s_PackedArray("PackedArray");

//////////////////////////////////////////////////////////////////////

bool wantPropSpecializedWarnings() {
  return !RuntimeOption::RepoAuthoritative ||
    !RuntimeOption::EvalDisableSomeRepoAuthNotices;
}

//////////////////////////////////////////////////////////////////////

enum class SimpleOp {
  None,
  Array,
  ProfiledArray,
  PackedArray,
  String,
  Vector, // c_Vector* or c_ImmVector*
  Map,    // c_Map*
  Pair,   // c_Pair*
};

/*
 * Minstr Translation State.  Member instructions are complex enough that we
 * need our own state environment while processing one.
 *
 * This is implicitly convertible to HTS so you can use ht-internal functions
 * on it.  Effectively MTS <: HTS (except the dot operator).
 */
struct MTS {
  explicit MTS(HTS& hts, Op effectiveOp)
    : hts(hts)
    , op(effectiveOp)
    , immVec(hts.currentNormalizedInstruction->immVec)
    , immVecM(hts.currentNormalizedInstruction->immVecM)
    , ni(*hts.currentNormalizedInstruction)
    , irb(*hts.irb)
    , unit(hts.unit)
    , mii(getMInstrInfo(effectiveOp))
    , iInd(mii.valCount())
  {}
  /* implicit */ operator HTS&() { return hts; }
  /* implicit */ operator const HTS&() const { return hts; }

  HTS& hts;
  Op op;
  ImmVector immVec;
  jit::vector<MemberCode> immVecM;
  const NormalizedInstruction& ni;
  IRBuilder& irb;
  IRUnit& unit;
  MInstrInfo mii;

  hphp_hash_map<unsigned,unsigned> stackInputs;

  unsigned mInd;
  unsigned iInd;

  bool needMIS{true};

  // The base for any accesses to the current MInstrState.
  SSATmp* misBase{nullptr};

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
    Type type{Type::Bottom};
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

void constrainBase(MTS& env, TypeConstraint tc) {
  // Member operations only care about the inner type of the base if it's
  // boxed, so this handles the logic of using the inner constraint when
  // appropriate.
  if (env.base.type.maybeBoxed()) {
    tc.category = DataTypeCountness;
  }
  env.irb.constrainValue(env.base.value, tc);
}

bool constrainCollectionOpBase(MTS& env) {
  switch (env.simpleOp) {
    case SimpleOp::None:
      return false;

    case SimpleOp::Array:
    case SimpleOp::ProfiledArray:
    case SimpleOp::String:
      env.irb.constrainValue(env.base.value, DataTypeSpecific);
      return true;

    case SimpleOp::PackedArray:
      constrainBase(
        env,
        TypeConstraint(DataTypeSpecialized).setWantArrayKind()
      );
      return true;

    case SimpleOp::Vector:
    case SimpleOp::Map:
    case SimpleOp::Pair:
      always_assert(env.base.type < Type::Obj &&
                    env.base.type.isSpecialized());
      constrainBase(env, TypeConstraint(env.base.type.getClass()));
      return true;
  }

  not_reached();
  return false;
}

void specializeBaseIfPossible(MTS& env, Type baseType) {
  if (constrainCollectionOpBase(env)) return;
  if (baseType >= Type::Obj) return;
  if (!baseType.isSpecialized()) return;
  constrainBase(env, TypeConstraint(baseType.getClass()));
}

//////////////////////////////////////////////////////////////////////

// Return the first SSATmp* from a parameter pack.
UNUSED SSATmp* find_base() { always_assert(0 && "genStk with no base"); }
template<class... T> SSATmp* find_base(SSATmp* h, T... t) { return h; }
template<class H,
         class... T> SSATmp* find_base(H h, T... t) { return find_base(t...); }

template<class... Args>
SSATmp* genStk(MTS& env, Opcode opc, Args... args) {
  assert(minstrBaseIdx(opc) == 0);

  auto const base = find_base(args...);

  /* If the base is a pointer to a stack cell and the operation might change
   * its type and/or value, use the version of the opcode that returns a new
   * StkPtr. */
  if (base->inst()->is(LdStackAddr)) {
    auto const prev = getStackValue(
      base->inst()->src(0),
      base->inst()->template extra<LdStackAddr>()->offset
    );
    MInstrEffects effects(opc, prev.knownType.ptr(Ptr::Stk));
    if (effects.baseTypeChanged || effects.baseValChanged) {
      return gen(env, getStackModifyingOpcode(opc), args..., sp(env));
    }
  }
  return gen(env, opc, args...);
}

//////////////////////////////////////////////////////////////////////

// Returns a pointer to the base of the current MInstrState struct, or a null
// pointer if it's not needed.
SSATmp* misPtr(MTS& env) {
  assert(env.base.value && "misPtr called before emitBaseOp");
  if (env.needMIS) {
    return gen(env, LdMIStateAddr, env.misBase,
      cns(env, RDS::kVmMInstrStateOff));
  }
  return cns(env, Type::cns(nullptr, Type::PtrToMISUninit));
}

bool mightCallMagicPropMethod(MInstrAttr mia, const Class* cls,
                              PropInfo propInfo) {
  if (convertToType(propInfo.repoAuthType).not(Type::Uninit)) {
    return false;
  }
  if (!cls) return true;
  bool const no_override_magic =
    // NB: this function can't yet be used for unset or isset contexts.  Just
    // get and set.
    (mia & MIA_define) ? cls->attrs() & AttrNoOverrideMagicSet
                       : cls->attrs() & AttrNoOverrideMagicGet;
  return !no_override_magic;
}

bool mInstrHasUnknownOffsets(MTS& env) {
  auto const& mii = env.mii;
  unsigned mi = 0;
  unsigned ii = mii.valCount() + 1;
  for (; mi < env.immVecM.size(); ++mi) {
    auto const mc = env.immVecM[mi];
    if (mcodeIsProp(mc)) {
      const Class* cls = nullptr;
      auto propInfo = getPropertyOffset(env.ni, curClass(env), cls, mii, mi,
        ii);
      if (propInfo.offset == -1 ||
          mightCallMagicPropMethod(mii.getAttr(mc), cls, propInfo)) {
        return true;
      }
      ++ii;
    } else {
      return true;
    }
  }

  return false;
}

// "Simple" bases are stack cells and locals.
bool isSimpleBase(MTS& env) {
  auto const loc = env.immVec.locationCode();
  return loc == LL || loc == LC || loc == LR;
}

bool isSingleMember(MTS& env) { return env.immVecM.size() == 1; }

bool isOptimizableCollectionClass(const Class* klass) {
  return klass == c_Vector::classof() ||
         klass == c_Map::classof() ||
         klass == c_Pair::classof();
}

// Inspect the instruction we're about to translate and determine if it can be
// executed without using an MInstrState struct.
void checkMIState(MTS& env) {
  if (env.immVec.locationCode() == LNL ||
      env.immVec.locationCode() == LNC) {
    // We're definitely going to punt in emitBaseN, so we might not
    // have guarded the base's type.
    return;
  }

  Type baseType             = env.base.type.derefIfPtr();
  const bool baseArr        = baseType <= Type::Arr;
  const bool isCGetM        = env.op == Op::CGetM;
  const bool isSetM         = env.op == Op::SetM;
  const bool isIssetM       = env.op == Op::IssetM;
  const bool isUnsetM       = env.op == Op::UnsetM;
  const bool isSingle       = env.immVecM.size() == 1;
  const bool unknownOffsets = mInstrHasUnknownOffsets(env);

  if (baseType.maybeBoxed() && !baseType.isBoxed()) {
    // We don't need to bother with weird base types.
    return;
  }

  // Type::unbox() is a little dangerous since it can be more specific than
  // what LdRef actually returns, but in all cases where the base value comes
  // from a LdRef, m_base will be the dest of that LdRef and unbox() will be a
  // no-op here.
  baseType = baseType.unbox();

  // CGetM or SetM with no unknown property offsets
  const bool simpleProp = !unknownOffsets && (isCGetM || isSetM);

  // SetM with only one vector element, for props and elems
  const bool singleSet = isSingle && isSetM;

  // Element access with one element in the vector
  const bool singleElem = isSingle && mcodeIsElem(env.immVecM[0]);

  // IssetM with one vector array element and an Arr base
  const bool simpleArrayIsset = isIssetM && singleElem && baseArr;

  // IssetM with one vector array element and a collection type
  const bool simpleCollectionIsset = isIssetM && singleElem &&
    baseType < Type::Obj && isOptimizableCollectionClass(baseType.getClass());

  // UnsetM on an array with one vector element
  const bool simpleArrayUnset = isUnsetM && singleElem &&
    baseType <= Type::Arr;

  // CGetM on an array with a base that won't use MInstrState. Str
  // will use tvScratch and Obj will fatal or use tvRef.
  const bool simpleArrayGet = isCGetM && singleElem &&
    baseType.not(Type::Str | Type::Obj);
  const bool simpleCollectionGet = isCGetM && singleElem &&
    baseType < Type::Obj && isOptimizableCollectionClass(baseType.getClass());
  const bool simpleStringOp = (isCGetM || isIssetM) && isSingle &&
    isSimpleBase(env) && mcodeMaybeArrayIntKey(env.immVecM[0]) &&
    baseType <= Type::Str;

  if (simpleProp || singleSet ||
      simpleArrayGet || simpleCollectionGet ||
      simpleArrayUnset || simpleCollectionIsset ||
      simpleArrayIsset || simpleStringOp) {
    env.needMIS = false;
    if (simpleCollectionGet || simpleCollectionIsset) {
      constrainBase(env, TypeConstraint(baseType.getClass()));
    } else {
      constrainBase(env, DataTypeSpecific);
    }
  }
}

//////////////////////////////////////////////////////////////////////

void emitMTrace(MTS& env) {
  auto rttStr = [&](int i) {
    return Type(env.ni.inputs[i]->rtt).unbox().toString();
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

//////////////////////////////////////////////////////////////////////

SSATmp* getInput(MTS& env, unsigned i, TypeConstraint tc) {
  auto const& dl = *env.ni.inputs[i];
  auto const& l = dl.location;

  assert(!!env.stackInputs.count(i) == (l.space == Location::Stack));
  switch (l.space) {
    case Location::Stack:
      return top(env, Type::StackElem, env.stackInputs[i], tc);

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

void setBase(MTS& env,
             SSATmp* tmp,
             folly::Optional<Type> baseType = folly::none) {
  env.base.value = tmp;
  env.base.type = baseType ? *baseType : env.base.value->type();
  always_assert(env.base.type <= env.base.value->type());
}

SSATmp* getBase(MTS& env, TypeConstraint tc) {
  assert(env.iInd == env.mii.valCount());
  return getInput(env, env.iInd, tc);
}

SSATmp* getKey(MTS& env) {
  auto key = getInput(env, env.iInd, DataTypeSpecific);
  auto const keyType = key->type();
  assert(keyType.isBoxed() || keyType.notBoxed());
  if (keyType.isBoxed()) {
    key = gen(env, LdRef, Type::InitCell, key);
  }
  return key;
}

SSATmp* getValue(MTS& env) {
  // If an instruction takes an rhs, it's always input 0.
  assert(env.mii.valCount() == 1);
  const int kValIdx = 0;
  return getInput(env, kValIdx, DataTypeSpecific);
}

SSATmp* getValAddr(MTS& env) {
  assert(env.mii.valCount() == 1);
  auto const& dl = *env.ni.inputs[0];
  auto const& l = dl.location;
  if (l.space == Location::Local) {
    assert(!env.stackInputs.count(0));
    return ldLocAddr(env, l.offset);
  }

  assert(l.space == Location::Stack);
  assert(env.stackInputs.count(0));
  spillStack(env);
  env.irb.exceptionStackBoundary();
  return ldStackAddr(env, env.stackInputs[0]);
}

//////////////////////////////////////////////////////////////////////

// Compute whether the current instruction a 1-element simple collection
// (includes Array) operation.
SimpleOp computeSimpleCollectionOp(MTS& env) {
  // DataTypeGeneric is used in here to avoid constraining the base in case we
  // end up not caring about the type. Consumers of the return value must
  // constrain the base as appropriate.
  auto const base = getBase(env, DataTypeGeneric); // XXX: gens unneeded instrs
  if (!base->type().isBoxed() && base->type().maybeBoxed()) {
    // We might be doing a Base NL or something similar.  Either way we can't
    // do a simple op if we have a mixed boxed/unboxed type.
    return SimpleOp::None;
  }

  auto const baseType = [&] {
    auto const& baseDL = *env.ni.inputs[env.iInd];
    // Before we do any simpleCollectionOp on a local base, we will always emit
    // the appropriate CheckRefInner guard to allow us to use a predicted inner
    // type.  So when calculating the SimpleOp assume that type.
    if (base->type().maybeBoxed() && baseDL.location.isLocal()) {
      return env.irb.predictedInnerType(baseDL.location.offset);
    }
    return base->type();
  }();

  bool const readInst = (env.op == Op::CGetM || env.op == Op::IssetM);
  if ((env.op == OpSetM || readInst) && isSimpleBase(env) &&
      isSingleMember(env)) {
    if (baseType <= Type::Arr) {
      auto const isPacked =
        baseType.isSpecialized() &&
        baseType.hasArrayKind() &&
        baseType.getArrayKind() == ArrayData::kPackedKind;
      if (mcodeIsElem(env.immVecM[0])) {
        SSATmp* key = getInput(env, env.mii.valCount() + 1, DataTypeGeneric);
        if (key->isA(Type::Int) || key->isA(Type::Str)) {
          if (readInst && key->isA(Type::Int)) {
            return isPacked ? SimpleOp::PackedArray
                            : SimpleOp::ProfiledArray;
          }
          return SimpleOp::Array;
        }
      }
    } else if (baseType <= Type::Str &&
               mcodeMaybeArrayIntKey(env.immVecM[0])) {
      auto const key = getInput(env, env.mii.valCount() + 1, DataTypeGeneric);
      if (key->isA(Type::Int)) {
        // Don't bother with SetM on strings, because profile data
        // shows it basically never happens.
        if (readInst) {
          return SimpleOp::String;
        }
      }
    } else if (baseType.strictSubtypeOf(Type::Obj)) {
      const Class* klass = baseType.getClass();
      auto const isVector    = klass == c_Vector::classof();
      auto const isPair      = klass == c_Pair::classof();
      auto const isMap       = klass == c_Map::classof();

      if (isVector || isPair) {
        if (mcodeMaybeVectorKey(env.immVecM[0])) {
          auto const key = getInput(env, env.mii.valCount() + 1,
            DataTypeGeneric);
          if (key->isA(Type::Int)) {
            // We don't specialize setting pair elements.
            if (isPair && env.op == Op::SetM) return SimpleOp::None;

            return isVector ? SimpleOp::Vector : SimpleOp::Pair;
          }
        }
      } else if (isMap) {
        if (mcodeIsElem(env.immVecM[0])) {
          auto const key = getInput(env, env.mii.valCount() + 1,
            DataTypeGeneric);
          if (key->isA(Type::Int) || key->isA(Type::Str)) {
            return SimpleOp::Map;
          }
        }
      }
    }
  }

  return SimpleOp::None;
}

//////////////////////////////////////////////////////////////////////
// Base ops

void emitBaseLCR(MTS& env) {
  auto const& mia = env.mii.getAttr(env.immVec.locationCode());
  auto const& baseDL = *env.ni.inputs[env.iInd];
  // We use DataTypeGeneric here because we might not care about the type. If
  // we do, it's constrained further.
  auto base = getBase(env, DataTypeGeneric);
  auto baseType = base->type();
  assert(baseType.isBoxed() || baseType.notBoxed());

  if (baseDL.location.isLocal()) {
    // Check for Uninit and warn/promote to InitNull as appropriate
    if (baseType <= Type::Uninit) {
      if (mia & MIA_warn) {
        gen(env,
            RaiseUninitLoc,
            cns(env, curFunc(env)->localVarName(baseDL.location.offset)));
      }
      if (mia & MIA_define) {
        // We care whether or not the local is Uninit, and
        // CountnessInit will tell us that.
        env.irb.constrainLocal(baseDL.location.offset, DataTypeSpecific,
                              "emitBaseLCR: Uninit base local");
        base = cns(env, Type::InitNull);
        baseType = Type::InitNull;
        gen(
          env,
          StLoc,
          LocalId(baseDL.location.offset),
          fp(env),
          base
        );
      }
    }
  }

  /*
   * If the base is boxed, and from a local, we can do a better translation
   * using the inner type after guarding.  If we're going to do a generic
   * translation that uses a pointer to the local we still want this
   * LdRef---some of the translations will be smarter if they know the inner
   * type.  This is the first code emitted for the minstr so it's ok to
   * side-exit here.
   */
  Block* failedRef = baseType.isBoxed() ? makeExit(env) : nullptr;
  if (baseType.isBoxed() && baseDL.location.isLocal()) {
    auto const predTy = env.irb.predictedInnerType(baseDL.location.offset);
    gen(env, CheckRefInner, predTy, failedRef, base);
    base = gen(env, LdRef, predTy, base);
    baseType = base->type();
  }

  // Check for common cases where we can pass the base by value, we unboxed
  // above if it was needed.
  if ((baseType.subtypeOfAny(Type::Obj) && mcodeIsProp(env.immVecM[0])) ||
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
  if (baseDL.location.space == Location::Local) {
    setBase(
      env,
      ldLocAddr(env, baseDL.location.offset),
      env.irb.localType(
        baseDL.location.offset,
        DataTypeSpecific
      ).ptr(Ptr::Frame)
    );
  } else {
    assert(baseDL.location.space == Location::Stack);
    // Make sure the stack is clean before getting a pointer to one of its
    // elements.
    spillStack(env);
    env.irb.exceptionStackBoundary();
    assert(env.stackInputs.count(env.iInd));
    auto const sinfo = getStackValue(sp(env), env.stackInputs[env.iInd]);
    setBase(
      env,
      ldStackAddr(env, env.stackInputs[env.iInd]),
      sinfo.knownType.ptr(Ptr::Stk)
    );
  }
  assert(env.base.value->type().isPtr());
  assert(env.base.type.isPtr());

  // TODO(t2598894): We do this for consistency with the old guard relaxation
  // code, but may change it in the future.
  constrainBase(env, DataTypeSpecific);
}

void emitBaseH(MTS& env) { setBase(env, getBase(env, DataTypeSpecific)); }

void emitBaseN(MTS& env) {
  // If this is ever implemented, the check at the beginning of
  // checkMIState must be removed/adjusted as appropriate.
  PUNT(emitBaseN);
}

void emitBaseG(MTS& env) {
  auto const& mia = env.mii.getAttr(env.immVec.locationCode());
  auto const gblName = getBase(env, DataTypeSpecific);
  if (!gblName->isA(Type::Str)) PUNT(BaseG-non-string-name);
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
  default:                   not_reached();
  }
}

//////////////////////////////////////////////////////////////////////
// Intermediate ops

PropInfo getCurrentPropertyOffset(MTS& env, const Class*& knownCls) {
  auto const baseType = env.base.type.derefIfPtr();
  if (!knownCls) {
    if (baseType < (Type::Obj|Type::InitNull) && baseType.isSpecialized()) {
      knownCls = baseType.getClass();
    }
  }

  /*
   * TODO(#5616733): If we still don't have a knownCls, we can't do anything
   * good.  It's possible we still have the known information statically, and
   * it might be in env.ni.inputs, but right now we can't really trust that
   * because it's not very clear what it means.  See task for more information.
   */
  if (!knownCls) return PropInfo{};

  auto const info = getPropertyOffset(env.ni, curClass(env), knownCls,
                                      env.mii, env.mInd, env.iInd);
  if (info.offset == -1) return info;

  auto const baseCls = baseType.getClass();

  /*
   * baseCls and knownCls may differ due to a number of factors but they must
   * always be related to each other somehow if they are both non-null.
   *
   * TODO(#5616733): stop using ni.inputs here.  Just use the class we know
   * from this translation, if there is one.
   */
  always_assert_flog(
    baseCls->classof(knownCls) || knownCls->classof(baseCls),
    "Class mismatch between baseType({}) and knownCls({})",
    baseCls->name()->data(), knownCls->name()->data()
  );

  if (env.irb.constrainValue(env.base.value,
                             TypeConstraint(baseCls).setWeak())) {
    // We can't use this specialized class without making a guard more
    // expensive, so don't do it.
    knownCls = nullptr;
    return PropInfo{};
  }
  return info;
}

/*
 * Helper for emitPropSpecialized to check if a property is Uninit. It returns
 * a pointer to the property's address, or init_null_variant if the property
 * was Uninit and doDefine is false.
 *
 * We can omit the uninit check for properties that we know may not be uninit
 * due to the frontend's type inference.
 */
SSATmp* checkInitProp(MTS& env,
                      SSATmp* baseAsObj,
                      SSATmp* propAddr,
                      PropInfo propInfo,
                      bool doWarn,
                      bool doDefine) {
  auto const key = getKey(env);
  assert(key->isA(Type::StaticStr));
  assert(baseAsObj->isA(Type::Obj));
  assert(propAddr->type().isPtr());

  auto const needsCheck =
    Type::Uninit <= propAddr->type().deref() &&
    // The m_mInd check is to avoid initializing a property to
    // InitNull right before it's going to be set to something else.
    (doWarn || (doDefine && env.mInd < env.immVecM.size() - 1));

  if (!needsCheck) return propAddr;

  return env.irb.cond(
    0,
    [&] (Block* taken) {
      gen(env, CheckInitMem, taken, propAddr, cns(env, 0));
    },
    [&] { // Next: Property isn't Uninit. Do nothing.
      return propAddr;
    },
    [&] { // Taken: Property is Uninit. Raise a warning and return
          // a pointer to InitNull, either in the object or
          // init_null_variant.
      env.irb.hint(Block::Hint::Unlikely);
      if (doWarn && wantPropSpecializedWarnings()) {
        gen(env, RaiseUndefProp, baseAsObj, key);
      }
      if (doDefine) {
        gen(
          env,
          StProp,
          baseAsObj,
          cns(env, propInfo.offset),
          cns(env, Type::InitNull)
        );
        return propAddr;
      }
      return env.irb.genPtrToInitNull();
    }
  );
}

void emitPropSpecialized(MTS& env, const MInstrAttr mia, PropInfo propInfo) {
  assert(!(mia & MIA_warn) || !(mia & MIA_unset));
  const bool doWarn   = mia & MIA_warn;
  const bool doDefine = mia & MIA_define || mia & MIA_unset;

  auto const initNull = env.irb.genPtrToInitNull();

  SCOPE_EXIT {
    // After this function, m_base is either a pointer to init_null_variant or
    // a property in the object that we've verified isn't uninit.
    assert(env.base.type.isPtr());
  };

  /*
   * Normal case, where the base is an object (and not a pointer to
   * something)---just do a lea with the type information we got from static
   * analysis.  The caller of this function will use it to know whether it can
   * avoid a generic incref, unbox, etc.
   */
  if (env.base.type <= Type::Obj) {
    auto const propAddr = gen(
      env,
      LdPropAddr,
      convertToType(propInfo.repoAuthType).ptr(Ptr::Prop),
      env.base.value,
      cns(env, propInfo.offset)
    );
    setBase(
      env,
      checkInitProp(env, env.base.value, propAddr, propInfo, doWarn, doDefine)
    );
    return;
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
  auto const newBase = env.irb.cond(
    0,
    [&] (Block* taken) {
      gen(env, CheckTypeMem, Type::Obj, taken, env.base.value);
    },
    [&] {
      // Next: Base is an object. Load property and check for uninit.
      auto const obj = gen(
        env,
        LdMem,
        env.base.type.deref() & Type::Obj,
        env.base.value,
        cns(env, 0)
      );
      auto const propAddr = gen(
        env,
        LdPropAddr,
        convertToType(propInfo.repoAuthType).ptr(Ptr::Prop),
        obj,
        cns(env, propInfo.offset)
      );
      return checkInitProp(env, obj, propAddr, propInfo, doWarn, doDefine);
    },
    [&] { // Taken: Base is Null. Raise warnings/errors and return InitNull.
      env.irb.hint(Block::Hint::Unlikely);
      if (doWarn) {
        gen(env, WarnNonObjProp);
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
  setBase(env, newBase);
}

void emitPropGeneric(MTS& env) {
  auto const mCode = env.immVecM[env.mInd];
  auto const mia = MInstrAttr(env.mii.getAttr(mCode) & MIA_intermediate_prop);

  if ((mia & MIA_unset) && env.base.type.strip().not(Type::Obj)) {
    constrainBase(env, DataTypeSpecific);
    setBase(env, env.irb.genPtrToInitNull());
    return;
  }

  auto const key = getKey(env);
  if (mia & MIA_define) {
    setBase(
      env,
      gen(env,
          PropDX,
          MInstrAttrData { mia },
          env.base.value,
          key,
          misPtr(env))
    );
  } else {
    setBase(
      env,
      gen(env,
          PropX,
          MInstrAttrData { mia },
          env.base.value,
          key,
          misPtr(env))
    );
  }
}

void emitProp(MTS& env) {
  const Class* knownCls = nullptr;
  const auto propInfo   = getCurrentPropertyOffset(env, knownCls);
  auto mia = env.mii.getAttr(env.immVecM[env.mInd]);
  if (propInfo.offset == -1 || (mia & MIA_unset) ||
      mightCallMagicPropMethod(mia, knownCls, propInfo)) {
    emitPropGeneric(env);
  } else {
    emitPropSpecialized(env, mia, propInfo);
  }
}

void emitElem(MTS& env) {
  auto const mCode = env.immVecM[env.mInd];
  auto const mia = MInstrAttr(env.mii.getAttr(mCode) & MIA_intermediate);
  auto const key = getKey(env);

  // Fast path for the common/easy case
  const bool warn = mia & MIA_warn;
  const bool unset = mia & MIA_unset;
  const bool define = mia & MIA_define;
  if (env.base.type <= Type::PtrToArr &&
      !unset && !define &&
      (key->isA(Type::Int) || key->isA(Type::Str))) {
    setBase(
      env,
      gen(env,
          warn ? ElemArrayW : ElemArray,
          env.base.value,
          key)
    );
    return;
  }

  assert(!(define && unset));
  if (unset) {
    auto const uninit = env.irb.genPtrToUninit();
    auto const baseType = env.base.type.strip();
    constrainBase(env, DataTypeSpecific);
    if (baseType <= Type::Str) {
      gen(
        env,
        RaiseError,
        cns(env, makeStaticString(Strings::OP_NOT_SUPPORTED_STRING))
      );
      setBase(env, uninit);
      return;
    }
    if (baseType.not(Type::Arr | Type::Obj)) {
      setBase(env, uninit);
      return;
    }
  }

  if (define || unset) {
    setBase(
      env,
      genStk(env,
             define ? ElemDX : ElemUX,
             MInstrAttrData { mia },
             env.base.value,
             key,
             misPtr(env))
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
        misPtr(env))
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
    case MPC: case MPL: case MPT:
      emitProp(env);
      ++env.iInd;
      break;
    case MW:
      assert(env.mii.newElem());
      emitNewElem(env);
      break;
    default: not_reached();
  }
}

//////////////////////////////////////////////////////////////////////

bool needFirstRatchet(const MTS& env) {
  auto const firstTy = env.ni.inputs[env.mii.valCount()]->rtt.unbox();
  if (firstTy <= Type::Arr) {
    if (mcodeIsElem(env.immVecM[0])) return false;
    return true;
  }
  if (firstTy < Type::Obj && firstTy.isSpecialized()) {
    auto const klass = firstTy.getClass();
    auto const no_overrides = AttrNoOverrideMagicGet|
                              AttrNoOverrideMagicSet|
                              AttrNoOverrideMagicIsset|
                              AttrNoOverrideMagicUnset;
    if ((klass->attrs() & no_overrides) != no_overrides) {
      // Note: we could also add a check here on whether the first property RAT
      // contains Uninit---if not we can still return false.  See
      // mightCallMagicPropMethod.
      return true;
    }
    if (mcodeIsProp(env.immVecM[0])) return false;
    return true;
  }
  return true;
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
unsigned nLogicalRatchets(const MTS& env) {
  // If we've proven elsewhere that we don't need an MInstrState struct, we
  // know this translation won't need any ratchets
  if (!env.needMIS) return 0;

  unsigned ratchets = env.immVecM.size();
  if (!needFirstRatchet(env)) --ratchets;
  if (!needFinalRatchet(env)) --ratchets;
  return ratchets;
}

int ratchetInd(const MTS& env) {
  return needFirstRatchet(env) ? int(env.mInd) : int(env.mInd) - 1;
}

void emitRatchetRefs(MTS& env) {
  if (ratchetInd(env) < 0 || ratchetInd(env) >= int(nLogicalRatchets(env))) {
    return;
  }

  setBase(env, env.irb.cond(
    0,
    [&] (Block* taken) {
      gen(env, CheckInitMem, taken, env.misBase, cns(env, MISOFF(tvRef)));
    },
    [&] { // Next: tvRef isn't Uninit. Ratchet the refs
      // Clean up tvRef2 before overwriting it.
      if (ratchetInd(env) > 0) {
        gen(env, DecRefMem, Type::Gen, env.misBase, cns(env, MISOFF(tvRef2)));
      }
      // Copy tvRef to tvRef2.
      auto const tvRef = gen(
        env,
        LdMem,
        Type::Gen,
        env.misBase,
        cns(env, MISOFF(tvRef))
      );
      gen(env, StMem, env.misBase, cns(env, MISOFF(tvRef2)), tvRef);

      // Reset tvRef.
      gen(env, StMem, env.misBase, cns(env, MISOFF(tvRef)),
        cns(env, Type::Uninit));

      // Adjust base pointer.
      assert(env.base.type.isPtr());
      return gen(env, LdMIStateAddr, env.misBase, cns(env, MISOFF(tvRef2)));
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

  // The base location is input 0 or 1, and the location code is stored
  // separately from immVecM, so input indices (iInd) and member indices (mInd)
  // commonly differ.  Additionally, W members have no corresponding inputs, so
  // it is necessary to track the two indices separately.
  env.simpleOp = computeSimpleCollectionOp(env);
  emitBaseOp(env);
  ++env.iInd;

  checkMIState(env);
  if (env.needMIS) {
    env.misBase = gen(env, DefMIStateBase);
    auto const uninit = cns(env, Type::Uninit);
    if (nLogicalRatchets(env) > 0) {
      gen(env, StMem, env.misBase, cns(env, MISOFF(tvRef)), uninit);
      gen(env, StMem, env.misBase, cns(env, MISOFF(tvRef2)), uninit);
    }

    // If we're using an MInstrState, all the default-created catch blocks for
    // exception paths from here out will need to clean up the tvRef{,2}
    // storage, so install a custom catch creator.
    auto const penv = &env;
    env.hts.catchCreator = [penv] { return makeMISCatch(*penv); };
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

// Build a map from (stack) input index to stack index.
void numberStackInputs(MTS& env) {
  // Stack inputs are pushed in the order they appear in the vector from left
  // to right, so earlier elements in the vector are at higher offsets in the
  // stack. mii.valCount() tells us how many rvals the instruction takes on the
  // stack; they're pushed after any vector elements and we want to ignore them
  // here.
  bool stackRhs = env.mii.valCount() &&
    env.ni.inputs[0]->location.space == Location::Stack;
  int stackIdx = (int)stackRhs + env.immVec.numStackValues() - 1;
  for (unsigned i = env.mii.valCount(); i < env.ni.inputs.size(); ++i) {
    const Location& l = env.ni.inputs[i]->location;
    switch (l.space) {
      case Location::Stack:
        assert(stackIdx >= 0);
        env.stackInputs[i] = stackIdx--;
        break;

      default:
        break;
    }
  }
  assert(stackIdx == (stackRhs ? 0 : -1));

  if (stackRhs) {
    // If this instruction does have an RHS, it will be input 0 at
    // stack offset 0.
    assert(env.mii.valCount() == 1);
    env.stackInputs[0] = 0;
  }
}

//////////////////////////////////////////////////////////////////////
// "Simple op" handlers.

SSATmp* emitPackedArrayGet(MTS& env, SSATmp* base, SSATmp* key) {
  assert(base->isA(Type::Arr) &&
         base->type().getArrayKind() == ArrayData::kPackedKind);

  auto doLdElem = [&] {
    auto res = gen(env, LdPackedArrayElem, base, key);
    auto unboxed = unbox(env, res, nullptr);
    gen(env, IncRef, unboxed);
    return unboxed;
  };

  if (key->isConst() &&
      packedArrayBoundsCheckUnnecessary(base->type(), key->intVal())) {
    return doLdElem();
  }

  return env.irb.cond(
    1,
    [&] (Block* taken) {
      gen(env, CheckPackedArrayBounds, taken, base, key);
    },
    [&] { // Next:
      return doLdElem();
    },
    [&] { // Taken:
      env.irb.hint(Block::Hint::Unlikely);
      gen(env, RaiseArrayIndexNotice, key);
      return cns(env, Type::InitNull);
    }
  );
}

SSATmp* emitArrayGet(MTS& env, SSATmp* key) {
  return gen(env, ArrayGet, env.base.value, key);
}

void emitProfiledArrayGet(MTS& env, SSATmp* key) {
  TargetProfile<NonPackedArrayProfile> prof(env.hts.context,
                                            env.irb.curMarker(),
                                            s_PackedArray.get());
  if (prof.profiling()) {
    gen(env, ProfileArray, RDSHandleData { prof.handle() }, env.base.value);
    env.result = emitArrayGet(env, key);
    return;
  }

  if (prof.optimizing()) {
    auto const data = prof.data(NonPackedArrayProfile::reduce);
    // NonPackedArrayProfile data counts how many times a non-packed array was
    // observed.  Zero means it was monomorphic (or never executed).
    auto const typePackedArr = Type::Arr.specialize(ArrayData::kPackedKind);
    if (env.base.type.maybe(typePackedArr) &&
        (data.count == 0 || RuntimeOption::EvalJitPGOArrayGetStress)) {
      // It's safe to side-exit still because we only do these profiled array
      // gets on the first element, with simple bases and single-element dims.
      // See computeSimpleCollectionOp.
      auto const exit = makeExit(env);
      setBase(
        env,
        gen(env, CheckType, typePackedArr, exit, env.base.value)
      );
      env.irb.constrainValue(
        env.base.value,
        TypeConstraint(DataTypeSpecialized).setWantArrayKind()
      );
      env.result = emitPackedArrayGet(env, env.base.value, key);
      return;
    }
  }

  // Fall back to a generic array get.
  env.result = emitArrayGet(env, key);
}

void emitStringGet(MTS& env, SSATmp* key) {
  assert(key->isA(Type::Int));
  env.result = gen(env, StringGet, env.base.value, key);
}

void emitVectorGet(MTS& env, SSATmp* key) {
  assert(key->isA(Type::Int));
  if (key->isConst() && key->intVal() < 0) {
    PUNT(emitVectorGet);
  }
  auto const size = gen(env, LdVectorSize, env.base.value);
  gen(env, CheckBounds, key, size);
  auto const base = gen(env, LdVectorBase, env.base.value);
  static_assert(sizeof(TypedValue) == 16,
                "TypedValue size expected to be 16 bytes");
  auto idx = gen(env, Shl, key, cns(env, 4));
  env.result = gen(env, LdElem, base, idx);
  gen(env, IncRef, env.result);
}

void emitPairGet(MTS& env, SSATmp* key) {
  assert(key->isA(Type::Int));
  static_assert(sizeof(TypedValue) == 16,
                "TypedValue size expected to be 16 bytes");
  if (key->isConst()) {
    auto idx = key->intVal();
    if (idx < 0 || idx > 1) {
      PUNT(emitPairGet);
    }
    // no reason to check bounds
    auto const base = gen(env, LdPairBase, env.base.value);
    auto index = cns(env, key->intVal() << 4);
    env.result = gen(env, LdElem, base, index);
  } else {
    gen(env, CheckBounds, key, cns(env, 2));
    auto const base = gen(env, LdPairBase, env.base.value);
    auto idx = gen(env, Shl, key, cns(env, 4));
    env.result = gen(env, LdElem, base, idx);
  }
  gen(env, IncRef, env.result);
}

void emitPackedArrayIsset(MTS& env) {
  assert(env.base.type.getArrayKind() == ArrayData::kPackedKind);
  auto const key = getKey(env);
  env.result = env.irb.cond(
    0,
    [&] (Block* taken) {
      gen(env, CheckPackedArrayBounds, taken, env.base.value, key);
    },
    [&] { // Next:
      return gen(env, IsPackedArrayElemNull, env.base.value, key);
    },
    [&] { // Taken:
      return cns(env, false);
    }
  );
}

void emitArraySet(MTS& env, SSATmp* key, SSATmp* value) {
  assert(env.iInd == env.mii.valCount() + 1);
  const int baseStkIdx = env.mii.valCount();
  assert(key->type().notBoxed());
  assert(value->type().notBoxed());

  auto const& base = *env.ni.inputs[env.mii.valCount()];
  bool const setRef = base.rtt.isBoxed();

  // No catch trace below because the helper can't throw. It may reenter to
  // call destructors so it has a sync point in nativecalls.cpp, but exceptions
  // are swallowed at destructor boundaries right now: #2182869.
  if (setRef) {
    assert(base.location.space == Location::Local ||
           base.location.space == Location::Stack);
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
  if (base.location.space == Location::Local) {
    // We know it's not boxed (setRef above handles that), and
    // newArr has already been incref'd in the helper.
    gen(env, StLoc, LocalId(base.location.offset), fp(env), newArr);
  } else if (base.location.space == Location::Stack) {
    extendStack(env, baseStkIdx, Type::Gen);
    env.irb.evalStack().replace(baseStkIdx, newArr);
  } else {
    not_reached();
  }

  env.result = value;
}

void emitVectorSet(MTS& env, SSATmp* key, SSATmp* value) {
  assert(key->isA(Type::Int));
  if (key->isConst() && key->intVal() < 0) {
    PUNT(emitVectorSet); // will throw
  }
  auto const size = gen(env, LdVectorSize, env.base.value);
  gen(env, CheckBounds, key, size);

  env.irb.ifThen(
    [&](Block* taken) {
      gen(env, VectorHasImmCopy, taken, env.base.value);
    },
    [&] {
      env.irb.hint(Block::Hint::Unlikely);
      gen(env, VectorDoCow, env.base.value);
    }
  );

  gen(env, IncRef, value);
  auto const vecBase = gen(env, LdVectorBase, env.base.value);
  static_assert(sizeof(TypedValue) == 16,
                "TypedValue size expected to be 16 bytes");
  auto const idx = gen(env, Shl, key, cns(env, 4));
  auto const oldVal = gen(env, LdElem, vecBase, idx);
  gen(env, StElem, vecBase, idx, value);
  gen(env, DecRef, oldVal);

  env.result = value;
}

//////////////////////////////////////////////////////////////////////

void emitCGetProp(MTS& env) {
  const Class* knownCls = nullptr;
  const auto propInfo   = getCurrentPropertyOffset(env, knownCls);

  if (propInfo.offset != -1 &&
      !mightCallMagicPropMethod(MIA_none, knownCls, propInfo)) {
    emitPropSpecialized(env, MIA_warn, propInfo);

    if (!RuntimeOption::RepoAuthoritative) {
      auto const cellPtr = gen(env, UnboxPtr, env.base.value);
      env.result = gen(env, LdMem, Type::Cell, cellPtr, cns(env, 0));
      gen(env, IncRef, env.result);
      return;
    }

    auto const ty      = env.base.type.deref();
    auto const cellPtr = ty.maybeBoxed() ? gen(env, UnboxPtr, env.base.value)
                                         : env.base.value;
    env.result = gen(env, LdMem, ty.unbox(), cellPtr, cns(env, 0));
    gen(env, IncRef, env.result);
    return;
  }

  auto const key = getKey(env);
  env.result = gen(
    env,
    CGetProp,
    env.base.value,
    key,
    misPtr(env)
  );
}

void emitVGetProp(MTS& env) {
  auto const key = getKey(env);
  env.result = genStk(env, VGetProp, env.base.value, key, misPtr(env));
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
  const Class* knownCls = nullptr;
  const auto propInfo   = getCurrentPropertyOffset(env, knownCls);

  if (propInfo.offset != -1 &&
      !mightCallMagicPropMethod(MIA_define, knownCls, propInfo)) {
    emitPropSpecialized(env, MIA_define, propInfo);

    auto const propTy  = env.base.type.deref();
    auto const cellTy  = propTy.maybeBoxed() ? propTy.unbox() : propTy;
    auto const cellPtr = propTy.maybeBoxed() ? gen(env, UnboxPtr, env.base.value)
                                             : env.base.value;
    auto const oldVal  = gen(env, LdMem, cellTy, cellPtr, cns(env, 0));

    gen(env, IncRef, value);
    gen(env, StMem, cellPtr, cns(env, 0), value);
    gen(env, DecRef, oldVal);
    env.result = value;
    return;
  }

  // Emit the appropriate helper call.
  auto const key = getKey(env);
  genStk(env, SetProp, makeCatchSet(env), env.base.value, key, value);
  env.result = value;
}

void emitSetOpProp(MTS& env) {
  SetOpOp op = SetOpOp(env.ni.imm[0].u_OA);
  auto const key = getKey(env);
  auto const value = getValue(env);
  env.result = genStk(env, SetOpProp, SetOpData { op },
                      env.base.value, key, value, misPtr(env));
}

void emitIncDecProp(MTS& env) {
  IncDecOp op = static_cast<IncDecOp>(env.ni.imm[0].u_OA);
  auto const key = getKey(env);
  env.result = genStk(env, IncDecProp, IncDecData { op },
                      env.base.value, key, misPtr(env));
}

void emitBindProp(MTS& env) {
  auto const key = getKey(env);
  auto const box = getValue(env);
  genStk(env, BindProp, env.base.value, key, box, misPtr(env));
  env.result = box;
}

void emitUnsetProp(MTS& env) {
  auto const key = getKey(env);
  if (env.base.type.strip().not(Type::Obj)) {
    // Noop
    constrainBase(env, DataTypeSpecific);
    return;
  }
  gen(env, UnsetProp, env.base.value, key);
}

void emitCGetElem(MTS& env) {
  auto const key = getKey(env);

  switch (env.simpleOp) {
  case SimpleOp::Array:
    env.result = emitArrayGet(env, key);
    break;
  case SimpleOp::PackedArray:
    env.result = emitPackedArrayGet(env, env.base.value, key);
    break;
  case SimpleOp::ProfiledArray:
    emitProfiledArrayGet(env, key);
    break;
  case SimpleOp::String:
    emitStringGet(env, key);
    break;
  case SimpleOp::Vector:
    emitVectorGet(env, key);
    break;
  case SimpleOp::Pair:
    emitPairGet(env, key);
    break;
  case SimpleOp::Map:
    env.result = gen(env, MapGet, env.base.value, key);
    break;
  case SimpleOp::None:
    env.result = gen(env, CGetElem, env.base.value, key, misPtr(env));
    break;
  }
}

void emitVGetElem(MTS& env) {
  auto const key = getKey(env);
  env.result = genStk(env, VGetElem, env.base.value, key, misPtr(env));
}

void emitIssetElem(MTS& env) {
  switch (env.simpleOp) {
  case SimpleOp::Array:
  case SimpleOp::ProfiledArray:
    env.result = gen(env, ArrayIsset, env.base.value, getKey(env));
    break;
  case SimpleOp::PackedArray:
    emitPackedArrayIsset(env);
    break;
  case SimpleOp::String:
    env.result = gen(env, StringIsset, env.base.value, getKey(env));
    break;
  case SimpleOp::Vector:
    env.result = gen(env, VectorIsset, env.base.value, getKey(env));
    break;
  case SimpleOp::Pair:
    env.result = gen(env, PairIsset, env.base.value, getKey(env));
    break;
  case SimpleOp::Map:
    env.result = gen(env, MapIsset, env.base.value, getKey(env));
    break;
  case SimpleOp::None:
    {
      auto const key = getKey(env);
      env.result = gen(env, IssetElem, env.base.value, key, misPtr(env));
    }
    break;
  }
}

void emitEmptyElem(MTS& env) {
  auto const key = getKey(env);
  env.result = gen(env, EmptyElem, env.base.value, key, misPtr(env));
}

void emitSetNewElem(MTS& env) {
  auto const value = getValue(env);
  if (env.base.type <= Type::PtrToArr) {
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
  if (env.base.type.strip() <= Type::Arr && getValue(env)->type().notBoxed()) {
    constrainBase(env, DataTypeSpecific);
    emitSetNewElem(env);
  } else {
    genStk(env, SetWithRefNewElem, env.base.value, getValAddr(env),
      misPtr(env));
  }
  env.result = nullptr;
}

void emitSetElem(MTS& env) {
  auto const value = getValue(env);
  auto const key = getKey(env);

  switch (env.simpleOp) {
  case SimpleOp::Array:
  case SimpleOp::ProfiledArray:
    emitArraySet(env, key, value);
    break;
  case SimpleOp::PackedArray:
  case SimpleOp::String:
    always_assert(false && "Bad SimpleOp in emitSetElem");
    break;
  case SimpleOp::Vector:
    emitVectorSet(env, key, value);
    break;
  case SimpleOp::Map:
    gen(env, MapSet, env.base.value, key, value);
    env.result = value;
    break;
  case SimpleOp::Pair:
  case SimpleOp::None:
    constrainBase(env, DataTypeSpecific);
    auto const result = genStk(env, SetElem, makeCatchSet(env),
                               env.base.value, key, value);
    auto const t = result->type();
    if (t == Type::Nullptr) {
      // Base is not a string. Result is always value.
      env.result = value;
    } else if (t == Type::CountedStr) {
      // Base is a string. Stack result is a new string so we're responsible for
      // decreffing value.
      env.result = result;
      gen(env, DecRef, value);
    } else {
      assert(t.equals(Type::CountedStr | Type::Nullptr));
      // Base might be a string. Assume the result is value, then inform
      // emitMPost that it needs to test the actual result.
      env.result = value;
      env.strTestResult = result;
    }
    break;
  }
}

void emitSetWithRefLElem(MTS& env) {
  auto const key = getKey(env);
  auto const locAddr = getValAddr(env);
  if (env.base.type.strip() <= Type::Arr &&
      !locAddr->type().deref().maybeBoxed()) {
    constrainBase(env, DataTypeSpecific);
    emitSetElem(env);
    assert(env.strTestResult == nullptr);
  } else {
    genStk(env, SetWithRefElem, env.base.value, key, locAddr, misPtr(env));
  }
  env.result = nullptr;
}
void emitSetWithRefRElem(MTS& env) { emitSetWithRefLElem(env); }

void emitSetOpElem(MTS& env) {
  auto const op = static_cast<SetOpOp>(env.ni.imm[0].u_OA);
  env.result = genStk(env, SetOpElem, SetOpData{op},
                      env.base.value, getKey(env), getValue(env),
                      misPtr(env));
}

void emitIncDecElem(MTS& env) {
  auto const op = static_cast<IncDecOp>(env.ni.imm[0].u_OA);
  env.result = genStk(env, IncDecElem, IncDecData { op },
                      env.base.value, getKey(env), misPtr(env));
}

void emitBindElem(MTS& env) {
  auto const key = getKey(env);
  auto const box = getValue(env);
  genStk(env, BindElem, env.base.value, key, box, misPtr(env));
  env.result = box;
}

void emitUnsetElem(MTS& env) {
  auto const key = getKey(env);

  auto const baseType = env.base.type.strip();
  constrainBase(env, DataTypeSpecific);
  if (baseType <= Type::Str) {
    gen(env,
        RaiseError,
        cns(env, makeStaticString(Strings::CANT_UNSET_STRING)));
    return;
  }
  if (baseType.not(Type::Arr | Type::Obj)) {
    // Noop
    return;
  }

  genStk(env, UnsetElem, env.base.value, key);
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
  genStk(env, BindNewElem, env.base.value, box, misPtr(env));
  env.result = box;
}

void emitFinalMOp(MTS& env) {
  using MemFun = void (*)(MTS&);

  switch (env.immVecM[env.mInd]) {
  case MEC: case MEL: case MET: case MEI:
    static MemFun elemOps[] = {
#   define MII(instr, ...) &emit##instr##Elem,
    MINSTRS
#   undef MII
    };
    elemOps[env.mii.instr()](env);
    break;

  case MPC: case MPL: case MPT:
    static MemFun propOps[] = {
#   define MII(instr, ...) &emit##instr##Prop,
    MINSTRS
#   undef MII
    };
    propOps[env.mii.instr()](env);
    break;

  case MW:
    assert(env.mii.getAttr(MW) & MIA_final);
    static MemFun newOps[] = {
#   define MII(instr, attrs, bS, iS, vC, fN) \
      &emit##fN,
    MINSTRS
#   undef MII
    };
    newOps[env.mii.instr()](env);
    break;

  default: not_reached();
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
  constexpr ptrdiff_t refOffs[] = { MISOFF(tvRef), MISOFF(tvRef2) };
  for (unsigned i = 0; i < std::min(nLogicalRatchets(env), 2U); ++i) {
    gen(
      env,
      DecRefMem,
      Type::Gen,
      env.misBase,
      cns(env, refOffs[env.failedSetBlock ? 1 - i : i])
    );
  }
}

enum class DecRefStyle { FromCatch, FromMain };
uint32_t decRefStackInputs(MTS& env, DecRefStyle why) {
  uint32_t const startOff =
    env.op == Op::SetM || env.op == Op::BindM ? 1 : 0;
  auto const stackCnt = env.stackInputs.size();
  for (auto i = startOff; i < stackCnt; ++i) {
    switch (why) {
    case DecRefStyle::FromCatch:
      if (topType(env, i, DataTypeGeneric) <= Type::Gen) {
        gen(env,
            DecRefStack,
            StackOffset { static_cast<int32_t>(i) },
            Type::Gen,
            sp(env));
      }
      break;
    case DecRefStyle::FromMain:
      {
        auto const input = top(env, Type::StackElem, i, DataTypeSpecific);
        if (input->type() <= Type::Gen) {
          gen(env, DecRef, input);
        }
      }
      break;
    }
  }
  return stackCnt;
}

void handleStrTestResult(MTS& env) {
  if (!env.strTestResult) return;
  // We expected SetElem's base to not be a Str but might be wrong. Make an
  // exit trace to side exit to the next instruction, replacing our guess
  // with the correct stack output.
  env.irb.ifThen(
    [&] (Block* taken) {
      gen(env, CheckNullptr, taken, env.strTestResult);
    },
    [&] {
      env.irb.hint(Block::Hint::Unlikely);
      auto const str = gen(env, AssertNonNull, env.strTestResult);
      gen(env, DecRef, env.result);
      auto const stackCnt = decRefStackInputs(env, DecRefStyle::FromMain);
      discard(env, stackCnt);
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
  spillStack(env);
  gen(env, EndCatch, fp(env), sp(env));
  return exit;
}

Block* makeCatchSet(MTS& env) {
  env.failedSetBlock = env.unit.defBlock(Block::Hint::Unused);

  const bool isSetWithRef = env.op == Op::SetWithRefLM ||
                            env.op == Op::SetWithRefRM;

  BlockPusher bp(env.irb, makeMarker(env, bcOff(env)), env.failedSetBlock);
  gen(env, BeginCatch);
  spillStack(env);

  env.irb.ifThen(
    [&] (Block* taken) {
      gen(env, UnwindCheckSideExit, taken, fp(env), sp(env));
    },
    [&] {
      env.irb.hint(Block::Hint::Unused);
      cleanTvRefs(env);
      gen(env, EndCatch, fp(env), sp(env));
    }
  );
  env.irb.hint(Block::Hint::Unused);

  /*
   * Fallthrough from here on is side-exiting due to an InvalidSetMException.
   */

  // For consistency with the interpreter, decref the rhs before we decref the
  // stack inputs, and decref the ratchet storage after the stack inputs.
  if (!isSetWithRef) {
    gen(env, DecRefStack, StackOffset { 0 }, Type::Cell, sp(env));
  }
  auto const stackCnt = decRefStackInputs(env, DecRefStyle::FromCatch);
  discard(env, stackCnt);
  cleanTvRefs(env);
  if (!isSetWithRef) {
    auto const val = gen(env, LdUnwinderValue, Type::Cell);
    push(env, val);
  }
  gen(env, DeleteUnwinderException);
  gen(env, Jmp, makeExit(env, nextBcOff(env)));
  return env.failedSetBlock;
}

void emitMPost(MTS& env) {
  handleStrTestResult(env);

  auto const stackCnt = decRefStackInputs(env, DecRefStyle::FromMain);
  discard(env, stackCnt);

  // Push result, if one was produced. If we had a predicted result
  // (strTestResult case), it was already guarded on above.
  if (env.result) {
    push(env, env.result);
  } else {
    assert(env.op == Op::UnsetM ||
           env.op == Op::SetWithRefLM ||
           env.op == Op::SetWithRefRM);
  }

  cleanTvRefs(env);
}

//////////////////////////////////////////////////////////////////////

void implMInstr(HTS& hts, Op effectiveOp) {
  if (curFunc(hts)->isPseudoMain()) {
    interpOne(hts, *hts.currentNormalizedInstruction);
    return;
  }

  auto env = MTS { hts, effectiveOp };
  numberStackInputs(env); // Assign stack slots to our stack inputs
  emitMPre(env);          // Emit the base and every intermediate op
  emitFinalMOp(env);      // Emit the final operation
  emitMPost(env);         // Cleanup: decref inputs and scratch values
}

//////////////////////////////////////////////////////////////////////

}

void emitBindM(HTS& env, int)                 { implMInstr(env, Op::BindM); }
void emitCGetM(HTS& env, int)                 { implMInstr(env, Op::CGetM); }
void emitEmptyM(HTS& env, int)                { implMInstr(env, Op::EmptyM); }
void emitIncDecM(HTS& env, IncDecOp, int)     { implMInstr(env, Op::IncDecM); }
void emitIssetM(HTS& env, int)                { implMInstr(env, Op::IssetM); }
void emitSetM(HTS& env, int)                  { implMInstr(env, Op::SetM); }
void emitSetOpM(HTS& env, SetOpOp, int)       { implMInstr(env, Op::SetOpM); }
void emitUnsetM(HTS& env, int)                { implMInstr(env, Op::UnsetM); }
void emitVGetM(HTS& env, int)                 { implMInstr(env, Op::VGetM); }

void emitSetWithRefLM(HTS& env, int, int32_t) {
  implMInstr(env, Op::SetWithRefLM);
}

void emitSetWithRefRM(HTS& env, int) {
  implMInstr(env, Op::SetWithRefRM);
}

//////////////////////////////////////////////////////////////////////

}}}
