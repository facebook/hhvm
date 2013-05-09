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

#include "runtime/base/strings.h"
#include "runtime/vm/member_operations.h"
#include "runtime/vm/translator/hopt/ir.h"
#include "runtime/vm/translator/hopt/hhbctranslator.h"
#include "runtime/vm/translator/translator-x64.h"

// These files do ugly things with macros so include them last
#include "util/assert_throw.h"
#include "runtime/vm/translator/hopt/vectortranslator-internal.h"

namespace HPHP { namespace VM { namespace JIT {

TRACE_SET_MOD(hhir);

using namespace HPHP::VM::Transl;

static bool wantPropSpecializedWarnings() {
  return !RuntimeOption::RepoAuthoritative ||
    !RuntimeOption::EvalDisableSomeRepoAuthNotices;
}

bool VectorEffects::supported(Opcode op) {
  return opcodeHasFlags(op, VectorProp | VectorElem);
}

void VectorEffects::get(const IRInstruction* inst,
                        StoreLocFunc storeLocalValue,
                        SetLocTypeFunc setLocalType) {
  // If the base for this instruction is a local address, the
  // helper call might have side effects on the local's value
  SSATmp* base = inst->getSrc(vectorBaseIdx(inst));
  IRInstruction* locInstr = base->inst();
  if (locInstr->op() == LdLocAddr) {
    UNUSED Type baseType = locInstr->getDst()->type();
    assert(baseType.equals(base->type()));
    assert(baseType.isPtr() || baseType.isKnownDataType());
    int loc = locInstr->getExtra<LdLocAddr>()->locId;

    VectorEffects ve(inst);
    if (ve.baseTypeChanged || ve.baseValChanged) {
      storeLocalValue(loc, nullptr);
      setLocalType(loc, ve.baseType.derefIfPtr());
    }
  }
}

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

Opcode canonicalOp(Opcode op) {
  if (op == ElemUX || op == ElemUXStk ||
      op == UnsetElem || op == UnsetElemStk) {
    return UnsetElem;
  }
  return opcodeHasFlags(op, VectorProp) ? SetProp
       : opcodeHasFlags(op, VectorElem) || op == ArraySet ? SetElem
       : bad_value<Opcode>();
}
}

void VectorEffects::init(Opcode op, const Type origBase,
                         const Type key, const Type origVal) {
  baseType = origBase;
  bool basePtr, baseBoxed;
  stripBase(baseType, basePtr, baseBoxed);
  // Only certain types of bases are supported now, but this list may expand in
  // the future.
  assert_not_implemented(basePtr ||
                         baseType.subtypeOfAny(Type::Obj, Type::Arr));

  valType = origVal;
  baseTypeChanged = baseValChanged = valTypeChanged = false;

  // Canonicalize the op to SetProp or SetElem
  op = canonicalOp(op);

  // We're not expecting types other than specific known data types
  // (or for keys, Cell).  (At least for keys it might work since the
  // helpers generally operate on cells, but we're asserting anyway
  // since this shouldn't actually happen.)
  assert(key.equals(Type::None) || key.isKnownDataType() ||
         key.equals(Type::Cell));
  assert(origVal.equals(Type::None) || origVal.isKnownDataType());

  if ((op == SetElem || op == SetProp) &&
      baseType.maybe(Type::Null | Type::Bool | Type::Str)) {
    // stdClass or array promotion might happen
    auto newBase = op == SetElem ? Type::Arr : Type::Obj;
    // If the base is known to be null, promotion will happen. If we can ever
    // prove that the base is false or the empty string, promotion will
    // definitely happen but those cases aren't handled yet. In a perfect world
    // we would remove Type::Null from baseType here but that can produce types
    // that are tricky to guard against and doesn't buy us much right now.
    if (!baseBoxed || !baseType.isString()) {
      /*
       * Uses of boxed types are always guarded, in case the inner
       * type was modified. If the base type was String, its extremely
       * likely to still be a String, so leave it as such, and we'll
       * exit in the rare case that it changed.
       */
      baseType = baseType.isNull() ? newBase : (baseType | newBase);
    }
    baseValChanged = !baseBoxed;
  }
  if ((op == SetElem || op == UnsetElem) && baseType.maybe(Type::Arr)) {
    // possible COW when modifying an array, unless the base was boxed. If the
    // base is a box then the value of the box itself won't change, just what
    // it points to.
    baseValChanged = !baseBoxed;
  }
  if ((op == SetElem || op == UnsetElem) && baseType.maybe(Type::StaticArr)) {
    // the base might get promoted to a CountedArr. We can ignore the change if
    // the base is boxed, (for the same reasons as above).
    baseType = baseType | Type::CountedArr;
    baseValChanged = !baseBoxed;
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

  if (op == SetElem && baseType.maybe(Type::Str)) {
    // If the base is a String, the result will be different from the value
    // use valTypeChanged (even though the type may not have)
    valTypeChanged = true;
  }

  // The final baseType should be a pointer/box iff the input was
  baseType = baseBoxed ? baseType.box() : baseType;
  baseType = basePtr   ? baseType.ptr() : baseType;

  baseTypeChanged = baseTypeChanged || baseType != origBase;
  baseValChanged = baseValChanged || baseTypeChanged;
  valTypeChanged = valTypeChanged || valType != origVal;
}

// vectorBaseIdx returns the src index for inst's base operand.
int vectorBaseIdx(Opcode opc) {
  return opc == SetNewElem || opc == SetNewElemStk ? 0
         : opc == BindNewElem || opc == BindNewElemStk ? 0
         : opc == ArraySet ? 1
         : opc == SetOpProp || opc == SetOpPropStk ? 1
         : opcodeHasFlags(opc, VectorProp) ? 2
         : opcodeHasFlags(opc, VectorElem) ? 1
         : bad_value<int>();
}

// vectorKeyIdx returns the src index for inst's key operand.
int vectorKeyIdx(Opcode opc) {
  return opc == SetNewElem || opc == SetNewElemStk ? -1
         : opc == BindNewElem || opc == BindNewElem ? -1
         : opc == ArraySet ? 2
         : opc == SetOpProp || opc == SetOpPropStk ? 2
         : opcodeHasFlags(opc, VectorProp) ? 3
         : opcodeHasFlags(opc, VectorElem) ? 2
         : bad_value<int>();
}

// vectorValIdx returns the src index for inst's value operand.
int vectorValIdx(Opcode opc) {
  switch (opc) {
    case VGetProp: case VGetPropStk:
    case IncDecProp: case IncDecPropStk:
    case PropDX: case PropDXStk:
    case VGetElem: case VGetElemStk:
    case UnsetElem: case UnsetElemStk:
    case IncDecElem: case IncDecElemStk:
    case ElemDX: case ElemDXStk:
    case ElemUX: case ElemUXStk:
      return -1;

    case ArraySet: return 3;
    case SetNewElem: case SetNewElemStk: return 1;
    case BindNewElem: case BindNewElemStk: return 1;
    case SetOpProp: case SetOpPropStk: return 3;

    default:
      return opcodeHasFlags(opc, VectorProp) ? 4
           : opcodeHasFlags(opc, VectorElem) ? 3
           : bad_value<int>();
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

/* Copy varargs SSATmp*s into a vector */
template<typename... Srcs>
static void getSrcs(std::vector<SSATmp*>& srcVec, SSATmp* src, Srcs... srcs) {
  srcVec.push_back(src);
  getSrcs(srcVec, srcs...);
}
static void getSrcs(std::vector<SSATmp*>& srcVec) {}

template<typename... Srcs>
SSATmp* HhbcTranslator::VectorTranslator::genStk(Opcode opc, Srcs... srcs) {
  assert(opcodeHasFlags(opc, HasStackVersion));
  assert(!opcodeHasFlags(opc, ModifiesStack));
  std::vector<SSATmp*> srcVec;
  getSrcs(srcVec, srcs...);
  SSATmp* base = srcVec[vectorBaseIdx(opc)];

  /* If the base is a pointer to a stack cell and the operation might change
   * its type and/or value, use the version of the opcode that returns a new
   * StkPtr. */
  if (base->inst()->op() == LdStackAddr) {
    VectorEffects ve(opc, srcVec);
    if (ve.baseTypeChanged || ve.baseValChanged) {
      opc = getStackModifyingOpcode(opc);
    }
  }

  return gen(opc, srcs...);
}

void HhbcTranslator::VectorTranslator::emit() {
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
    return gen(LdAddr, m_misBase, cns(kReservedRSPSpillSpace));
  } else {
    return gen(DefConst, Type::PtrToCell, ConstData(nullptr));
  }
}

// Inspect the instruction we're about to translate and determine if
// it can be executed without using an MInstrState struct.
void HhbcTranslator::VectorTranslator::checkMIState() {
  auto const& baseRtt = m_ni.inputs[m_mii.valCount()]->rtt;
  Type baseType = Type::fromRuntimeType(baseRtt);
  const bool isCGetM = m_ni.mInstrOp() == OpCGetM;
  const bool isSetM = m_ni.mInstrOp() == OpSetM;
  const bool isIssetM = m_ni.mInstrOp() == OpIssetM;
  const bool isUnsetM = m_ni.mInstrOp() == OpUnsetM;
  const bool isSingle = m_ni.immVecM.size() == 1;

  assert(baseType.isBoxed() || baseType.notBoxed());
  baseType = baseType.unbox();

  // CGetM or SetM with no unknown property offsets
  const bool simpleProp = !mInstrHasUnknownOffsets(m_ni, contextClass()) &&
    (isCGetM || isSetM);

  // SetM with only one element
  const bool singlePropSet = isSingle && isSetM &&
    mcodeMaybePropName(m_ni.immVecM[0]);

  // Array access with one element in the vector
  const bool singleElem = isSingle && mcodeMaybeArrayKey(m_ni.immVecM[0]);

  // SetM with one vector array element
  const bool simpleArraySet = isSetM && singleElem;

  // IssetM with one vector array element and an Arr base
  const bool simpleArrayIsset = isIssetM && singleElem &&
    baseType.subtypeOf(Type::Arr);

  // UnsetM on an array with one vector element
  const bool simpleArrayUnset = isUnsetM && singleElem;

  // UnsetM on a non-standard base. Always a noop or fatal.
  const bool badUnset = isUnsetM && baseType.not(Type::Arr | Type::Obj);

  // CGetM on an array with a base that won't use MInstrState. Str
  // will use tvScratch and Obj will fatal or use tvRef.
  const bool simpleArrayGet = isCGetM && singleElem &&
    baseType.not(Type::Str | Type::Obj);

  if (simpleProp || singlePropSet ||
      simpleArraySet || simpleArrayGet ||
      simpleArrayUnset || badUnset ||
      simpleArrayIsset) {
    setNoMIState();
  }

  /*
   * If we're planning on RaiseUndefProp generation
   * (wantPropSpecializedWarnings) then pretty much in any case the
   * vector translator will do operations that can throw.
   *
   * Currently this means we have to have a ExceptionBarrier so the
   * unwinder can handle it (TODO(#2162354): eventually we'll hook
   * this into an unwind codepath).
   *
   * We also handle one special case where we know a spillStack won't
   * be needed: in a simple CGetM of a single property where hphpc has
   * told us it can't be KindOfUninit, even if
   * wantPropSpecializedWarnings we can't throw.
   */
  if (wantPropSpecializedWarnings()) {
    if (isCGetM && isSingle && simpleProp) {
      auto info = getFinalPropertyOffset(m_ni, contextClass(), m_mii);
      assert(info.offset != -1);
      if (info.hphpcType == KindOfInvalid) {
        m_ht.exceptionBarrier();
      }
    } else {
      m_ht.exceptionBarrier();
    }
  }
}

void HhbcTranslator::VectorTranslator::emitMPre() {
  checkMIState();

  if (HPHP::Trace::moduleEnabled(HPHP::Trace::minstr, 1)) {
    emitMTrace();
  }

  if (m_needMIS) {
    m_misBase = gen(DefMIStateBase);
    SSATmp* uninit = m_tb.genDefUninit();

    if (nLogicalRatchets() > 0) {
      gen(StMem, m_misBase, cns(HHIR_MISOFF(tvRef)), uninit);
      gen(StMem, m_misBase, cns(HHIR_MISOFF(tvRef2)), uninit);
    }
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

void HhbcTranslator::VectorTranslator::emitMTrace() {
  auto rttStr = [this](int i) {
    return Type::fromRuntimeType(m_ni.inputs[i]->rtt).unbox().toString();
  };
  std::ostringstream shape;
  int iInd = m_mii.valCount();
  const char* separator = "";

  shape << opcodeToName(m_ni.mInstrOp()) << " <";
  auto baseLoc = m_ni.immVec.locationCode();
  shape << folly::format("{}:{} ", locationCodeString(baseLoc), rttStr(iInd));
  ++iInd;

  for (int mInd = 0; mInd < m_ni.immVecM.size(); ++mInd) {
    auto mcode = m_ni.immVecM[mInd];
    shape << separator;
    if (mcode == MW) {
      shape << "MW";
    } else if (mcodeMaybeArrayKey(mcode)) {
      shape << "ME:" << rttStr(iInd);
    } else if (mcodeMaybePropName(mcode)) {
      shape << "MP:" << rttStr(iInd);
    } else {
      not_reached();
    }
    if (mcode != MW) ++iInd;
    separator = " ";
  }
  shape << '>';
  gen(IncStatGrouped,
           cns(StringData::GetStaticString("vector instructions")),
           cns(StringData::GetStaticString(shape.str())),
           cns(1));
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

SSATmp* HhbcTranslator::VectorTranslator::getBase() {
  assert(m_iInd == m_mii.valCount());
  return getInput(m_iInd);
}

SSATmp* HhbcTranslator::VectorTranslator::getKey() {
  SSATmp* key = getInput(m_iInd);
  auto keyType = key->type();
  assert(keyType.isBoxed() || keyType.notBoxed());
  if (keyType.isBoxed()) {
    key = gen(LdRef, Type::Cell, key);
  }
  return key;
}

SSATmp* HhbcTranslator::VectorTranslator::getValue() {
  // If an instruction takes an rhs, it's always input 0.
  assert(m_mii.valCount() == 1);
  const int kValIdx = 0;
  return getInput(kValIdx);
}

SSATmp* HhbcTranslator::VectorTranslator::getInput(unsigned i) {
  const DynLocation& dl = *m_ni.inputs[i];
  const Location& l = dl.location;

  assert(mapContains(m_stackInputs, i) == (l.space == Location::Stack));
  switch (l.space) {
    case Location::Stack: {
      SSATmp* val = m_ht.top(Type::Gen | Type::Cls, m_stackInputs[i]);
      // Check if the type on our eval stack is at least as specific as what
      // Transl::Translator came up with. We allow boxed types with differing
      // inner types because of the different ways the two systems deal with
      // reference types.
      auto t = Type::fromRuntimeType(dl.rtt);
      if (!val->isA(t) && !(val->isBoxed() && t.isBoxed())) {
        FTRACE(1, "{}: hhir stack has a {} where Translator had a {}\n",
               __func__, val->type().toString(), t.toString());
        // They'd better not be completely unrelated types...
        assert(t.subtypeOf(val->type()));
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
      return gen(LdThis, m_tb.getFp());

    default: not_reached();
  }
}

void HhbcTranslator::VectorTranslator::emitBaseLCR() {
  const MInstrAttr& mia = m_mii.getAttr(m_ni.immVec.locationCode());
  const DynLocation& base = *m_ni.inputs[m_iInd];
  auto baseType = Type::fromRuntimeType(base.rtt);
  assert(baseType.isKnownDataType());

  if (base.location.isLocal()) {
    // Check for Uninit and warn/promote to InitNull as appropriate
    if (baseType.subtypeOf(Type::Uninit)) {
      if (mia & MIA_warn) {
        m_ht.exceptionBarrier();
        gen(RaiseUninitLoc, LocalId(base.location.offset));
      }
      if (mia & MIA_define) {
        m_tb.genStLoc(base.location.offset, m_tb.genDefInitNull(),
                      false, true, nullptr);
        baseType = Type::InitNull;
      }
    }
  }

  // If the base is a box with a type that's changed, we need to bail out of
  // the tracelet and retranslate. Doing an exit here is a little sketchy since
  // we may have already emitted instructions with memory effects to initialize
  // the MInstrState. These particular stores are harmless though, and the
  // worst outcome here is that we'll end up doing the stores twice, once for
  // this instruction and once at the beginning of the retranslation.
  Trace* failedRef = baseType.isBoxed() ? m_ht.getExitTrace() : nullptr;
  if ((baseType.subtypeOfAny(Type::Obj, Type::BoxedObj) &&
       mcodeMaybePropName(m_ni.immVecM[0])) ||
      isSimpleArrayOp()) {
    // In these cases we can pass the base by value, after unboxing if needed.
    m_base = gen(Unbox, failedRef, getBase());
    assert(m_base->isA(baseType.unbox()));
  } else {
    // Everything else is passed by reference. We don't have to worry about
    // unboxing here, since all the generic helpers understand boxed bases.
    if (baseType.isBoxed()) {
      SSATmp* box = getBase();
      assert(box->isA(Type::BoxedCell));
      // Guard that the inner type hasn't changed
      gen(LdRef, baseType.innerType(), failedRef, box);
    }

    if (base.location.space == Location::Local) {
      m_base = m_tb.genLdLocAddr(base.location.offset);
    } else {
      assert(base.location.space == Location::Stack);
      m_ht.spillStack();
      assert(m_stackInputs.count(m_iInd));
      m_base = m_ht.loadStackAddr(m_stackInputs[m_iInd]);
    }
    assert(m_base->type().isPtr());
  }
}

// Is the current instruction a 1-element vector, with an Arr base and Int or
// Str key?
bool HhbcTranslator::VectorTranslator::isSimpleArrayOp() {
  SSATmp* base = getInput(m_mii.valCount());
  VM::Op op = m_ni.mInstrOp();
  if ((op == OpSetM || op == OpCGetM || op == OpIssetM) &&
      isSimpleBase() &&
      isSingleMember() &&
      mcodeMaybeArrayKey(m_ni.immVecM[0]) &&
      base->type().subtypeOfAny(Type::Arr, Type::BoxedArr)) {
    SSATmp* key = getInput(m_mii.valCount() + 1);
    return key->isA(Type::Int) || key->isA(Type::Str);
  }
  return false;
}

// "Simple" bases are stack cells and locals.
bool HhbcTranslator::VectorTranslator::isSimpleBase() {
  LocationCode loc = m_ni.immVec.locationCode();
  return loc == LL || loc == LC || loc == LR;
}

bool HhbcTranslator::VectorTranslator::isSingleMember() {
  return m_ni.immVecM.size() == 1;
}

void HhbcTranslator::VectorTranslator::emitBaseH() {
  m_base = gen(LdThis, m_tb.getFp());
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
      return const_cast<TypedValue*>(init_null_variant.asTypedValue());
    }
  }
  decRefStr(name);
  if (base->m_type == KindOfRef) {
    base = base->m_data.pref->tv();
  }
  return base;
}

namespace VectorHelpers {
TypedValue* baseG(TypedValue key, MInstrState* mis) {
  return baseGImpl<false, false>(&key, mis);
}

TypedValue* baseGW(TypedValue key, MInstrState* mis) {
  return baseGImpl<true, false>(&key, mis);
}

TypedValue* baseGD(TypedValue key, MInstrState* mis) {
  return baseGImpl<false, true>(&key, mis);
}

TypedValue* baseGWD(TypedValue key, MInstrState* mis) {
  return baseGImpl<true, true>(&key, mis);
}
}

void HhbcTranslator::VectorTranslator::emitBaseG() {
  const MInstrAttr& mia = m_mii.getAttr(m_ni.immVec.locationCode());
  typedef TypedValue* (*OpFunc)(TypedValue, MInstrState*);
  using namespace VectorHelpers;
  static const OpFunc opFuncs[] = {baseG, baseGW, baseGD, baseGWD};
  OpFunc opFunc = opFuncs[mia & MIA_base];
  SSATmp* gblName = getBase();
  m_ht.exceptionBarrier();
  m_base = gen(BaseG,
               cns(reinterpret_cast<TCA>(opFunc)),
               gblName,
               genMisPtr());
}

void HhbcTranslator::VectorTranslator::emitBaseS() {
  const int kClassIdx = m_ni.inputs.size() - 1;
  SSATmp* key = getKey();
  SSATmp* clsRef = getInput(kClassIdx);
  m_base = gen(LdClsPropAddr,
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
  const auto propInfo   = getPropertyOffset(m_ni, contextClass(),
                                            knownCls, m_mii,
                                            m_mInd, m_iInd);
  auto mia = m_mii.getAttr(m_ni.immVecM[m_mInd]);
  if (propInfo.offset == -1 || (mia & Unset)) {
    emitPropGeneric();
  } else {
    emitPropSpecialized(mia, propInfo);
  }
}

template <MInstrAttr attrs, bool isObj>
static inline TypedValue* propImpl(Class* ctx, TypedValue* base,
                                   TypedValue keyVal, MInstrState* mis) {
  return Prop<WDU(attrs), isObj>(
    mis->tvScratch, mis->tvRef, ctx, base, &keyVal);
}

#define HELPER_TABLE(m)                             \
  /* name     attrs        isObj */                 \
  m(propC,    None,        false)                   \
  m(propCD,   Define,      false)                   \
  m(propCDO,  Define,       true)                   \
  m(propCO,   None,         true)                   \
  m(propCU,   Unset,       false)                   \
  m(propCUO,  Unset,        true)                   \
  m(propCW,   Warn,        false)                   \
  m(propCWD,  WarnDefine,  false)                   \
  m(propCWDO, WarnDefine,   true)                   \
  m(propCWO,  Warn,         true)

#define PROP(nm, ...)                                                   \
TypedValue* nm(Class* ctx, TypedValue* base, TypedValue key,            \
               MInstrState* mis) {                                      \
  return propImpl<__VA_ARGS__>(ctx, base, key, mis);                    \
}
namespace VectorHelpers {
HELPER_TABLE(PROP)
}
#undef PROP

void HhbcTranslator::VectorTranslator::emitPropGeneric() {
  MemberCode mCode = m_ni.immVecM[m_mInd];
  MInstrAttr mia = MInstrAttr(m_mii.getAttr(mCode) & MIA_intermediate_prop);

  if ((mia & Unset) && m_base->type().strip().not(Type::Obj)) {
    m_base = m_tb.genPtrToInitNull();
    return;
  }

  typedef TypedValue* (*OpFunc)(Class*, TypedValue*, TypedValue, MInstrState*);
  SSATmp* key = getKey();
  BUILD_OPTAB(mia, m_base->isA(Type::Obj));
  m_ht.exceptionBarrier();
  if (mia & Define) {
    m_base = genStk(PropDX, cns((TCA)opFunc), CTX(), m_base, key, genMisPtr());
  } else {
    m_base = gen(PropX, cns((TCA)opFunc), CTX(), m_base, key, genMisPtr());
  }
}
#undef HELPER_TABLE

/*
 * Helper for emitPropSpecialized to check if a property is Uninit. It
 * returns a pointer to the property's address, or init_null_variant
 * if the property was Uninit and doDefine is false.
 *
 * We can omit the uninit check for properties that we know may not be
 * uninit due to the frontend's type inference.
 */
SSATmp* HhbcTranslator::VectorTranslator::checkInitProp(
    SSATmp* baseAsObj,
    SSATmp* propAddr,
    PropInfo propInfo,
    bool doWarn,
    bool doDefine) {
  SSATmp* key = getKey();
  assert(key->isA(Type::StaticStr));
  assert(baseAsObj->isA(Type::Obj));
  assert(propAddr->type().isPtr());

  auto const needsCheck =
    propInfo.hphpcType == KindOfInvalid &&
    // The m_mInd check is to avoid initializing a property to
    // InitNull right before it's going to be set to something else.
    (doWarn || (doDefine && m_mInd < m_ni.immVecM.size() - 1));

  if (!needsCheck) return propAddr;

  return m_tb.cond(m_ht.getCurFunc(),
    [&] (Block* taken) {
      gen(CheckInitMem, taken, propAddr, cns(0));
    },
    [&] { // Next: Property isn't Uninit. Do nothing.
      return propAddr;
    },
    [&] { // Taken: Property is Uninit. Raise a warning and return
          // a pointer to InitNull, either in the object or
          // init_null_variant.
      m_tb.hint(Block::Unlikely);
      if (doWarn && wantPropSpecializedWarnings()) {
        // We did the exceptionBarrier for this back in emitMPre.
        gen(RaiseUndefProp, baseAsObj, key);
      }
      if (doDefine) {
        gen(
          StProp,
          baseAsObj,
          cns(propInfo.offset),
          m_tb.genDefInitNull()
        );
        return propAddr;
      }
      return cns((const TypedValue*)&init_null_variant);
    }
  );
}

Class* HhbcTranslator::VectorTranslator::contextClass() const {
  return m_ht.getCurFunc()->cls();
}

void HhbcTranslator::VectorTranslator::emitPropSpecialized(const MInstrAttr mia,
                                                           PropInfo propInfo) {
  assert(!(mia & MIA_warn) || !(mia & MIA_unset));
  const bool doWarn   = mia & MIA_warn;
  const bool doDefine = mia & MIA_define || mia & MIA_unset;

  SSATmp* initNull = cns((const TypedValue*)&init_null_variant);

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
    SSATmp* propAddr = gen(LdPropAddr, m_base, cns(propInfo.offset));
    m_base = checkInitProp(m_base, propAddr, propInfo, doWarn, doDefine);
  } else {
    SSATmp* baseAsObj = nullptr;
    m_base = m_tb.cond(m_ht.getCurFunc(),
      [&] (Block* taken) {
        // baseAsObj is only available in the Next branch
        baseAsObj = gen(LdMem, Type::Obj, taken, m_base, cns(0));
      },
      [&] { // Next: Base is an object. Load property address and
            // check for uninit
        return checkInitProp(baseAsObj,
                             gen(LdPropAddr, baseAsObj,
                                                  cns(propInfo.offset)),
                             propInfo,
                             doWarn,
                             doDefine);
      },
      [&] { // Taken: Base is Null. Raise warnings/errors and return InitNull.
        m_tb.hint(Block::Unlikely);
        if (doWarn) {
          gen(WarnNonObjProp);
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
          gen(ThrowNonObjProp);
        }
        return initNull;
      }
    );
  }

  // At this point m_base is either a pointer to init_null_variant or
  // a property in the object that we've verified isn't uninit.
  assert(m_base->type().isPtr());
}

template <KeyType keyType, bool warn, bool define, bool reffy,
          bool unset>
static inline TypedValue* elemImpl(TypedValue* base, TypedValue keyVal,
                                   MInstrState* mis) {
  TypedValue* key = keyPtr<keyType>(keyVal);
  if (unset) {
    return ElemU<keyType>(mis->tvScratch, mis->tvRef, base, key);
  } else if (define) {
    return ElemD<warn, reffy, keyType>(mis->tvScratch, mis->tvRef, base, key);
  } else {
    return Elem<warn, keyType>(mis->tvScratch, mis->tvRef, base, key);
  }
}

#define HELPER_TABLE(m)                                       \
  /* name        hot        key       attrs  */               \
  m(elemC,     ,            AnyKey,   None)                   \
  m(elemCD,    ,            AnyKey,   Define)                 \
  m(elemCDR,   ,            AnyKey,   DefineReffy)            \
  m(elemCU,    ,            AnyKey,   Unset)                  \
  m(elemCW,    ,            AnyKey,   Warn)                   \
  m(elemCWD,   ,            AnyKey,   WarnDefine)             \
  m(elemCWDR,  ,            AnyKey,   WarnDefineReffy)        \
  m(elemI,     ,            IntKey,   None)                   \
  m(elemID,    HOT_FUNC_VM, IntKey,   Define)                 \
  m(elemIDR,   ,            IntKey,   DefineReffy)            \
  m(elemIU,    ,            IntKey,   Unset)                  \
  m(elemIW,    ,            IntKey,   Warn)                   \
  m(elemIWD,   ,            IntKey,   WarnDefine)             \
  m(elemIWDR,  ,            IntKey,   WarnDefineReffy)        \
  m(elemS,     HOT_FUNC_VM, StrKey,   None)                   \
  m(elemSD,    HOT_FUNC_VM, StrKey,   Define)                 \
  m(elemSDR,   ,            StrKey,   DefineReffy)            \
  m(elemSU,    ,            StrKey,   Unset)                  \
  m(elemSW,    HOT_FUNC_VM, StrKey,   Warn)                   \
  m(elemSWD,   ,            StrKey,   WarnDefine)             \
  m(elemSWDR,  ,            StrKey,   WarnDefineReffy)

#define ELEM(nm, hot, keyType, attrs)                                   \
hot                                                                     \
TypedValue* nm(TypedValue* base, TypedValue key, MInstrState* mis) {    \
  return elemImpl<keyType, WDRU(attrs)>(base, key, mis);                \
}
namespace VectorHelpers {
HELPER_TABLE(ELEM)
}
#undef ELEM

void HhbcTranslator::VectorTranslator::emitElem() {
  MemberCode mCode = m_ni.immVecM[m_mInd];
  MInstrAttr mia = MInstrAttr(m_mii.getAttr(mCode) & MIA_intermediate);
  SSATmp* key = getKey();

  const bool unset = mia & Unset;
  const bool define = mia & Define;
  assert(!(define && unset));
  if (unset) {
    SSATmp* uninit = m_tb.genPtrToUninit();
    Type baseType = m_base->type().strip();
    if (baseType.subtypeOf(Type::Str)) {
      m_ht.exceptionBarrier();
      gen(
        RaiseError,
        cns(StringData::GetStaticString(Strings::OP_NOT_SUPPORTED_STRING))
      );
      m_base = uninit;
      return;
    }
    if (baseType.not(Type::Arr | Type::Obj)) {
      m_base = uninit;
      return;
    }
  }

  typedef TypedValue* (*OpFunc)(TypedValue*, TypedValue, MInstrState*);
  BUILD_OPTAB_HOT(getKeyTypeIS(key), mia);
  m_ht.exceptionBarrier();
  if (define || unset) {
    m_base = genStk(define ? ElemDX : ElemUX, cns((TCA)opFunc), m_base,
                    key, genMisPtr());
  } else {
    m_base = gen(ElemX, cns((TCA)opFunc), m_base, key, genMisPtr());
  }
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
      gen(CheckInitMem, taken, m_misBase, cns(HHIR_MISOFF(tvRef)));
    },
    [&] { // Next: tvRef isn't Uninit. Ratchet the refs
      // Clean up tvRef2 before overwriting it.
      if (ratchetInd() > 0) {
        gen(DecRefMem, Type::Gen, m_misBase, cns(HHIR_MISOFF(tvRef2)));
      }
      // Copy tvRef to tvRef2. Use mmx at some point
      SSATmp* tvRef = gen(
        LdMem, Type::Gen, m_misBase, cns(HHIR_MISOFF(tvRef))
      );
      gen(StMem, m_misBase, cns(HHIR_MISOFF(tvRef2)), tvRef);

      // Reset tvRef.
      gen(StMem, m_misBase, cns(HHIR_MISOFF(tvRef)), m_tb.genDefUninit());

      // Adjust base pointer.
      assert(m_base->type().isPtr());
      return gen(LdAddr, m_misBase, cns(HHIR_MISOFF(tvRef2)));
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

template <KeyType keyType, bool isObj>
static inline TypedValue cGetPropImpl(Class* ctx, TypedValue* base,
                                      TypedValue keyVal, MInstrState* mis) {
  TypedValue result;
  TypedValue* key = keyPtr<keyType>(keyVal);
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

#define HELPER_TABLE(m)                                \
  /* name           hot        key     isObj */        \
  m(cGetPropC,    ,            AnyKey, false)          \
  m(cGetPropCO,   ,            AnyKey,  true)          \
  m(cGetPropS,    ,            StrKey, false)          \
  m(cGetPropSO,   HOT_FUNC_VM, StrKey,  true)

#define PROP(nm, hot, ...)                                              \
hot                                                                     \
TypedValue nm(Class* ctx, TypedValue* base, TypedValue key,             \
                     MInstrState* mis) {                                \
  return cGetPropImpl<__VA_ARGS__>(ctx, base, key, mis);                \
}
namespace VectorHelpers {
HELPER_TABLE(PROP)
}
#undef PROP

void HhbcTranslator::VectorTranslator::emitCGetProp() {
  assert(!m_ni.outLocal);

  const Class* knownCls = nullptr;
  const auto propInfo   = getPropertyOffset(m_ni, contextClass(), knownCls,
                                            m_mii, m_mInd, m_iInd);
  if (propInfo.offset != -1) {
    emitPropSpecialized(MIA_warn, propInfo);
    SSATmp* cellPtr = gen(UnboxPtr, m_base);
    SSATmp* propVal = gen(LdMem, Type::Cell, cellPtr, cns(0));
    m_result = gen(IncRef, propVal);
    return;
  }

  typedef TypedValue (*OpFunc)(Class*, TypedValue*, TypedValue, MInstrState*);
  SSATmp* key = getKey();
  BUILD_OPTAB_HOT(getKeyTypeS(key), m_base->isA(Type::Obj));
  m_ht.exceptionBarrier();
  m_result = gen(CGetProp, cns((TCA)opFunc), CTX(),
                      m_base, key, genMisPtr());
}
#undef HELPER_TABLE

template <KeyType keyType, bool isObj>
static inline TypedValue vGetPropImpl(Class* ctx, TypedValue* base,
                                      TypedValue keyVal, MInstrState* mis) {
  TypedValue* key = keyPtr<keyType>(keyVal);
  base = VM::Prop<false, true, false, isObj, keyType>(
    mis->tvScratch, mis->tvRef, ctx, base, key);

  if (base == &mis->tvScratch && base->m_type == KindOfUninit) {
    // Error (no result was set).
    return tv(KindOfRef, NEW(RefData)(RefData::nullinit));
  } else {
    if (base->m_type != KindOfRef) {
      tvBox(base);
    }
    assert(base->m_type == KindOfRef);
    base->m_data.pref->incRefCount();
    return *base;
  }
}

#define HELPER_TABLE(m)                         \
  /* name          hot        key     isObj */  \
  m(vGetPropC,   ,            AnyKey, false)    \
  m(vGetPropCO,  ,            AnyKey,  true)    \
  m(vGetPropS,   ,            StrKey, false)    \
  m(vGetPropSO,  HOT_FUNC_VM, StrKey,  true)

#define PROP(nm, hot, ...)                                              \
hot                                                                     \
TypedValue nm(Class* ctx, TypedValue* base, TypedValue key,             \
                     MInstrState* mis) {                                \
  return vGetPropImpl<__VA_ARGS__>(ctx, base, key, mis);                \
}
namespace VectorHelpers {
HELPER_TABLE(PROP)
}
#undef PROP

void HhbcTranslator::VectorTranslator::emitVGetProp() {
  SSATmp* key = getKey();
  typedef TypedValue (*OpFunc)(Class*, TypedValue*, TypedValue, MInstrState*);
  BUILD_OPTAB_HOT(getKeyTypeS(key), m_base->isA(Type::Obj));
  m_ht.exceptionBarrier();
  m_result = genStk(VGetProp, cns((TCA)opFunc), CTX(),
                    m_base, key, genMisPtr());
}
#undef HELPER_TABLE

template <bool useEmpty, bool isObj>
static inline bool issetEmptyPropImpl(Class* ctx, TypedValue* base,
                                      TypedValue keyVal) {
  return VM::IssetEmptyProp<useEmpty, isObj>(ctx, base, &keyVal);
}

#define HELPER_TABLE(m)                                         \
  /* name         useEmpty isObj */                             \
  m(issetPropC,   false,   false)                               \
  m(issetPropCE,   true,   false)                               \
  m(issetPropCEO,  true,    true)                               \
  m(issetPropCO,  false,    true)

#define ISSET(nm, ...)                                                  \
/* This returns int64_t to ensure all 64 bits of rax are valid */       \
uint64_t nm(Class* ctx, TypedValue* base, TypedValue key) {             \
  return issetEmptyPropImpl<__VA_ARGS__>(ctx, base, key);               \
}
namespace VectorHelpers {
HELPER_TABLE(ISSET)
}
#undef ISSET

void HhbcTranslator::VectorTranslator::emitIssetEmptyProp(bool isEmpty) {
  SSATmp* key = getKey();
  typedef uint64_t (*OpFunc)(Class*, TypedValue*, TypedValue);
  BUILD_OPTAB(isEmpty, m_base->isA(Type::Obj));
  m_ht.exceptionBarrier();
  m_result = gen(isEmpty ? EmptyProp : IssetProp, cns((TCA)opFunc),
                      CTX(), m_base, key);
}
#undef HELPER_TABLE

void HhbcTranslator::VectorTranslator::emitIssetProp() {
  emitIssetEmptyProp(false);
}

void HhbcTranslator::VectorTranslator::emitEmptyProp() {
  emitIssetEmptyProp(true);
}

template <bool isObj>
static inline TypedValue setPropImpl(Class* ctx, TypedValue* base,
                                     TypedValue keyVal, Cell val) {
  VM::SetProp<true, isObj>(ctx, base, &keyVal, &val);
  return val;
}

#define HELPER_TABLE(m)                     \
  /* name        isObj */                   \
  m(setPropC,    false)                     \
  m(setPropCO,    true)

#define PROP(nm, ...)                                                   \
TypedValue nm(Class* ctx, TypedValue* base, TypedValue key, Cell val) { \
  return setPropImpl<__VA_ARGS__>(ctx, base, key, val);                 \
}
namespace VectorHelpers {
HELPER_TABLE(PROP)
}
#undef PROP

void HhbcTranslator::VectorTranslator::emitSetProp() {
  SSATmp* value = getValue();

  /* If we know the class for the current base, emit a direct property set. */
  const Class* knownCls = nullptr;
  const auto propInfo   = getPropertyOffset(m_ni, contextClass(), knownCls,
                                            m_mii, m_mInd, m_iInd);
  if (propInfo.offset != -1) {
    emitPropSpecialized(MIA_define, propInfo);
    SSATmp* cellPtr = gen(UnboxPtr, m_base);
    SSATmp* oldVal = gen(LdMem, Type::Cell, cellPtr, cns(0));
    // The object owns a reference now
    SSATmp* increffed = gen(IncRef, value);
    gen(StMem, cellPtr, cns(0), value);
    gen(DecRef, oldVal);
    m_result = increffed;
    return;
  }

  // Emit the appropriate helper call.
  typedef TypedValue (*OpFunc)(Class*, TypedValue*, TypedValue, Cell);
  SSATmp* key = getKey();
  BUILD_OPTAB(m_base->isA(Type::Obj));
  m_ht.exceptionBarrier();
  SSATmp* result = genStk(SetProp, cns((TCA)opFunc), CTX(),
                          m_base, key, value);
  VectorEffects ve(result->inst());
  m_result = ve.valTypeChanged ? result : value;
}
#undef HELPER_TABLE

template <SetOpOp op, bool isObj>
static inline TypedValue setOpPropImpl(TypedValue* base, TypedValue keyVal,
                                       Cell val, MInstrState* mis) {
  TypedValue* result = VM::SetOpProp<isObj>(
    mis->tvScratch, mis->tvRef, mis->ctx, op, base, &keyVal, &val);

  TypedValue ret;
  tvReadCell(result, &ret);
  return ret;
}

#define OPPROP_TABLE(m, nm, op)                                \
  /* name             op  isObj */                             \
  m(nm##op##PropC,    op,  false)                              \
  m(nm##op##PropCO,   op,   true)

#define HELPER_TABLE(m, op) OPPROP_TABLE(m, setOp, SetOp##op)
#define SETOP(nm, ...)                                                 \
TypedValue nm(TypedValue* base, TypedValue key,                        \
                     Cell val, MInstrState* mis) {                     \
  return setOpPropImpl<__VA_ARGS__>(base, key, val, mis);              \
}
#define SETOP_OP(op, bcOp) HELPER_TABLE(SETOP, op)
namespace VectorHelpers {
SETOP_OPS
}
#undef SETOP_OP
#undef SETOP

void HhbcTranslator::VectorTranslator::emitSetOpProp() {
  SetOpOp op = SetOpOp(m_ni.imm[0].u_OA);
  SSATmp* key = getKey();
  SSATmp* value = getValue();
  typedef TypedValue (*OpFunc)(TypedValue*, TypedValue,
                               Cell, MInstrState*);
# define SETOP_OP(op, bcOp) HELPER_TABLE(FILL_ROW, op)
  BUILD_OPTAB_ARG(SETOP_OPS, op, m_base->isA(Type::Obj));
# undef SETOP_OP
  gen(StRaw, m_misBase, cns(RawMemSlot::MisCtx), CTX());
  m_ht.exceptionBarrier();
  m_result =
    genStk(SetOpProp, cns((TCA)opFunc), m_base, key, value, genMisPtr());
}
#undef HELPER_TABLE

template <IncDecOp op, bool isObj>
static inline TypedValue incDecPropImpl(Class* ctx, TypedValue* base,
                                        TypedValue keyVal, MInstrState* mis) {
  TypedValue result;
  result.m_type = KindOfUninit;
  VM::IncDecProp<true, isObj>(
    mis->tvScratch, mis->tvRef, ctx, op, base, &keyVal, result);
  assert(result.m_type != KindOfRef);
  return result;
}


#define HELPER_TABLE(m, op) OPPROP_TABLE(m, incDec, op)
#define INCDEC(nm, ...)                                                 \
TypedValue nm(Class* ctx, TypedValue* base, TypedValue key,             \
                     MInstrState* mis) {                                \
  return incDecPropImpl<__VA_ARGS__>(ctx, base, key, mis);              \
}
#define INCDEC_OP(op) HELPER_TABLE(INCDEC, op)
namespace VectorHelpers {
INCDEC_OPS
}
#undef INCDEC_OP
#undef INCDEC

void HhbcTranslator::VectorTranslator::emitIncDecProp() {
  IncDecOp op = IncDecOp(m_ni.imm[0].u_OA);
  SSATmp* key = getKey();
  typedef TypedValue (*OpFunc)(Class*, TypedValue*, TypedValue, MInstrState*);
# define INCDEC_OP(op) HELPER_TABLE(FILL_ROW, op)
  BUILD_OPTAB_ARG(INCDEC_OPS, op, m_base->isA(Type::Obj));
# undef INCDEC_OP
  m_ht.exceptionBarrier();
  m_result =
    genStk(IncDecProp, cns((TCA)opFunc), CTX(), m_base, key, genMisPtr());
}
#undef HELPER_TABLE

template <bool isObj>
static inline void bindPropImpl(Class* ctx, TypedValue* base, TypedValue keyVal,
                                RefData* val, MInstrState* mis) {
  TypedValue* prop = VM::Prop<false, true, false, isObj>(
    mis->tvScratch, mis->tvRef, ctx, base, &keyVal);
  if (!(prop == &mis->tvScratch && prop->m_type == KindOfUninit)) {
    tvBindRef(val, prop);
  }
}

#define HELPER_TABLE(m)            \
  /* name        isObj */          \
  m(bindPropC,   false)            \
  m(bindPropCO,   true)

#define PROP(nm, ...)                                                   \
void nm(Class* ctx, TypedValue* base, TypedValue key,                   \
                      RefData* val, MInstrState* mis) {                 \
  bindPropImpl<__VA_ARGS__>(ctx, base, key, val, mis);                  \
}
namespace VectorHelpers {
HELPER_TABLE(PROP)
}
#undef PROP

void HhbcTranslator::VectorTranslator::emitBindProp() {
  SSATmp* key = getKey();
  SSATmp* box = getValue();
  typedef void (*OpFunc)(Class*, TypedValue*, TypedValue*, RefData*,
                         MInstrState*);
  BUILD_OPTAB(m_base->isA(Type::Obj));
  m_ht.exceptionBarrier();
  genStk(BindProp, cns((TCA)opFunc), CTX(), m_base, key, box, genMisPtr());
  m_result = box;
}
#undef HELPER_TABLE

template <bool isObj>
static inline void unsetPropImpl(Class* ctx, TypedValue* base,
                                 TypedValue keyVal) {
  VM::UnsetProp<isObj>(ctx, base, &keyVal);
}

#define HELPER_TABLE(m)            \
  /* name        isObj */          \
  m(unsetPropC,  false)            \
  m(unsetPropCO,  true)

#define PROP(nm, ...)                                                   \
  static void nm(Class* ctx, TypedValue* base, TypedValue key) {        \
    unsetPropImpl<__VA_ARGS__>(ctx, base, key);                         \
  }
namespace VectorHelpers {
HELPER_TABLE(PROP)
}
#undef PROP

void HhbcTranslator::VectorTranslator::emitUnsetProp() {
  SSATmp* key = getKey();

  if (m_base->type().strip().not(Type::Obj)) {
    // Noop
    return;
  }

  typedef void (*OpFunc)(Class*, TypedValue*, TypedValue);
  BUILD_OPTAB(m_base->isA(Type::Obj));
  gen(UnsetProp, cns((TCA)opFunc), CTX(), m_base, key);
}
#undef HELPER_TABLE

void HhbcTranslator::VectorTranslator::checkStrictlyInteger(
  SSATmp*& key, KeyType& keyType, bool& checkForInt) {
  checkForInt = false;
  if (key->isA(Type::Int)) {
    keyType = IntKey;
  } else {
    assert(key->isA(Type::Str));
    keyType = StrKey;
    if (key->isConst()) {
      int64_t i;
      if (key->getValStr()->isStrictlyInteger(i)) {
        keyType = IntKey;
        key = cns(i);
      }
    } else {
      checkForInt = true;
    }
  }
}

static inline TypedValue* checkedGetCell(ArrayData* a, StringData* key) {
  int64_t i;
  return UNLIKELY(key->isStrictlyInteger(i)) ? a->nvGetCell(i)
                                             : a->nvGetCell(key);
}

static inline TypedValue* checkedGetCell(ArrayData* a, int64_t key) {
  not_reached();
}

template<KeyType keyType, bool checkForInt>
static inline TypedValue arrayGetImpl(
  ArrayData* a, typename KeyTypeTraits<keyType>::rawType key) {
  TypedValue* ret = checkForInt ? checkedGetCell(a, key)
                                : a->nvGetCell(key);
  tvRefcountedIncRef(ret);
  return *ret;
}

#define HELPER_TABLE(m)                                 \
  /* name        hot        keyType  checkForInt */     \
  m(arrayGetS,   HOT_FUNC_VM, StrKey,   false)          \
  m(arrayGetSi,  HOT_FUNC_VM, StrKey,    true)          \
  m(arrayGetI,   HOT_FUNC_VM, IntKey,   false)

#define ELEM(nm, hot, keyType, checkForInt)                             \
hot                                                                     \
TypedValue nm(ArrayData* a, TypedValue* key) {                          \
  return arrayGetImpl<keyType, checkForInt>(a, keyAsRaw<keyType>(key)); \
}
namespace VectorHelpers {
HELPER_TABLE(ELEM)
}
#undef ELEM

void HhbcTranslator::VectorTranslator::emitArrayGet(SSATmp* key) {
  KeyType keyType;
  bool checkForInt;
  checkStrictlyInteger(key, keyType, checkForInt);

  typedef TypedValue (*OpFunc)(ArrayData*, TypedValue*);
  BUILD_OPTAB_HOT(keyType, checkForInt);
  assert(m_base->isA(Type::Arr));
  m_result = gen(ArrayGet, cns((TCA)opFunc), m_base, key);
}
#undef HELPER_TABLE

template <KeyType keyType>
static inline TypedValue cGetElemImpl(TypedValue* base, TypedValue keyVal,
                                      MInstrState* mis) {
  TypedValue result;
  TypedValue* key = keyPtr<keyType>(keyVal);
  base = Elem<true, keyType>(result, mis->tvRef, base, key);
  if (base != &result) {
    // Save a copy of the result.
    tvDup(base, &result);
  }
  if (result.m_type == KindOfRef) {
    tvUnbox(&result);
  }
  return result;
}

#define HELPER_TABLE(m)                                 \
  /* name         hot         key  */                   \
  m(cGetElemC,  ,            AnyKey)                    \
  m(cGetElemI,  ,            IntKey)                    \
  m(cGetElemS,  HOT_FUNC_VM, StrKey)

#define ELEM(nm, hot, ...)                                              \
hot                                                                     \
TypedValue nm(TypedValue* base, TypedValue key, MInstrState* mis) {     \
  return cGetElemImpl<__VA_ARGS__>(base, key, mis);                     \
}
namespace VectorHelpers {
HELPER_TABLE(ELEM)
}
#undef ELEM

void HhbcTranslator::VectorTranslator::emitCGetElem() {
  SSATmp* key = getKey();

  if (isSimpleArrayOp()) {
    emitArrayGet(key);
    return;
  }

  typedef TypedValue (*OpFunc)(TypedValue*, TypedValue, MInstrState*);
  BUILD_OPTAB_HOT(getKeyTypeIS(key));
  m_ht.exceptionBarrier();
  m_result = gen(CGetElem, cns((TCA)opFunc),
                      m_base, key, genMisPtr());
}
#undef HELPER_TABLE

template <KeyType keyType>
static inline TypedValue vGetElemImpl(TypedValue* base, TypedValue keyVal,
                                      MInstrState* mis) {
  TypedValue* key = keyPtr<keyType>(keyVal);
  base = VM::ElemD<false, true, keyType>(mis->tvScratch, mis->tvRef, base, key);

  TypedValue result;
  if (base == &mis->tvScratch && base->m_type == KindOfUninit) {
    // Error (no result was set).
    tvWriteNull(&result);
    tvBox(&result);
  } else {
    if (base->m_type != KindOfRef) {
      tvBox(base);
    }
    tvDupVar(base, &result);
  }
  return result;
}

#define HELPER_TABLE(m)                      \
  /* name         keyType */                 \
  m(vGetElemC,    AnyKey)                    \
  m(vGetElemI,    IntKey)                    \
  m(vGetElemS,    StrKey)

#define ELEM(nm, ...)                                                   \
TypedValue nm(TypedValue* base, TypedValue key, MInstrState* mis) {     \
  return vGetElemImpl<__VA_ARGS__>(base, key,  mis);                    \
}
namespace VectorHelpers {
HELPER_TABLE(ELEM)
}
#undef ELEM

void HhbcTranslator::VectorTranslator::emitVGetElem() {
  SSATmp* key = getKey();
  typedef TypedValue (*OpFunc)(TypedValue*, TypedValue, MInstrState*);
  BUILD_OPTAB(getKeyTypeIS(key));
  m_ht.exceptionBarrier();
  m_result = genStk(VGetElem, cns((TCA)opFunc), m_base, key, genMisPtr());
}
#undef HELPER_TABLE

template <KeyType keyType, bool isEmpty>
static inline bool issetEmptyElemImpl(TypedValue* base, TypedValue keyVal,
                                      MInstrState* mis) {
  TypedValue* key = keyPtr<keyType>(keyVal);
  // mis == nullptr if we proved that it won't be used. mis->tvScratch and
  // mis->tvRef are ok because those params are passed by
  // reference.
  return VM::IssetEmptyElem<isEmpty, false, keyType>(
    mis->tvScratch, mis->tvRef, base, key);
}

#define HELPER_TABLE(m)                              \
  /* name          hot        keyType  isEmpty */    \
  m(issetElemC,   ,            AnyKey, false)        \
  m(issetElemCE,  ,            AnyKey,  true)        \
  m(issetElemI,   HOT_FUNC_VM, IntKey, false)        \
  m(issetElemIE,  ,            IntKey,  true)        \
  m(issetElemS,   HOT_FUNC_VM, StrKey, false)        \
  m(issetElemSE,  ,            StrKey,  true)

#define ISSET(nm, hot, ...)                                             \
hot                                                                     \
uint64_t nm(TypedValue* base, TypedValue key, MInstrState* mis) {       \
  return issetEmptyElemImpl<__VA_ARGS__>(base, key, mis);               \
}
namespace VectorHelpers {
HELPER_TABLE(ISSET)
}
#undef ISSET

void HhbcTranslator::VectorTranslator::emitIssetEmptyElem(bool isEmpty) {
  SSATmp* key = getKey();

  typedef uint64_t (*OpFunc)(TypedValue*, TypedValue, MInstrState*);
  BUILD_OPTAB_HOT(getKeyTypeIS(key), isEmpty);
  m_ht.exceptionBarrier();
  m_result = gen(isEmpty ? EmptyElem : IssetElem,
                      cns((TCA)opFunc), m_base, key, genMisPtr());
}
#undef HELPER_TABLE

static inline TypedValue* checkedGet(ArrayData* a, StringData* key) {
  int64_t i;
  return UNLIKELY(key->isStrictlyInteger(i)) ? a->nvGet(i)
                                             : a->nvGet(key);
}

static inline TypedValue* checkedGet(ArrayData* a, int64_t key) {
  not_reached();
}

template<KeyType keyType, bool checkForInt>
static inline uint64_t arrayIssetImpl(
  ArrayData* a, typename KeyTypeTraits<keyType>::rawType key) {
  TypedValue* value = checkForInt ? checkedGet(a, key)
                                  : a->nvGet(key);
  Variant* var = &tvAsVariant(value);
  return var && !var->isNull();
}

#define HELPER_TABLE(m)                         \
  /* name          keyType  checkForInt */      \
  m(arrayIssetS,    StrKey,   false)            \
  m(arrayIssetSi,   StrKey,    true)            \
  m(arrayIssetI,    IntKey,   false)

#define ISSET(nm, keyType, checkForInt)                                 \
  uint64_t nm(ArrayData* a, TypedValue* key) {                          \
    return arrayIssetImpl<keyType, checkForInt>(a, keyAsRaw<keyType>(key)); \
  }
namespace VectorHelpers {
HELPER_TABLE(ISSET)
}
#undef ISSET

void HhbcTranslator::VectorTranslator::emitArrayIsset() {
  SSATmp* key = getKey();
  KeyType keyType;
  bool checkForInt;
  checkStrictlyInteger(key, keyType, checkForInt);

  typedef uint64_t (*OpFunc)(ArrayData*, TypedValue*);
  BUILD_OPTAB(keyType, checkForInt);
  assert(m_base->isA(Type::Arr));
  m_ht.exceptionBarrier();
  m_result = gen(ArrayIsset, cns((TCA)opFunc), m_base, key);
}
#undef HELPER_TABLE

void HhbcTranslator::VectorTranslator::emitIssetElem() {
  if (isSimpleArrayOp()) {
    emitArrayIsset();
    return;
  }

  emitIssetEmptyElem(false);
}

void HhbcTranslator::VectorTranslator::emitEmptyElem() {
  emitIssetEmptyElem(true);
}

static inline ArrayData* checkedSet(ArrayData* a, StringData* key,
                                    CVarRef value, bool copy) {
  int64_t i;
  return UNLIKELY(key->isStrictlyInteger(i)) ? a->set(i, value, copy)
                                             : a->set(key, value, copy);
}

static inline ArrayData* checkedSet(ArrayData* a, int64_t key,
                                    CVarRef value, bool copy) {
  not_reached();
}

template<KeyType keyType, bool checkForInt, bool setRef>
static inline typename ShuffleReturn<setRef>::return_type arraySetImpl(
  ArrayData* a, typename KeyTypeTraits<keyType>::rawType key,
  CVarRef value, RefData* ref) {
  static_assert(keyType != AnyKey, "AnyKey is not supported in arraySetMImpl");
  const bool copy = a->getCount() > 1;
  ArrayData* ret = checkForInt ? checkedSet(a, key, value, copy)
                               : a->set(key, value, copy);

  return arrayRefShuffle<setRef>(a, ret, setRef ? ref->tv() : nullptr);
}

#define HELPER_TABLE(m)                                                 \
  /* name        hot          keyType  checkForInt setRef */            \
  m(arraySetS,   HOT_FUNC_VM, StrKey,   false,     false)               \
  m(arraySetSi,  HOT_FUNC_VM, StrKey,    true,     false)               \
  m(arraySetI,   HOT_FUNC_VM, IntKey,   false,     false)               \
  m(arraySetSR,  ,            StrKey,   false,      true)               \
  m(arraySetSiR, ,            StrKey,    true,      true)               \
  m(arraySetIR,  ,            IntKey,   false,      true)

#define ELEM(nm, hot, keyType, checkForInt, setRef)                     \
hot                                                                     \
typename ShuffleReturn<setRef>::return_type                             \
nm(ArrayData* a, TypedValue* key, TypedValue value, RefData* ref) {     \
  return arraySetImpl<keyType, checkForInt, setRef>(                    \
    a, keyAsRaw<keyType>(key), tvAsCVarRef(&value), ref);               \
}
namespace VectorHelpers {
HELPER_TABLE(ELEM)
}
#undef ELEM

void HhbcTranslator::VectorTranslator::emitArraySet(SSATmp* key,
                                                    SSATmp* value) {
  assert(m_iInd == m_mii.valCount() + 1);
  const int baseStkIdx = m_mii.valCount();
  assert(key->type().notBoxed());
  assert(value->type().notBoxed());
  KeyType keyType;
  bool checkForInt;
  checkStrictlyInteger(key, keyType, checkForInt);
  const DynLocation& base = *m_ni.inputs[m_mii.valCount()];
  bool setRef = base.outerType() == KindOfRef;
  typedef ArrayData* (*OpFunc)(ArrayData*, TypedValue*, TypedValue, RefData*);
  BUILD_OPTAB_HOT(keyType, checkForInt, setRef);

  // Don't exceptionBarrier below because the helper can't throw. It
  // may reenter to call destructors so it has a sync point in
  // nativecalls.cpp, but exceptions are swallowed at destructor
  // boundaries right now: #2182869.
  if (setRef) {
    assert(base.location.space == Location::Local ||
           base.location.space == Location::Stack);
    SSATmp* box = getInput(baseStkIdx);
    gen(ArraySetRef, cns((TCA)opFunc), m_base, key, value, box);
    // Unlike the non-ref case, we don't need to do anything to the stack
    // because any load of the box will be guarded.
  } else {
    SSATmp* newArr = gen(ArraySet, cns((TCA)opFunc), m_base, key, value);

    // Update the base's value with the new array
    if (base.location.space == Location::Local) {
      m_tb.genStLoc(base.location.offset, newArr, false, false, nullptr);
    } else if (base.location.space == Location::Stack) {
      VectorEffects ve(newArr->inst());
      assert(ve.baseValChanged);
      assert(ve.baseType.subtypeOf(Type::Arr));
      m_ht.extendStack(baseStkIdx, Type::Gen);
      m_ht.replace(baseStkIdx, newArr);
    } else {
      not_reached();
    }
  }

  m_result = value;
}
#undef HELPER_TABLE

template <KeyType keyType>
static inline TypedValue setElemImpl(TypedValue* base, TypedValue keyVal,
                                     Cell val) {
  TypedValue* key = keyPtr<keyType>(keyVal);
  HPHP::VM::SetElem<true, keyType>(base, key, &val);
  return val;
}

#define HELPER_TABLE(m)                                 \
  /* name         hot        key    */                  \
  m(setElemC,   ,            AnyKey)                    \
  m(setElemI,   ,            IntKey)                    \
  m(setElemS,   HOT_FUNC_VM, StrKey)

#define ELEM(nm, hot, ...)                                              \
hot                                                                     \
TypedValue nm(TypedValue* base, TypedValue key, Cell val) {             \
  return setElemImpl<__VA_ARGS__>(base, key, val);                      \
}
namespace VectorHelpers {
HELPER_TABLE(ELEM)
}
#undef ELEM

void HhbcTranslator::VectorTranslator::emitSetElem() {
  SSATmp* value = getValue();
  SSATmp* key = getKey();

  if (isSimpleArrayOp()) {
    emitArraySet(key, value);
    return;
  }

  // Emit the appropriate helper call.
  typedef TypedValue (*OpFunc)(TypedValue*, TypedValue, Cell);
  BUILD_OPTAB_HOT(getKeyTypeIS(key));
  m_ht.exceptionBarrier();
  SSATmp* result = genStk(SetElem, cns((TCA)opFunc), m_base, key, value);
  VectorEffects ve(result->inst());
  m_result = ve.valTypeChanged ? result : value;
}
#undef HELPER_TABLE

template <SetOpOp op>
static inline TypedValue setOpElemImpl(TypedValue* base, TypedValue keyVal,
                                       Cell val, MInstrState* mis) {
  TypedValue* result =
    VM::SetOpElem(mis->tvScratch, mis->tvRef, op, base, &keyVal, &val);

  TypedValue ret;
  tvReadCell(result, &ret);
  return ret;
}

#define OPELEM_TABLE(m, nm, op)                 \
  /* name          op */                        \
  m(nm##op##ElemC, op)

#define HELPER_TABLE(m, op) OPELEM_TABLE(m, setOp, SetOp##op)
#define SETOP(nm, ...)                                                  \
TypedValue nm(TypedValue* base, TypedValue key, Cell val,               \
                     MInstrState* mis) {                                \
  return setOpElemImpl<__VA_ARGS__>(base, key, val, mis);               \
}
#define SETOP_OP(op, bcOp) HELPER_TABLE(SETOP, op)
namespace VectorHelpers {
SETOP_OPS
}
#undef SETOP_OP
#undef SETOP

void HhbcTranslator::VectorTranslator::emitSetOpElem() {
  SetOpOp op = SetOpOp(m_ni.imm[0].u_OA);
  SSATmp* key = getKey();
  typedef TypedValue (*OpFunc)(TypedValue*, TypedValue, Cell, MInstrState*);
# define SETOP_OP(op, bcOp) HELPER_TABLE(FILL_ROW, op)
  BUILD_OPTAB_ARG(SETOP_OPS, op);
# undef SETOP_OP
  m_ht.exceptionBarrier();
  m_result =
    genStk(SetOpElem, cns((TCA)opFunc), m_base, key, getValue(), genMisPtr());
}
#undef HELPER_TABLE

template <IncDecOp op>
static inline TypedValue incDecElemImpl(TypedValue* base, TypedValue keyVal,
                                        MInstrState* mis) {
  TypedValue result;
  VM::IncDecElem<true>(
    mis->tvScratch, mis->tvRef, op, base, &keyVal, result);
  assert(result.m_type != KindOfRef);
  return result;
}

#define HELPER_TABLE(m, op) OPELEM_TABLE(m, incDec, op)
#define INCDEC(nm, ...)                                                 \
TypedValue nm(TypedValue* base, TypedValue key, MInstrState* mis) {     \
  return incDecElemImpl<__VA_ARGS__>(base, key, mis);                   \
}
#define INCDEC_OP(op) HELPER_TABLE(INCDEC, op)
namespace VectorHelpers {
INCDEC_OPS
}
#undef INCDEC_OP
#undef INCDEC

void HhbcTranslator::VectorTranslator::emitIncDecElem() {
  IncDecOp op = IncDecOp(m_ni.imm[0].u_OA);
  SSATmp* key = getKey();
  typedef TypedValue (*OpFunc)(TypedValue*, TypedValue, MInstrState*);
# define INCDEC_OP(op) HELPER_TABLE(FILL_ROW, op)
  BUILD_OPTAB_ARG(INCDEC_OPS, op);
# undef INCDEC_OP
  m_ht.exceptionBarrier();
  m_result = genStk(IncDecElem, cns((TCA)opFunc), m_base, key, genMisPtr());
}
#undef HELPER_TABLE

namespace VectorHelpers {
void bindElemC(TypedValue* base, TypedValue keyVal, RefData* val,
               MInstrState* mis) {
  base = VM::ElemD<false, true>(mis->tvScratch, mis->tvRef, base, &keyVal);
  if (!(base == &mis->tvScratch && base->m_type == KindOfUninit)) {
    tvBindRef(val, base);
  }
}
}

void HhbcTranslator::VectorTranslator::emitBindElem() {
  SSATmp* key = getKey();
  SSATmp* box = getValue();
  m_ht.exceptionBarrier();
  genStk(BindElem, cns((TCA)VectorHelpers::bindElemC),
         m_base, key, box, genMisPtr());
  m_result = box;
}

template <KeyType keyType>
static inline void unsetElemImpl(TypedValue* base, TypedValue keyVal) {
  TypedValue* key = keyPtr<keyType>(keyVal);
  VM::UnsetElem<keyType>(base, key);
}

#define HELPER_TABLE(m)                              \
  /* name           hot       keyType */             \
  m(unsetElemC,   ,            AnyKey)                \
  m(unsetElemI,   ,            IntKey)                \
  m(unsetElemS,   HOT_FUNC_VM, StrKey)

#define ELEM(nm, hot, ...)                                      \
hot                                                             \
void nm(TypedValue* base, TypedValue key) {                     \
  unsetElemImpl<__VA_ARGS__>(base, key);                        \
}
namespace VectorHelpers {
HELPER_TABLE(ELEM)
}
#undef ELEM

void HhbcTranslator::VectorTranslator::emitUnsetElem() {
  SSATmp* key = getKey();

  Type baseType = m_base->type().strip();
  if (baseType.subtypeOf(Type::Str)) {
    m_ht.exceptionBarrier();
    gen(RaiseError,
             cns(StringData::GetStaticString(Strings::CANT_UNSET_STRING)));
    return;
  }
  if (baseType.not(Type::Arr | Type::Obj)) {
    // Noop
    return;
  }

  typedef void (*OpFunc)(TypedValue*, TypedValue);
  BUILD_OPTAB_HOT(getKeyTypeIS(key));
  m_ht.exceptionBarrier();
  genStk(UnsetElem, cns((TCA)opFunc), m_base, key);
}
#undef HELPER_TABLE

void HhbcTranslator::VectorTranslator::emitNotSuppNewElem() {
  not_reached();
}

void HhbcTranslator::VectorTranslator::emitVGetNewElem() {
  SPUNT(__func__);
}

void HhbcTranslator::VectorTranslator::emitSetNewElem() {
  SSATmp* value = getValue();
  m_ht.exceptionBarrier();
  SSATmp* result = gen(SetNewElem, m_base, value);
  VectorEffects ve(result->inst());
  m_result = ve.valTypeChanged ? result : value;
}

void HhbcTranslator::VectorTranslator::emitSetOpNewElem() {
  SPUNT(__func__);
}

void HhbcTranslator::VectorTranslator::emitIncDecNewElem() {
  SPUNT(__func__);
}

void HhbcTranslator::VectorTranslator::emitBindNewElem() {
  SSATmp* box = getValue();
  m_ht.exceptionBarrier();
  genStk(BindNewElem, m_base, box, genMisPtr());
  m_result = box;
}

void HhbcTranslator::VectorTranslator::emitMPost() {
  // Decref stack inputs. If we're translating a SetM or BindM, then input 0 is
  // both our input and output so leave its refcount alone. The helpers for
  // SetM can overwrite this value with InitNull if the operation fails, but
  // they also decref the old value so it's still safe to leave its refcount
  // alone.
  unsigned nStack =
    (m_ni.mInstrOp() == OpSetM || m_ni.mInstrOp() == OpBindM) ? 1 : 0;
  for (unsigned i = nStack; i < m_ni.inputs.size(); ++i) {
    const DynLocation& input = *m_ni.inputs[i];
    switch (input.location.space) {
    case Location::Stack: {
      ++nStack;
      auto input = getInput(i);
      if (input->isA(Type::Gen)) {
        gen(DecRef, input);
      }
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

  // Push result, if one was produced
  if (m_result) {
    m_ht.push(m_result);
  } else {
    assert(m_ni.mInstrOp() == OpUnsetM);
  }

  // Clean up tvRef(2)
  if (nLogicalRatchets() > 1) {
    gen(DecRefMem, Type::Gen, m_misBase, cns(HHIR_MISOFF(tvRef2)));
  }
  if (nLogicalRatchets() > 0) {
    gen(DecRefMem, Type::Gen, m_misBase, cns(HHIR_MISOFF(tvRef)));
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
