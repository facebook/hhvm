/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "runtime/base/strings.h"
#include "runtime/vm/member_operations.h"
#include "runtime/vm/translator/hopt/ir.h"
#include "runtime/vm/translator/hopt/hhbctranslator.h"
#include "runtime/vm/translator/translator-x64.h"

// This file does ugly things with macros so include it last
#include "runtime/vm/translator/hopt/vectortranslator-internal.h"

namespace HPHP { namespace VM { namespace JIT {

TRACE_SET_MOD(hhir);

using Transl::MInstrState;
using Transl::mInstrHasUnknownOffsets;

namespace {

// Reduce baseType to a canonical unboxed, non-pointer form, returning (in
// basePtr and baseBoxed) whether the original type was a pointer or boxed,
// respectively. Whether or not the type is a pointer and/or boxed must be
// statically known, unless it's exactly equal to Gen.
void stripBase(Type& baseType, bool& basePtr, bool& baseBoxed) {
  assert_not_implemented(baseType.isPtr() || baseType.notPtr());
  basePtr = baseType.isPtr();
  baseType = basePtr ? baseType.deref() : baseType;
  assert_not_implemented(
    baseType.equals(Type::Gen) || baseType.isBoxed() || baseType.notBoxed());
  baseBoxed = baseType.isBoxed();
  baseType = baseBoxed ? baseType.innerType() : baseType;
}

}

bool VectorEffects::supported(const IRInstruction* inst) {
  switch (inst->getOpcode()) {
    case SetProp:
    case SetElem:
    case SetNewElem:
    case ElemDX:
      return true;

    default:
      return false;
  }
}

void VectorEffects::get(const IRInstruction* inst,
                        StoreLocFunc storeLocalValue,
                        SetLocTypeFunc setLocalType) {
  // If the base for this instruction is a local address, the
  // helper call might have side effects on the local's value
  SSATmp* base = inst->getSrc(vectorBaseIdx(inst));
  IRInstruction* locInstr = base->getInstruction();
  if (locInstr->getOpcode() == LdLocAddr) {
    UNUSED Type baseType = locInstr->getDst()->getType();
    assert(baseType.equals(base->getType()));
    assert(baseType.isPtr() || baseType.isKnownDataType());
    int loc = locInstr->getExtra<LdLocAddr>()->locId;

    VectorEffects ve(inst);
    if (ve.baseTypeChanged || ve.baseValChanged) {
      storeLocalValue(loc, nullptr);
      setLocalType(loc, ve.baseType.derefIfPtr());
    }
  }
}

void VectorEffects::init(Opcode op, const Type origBase,
                         const Type key, const Type origVal) {
  baseType = origBase;
  bool basePtr, baseBoxed;
  stripBase(baseType, basePtr, baseBoxed);
  // Every base coming through here should be a pointer or object for now, but
  // may not be in the future.
  assert_not_implemented(basePtr || baseType.subtypeOf(Type::Obj));

  valType = origVal;
  baseTypeChanged = baseValChanged = valTypeChanged = false;

  // Canonicalize the op to SetProp or SetElem
  switch (op) {
    case SetProp:
      break;

    case SetElem:
    case SetNewElem:
    case ElemDX:
      op = SetElem;
      break;

    default:
      not_implemented();
  }
  assert(op == SetProp || op == SetElem);
  assert(key.equals(Type::None) || key.isKnownDataType());
  assert(origVal.equals(Type::None) || origVal.isKnownDataType());

  if (baseType.maybe(Type::Null | Type::Bool | Type::Str)) {
    // stdClass or array promotion might happen
    auto newBase = op == SetElem ? Type::Arr : Type::Obj;
    // If the base is known to be null, promotion will happen. If we can ever
    // prove that the base is false or the empty string, promotion will
    // definitely happen but those cases aren't handled yet. In a perfect world
    // we would remove Type::Null from baseType here but that can produce types
    // that are tricky to guard against and doesn't buy us much right now.
    baseType = baseType.isNull() ? newBase : (baseType | newBase);
    baseValChanged = true;
  }
  if (op == SetElem && baseType.maybe(Type::Arr)) {
    // possible COW when modifying an array
    baseValChanged = true;
  }
  if (op == SetElem && baseType.maybe(Type::StaticArr)) {
    // the base might get promoted to a CountedArr
    baseType = baseType | Type::CountedArr;
    baseValChanged = true;
  }

  // Setting an element with a base of one of these types will either succeed
  // (with possible promotion) or fatal
  const Type okBase = Type::Str | Type::Arr | Type::Obj;

  // Setting an element with one of these types as the key will fail, issue a
  // warning, and evaulate to null.
  const Type badArrKey = Type::Arr | Type::Obj;

  const bool maybeBadArrKey = key.maybe(badArrKey);
  if ((op == SetElem && (!baseType.subtypeOf(okBase) || maybeBadArrKey)) ||
      (op == SetProp && !baseType.subtypeOf(Type::Obj))) {
    // The set operation might fail, issue a warning, and evaluate to null. Any
    // other invalid combinations of base/key will throw an exception/fatal.

    const bool definitelyFail =
      // SetElem and the base is known to not be a good type
      (op == SetElem && baseType.not(okBase)) ||
      // SetElem and the array key is known to be bad
      (op == SetElem && key.subtypeOf(badArrKey)) ||
      // SetProp and the base is known to not be an object
      (op == SetProp && baseType.not(Type::Obj));

    valType = definitelyFail ? Type::InitNull : (valType | Type::InitNull);
  }

  // The final baseType should be a pointer/box iff the input was
  baseType = baseBoxed ? baseType.box() : baseType;
  baseType = basePtr   ? baseType.ptr() : baseType;

  baseTypeChanged = baseTypeChanged || baseType != origBase;
  baseValChanged = baseValChanged || baseTypeChanged;
  valTypeChanged = valTypeChanged || valType != origVal;
}

// vectorBaseIdx returns the src index for inst's base operand.
int vectorBaseIdx(const IRInstruction* inst) {
  switch (inst->getOpcode()) {
    case SetProp: return 2;
    case SetElem: return 1;
    case ElemDX:  return 1;
    case SetNewElem: return 0;
    default:      not_reached();
  }
}

// vectorKeyIdx returns the src index for inst's key operand.
int vectorKeyIdx(const IRInstruction* inst) {
  switch (inst->getOpcode()) {
    case SetProp: return 3;
    case SetElem: return 2;
    case ElemDX:  return 2;
    case SetNewElem: return -1;
    default:      not_reached();
  }
}

// vectorValIdx returns the src index for inst's value operand.
int vectorValIdx(const IRInstruction* inst) {
  switch (inst->getOpcode()) {
    case SetProp: return 4;
    case SetElem: return 3;
    case ElemDX:  return -1;
    case SetNewElem: return 1;
    default:      not_reached();
  }
}

HhbcTranslator::VectorTranslator::VectorTranslator(
  const NormalizedInstruction& ni,
  HhbcTranslator& ht)
    : m_ni(ni)
    , m_ht(ht)
    , m_tb(*m_ht.m_tb)
    , m_mii(getMInstrInfo(ni.mInstrOp()))
    , m_needMIS(true)
    , m_misBase(nullptr)
    , m_base(nullptr)
    , m_result(nullptr)
{
}

void HhbcTranslator::VectorTranslator::emit() {
  PUNT_WITH_TX64(VectorTranslator);
  // Assign stack slots to our stack inputs
  numberStackInputs();
  // Emit the base and every intermediate op
  emitMPre();
  // Emit the final operation
  emitFinalMOp();
  // Cleanup: decref inputs and scratch values
  emitMPost();
}

// Returns a pointer to the base of the current MInstrState struct, or
// a null pointer if it's not needed.
SSATmp* HhbcTranslator::VectorTranslator::genMisPtr() {
  if (m_needMIS) {
    return m_tb.genLdAddr(m_misBase, kReservedRSPSpillSpace);
  } else {
    ConstData cdata(nullptr);
    return m_tb.gen(DefConst, Type::PtrToCell, &cdata);
  }
}

// Inspect the instruction we're about to translate and determine if
// it can be executed without using an MInstrState struct.
void HhbcTranslator::VectorTranslator::checkMIState() {
  auto const& baseRtt = m_ni.inputs[m_mii.valCount()]->rtt;
  Type baseType = Type::fromRuntimeType(baseRtt);
  const bool isCGetM = m_ni.mInstrOp() == OpCGetM;
  const bool isSetM = m_ni.mInstrOp() == OpSetM;

  assert(baseType.isBoxed() || baseType.notBoxed());
  baseType = baseType.unbox();

  // CGetM or SetM with no unknown property offsets
  const bool simpleProp = !mInstrHasUnknownOffsets(m_ni) && (isCGetM || isSetM);

  // Array access with one element in the vector
  const bool simpleElem = m_ni.immVecM.size() == 1 &&
    mcodeMaybeArrayKey(m_ni.immVecM[0]);

  // SetM on an array with one vector element
  const bool simpleArraySet = isSetM && simpleElem;

  // CGetM on an array with a base that won't use MInstrState. Str
  // will use tvScratch and baseStrOff and Obj will fatal or use
  // tvRef.
  const bool simpleArrayGet = isCGetM && simpleElem &&
    baseType.not(Type::Str | Type::Obj);

  if (simpleProp || simpleArraySet || simpleArrayGet) {
    setNoMIState();
  }
}

void HhbcTranslator::VectorTranslator::emitMPre() {
  checkMIState();

  if (m_needMIS) {
    m_misBase = m_tb.gen(DefMIStateBase);
    SSATmp* uninit = m_tb.genDefUninit();

    if (nLogicalRatchets() > 0) {
      m_tb.genStMem(m_misBase, HHIR_MISOFF(tvRef), uninit, true);
      m_tb.genStMem(m_misBase, HHIR_MISOFF(tvRef2), uninit, true);
    }
    m_tb.genStRaw(m_misBase, RawMemSlot::MisBaseStrOff, cns(false));
  }

  // The base location is input 0 or 1, and the location code is stored
  // separately from m_ni.immVecM, so input indices (iInd) and member indices
  // (mInd) commonly differ.  Additionally, W members have no corresponding
  // inputs, so it is necessary to track the two indices separately.
  m_iInd = m_mii.valCount();
  emitBaseOp();
  ++m_iInd;

  // Iterate over all but the last member, which is consumed by a final
  // operation.
  for (m_mInd = 0; m_mInd < m_ni.immVecM.size() - 1; ++m_mInd) {
    emitIntermediateOp();
    emitRatchetRefs();
  }
}

// Build a map from (stack) input index to stack index.
void HhbcTranslator::VectorTranslator::numberStackInputs() {
  // Stack inputs are pushed in the order they appear in the vector
  // from left to right, so earlier elements in the vector are at
  // higher offsets in the stack. m_mii.valCount() tells us how many
  // rvals the instruction takes on the stack; they're pushed after
  // any vector elements and we want to ignore them here.
  int stackIdx = m_mii.valCount() + m_ni.immVec.numStackValues() - 1;
  for (unsigned i = m_mii.valCount(); i < m_ni.inputs.size(); ++i) {
    const Location& l = m_ni.inputs[i]->location;
    switch (l.space) {
      case Location::Stack:
        assert(stackIdx >= 0);
        m_stackInputs[i] = stackIdx--;
        break;

      default:
        break;
    }
  }
  assert(stackIdx == (m_mii.valCount() - 1));

  if (m_mii.valCount()) {
    // If this instruction does have an RHS, it will be input 0 at
    // stack offset 0.
    assert(m_mii.valCount() == 1);
    m_stackInputs[0] = 0;
  }
}

SSATmp* HhbcTranslator::VectorTranslator::getInput(unsigned i) {
  const DynLocation& dl = *m_ni.inputs[i];
  const Location& l = dl.location;

  assert(mapContains(m_stackInputs, i) == (l.space == Location::Stack));
  switch (l.space) {
    case Location::Stack: {
      SSATmp* val = m_ht.top(Type::Gen | Type::Cls, m_stackInputs[i]);
      // Check if the type on our eval stack is at least as specific
      // as what Transl::Translator came up with.
      auto t = Type::fromRuntimeType(dl.rtt);
      if (!val->isA(t)) {
        FTRACE(0, "{}: hhir stack has a {} where Translator had a {}\n",
               __func__, val->getType().toString(), t.toString());
        // They'd better not be completely unrelated types...
        assert(t.subtypeOf(val->getType()));

        m_ht.refineType(val, t);
      }
      return val;
    }

    case Location::Local:
      return m_tb.genLdLoc(l.offset);

    case Location::Litstr:
      return cns(m_ht.lookupStringId(l.offset));

    case Location::Litint:
      return cns(l.offset);

    case Location::This:
      return m_tb.genLdThis(nullptr);

    default: not_reached();
  }
}

void HhbcTranslator::VectorTranslator::emitBaseLCR() {
  const MInstrAttr& mia = m_mii.getAttr(m_ni.immVec.locationCode());
  const DynLocation& base = *m_ni.inputs[m_iInd];
  if (mia & MIA_warn) {
    if (base.rtt.isUninit()) {
      m_ht.spillStack();
      LocalId data(base.location.offset);
      m_tb.gen(RaiseUninitLoc, &data);
    }
  }

  if (base.location.isLocal()) {
    uint32_t loc = base.location.offset;
    auto baseType = m_tb.getLocalType(loc);

    if ((mia & MIA_define) && baseType.subtypeOf(Type::Uninit)) {
      // Promote Uninit to InitNull before doing anything else
      m_tb.genStLoc(loc, m_tb.genDefInitNull(), false, true, nullptr);
      baseType = Type::InitNull;
    }
    if (baseType.unbox().isNull()) {
      // The base local might get promoted to an Array or stdClass for set
      // operations. Punt for now.
      PUNT(emitBaseLCR-NullBase);
    }

    // We should never have a union type for the base with mixed boxes
    assert(baseType.isBoxed() || baseType.notBoxed());

    // If the base is a box with a type that's changed, we need to bail out of
    // the tracelet and retranslate. Doing an exit here is a little sketchy
    // since we may have already emitted instructions with memory effects to
    // initialize the MInstrState. These particular stores are harmless though,
    // and the worst outcome here is that we'll ending up doing the stores
    // twice, once for this instruction and once at the beginning of the
    // retranslation.
    Trace* failedRef = baseType.isBoxed() ? m_ht.getExitTrace() : nullptr;
    if (baseType.unbox().subtypeOf(Type::Obj) &&
        mcodeMaybePropName(m_ni.immVecM[0])) {
      // We can pass the base to helpers by value in this case
      m_base = m_tb.genLdLoc(loc);
      if (baseType.isBoxed()) {
        assert(m_base->isA(Type::BoxedObj));
        m_base = m_tb.gen(LdRef, Type::Obj, failedRef, m_base);
      }
      assert(m_base->isA(Type::Obj));
    } else {
      // Everything else is passed by reference. We don't have to worry about
      // unboxing here, since all the generic helpers understand boxed bases.
      m_base = m_tb.genLdLocAddr(loc, failedRef);
      assert(m_base->getType().isPtr());
    }
  } else {
    // Stack bases require a little more stack magic than we have right
    // now. Refcounting correctly is tricky too.
    PUNT(emitBaseLCR-CellBase);
    if (base.isVariant()) {
      PUNT(emitBaseLCR-R);
    }
  }
}

void HhbcTranslator::VectorTranslator::emitBaseH() {
  m_base = m_tb.genLdThis(nullptr);
}

void HhbcTranslator::VectorTranslator::emitBaseN() {
  PUNT(emitBaseN);
}

template <bool warn, bool define>
static inline TypedValue* baseGImpl(TypedValue *key,
                                    MInstrState* mis) {
  TypedValue* base;
  StringData* name = prepareKey(key);
  VarEnv* varEnv = g_vmContext->m_globalVarEnv;
  assert(varEnv != NULL);
  base = varEnv->lookup(name);
  if (base == NULL) {
    if (warn) {
      raise_notice(Strings::UNDEFINED_VARIABLE, name->data());
    }
    if (define) {
      TypedValue tv;
      tvWriteNull(&tv);
      varEnv->set(name, &tv);
      base = varEnv->lookup(name);
    } else {
      tvWriteNull(&mis->tvScratch);
      base = &mis->tvScratch;
    }
  }
  decRefStr(name);
  if (base->m_type == KindOfRef) {
    base = base->m_data.pref->tv();
  }
  return base;
}

static TypedValue* baseG(TypedValue key, MInstrState* mis) {
  return baseGImpl<false, false>(&key, mis);
}

static TypedValue* baseGW(TypedValue key, MInstrState* mis) {
  return baseGImpl<true, false>(&key, mis);
}

static TypedValue* baseGD(TypedValue key, MInstrState* mis) {
  return baseGImpl<false, true>(&key, mis);
}

static TypedValue* baseGWD(TypedValue key, MInstrState* mis) {
  return baseGImpl<true, true>(&key, mis);
}

void HhbcTranslator::VectorTranslator::emitBaseG() {
  const MInstrAttr& mia = m_mii.getAttr(m_ni.immVec.locationCode());
  typedef TypedValue* (*OpFunc)(TypedValue, MInstrState*);
  static const OpFunc opFuncs[] = {baseG, baseGW, baseGD, baseGWD};
  OpFunc opFunc = opFuncs[mia & MIA_base];
  SSATmp* gblName = getInput(m_iInd);
  m_ht.spillStack();
  m_base = m_tb.gen(BaseG,
                    m_tb.genDefConst((TCA)opFunc),
                    noLitInt(gblName),
                    genMisPtr());
}

void HhbcTranslator::VectorTranslator::emitBaseS() {
  const int kClassIdx = m_ni.inputs.size() - 1;
  SSATmp* key = getInput(m_iInd);
  SSATmp* clsRef = getInput(kClassIdx);
  m_base = m_tb.gen(LdClsPropAddr,
                    clsRef,
                    key,
                    CTX());
}

void HhbcTranslator::VectorTranslator::emitBaseOp() {
  LocationCode lCode = m_ni.immVec.locationCode();
  switch (lCode) {
  case LL: case LC: case LR: emitBaseLCR(); break;
  case LH:                   emitBaseH();   break;
  case LGL: case LGC:        emitBaseG();   break;
  case LNL: case LNC:        emitBaseN();   break;
  case LSL: case LSC:        emitBaseS();   break;
  default:                   not_reached();
  }
}

void HhbcTranslator::VectorTranslator::emitIntermediateOp() {
  switch (m_ni.immVecM[m_mInd]) {
    case MEC: case MEL: case MET: case MEI: {
      emitElem();
      ++m_iInd;
      break;
    }
    case MPC: case MPL: case MPT:
      emitProp();
      ++m_iInd;
      break;
    case MW:
      assert(m_mii.newElem());
      emitNewElem();
      break;
    default: not_reached();
  }
}


void HhbcTranslator::VectorTranslator::emitProp() {
  const Class* knownCls = nullptr;
  const int propOffset  = getPropertyOffset(m_ni, knownCls, m_mii,
                                            m_mInd, m_iInd);
  if (propOffset == -1) {
    emitPropGeneric();
  } else {
    emitPropSpecialized(m_mii.getAttr(m_ni.immVecM[m_mInd]), propOffset);
  }
}

template <KeyType keyType, bool unboxKey, MInstrAttr attrs, bool isObj>
static inline TypedValue* propImpl(Class* ctx, TypedValue* base,
                                   TypedValue keyVal, MInstrState* mis) {
  TypedValue* key = keyPtr<keyType>(keyVal);
  key = unbox<keyType, unboxKey>(key);
  return Prop<WDU(attrs), isObj, keyType>(
    mis->tvScratch, mis->tvRef, ctx, base, key);
}

#define HELPER_TABLE(m)                                              \
  /* name        key  unboxKey attrs        isObj */                 \
  m(propC,     AnyKey,  false, None,        false)                   \
  m(propCD,    AnyKey,  false, Define,      false)                   \
  m(propCDO,   AnyKey,  false, Define,       true)                   \
  m(propCO,    AnyKey,  false, None,         true)                   \
  m(propCU,    AnyKey,  false, Unset,       false)                   \
  m(propCUO,   AnyKey,  false, Unset,        true)                   \
  m(propCW,    AnyKey,  false, Warn,        false)                   \
  m(propCWD,   AnyKey,  false, WarnDefine,  false)                   \
  m(propCWDO,  AnyKey,  false, WarnDefine,   true)                   \
  m(propCWO,   AnyKey,  false, Warn,         true)                   \
  m(propL,     AnyKey,   true, None,        false)                   \
  m(propLD,    AnyKey,   true, Define,      false)                   \
  m(propLDO,   AnyKey,   true, Define,       true)                   \
  m(propLO,    AnyKey,   true, None,         true)                   \
  m(propLU,    AnyKey,   true, Unset,       false)                   \
  m(propLUO,   AnyKey,   true, Unset,        true)                   \
  m(propLW,    AnyKey,   true, Warn,        false)                   \
  m(propLWD,   AnyKey,   true, WarnDefine,  false)                   \
  m(propLWDO,  AnyKey,   true, WarnDefine,   true)                   \
  m(propLWO,   AnyKey,   true, Warn,         true)                   \
  m(propLS,    StrKey,   true, None,        false)                   \
  m(propLSD,   StrKey,   true, Define,      false)                   \
  m(propLSDO,  StrKey,   true, Define,       true)                   \
  m(propLSO,   StrKey,   true, None,         true)                   \
  m(propLSU,   StrKey,   true, Unset,       false)                   \
  m(propLSUO,  StrKey,   true, Unset,        true)                   \
  m(propLSW,   StrKey,   true, Warn,        false)                   \
  m(propLSWD,  StrKey,   true, WarnDefine,  false)                   \
  m(propLSWDO, StrKey,   true, WarnDefine,   true)                   \
  m(propLSWO,  StrKey,   true, Warn,         true)                   \
  m(propS,     StrKey,  false, None,        false)                   \
  m(propSD,    StrKey,  false, Define,      false)                   \
  m(propSDO,   StrKey,  false, Define,       true)                   \
  m(propSO,    StrKey,  false, None,         true)                   \
  m(propSU,    StrKey,  false, Unset,       false)                   \
  m(propSUO,   StrKey,  false, Unset,        true)                   \
  m(propSW,    StrKey,  false, Warn,        false)                   \
  m(propSWD,   StrKey,  false, WarnDefine,  false)                   \
  m(propSWDO,  StrKey,  false, WarnDefine,   true)                   \
  m(propSWO,   StrKey,  false, Warn,         true)

#define PROP(nm, ...)                                                   \
static TypedValue* nm(Class* ctx, TypedValue* base, TypedValue key,     \
                      MInstrState* mis) {                               \
  return propImpl<__VA_ARGS__>(ctx, base, key, mis);                    \
}
HELPER_TABLE(PROP)
#undef PROP

void HhbcTranslator::VectorTranslator::emitPropGeneric() {
  MemberCode mCode = m_ni.immVecM[m_mInd];
  MInstrAttr mia = MInstrAttr(m_mii.getAttr(mCode) & MIA_intermediate_prop);

  typedef TypedValue* (*OpFunc)(Class*, TypedValue*, TypedValue, MInstrState*);
  SSATmp* key = getInput(m_iInd);
  BUILD_OPTAB(getKeyTypeS(key), key->isBoxed(), mia, m_base->isA(Type::Obj));
  m_ht.spillStack();
  m_base = m_tb.gen(PropX, cns((TCA)opFunc), CTX(),
                    objOrPtr(m_base), noLitInt(key), genMisPtr());
}
#undef HELPER_TABLE

/*
 * Helper for emitPropSpecialized to check if a property is Uninit. It
 * returns a pointer to the property's address, or init_null_variant
 * if the property was Uninit and doDefine is false.
 */
SSATmp* HhbcTranslator::VectorTranslator::checkInitProp(
  SSATmp* baseAsObj, SSATmp* propAddr,
  int propOffset, bool doWarn, bool doDefine) {
  SSATmp* key = getInput(m_iInd);
  assert(baseAsObj->isA(Type::Obj));
  assert(propAddr->getType().isPtr());
  // The m_mInd check is to avoid initializing a property to
  // InitNull right before it's going to be set to something else.
  if (doWarn || (doDefine && m_mInd < m_ni.immVecM.size() - 1)) {
    return m_tb.cond(m_ht.getCurFunc(),
      [&] (Block* taken) {
        m_tb.gen(CheckInitMem, taken, propAddr, cns(0));
      },
      [&] { // Next: Property isn't Uninit. Do nothing.
        return propAddr;
      },
      [&] { // Taken: Property is Uninit. Raise a warning and return
            // a pointer to InitNull, either in the object or
            // init_null_variant.
        m_tb.hint(Block::Unlikely);
        if (doWarn) {
          m_tb.gen(RaiseUndefProp, baseAsObj, key);
        }
        if (doDefine) {
          m_tb.gen(StProp, baseAsObj, cns(propOffset), m_tb.genDefInitNull());
          return propAddr;
        }
        return cns((const TypedValue*)&init_null_variant);;
      }
    );
  }
  // No need to do the check
  return propAddr;
}

void HhbcTranslator::VectorTranslator::emitPropSpecialized(const MInstrAttr mia,
                                                           int propOffset) {
  assert(!(mia & MIA_warn) || !(mia & MIA_unset));
  const bool doWarn   = mia & MIA_warn;
  const bool doDefine = mia & MIA_define || mia & MIA_unset;

  SSATmp* initNull = cns((const TypedValue*)&init_null_variant);
  SSATmp* key = getInput(m_iInd);
  assert(key->isA(Type::StaticStr));

  /*
   * Type-inference from hphpc only tells us that this is either an object of a
   * given class type or null.  If it's not an object, it has to be a null type
   * based on type inference.  (It could be KindOfRef with an object inside,
   * except that this isn't inferred for object properties so we're fine not
   * checking KindOfRef in that case.)
   *
   * On the other hand, if m_base->isA(Type::Obj), we're operating on the base
   * which was already guarded by tracelet guards (and may have been KindOfRef,
   * but the Base* op already handled this). So we only need to do a type
   * check against null here in the intermediate cases.
   */
  if (m_base->isA(Type::Obj)) {
    SSATmp* propAddr = m_tb.genLdPropAddr(m_base, cns(propOffset));
    m_base = checkInitProp(m_base, propAddr, propOffset, doWarn, doDefine);
  } else {
    SSATmp* baseAsObj = nullptr;
    m_base = m_tb.cond(m_ht.getCurFunc(),
      [&] (Block* taken) {
        // baseAsObj is only available in the Next branch
        baseAsObj = m_tb.gen(LdMem, Type::Obj, taken, m_base, cns(0));
      },
      [&] { // Next: Base is an object. Load property address and
            // check for uninit
        return checkInitProp(baseAsObj,
                             m_tb.genLdPropAddr(baseAsObj, cns(propOffset)),
                             propOffset, doWarn, doDefine);
      },
      [&] { // Taken: Base is Null. Raise warnings/errors and return InitNull.
        m_tb.hint(Block::Unlikely);
        if (doWarn) {
          m_tb.gen(WarnNonObjProp);
        }
        if (doDefine) {
          /*
           * NOTE:
           *
           * This case logically is supposed to do a stdClass promotion.  It
           * should ideally not be possible (since we have a known class type),
           * except that the static compiler doesn't correctly infer object
           * class types in some edge cases involving stdClass promotion.
           *
           * This is impossible to handle "correctly" if we're in the middle of
           * a multi-dim property expression, because things further along may
           * also have type inference telling them that object properties are
           * at a given slot, but the object could actually be a stdClass
           * instead of the knownCls type if we were to promote here.
           *
           * So, we throw a fatal error, which is what hphpc's generated C++
           * would do in this case too.
           *
           * Relevant TODOs:
           *   #1789661 (this can cause bugs if bytecode.cpp promotes)
           *   #1124706 (we want to get rid of stdClass promotion in general)
           */
          m_tb.gen(ThrowNonObjProp);
        }
        return initNull;
      }
    );
  }

  // At this point m_base is either a pointer to init_null_variant or
  // a property in the object that we've verified isn't uninit.
  assert(m_base->getType().isPtr());
}

template <KeyType keyType, bool unboxKey, bool warn, bool define, bool reffy,
          bool unset>
static inline TypedValue* elemImpl(TypedValue* base, TypedValue keyVal,
                                   MInstrState* mis) {
  TypedValue* key = keyPtr<keyType>(keyVal);
  key = unbox<keyType, unboxKey>(key);
  if (unset) {
    return ElemU<keyType>(mis->tvScratch, mis->tvRef, base, key);
  } else if (define) {
    return ElemD<warn, reffy, keyType>(mis->tvScratch, mis->tvRef, base, key);
  } else {
    return Elem<warn, keyType>(mis->tvScratch, mis->tvRef, base,
                               mis->baseStrOff, key);
  }
}

#define HELPER_TABLE(m)                                               \
  /* name        hot        key     unboxKey  attrs  */               \
  m(elemC,     ,            AnyKey,  false,   None)                   \
  m(elemCD,    ,            AnyKey,  false,   Define)                 \
  m(elemCDR,   ,            AnyKey,  false,   DefineReffy)            \
  m(elemCU,    ,            AnyKey,  false,   Unset)                  \
  m(elemCW,    ,            AnyKey,  false,   Warn)                   \
  m(elemCWD,   ,            AnyKey,  false,   WarnDefine)             \
  m(elemCWDR,  ,            AnyKey,  false,   WarnDefineReffy)        \
  m(elemI,     ,            IntKey,  false,   None)                   \
  m(elemID,    HOT_FUNC_VM, IntKey,  false,   Define)                 \
  m(elemIDR,   ,            IntKey,  false,   DefineReffy)            \
  m(elemIU,    ,            IntKey,  false,   Unset)                  \
  m(elemIW,    ,            IntKey,  false,   Warn)                   \
  m(elemIWD,   ,            IntKey,  false,   WarnDefine)             \
  m(elemIWDR,  ,            IntKey,  false,   WarnDefineReffy)        \
  m(elemL,     ,            AnyKey,  true,    None)                   \
  m(elemLD,    ,            AnyKey,  true,    Define)                 \
  m(elemLDR,   ,            AnyKey,  true,    DefineReffy)            \
  m(elemLU,    ,            AnyKey,  true,    Unset)                  \
  m(elemLW,    ,            AnyKey,  true,    Warn)                   \
  m(elemLWD,   ,            AnyKey,  true,    WarnDefine)             \
  m(elemLWDR,  ,            AnyKey,  true,    WarnDefineReffy)        \
  m(elemLI,    ,            IntKey,  true,    None)                   \
  m(elemLID,   ,            IntKey,  true,    Define)                 \
  m(elemLIDR,  ,            IntKey,  true,    DefineReffy)            \
  m(elemLIU,   ,            IntKey,  true,    Unset)                  \
  m(elemLIW,   ,            IntKey,  true,    Warn)                   \
  m(elemLIWD,  ,            IntKey,  true,    WarnDefine)             \
  m(elemLIWDR, ,            IntKey,  true,    WarnDefineReffy)        \
  m(elemLS,    ,            StrKey,  true,    None)                   \
  m(elemLSD,   ,            StrKey,  true,    Define)                 \
  m(elemLSDR,  ,            StrKey,  true,    DefineReffy)            \
  m(elemLSU,   ,            StrKey,  true,    Unset)                  \
  m(elemLSW,   ,            StrKey,  true,    Warn)                   \
  m(elemLSWD,  ,            StrKey,  true,    WarnDefine)             \
  m(elemLSWDR, ,            StrKey,  true,    WarnDefineReffy)        \
  m(elemS,     HOT_FUNC_VM, StrKey,  false,   None)                   \
  m(elemSD,    HOT_FUNC_VM, StrKey,  false,   Define)                 \
  m(elemSDR,   ,            StrKey,  false,   DefineReffy)            \
  m(elemSU,    ,            StrKey,  false,   Unset)                  \
  m(elemSW,    HOT_FUNC_VM, StrKey,  false,   Warn)                   \
  m(elemSWD,   ,            StrKey,  false,   WarnDefine)             \
  m(elemSWDR,  ,            StrKey,  false,   WarnDefineReffy)

#define ELEM(nm, hot, keyType, unboxKey, attrs)                         \
hot                                                                     \
static TypedValue* nm(TypedValue* base, TypedValue key, MInstrState* mis) { \
  return elemImpl<keyType, unboxKey, WDRU(attrs)>(base, key, mis);      \
}
HELPER_TABLE(ELEM)
#undef ELEM

void HhbcTranslator::VectorTranslator::emitElem() {
  MemberCode mCode = m_ni.immVecM[m_mInd];
  MInstrAttr mia = MInstrAttr(m_mii.getAttr(mCode) & MIA_intermediate);
  SSATmp* key = getInput(m_iInd);

  typedef TypedValue* (*OpFunc)(TypedValue*, TypedValue, MInstrState*);
  BUILD_OPTAB_HOT(getKeyTypeIS(key), key->isBoxed(), mia);
  m_ht.spillStack();
  m_base = m_tb.gen(mia & Define ? ElemDX : ElemX,
                    cns((TCA)opFunc), ptr(m_base), key, genMisPtr());
}
#undef HELPER_TABLE

void HhbcTranslator::VectorTranslator::emitNewElem() {
  PUNT(emitNewElem);
}

void HhbcTranslator::VectorTranslator::emitRatchetRefs() {
  if (ratchetInd() < 0 || ratchetInd() >= int(nLogicalRatchets())) {
    return;
  }

  m_base = m_tb.cond(m_ht.getCurFunc(),
    [&] (Block* taken) {
      m_tb.gen(CheckInitMem, taken, m_misBase, cns(HHIR_MISOFF(tvRef)));
    },
    [&] { // Next: tvRef isn't Uninit. Ratchet the refs
      // Clean up tvRef2 before overwriting it.
      if (ratchetInd() > 0) {
        m_tb.genDecRefMem(m_misBase, HHIR_MISOFF(tvRef2), Type::Gen);
      }
      // Copy tvRef to tvRef2. Use mmx at some point
      SSATmp* tvRef = m_tb.genLdMem(m_misBase, HHIR_MISOFF(tvRef), Type::Gen,
                                    nullptr);
      m_tb.genStMem(m_misBase, HHIR_MISOFF(tvRef2), tvRef, true);

      // Reset tvRef.
      m_tb.genStMem(m_misBase, HHIR_MISOFF(tvRef), m_tb.genDefUninit(), true);

      // Adjust base pointer.
      assert(m_base->getType().isPtr());
      return m_tb.genLdAddr(m_misBase, HHIR_MISOFF(tvRef2));
    },
    [&] { // Taken: tvRef is Uninit. Do nothing.
      return m_base;
    }
  );
}

void HhbcTranslator::VectorTranslator::emitFinalMOp() {
  typedef void (HhbcTranslator::VectorTranslator::*MemFun)();

  switch (m_ni.immVecM[m_mInd]) {
  case MEC: case MEL: case MET: case MEI:
    static MemFun elemOps[] = {
#   define MII(instr, ...) &HhbcTranslator::VectorTranslator::emit##instr##Elem,
    MINSTRS
#   undef MII
    };
    (this->*elemOps[m_mii.instr()])();
    break;

  case MPC: case MPL: case MPT:
    static MemFun propOps[] = {
#   define MII(instr, ...) &HhbcTranslator::VectorTranslator::emit##instr##Prop,
    MINSTRS
#   undef MII
    };
    (this->*propOps[m_mii.instr()])();
    break;

  case MW:
    assert(m_mii.getAttr(MW) & MIA_final);
    static MemFun newOps[] = {
#   define MII(instr, attrs, bS, iS, vC, fN) \
      &HhbcTranslator::VectorTranslator::emit##fN,
    MINSTRS
#   undef MII
    };
    (this->*newOps[m_mii.instr()])();
    break;

  default: not_reached();
  }
}

template <KeyType keyType, bool unboxKey, bool isObj>
static inline TypedValue cGetPropImpl(Class* ctx, TypedValue* base,
                                      TypedValue keyVal, MInstrState* mis) {
  TypedValue result;
  TypedValue* key = keyPtr<keyType>(keyVal);
  key = unbox<keyType, unboxKey>(key);
  base = Prop<true, false, false, isObj, keyType>(
    result, mis->tvRef, ctx, base, key);
  if (base != &result) {
    // Grab a reference to the result.
    tvDup(base, &result);
  }
  if (result.m_type == KindOfRef) {
    tvUnbox(&result);
  }
  return result;
}

#define HELPER_TABLE(m)                                       \
  /* name           hot        key  unboxKey  isObj */        \
  m(cGetPropC,    ,            AnyKey, false, false)          \
  m(cGetPropCO,   ,            AnyKey, false,  true)          \
  m(cGetPropL,    ,            AnyKey,  true, false)          \
  m(cGetPropLO,   ,            AnyKey,  true,  true)          \
  m(cGetPropLS,   ,            StrKey,  true, false)          \
  m(cGetPropLSO,  ,            StrKey,  true,  true)          \
  m(cGetPropS,    ,            StrKey, false, false)          \
  m(cGetPropSO,   HOT_FUNC_VM, StrKey, false,  true)

#define PROP(nm, hot, ...)                                              \
hot                                                                     \
static TypedValue nm(Class* ctx, TypedValue* base, TypedValue key,      \
                     MInstrState* mis) {                                \
  return cGetPropImpl<__VA_ARGS__>(ctx, base, key, mis);                \
}
HELPER_TABLE(PROP)
#undef PROP

void HhbcTranslator::VectorTranslator::emitCGetProp() {
  assert(!m_ni.outLocal);

  const Class* knownCls = nullptr;
  const int propOffset  = getPropertyOffset(m_ni, knownCls,
                                            m_mii, m_mInd, m_iInd);
  if (propOffset != -1) {
    emitPropSpecialized(MIA_warn, propOffset);
    SSATmp* cellPtr = m_tb.genUnboxPtr(m_base);
    SSATmp* propVal = m_tb.gen(LdMem, Type::Cell, cellPtr, cns(0));
    m_result = m_tb.genIncRef(propVal);
    return;
  }

  typedef TypedValue (*OpFunc)(Class*, TypedValue*, TypedValue, MInstrState*);
  SSATmp* key = getInput(m_iInd);
  BUILD_OPTAB_HOT(getKeyTypeS(key), key->isBoxed(), m_base->isA(Type::Obj));
  m_ht.spillStack();
  m_result = m_tb.gen(CGetProp, cns((TCA)opFunc), CTX(),
                      objOrPtr(m_base), noLitInt(key), genMisPtr());
}
#undef HELPER_TABLE

void HhbcTranslator::VectorTranslator::emitVGetProp() {
  SPUNT(__func__);
}

void HhbcTranslator::VectorTranslator::emitIssetProp() {
  SPUNT(__func__);
}

void HhbcTranslator::VectorTranslator::emitEmptyProp() {
  SPUNT(__func__);
}

template <KeyType keyType, bool unboxKey, bool isObj>
static inline TypedValue setPropImpl(Class* ctx, TypedValue* base,
                                     TypedValue keyVal, Cell val) {
  TypedValue* key = keyPtr<keyType>(keyVal);
  key = unbox<keyType, unboxKey>(key);
  HPHP::VM::SetProp<true, isObj, keyType>(ctx, base, key, &val);
  return val;
}

#define HELPER_TABLE(m)                                       \
  /* name         key   unboxKey isObj */           \
  m(setPropC,    AnyKey,  false, false)             \
  m(setPropCO,   AnyKey,  false,  true)             \
  m(setPropL,    AnyKey,   true, false)             \
  m(setPropLO,   AnyKey,   true,  true)             \
  m(setPropLS,   StrKey,   true, false)             \
  m(setPropLSO,  StrKey,   true,  true)             \
  m(setPropS,    StrKey,  false, false)             \
  m(setPropSO,   StrKey,  false,  true)

#define PROP(nm, ...)                                                   \
static TypedValue nm(Class* ctx, TypedValue* base, TypedValue key, Cell val) { \
  return setPropImpl<__VA_ARGS__>(ctx, base, key, val);                 \
}
HELPER_TABLE(PROP)
#undef PROP

void HhbcTranslator::VectorTranslator::emitSetProp() {
  const int kValIdx = 0;
  SSATmp* value = getInput(kValIdx);

  /* If we know the class for the current base, emit a direct property set. */
  const Class* knownCls = nullptr;
  const int propOffset  = getPropertyOffset(m_ni, knownCls,
                                            m_mii, m_mInd, m_iInd);
  if (propOffset != -1) {
    emitPropSpecialized(MIA_define, propOffset);
    SSATmp* cellPtr = m_tb.genUnboxPtr(m_base);
    SSATmp* oldVal = m_tb.gen(LdMem, Type::Cell, cellPtr, cns(0));
    // The object owns a reference now
    SSATmp* increffed = m_tb.genIncRef(value);
    m_tb.gen(StMem, cellPtr, cns(0), value);
    m_tb.genDecRef(oldVal);
    m_result = increffed;
    return;
  }

  // Emit the appropriate helper call.
  typedef TypedValue (*OpFunc)(Class*, TypedValue*, TypedValue, Cell);
  SSATmp* key = getInput(m_iInd);
  BUILD_OPTAB(getKeyTypeS(key), key->isBoxed(), m_base->isA(Type::Obj));
  m_ht.spillStack();
  SSATmp* result =
    m_tb.gen(SetProp, cns((TCA)opFunc), CTX(),
             objOrPtr(m_base), noLitInt(key), value);
  VectorEffects ve(result->getInstruction());
  m_result = ve.valTypeChanged ? result : value;
}
#undef HELPER_TABLE

void HhbcTranslator::VectorTranslator::emitSetOpProp() {
  SPUNT(__func__);
}

void HhbcTranslator::VectorTranslator::emitIncDecProp() {
  SPUNT(__func__);
}

void HhbcTranslator::VectorTranslator::emitBindProp() {
  SPUNT(__func__);
}

void HhbcTranslator::VectorTranslator::emitUnsetProp() {
  SPUNT(__func__);
}

template <KeyType keyType, bool unboxKey>
static inline TypedValue cGetElemImpl(TypedValue* base, TypedValue keyVal,
                                      MInstrState* mis) {
  TypedValue result;
  TypedValue* key = keyPtr<keyType>(keyVal);
  key = unbox<keyType, unboxKey>(key);
  base = Elem<true, keyType>(result, mis->tvRef, base, mis->baseStrOff, key);
  if (base != &result) {
    // Save a copy of the result.
    tvDup(base, &result);
  }
  if (result.m_type == KindOfRef) {
    tvUnbox(&result);
  }
  return result;
}

#define HELPER_TABLE(m)                                          \
  /* name         hot         key   unboxKey */                  \
  m(cGetElemC,  ,            AnyKey,   false)                    \
  m(cGetElemI,  ,            IntKey,   false)                    \
  m(cGetElemL,  HOT_FUNC_VM, AnyKey,    true)                    \
  m(cGetElemLI, HOT_FUNC_VM, IntKey,    true)                    \
  m(cGetElemLS, ,            StrKey,    true)                    \
  m(cGetElemS,  HOT_FUNC_VM, StrKey,   false)

#define ELEM(nm, hot, ...)                                              \
hot                                                                     \
static TypedValue nm(TypedValue* base, TypedValue key, MInstrState* mis) { \
  return cGetElemImpl<__VA_ARGS__>(base, key, mis);                     \
}
HELPER_TABLE(ELEM)
#undef ELEM

void HhbcTranslator::VectorTranslator::emitCGetElem() {
  typedef TypedValue (*OpFunc)(TypedValue*, TypedValue, MInstrState*);
  SSATmp* key = getInput(m_iInd);
  BUILD_OPTAB_HOT(getKeyTypeIS(key), key->isBoxed());
  m_ht.spillStack();
  m_result = m_tb.gen(CGetElem, cns((TCA)opFunc),
                      ptr(m_base), key, genMisPtr());
}
#undef HELPER_TABLE

void HhbcTranslator::VectorTranslator::emitVGetElem() {
  SPUNT(__func__);
}

template <KeyType keyType, bool unboxKey, bool isEmpty>
static inline bool issetEmptyElemImpl(TypedValue* base, TypedValue keyVal,
                                      MInstrState* mis) {
  TypedValue* key = keyPtr<keyType>(keyVal);
  key = unbox<keyType, unboxKey>(key);
  return HPHP::VM::IssetEmptyElem<isEmpty, false, keyType>(
    mis->tvScratch, mis->tvRef, base, mis->baseStrOff, key);
}

#define HELPER_TABLE(m)                                      \
  /* name          hot        keyType unboxKey isEmpty */    \
  m(issetElemC,   ,            AnyKey, false,  false)        \
  m(issetElemCE,  ,            AnyKey, false,   true)        \
  m(issetElemI,   HOT_FUNC_VM, IntKey, false,  false)        \
  m(issetElemIE,  ,            IntKey, false,   true)        \
  m(issetElemL,   ,            AnyKey,  true,  false)        \
  m(issetElemLE,  ,            AnyKey,  true,   true)        \
  m(issetElemLI,  ,            IntKey,  true,  false)        \
  m(issetElemLIE, ,            IntKey,  true,   true)        \
  m(issetElemLS,  ,            StrKey,  true,  false)        \
  m(issetElemLSE, ,            StrKey,  true,   true)        \
  m(issetElemS,   HOT_FUNC_VM, StrKey, false,  false)        \
  m(issetElemSE,  HOT_FUNC_VM, StrKey, false,   true)

#define ISSET(nm, hot, ...)                                             \
hot                                                                     \
static uint64_t nm(TypedValue* base, TypedValue key, MInstrState* mis) { \
  return issetEmptyElemImpl<__VA_ARGS__>(base, key, mis);               \
}
HELPER_TABLE(ISSET)
#undef ISSET

void HhbcTranslator::VectorTranslator::emitIssetEmptyElem(bool isEmpty) {
  SSATmp* key = getInput(m_iInd);

  typedef uint64_t (*OpFunc)(TypedValue*, TypedValue, MInstrState*);
  BUILD_OPTAB_HOT(getKeyTypeIS(key), key->isBoxed(), isEmpty);
  m_ht.spillStack();
  m_result = m_tb.gen(isEmpty ? EmptyElem : IssetElem,
                      cns((TCA)opFunc), ptr(m_base), key, genMisPtr());
}
#undef HELPER_TABLE

void HhbcTranslator::VectorTranslator::emitIssetElem() {
  emitIssetEmptyElem(false);
}

void HhbcTranslator::VectorTranslator::emitEmptyElem() {
  emitIssetEmptyElem(true);
}

template <KeyType keyType, bool unboxKey>
static inline TypedValue setElemImpl(TypedValue* base, TypedValue keyVal,
                                     Cell val) {
  TypedValue* key = keyPtr<keyType>(keyVal);
  key = unbox<keyType, unboxKey>(key);
  HPHP::VM::SetElem<true, keyType>(base, key, &val);
  return val;
}

#define HELPER_TABLE(m)                                        \
  /* name         hot        key   unboxKey */                 \
  m(setElemC,   ,            AnyKey, false)                    \
  m(setElemI,   ,            IntKey, false)                    \
  m(setElemL,   ,            AnyKey,  true)                    \
  m(setElemLI,  ,            IntKey,  true)                    \
  m(setElemLS,  ,            StrKey,  true)                    \
  m(setElemS,   HOT_FUNC_VM, StrKey, false)

#define ELEM(nm, hot, ...)                                              \
hot                                                                     \
static TypedValue nm(TypedValue* base, TypedValue key, Cell val) {      \
  return setElemImpl<__VA_ARGS__>(base, key, val);                      \
}
HELPER_TABLE(ELEM)
#undef ELEM

void HhbcTranslator::VectorTranslator::emitSetElem() {
  const int kValIdx = 0;
  SSATmp* value = getInput(kValIdx);

  // Emit the appropriate helper call.
  typedef TypedValue (*OpFunc)(TypedValue*, TypedValue, Cell);
  SSATmp* key = getInput(m_iInd);
  BUILD_OPTAB_HOT(getKeyTypeIS(key), key->isBoxed());
  m_ht.spillStack();
  SSATmp* result = m_tb.gen(SetElem, cns((TCA)opFunc), ptr(m_base), key, value);
  VectorEffects ve(result->getInstruction());
  m_result = ve.valTypeChanged ? result : value;
}
#undef HELPER_TABLE

void HhbcTranslator::VectorTranslator::emitSetOpElem() {
  SPUNT(__func__);
}

void HhbcTranslator::VectorTranslator::emitIncDecElem() {
  SPUNT(__func__);
}

void HhbcTranslator::VectorTranslator::emitBindElem() {
  SPUNT(__func__);
}

void HhbcTranslator::VectorTranslator::emitUnsetElem() {
  SPUNT(__func__);
}

void HhbcTranslator::VectorTranslator::emitNotSuppNewElem() {
  not_reached();
}

void HhbcTranslator::VectorTranslator::emitVGetNewElem() {
  SPUNT(__func__);
}

void HhbcTranslator::VectorTranslator::emitSetNewElem() {
  const int kValIdx = 0;
  SSATmp* value = getInput(kValIdx);

  m_ht.spillStack();
  SSATmp* result = m_tb.gen(SetNewElem, ptr(m_base), value);
  VectorEffects ve(result->getInstruction());
  m_result = ve.valTypeChanged ? result : value;
}

void HhbcTranslator::VectorTranslator::emitSetOpNewElem() {
  SPUNT(__func__);
}

void HhbcTranslator::VectorTranslator::emitIncDecNewElem() {
  SPUNT(__func__);
}

void HhbcTranslator::VectorTranslator::emitBindNewElem() {
  SPUNT(__func__);
}

void HhbcTranslator::VectorTranslator::emitMPost() {
  // Decref stack inputs. If we have an rhs (valCount() == 1), don't
  // decref it since it's also our output. There are cases where the
  // rhs stack cell gets clobbered to be null, but the helper that
  // does that decrefs the existing value so we still don't have to do
  // anything here.
  unsigned nStack = m_mii.valCount();
  for (unsigned i = m_mii.valCount(); i < m_ni.inputs.size(); ++i) {
    const DynLocation& input = *m_ni.inputs[i];
    switch (input.location.space) {
    case Location::Stack: {
      ++nStack;
      m_tb.genDecRef(getInput(i));
      break;
    }
    case Location::Local:
    case Location::Litstr:
    case Location::Litint:
    case Location::This: {
      // Do nothing.
      break;
    }

    default: not_reached();
    }
  }

  // Pop off all stack inputs
  m_ht.discard(nStack);

  // Push result
  assert(m_result);
  m_ht.push(m_result);

  // Clean up tvRef(2)
  if (nLogicalRatchets() > 1) {
    m_tb.genDecRefMem(m_misBase, HHIR_MISOFF(tvRef2), Type::Gen);
  }
  if (nLogicalRatchets() > 0) {
    m_tb.genDecRefMem(m_misBase, HHIR_MISOFF(tvRef), Type::Gen);
  }
}

bool HhbcTranslator::VectorTranslator::needFirstRatchet() const {
  if (m_ni.inputs[m_mii.valCount()]->valueType() == KindOfArray) {
    switch (m_ni.immVecM[0]) {
    case MEC: case MEL: case MET: case MEI: return false;
    case MPC: case MPL: case MPT: case MW:  return true;
    default: not_reached();
    }
  }
  return true;
}

bool HhbcTranslator::VectorTranslator::needFinalRatchet() const {
  return m_mii.finalGet();
}

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
//   (base is array)      (base is object)   (base is object)
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
//   (base is array)
//   BaseL
//   ElemL
//     no ratchet
//   ElemL
//     ratchet
//   ElemL
//     logical ratchet
//   SetElemL
//     no ratchet
unsigned HhbcTranslator::VectorTranslator::nLogicalRatchets() const {
  // If we've proven elsewhere that we don't need an MInstrState struct, we
  // know this translation won't need any ratchets
  if (!m_needMIS) return 0;

  unsigned ratchets = m_ni.immVecM.size();
  if (!needFirstRatchet()) --ratchets;
  if (!needFinalRatchet()) --ratchets;
  return ratchets;
}

int HhbcTranslator::VectorTranslator::ratchetInd() const {
  return needFirstRatchet() ? int(m_mInd) : int(m_mInd) - 1;
}

} } }
