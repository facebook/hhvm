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


#include "hphp/runtime/base/hphp-array-defs.h"
#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/vm/jit/frame-state.h"
#include "hphp/runtime/vm/jit/hhbc-translator.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/translator-x64.h"

// These files do ugly things with macros so include them last
#include "hphp/util/assert-throw.h"
#include "hphp/runtime/vm/jit/minstr-translator-internal.h"

namespace HPHP { namespace JIT {

TRACE_SET_MOD(hhir);

using HPHP::JIT::Location;

static bool wantPropSpecializedWarnings() {
  return !RuntimeOption::RepoAuthoritative ||
    !RuntimeOption::EvalDisableSomeRepoAuthNotices;
}

bool MInstrEffects::supported(Opcode op) {
  return opcodeHasFlags(op, MInstrProp | MInstrElem);
}
bool MInstrEffects::supported(const IRInstruction* inst) {
  return supported(inst->op());
}

void MInstrEffects::get(const IRInstruction* inst, LocalStateHook& hook) {
  // If the base for this instruction is a local address, the helper call might
  // have side effects on the local's value
  SSATmp* base = inst->src(minstrBaseIdx(inst));
  IRInstruction* locInstr = base->inst();
  if (locInstr->op() == LdLocAddr) {
    UNUSED Type baseType = locInstr->dst()->type();
    assert(baseType.equals(base->type()));
    assert(baseType.isPtr() || baseType.isKnownDataType());
    int loc = locInstr->extra<LdLocAddr>()->locId;

    MInstrEffects effects(inst);
    if (effects.baseTypeChanged || effects.baseValChanged) {
      hook.setLocalType(loc, effects.baseType.derefIfPtr());
    }
  }
}

MInstrEffects::MInstrEffects(const IRInstruction* inst) {
  init(inst->op(), inst->src(minstrBaseIdx(inst))->type());
}

MInstrEffects::MInstrEffects(Opcode op, Type base) {
  init(op, base);
}

MInstrEffects::MInstrEffects(Opcode op, SSATmp* base) {
  auto typeOrNone =
    [](SSATmp* val){ return val ? val->type() : Type::None; };
  init(op, typeOrNone(base));
}

MInstrEffects::MInstrEffects(Opcode opc, const std::vector<SSATmp*>& srcs) {
  init(opc, srcs[minstrBaseIdx(opc)]->type());
}

namespace {
Opcode canonicalOp(Opcode op) {
  if (op == ElemUX || op == ElemUXStk ||
      op == UnsetElem || op == UnsetElemStk) {
    return UnsetElem;
  }
  if (op == SetWithRefElem ||
      op == SetWithRefElemStk ||
      op == SetWithRefNewElem ||
      op == SetWithRefNewElemStk) {
    return SetWithRefElem;
  }
  return opcodeHasFlags(op, MInstrProp) ? SetProp
       : opcodeHasFlags(op, MInstrElem) || op == ArraySet ? SetElem
       : bad_value<Opcode>();
}

void getBaseType(Opcode rawOp, bool predict,
                 Type& baseType, bool& baseValChanged) {
  always_assert(baseType.notBoxed());
  auto const op = canonicalOp(rawOp);

  // Deal with possible promotion to stdClass or array
  if ((op == SetElem || op == SetProp || op == SetWithRefElem) &&
      baseType.maybe(Type::Null | Type::Bool | Type::Str)) {
    auto newBase = op == SetProp ? Type::Obj : Type::Arr;

    if (predict) {
      /* If the output type will be used as a prediction and not as fact, we
       * can be optimistic here. Assume no promotion for string bases and
       * promotion in other cases. */
      baseType = baseType.isString() ? Type::Str : newBase;
    } else if (baseType.isString() &&
               (rawOp == SetElem || rawOp == SetElemStk)) {
      /* If the base is known to be a string and the operation is exactly
       * SetElem, we're guaranteed that either the base will end as a
       * CountedStr or the instruction will throw an exception and side
       * exit. */
      baseType = Type::CountedStr;
    } else if (baseType.isString() &&
               (rawOp == SetNewElem || rawOp == SetNewElemStk)) {
      /* If the string base is empty, it will be promoted to an
       * array. Otherwise the base will be left alone and we'll fatal. */
      baseType = Type::Arr;
    } else {
      /* Regardless of whether or not promotion happens, we know the base
       * cannot be Null after the operation. If the base was a subtype of Null
       * this will give newBase. */
      baseType = (baseType - Type::Null) | newBase;
    }

    baseValChanged = true;
  }

  if ((op == SetElem || op == UnsetElem || op == SetWithRefElem) &&
      baseType.maybe(Type::Arr | Type::Str)) {
    /* Modifying an array or string element, even when COW doesn't kick in,
     * produces a new SSATmp for the base. StaticArr/StaticStr may be promoted
     * to CountedArr/CountedStr. */
    baseValChanged = true;
    if (baseType.maybe(Type::StaticArr)) baseType |= Type::CountedArr;
    if (baseType.maybe(Type::StaticStr)) baseType |= Type::CountedStr;
  }
}
}

void MInstrEffects::init(const Opcode rawOp, const Type origBase) {
  baseType = origBase;
  always_assert(baseType.isPtr() ^ baseType.notPtr());
  auto const basePtr = baseType.isPtr();
  baseType = baseType.derefIfPtr();

  // Only certain types of bases are supported now but this list may expand in
  // the future.
  assert_not_implemented(basePtr ||
                         baseType.subtypeOfAny(Type::Obj, Type::Arr));

  baseTypeChanged = baseValChanged = false;

  // Process the inner and outer types separately and then recombine them,
  // since the minstr operations all operate on the inner cell of boxed
  // bases. We treat the new inner type as a prediction because it will be
  // verified the next time we load from the box.
  auto inner = (baseType & Type::BoxedCell).innerType();
  auto outer = baseType & Type::Cell;
  getBaseType(rawOp, false, outer, baseValChanged);
  getBaseType(rawOp, true, inner, baseValChanged);

  baseType = inner.box() | outer;
  baseType = basePtr ? baseType.ptr() : baseType;

  baseTypeChanged = baseType != origBase;

  /* Boxed bases may have their inner value changed but the value of the box
   * will never change. */
  baseValChanged = !origBase.isBoxed() && (baseValChanged || baseTypeChanged);
}

// minstrBaseIdx returns the src index for inst's base operand.
int minstrBaseIdx(Opcode opc) {
  return opc == SetNewElem || opc == SetNewElemStk ? 0
         : opc == SetNewElemArray || opc == SetNewElemArrayStk ? 0
         : opc == BindNewElem || opc == BindNewElemStk ? 0
         : opc == ArraySet ? 1
         : opc == SetOpProp || opc == SetOpPropStk ? 1
         : opcodeHasFlags(opc, MInstrProp) ? 2
         : opcodeHasFlags(opc, MInstrElem) ? 1
         : bad_value<int>();
}
int minstrBaseIdx(const IRInstruction* inst) {
  return minstrBaseIdx(inst->op());
}

// minstrKeyIdx returns the src index for inst's key operand.
int minstrKeyIdx(Opcode opc) {
  return opc == SetNewElem || opc == SetNewElemStk ? -1
         : opc == SetNewElemArray || opc == SetNewElemArrayStk ? -1
         : opc == SetWithRefNewElem || opc == SetWithRefNewElemStk ? -1
         : opc == BindNewElem || opc == BindNewElem ? -1
         : opc == ArraySet ? 2
         : opc == SetOpProp || opc == SetOpPropStk ? 2
         : opcodeHasFlags(opc, MInstrProp) ? 3
         : opcodeHasFlags(opc, MInstrElem) ? 2
         : bad_value<int>();
}
int minstrKeyIdx(const IRInstruction* inst) {
  return minstrKeyIdx(inst->op());
}

// minstrValueIdx returns the src index for inst's value operand.
int minstrValueIdx(Opcode opc) {
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
    case SetNewElemArray: case SetNewElemArrayStk: return 1;
    case SetWithRefNewElem: case SetWithRefNewElemStk: return 2;
    case BindNewElem: case BindNewElemStk: return 1;
    case SetOpProp: case SetOpPropStk: return 3;

    default:
      return opcodeHasFlags(opc, MInstrProp) ? 4
           : opcodeHasFlags(opc, MInstrElem) ? 3
           : bad_value<int>();
  }
}
int minstrValueIdx(const IRInstruction* inst) {
  return minstrValueIdx(inst->op());
}

HhbcTranslator::MInstrTranslator::MInstrTranslator(
  const NormalizedInstruction& ni,
  HhbcTranslator& ht)
    : m_ni(ni)
    , m_ht(ht)
    , m_tb(*m_ht.m_tb)
    , m_irf(m_ht.m_unit)
    , m_mii(getMInstrInfo(ni.mInstrOp()))
    , m_marker(ht.makeMarker(ht.bcOff()))
    , m_iInd(m_mii.valCount())
    , m_needMIS(true)
    , m_misBase(nullptr)
    , m_base(nullptr)
    , m_result(nullptr)
    , m_strTestResult(nullptr)
    , m_failedSetBlock(nullptr)
{
}

template<typename... Srcs>
SSATmp* HhbcTranslator::MInstrTranslator::genStk(Opcode opc, Block* taken,
                                                 Srcs... srcs) {
  assert(opcodeHasFlags(opc, HasStackVersion));
  assert(!opcodeHasFlags(opc, ModifiesStack));

  // We're going to make decisions based on the type of the base.
  constrainBase(DataTypeSpecific);

  std::vector<SSATmp*> srcVec({srcs...});
  SSATmp* base = srcVec[minstrBaseIdx(opc)];

  /* If the base is a pointer to a stack cell and the operation might change
   * its type and/or value, use the version of the opcode that returns a new
   * StkPtr. */
  if (base->inst()->op() == LdStackAddr) {
    MInstrEffects effects(opc, srcVec);
    if (effects.baseTypeChanged || effects.baseValChanged) {
      opc = getStackModifyingOpcode(opc);
    }
  }

  return gen(opc, taken, srcs...);
}

void HhbcTranslator::MInstrTranslator::emit() {
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
SSATmp* HhbcTranslator::MInstrTranslator::genMisPtr() {
  if (m_needMIS) {
    return gen(LdAddr, m_misBase, cns(X64::kReservedRSPSpillSpace));
  } else {
    return gen(DefConst, Type::PtrToCell, ConstData(nullptr));
  }
}

// Inspect the instruction we're about to translate and determine if
// it can be executed without using an MInstrState struct.
void HhbcTranslator::MInstrTranslator::checkMIState() {
  if (m_ni.immVec.locationCode() == LNL || m_ni.immVec.locationCode() == LNC) {
    // We're definitely going to punt in emitBaseN, so we might not
    // have guarded the base's type.
    return;
  }

  // DataTypeGeneric is used here because if we can't prove anything useful
  // about the operation, this function doesn't care what the type is.
  auto baseVal = getBase(DataTypeGeneric);
  Type baseType = baseVal->type();
  const bool isCGetM = m_ni.mInstrOp() == OpCGetM;
  const bool isSetM = m_ni.mInstrOp() == OpSetM;
  const bool isIssetM = m_ni.mInstrOp() == OpIssetM;
  const bool isUnsetM = m_ni.mInstrOp() == OpUnsetM;
  const bool isSingle = m_ni.immVecM.size() == 1;

  if (baseType.maybeBoxed() && !baseType.isBoxed()) {
    // We don't need to bother with weird base types.
    return;
  }
  baseType = baseType.unbox();

  // CGetM or SetM with no unknown property offsets
  const bool simpleProp = !mInstrHasUnknownOffsets(m_ni, contextClass()) &&
    (isCGetM || isSetM);

  // SetM with only one element
  const bool singlePropSet = isSingle && isSetM &&
    mcodeMaybePropName(m_ni.immVecM[0]);

  // Array access with one element in the vector
  const bool singleElem = isSingle && mcodeMaybeArrayOrMapKey(m_ni.immVecM[0]);

  // SetM with one vector array element
  const bool simpleArraySet = isSetM && singleElem;

  // IssetM with one vector array element and an Arr base
  const bool simpleArrayIsset = isIssetM && singleElem &&
    baseType <= Type::Arr;
  // IssetM with one vector array element and a collection type
  const bool simpleCollectionIsset = isIssetM && singleElem &&
    baseType.strictSubtypeOf(Type::Obj) &&
    isOptimizableCollectionClass(baseType.getClass());

  // UnsetM on an array with one vector element
  const bool simpleArrayUnset = isUnsetM && singleElem;

  // UnsetM on a non-standard base. Always a noop or fatal.
  const bool badUnset = isUnsetM && baseType.not(Type::Arr | Type::Obj);

  // CGetM on an array with a base that won't use MInstrState. Str
  // will use tvScratch and Obj will fatal or use tvRef.
  const bool simpleArrayGet = isCGetM && singleElem &&
    baseType.not(Type::Str | Type::Obj);
  const bool simpleCollectionGet = isCGetM && singleElem &&
    baseType.strictSubtypeOf(Type::Obj) &&
    isOptimizableCollectionClass(baseType.getClass());
  const bool simpleStringOp = (isCGetM || isIssetM) && isSingle &&
    isSimpleBase() && mcodeMaybeArrayIntKey(m_ni.immVecM[0]) &&
    baseType <= Type::Str;

  if (simpleProp || singlePropSet ||
      simpleArraySet || simpleArrayGet || simpleCollectionGet ||
      simpleArrayUnset || badUnset || simpleCollectionIsset ||
      simpleArrayIsset || simpleStringOp) {
    setNoMIState();
    if (simpleCollectionGet || simpleCollectionIsset) {
      constrainBase(DataTypeSpecialized, baseVal);
    } else {
      constrainBase(DataTypeSpecific, baseVal);
    }
  }
}

void HhbcTranslator::MInstrTranslator::emitMPre() {
  checkMIState();

  if (HPHP::Trace::moduleEnabled(HPHP::Trace::minstr, 1)) {
    emitMTrace();
  }

  if (m_needMIS) {
    m_misBase = gen(DefMIStateBase);
    SSATmp* uninit = m_tb.genDefUninit();

    if (nLogicalRatchets() > 0) {
      gen(StMem, m_misBase, cns(MISOFF(tvRef)), uninit);
      gen(StMem, m_misBase, cns(MISOFF(tvRef2)), uninit);
    }
  }

  // The base location is input 0 or 1, and the location code is stored
  // separately from m_ni.immVecM, so input indices (iInd) and member indices
  // (mInd) commonly differ.  Additionally, W members have no corresponding
  // inputs, so it is necessary to track the two indices separately.
  emitBaseOp();
  ++m_iInd;

  // Iterate over all but the last member, which is consumed by a final
  // operation.
  for (m_mInd = 0; m_mInd < m_ni.immVecM.size() - 1; ++m_mInd) {
    emitIntermediateOp();
    emitRatchetRefs();
  }
}

void HhbcTranslator::MInstrTranslator::emitMTrace() {
  auto rttStr = [this](int i) {
    return Type(m_ni.inputs[i]->rtt).unbox().toString();
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
    } else if (mcodeMaybeArrayOrMapKey(mcode)) {
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
      cns(makeStaticString("vector instructions")),
      cns(makeStaticString(shape.str())),
      cns(1));
}

// Build a map from (stack) input index to stack index.
void HhbcTranslator::MInstrTranslator::numberStackInputs() {
  // Stack inputs are pushed in the order they appear in the vector
  // from left to right, so earlier elements in the vector are at
  // higher offsets in the stack. m_mii.valCount() tells us how many
  // rvals the instruction takes on the stack; they're pushed after
  // any vector elements and we want to ignore them here.
  bool stackRhs = m_mii.valCount() &&
    m_ni.inputs[0]->location.space == Location::Stack;
  int stackIdx = (int)stackRhs + m_ni.immVec.numStackValues() - 1;
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
  assert(stackIdx == (stackRhs ? 0 : -1));

  if (stackRhs) {
    // If this instruction does have an RHS, it will be input 0 at
    // stack offset 0.
    assert(m_mii.valCount() == 1);
    m_stackInputs[0] = 0;
  }
}

SSATmp* HhbcTranslator::MInstrTranslator::getBase(TypeConstraint tc) {
  assert(m_iInd == m_mii.valCount());
  return getInput(m_iInd, tc);
}

SSATmp* HhbcTranslator::MInstrTranslator::getKey() {
  SSATmp* key = getInput(m_iInd, DataTypeSpecific);
  auto keyType = key->type();
  assert(keyType.isBoxed() || keyType.notBoxed());
  if (keyType.isBoxed()) {
    key = gen(LdRef, Type::Cell, key);
  }
  return key;
}

SSATmp* HhbcTranslator::MInstrTranslator::getValue() {
  // If an instruction takes an rhs, it's always input 0.
  assert(m_mii.valCount() == 1);
  const int kValIdx = 0;
  return getInput(kValIdx, DataTypeSpecific);
}

SSATmp* HhbcTranslator::MInstrTranslator::getValAddr() {
  assert(m_mii.valCount() == 1);
  const DynLocation& dl = *m_ni.inputs[0];
  const Location& l = dl.location;
  if (l.space == Location::Local) {
    assert(!mapContains(m_stackInputs, 0));
    return m_ht.ldLocAddr(l.offset, DataTypeGeneric); // teleported to container
  } else {
    assert(l.space == Location::Stack);
    assert(mapContains(m_stackInputs, 0));
    m_ht.spillStack();
    return m_ht.ldStackAddr(m_stackInputs[0],
                            DataTypeGeneric); // teleported to container
  }
}

void HhbcTranslator::MInstrTranslator::constrainBase(TypeConstraint tc,
                                                     SSATmp* value) {
  if (!value) value = m_base;

  // Lots of operations change their behavior based on the value type of the
  // base, so this handles the logic of using the inner constraint when
  // appropriate.
  auto baseType = value->type().derefIfPtr();
  assert(baseType == Type::Gen || baseType.isBoxed() || baseType.notBoxed());
  m_tb.constrainValue(value, tc);
  if (baseType.isBoxed()) {
    tc.innerCat = tc.category;
    m_tb.constrainValue(value, tc);
  }
}

SSATmp* HhbcTranslator::MInstrTranslator::getInput(unsigned i,
                                                   TypeConstraint tc) {
  const DynLocation& dl = *m_ni.inputs[i];
  const Location& l = dl.location;

  assert(mapContains(m_stackInputs, i) == (l.space == Location::Stack));
  switch (l.space) {
    case Location::Stack:
      return m_ht.top(Type::StackElem, m_stackInputs[i], tc);

    case Location::Local:
      return m_ht.ldLoc(l.offset, tc);

    case Location::Litstr:
      return cns(m_ht.lookupStringId(l.offset));

    case Location::Litint:
      return cns(l.offset);

    case Location::This:
      // If we don't have a current class context, this instruction will be
      // unreachable.
      if (!m_ht.curClass()) PUNT(Unreachable-LdThis);

      return gen(LdThis, m_tb.fp());

    default: not_reached();
  }
}

void HhbcTranslator::MInstrTranslator::emitBaseLCR() {
  const MInstrAttr& mia = m_mii.getAttr(m_ni.immVec.locationCode());
  const DynLocation& base = *m_ni.inputs[m_iInd];
  // We use DataTypeGeneric here because we might not care about the type. If
  // we do, it's constrained further.
  auto baseType = getBase(DataTypeGeneric)->type();

  if (base.location.isLocal()) {
    // Check for Uninit and warn/promote to InitNull as appropriate
    if (baseType <= Type::Uninit) {
      if (mia & MIA_warn) {
        gen(RaiseUninitLoc, makeEmptyCatch(),
            cns(m_ht.curFunc()->localVarName(base.location.offset)));
      }
      if (mia & MIA_define) {
        // We care whether or not the local is Uninit, and
        // CountnessInit will tell us that.
        m_tb.constrainLocal(base.location.offset, DataTypeCountnessInit,
                            "emitBaseLCR: Uninit base local");
        gen(
          StLoc,
          LocalId(base.location.offset),
          m_tb.fp(),
          m_tb.genDefInitNull()
        );
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
  Block* failedRef = baseType.isBoxed() ? m_ht.makeExit() : nullptr;
  if ((baseType.subtypeOfAny(Type::Obj, Type::BoxedObj) &&
       mcodeMaybePropName(m_ni.immVecM[0])) ||
      simpleCollectionOp() != SimpleOp::None) {
    // In these cases we can pass the base by value, after unboxing if needed.
    m_base = gen(Unbox, failedRef, getBase(DataTypeSpecific));

    // Register that we care about the specific type of the base, and might
    // care about its specialized type.
    constrainBase(DataTypeSpecific);
    constrainCollectionOpBase();
  } else {
    // Everything else is passed by pointer. We don't have to worry about
    // unboxing here, since all the generic helpers understand boxed bases.
    if (baseType.isBoxed()) {
      SSATmp* box = getBase(DataTypeSpecific);
      assert(box->isA(Type::BoxedCell));
      assert(baseType.innerType().isKnownDataType() ||
             baseType.innerType().equals(Type::InitCell));

      // Guard that the inner type hasn't changed. Even though we don't use the
      // unboxed value, some emit* methods change their behavior based on the
      // inner type.
      auto inner = gen(LdRef, baseType.innerType(), failedRef, box);

      // TODO(t2598894): We do this for consistency with the old guard
      // relaxation code, but may change it in the future.
      m_tb.constrainValue(inner, DataTypeSpecific);
    }

    if (base.location.space == Location::Local) {
      m_base = m_ht.ldLocAddr(base.location.offset, DataTypeGeneric);
    } else {
      assert(base.location.space == Location::Stack);
      // Make sure the stack is clean before getting a pointer to one of its
      // elements.
      m_ht.spillStack();
      assert(m_stackInputs.count(m_iInd));
      m_base = m_ht.ldStackAddr(m_stackInputs[m_iInd], DataTypeGeneric);
    }
    assert(m_base->type().isPtr());
  }

  // TODO(t2598894): We do this for consistency with the old guard relaxation
  // code, but may change it in the future.
  constrainBase(DataTypeSpecific);
}

// Is the current instruction a 1-element simple collection (includes Array),
// operation?
HhbcTranslator::MInstrTranslator::SimpleOp
HhbcTranslator::MInstrTranslator::simpleCollectionOp() {
  // DataTypeGeneric is used in here to avoid constraining the base in case we
  // end up not caring about the type. Consumers of the return value must
  // constrain the base as appropriate.

  SSATmp* base = getInput(m_mii.valCount(), DataTypeGeneric);
  auto baseType = base->type().unbox();
  HPHP::Op op = m_ni.mInstrOp();
  bool readInst = (op == OpCGetM || op == OpIssetM);
  if ((op == OpSetM || readInst) &&
      isSimpleBase() &&
      isSingleMember()) {

    if (baseType <= Type::Arr) {
      auto const isPacked = (baseType.hasArrayKind() &&
                            baseType.getArrayKind() == ArrayData::kPackedKind);
      if (mcodeMaybeArrayOrMapKey(m_ni.immVecM[0])) {
        SSATmp* key = getInput(m_mii.valCount() + 1, DataTypeGeneric);
        if (key->isA(Type::Int) || key->isA(Type::Str)) {
          if (isPacked && readInst && key->isA(Type::Int)) {
            return SimpleOp::PackedArray;
          }
          return SimpleOp::Array;
        }
      }
    } else if (baseType <= Type::Str &&
               mcodeMaybeArrayIntKey(m_ni.immVecM[0])) {
      SSATmp* key = getInput(m_mii.valCount() + 1, DataTypeGeneric);
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
      auto const isStableMap = klass == c_StableMap::classof();

      if (isVector || isPair) {
        if (mcodeMaybeVectorKey(m_ni.immVecM[0])) {
          SSATmp* key = getInput(m_mii.valCount() + 1, DataTypeGeneric);
          if (key->isA(Type::Int)) {
            // We don't specialize setting pair elements.
            if (isPair && op == OpSetM) return SimpleOp::None;

            return isVector ? SimpleOp::Vector : SimpleOp::Pair;
          }
        }
      } else if (isMap || isStableMap) {
        if (mcodeMaybeArrayOrMapKey(m_ni.immVecM[0])) {
          SSATmp* key = getInput(m_mii.valCount() + 1, DataTypeGeneric);
          if (key->isA(Type::Int) || key->isA(Type::Str)) {
            return isMap ? SimpleOp::Map : SimpleOp::StableMap;
          }
        }
      }
    }
  }

  return SimpleOp::None;
}

void HhbcTranslator::MInstrTranslator::constrainCollectionOpBase() {
  auto type = simpleCollectionOp();
  switch (type) {
    case SimpleOp::None:
      return;

    case SimpleOp::Array:
    case SimpleOp::String:
      m_tb.constrainValue(m_base, DataTypeSpecific);
      return;

    case SimpleOp::PackedArray:
    case SimpleOp::Vector:
    case SimpleOp::Map:
    case SimpleOp::StableMap:
    case SimpleOp::Pair:
      constrainBase(DataTypeSpecialized);
      return;
  }
}

// "Simple" bases are stack cells and locals.
bool HhbcTranslator::MInstrTranslator::isSimpleBase() {
  LocationCode loc = m_ni.immVec.locationCode();
  return loc == LL || loc == LC || loc == LR;
}

bool HhbcTranslator::MInstrTranslator::isSingleMember() {
  return m_ni.immVecM.size() == 1;
}

void HhbcTranslator::MInstrTranslator::emitBaseH() {
  m_base = gen(LdThis, m_tb.fp());
}

void HhbcTranslator::MInstrTranslator::emitBaseN() {
  // If this is ever implemented, the check at the beginning of
  // checkMIState must be removed/adjusted as appropriate.
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

namespace MInstrHelpers {
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

void HhbcTranslator::MInstrTranslator::emitBaseG() {
  const MInstrAttr& mia = m_mii.getAttr(m_ni.immVec.locationCode());
  typedef TypedValue* (*OpFunc)(TypedValue, MInstrState*);
  using namespace MInstrHelpers;
  static const OpFunc opFuncs[] = {baseG, baseGW, baseGD, baseGWD};
  OpFunc opFunc = opFuncs[mia & MIA_base];
  SSATmp* gblName = getBase(DataTypeSpecific);
  if (!gblName->isA(Type::Str)) PUNT(BaseG-non-string-name);

  m_base = gen(BaseG,
               makeEmptyCatch(),
               cns(reinterpret_cast<TCA>(opFunc)),
               gblName,
               genMisPtr());
}

void HhbcTranslator::MInstrTranslator::emitBaseS() {
  const int kClassIdx = m_ni.inputs.size() - 1;
  SSATmp* key = getKey();
  SSATmp* clsRef = getInput(kClassIdx, DataTypeGeneric /* will be a Cls */);
  m_base = gen(LdClsPropAddr, makeCatch(), clsRef, key, CTX());
}

void HhbcTranslator::MInstrTranslator::emitBaseOp() {
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

void HhbcTranslator::MInstrTranslator::emitIntermediateOp() {
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


void HhbcTranslator::MInstrTranslator::emitProp() {
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
namespace MInstrHelpers {
HELPER_TABLE(PROP)
}
#undef PROP

void HhbcTranslator::MInstrTranslator::emitPropGeneric() {
  MemberCode mCode = m_ni.immVecM[m_mInd];
  MInstrAttr mia = MInstrAttr(m_mii.getAttr(mCode) & MIA_intermediate_prop);

  if ((mia & Unset) && m_base->type().strip().not(Type::Obj)) {
    constrainBase(DataTypeSpecific);
    m_base = m_tb.genPtrToInitNull();
    return;
  }

  typedef TypedValue* (*OpFunc)(Class*, TypedValue*, TypedValue, MInstrState*);
  SSATmp* key = getKey();
  BUILD_OPTAB(mia, m_base->isA(Type::Obj));
  if (mia & Define) {
    m_base = genStk(PropDX, makeCatch(), cns((TCA)opFunc), CTX(),
                    m_base, key, genMisPtr());
  } else {
    m_base = gen(PropX, makeCatch(),
                 cns((TCA)opFunc), CTX(), m_base, key, genMisPtr());
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
SSATmp* HhbcTranslator::MInstrTranslator::checkInitProp(
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

  return m_tb.cond(
    [&] (Block* taken) {
      gen(CheckInitMem, taken, propAddr, cns(0));
    },
    [&] { // Next: Property isn't Uninit. Do nothing.
      return propAddr;
    },
    [&] { // Taken: Property is Uninit. Raise a warning and return
          // a pointer to InitNull, either in the object or
          // init_null_variant.
      m_tb.hint(Block::Hint::Unlikely);
      if (doWarn && wantPropSpecializedWarnings()) {
        gen(RaiseUndefProp, m_ht.makeCatch(), baseAsObj, key);
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

Class* HhbcTranslator::MInstrTranslator::contextClass() const {
  return m_ht.curFunc()->cls();
}

void HhbcTranslator::MInstrTranslator::emitPropSpecialized(const MInstrAttr mia,
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
    m_base = m_tb.cond(
      [&] (Block* taken) {
        return gen(LdMem, Type::Obj, taken, m_base, cns(0));
      },
      [&] (SSATmp* baseAsObj) {
        // Next: Base is an object. Load property address and check for uninit
        return checkInitProp(baseAsObj,
                             gen(LdPropAddr, baseAsObj,
                                 cns(propInfo.offset)),
                             propInfo,
                             doWarn,
                             doDefine);
      },
      [&] { // Taken: Base is Null. Raise warnings/errors and return InitNull.
        m_tb.hint(Block::Hint::Unlikely);
        if (doWarn) {
          gen(WarnNonObjProp, makeCatch());
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
          gen(ThrowNonObjProp, makeCatch());
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

#define HELPER_TABLE(m)                          \
  /* name      keyType         attrs  */         \
  m(elemC,     KeyType::Any,   None)             \
  m(elemCD,    KeyType::Any,   Define)           \
  m(elemCDR,   KeyType::Any,   DefineReffy)      \
  m(elemCU,    KeyType::Any,   Unset)            \
  m(elemCW,    KeyType::Any,   Warn)             \
  m(elemCWD,   KeyType::Any,   WarnDefine)       \
  m(elemCWDR,  KeyType::Any,   WarnDefineReffy)  \
  m(elemI,     KeyType::Int,   None)             \
  m(elemID,    KeyType::Int,   Define)           \
  m(elemIDR,   KeyType::Int,   DefineReffy)      \
  m(elemIU,    KeyType::Int,   Unset)            \
  m(elemIW,    KeyType::Int,   Warn)             \
  m(elemIWD,   KeyType::Int,   WarnDefine)       \
  m(elemIWDR,  KeyType::Int,   WarnDefineReffy)  \
  m(elemS,     KeyType::Str,   None)             \
  m(elemSD,    KeyType::Str,   Define)           \
  m(elemSDR,   KeyType::Str,   DefineReffy)      \
  m(elemSU,    KeyType::Str,   Unset)            \
  m(elemSW,    KeyType::Str,   Warn)             \
  m(elemSWD,   KeyType::Str,   WarnDefine)       \
  m(elemSWDR,  KeyType::Str,   WarnDefineReffy)

#define ELEM(nm, keyType, attrs)                                        \
TypedValue* nm(TypedValue* base, TypedValue key, MInstrState* mis) {    \
  return elemImpl<keyType, WDRU(attrs)>(base, key, mis);                \
}
namespace MInstrHelpers {
HELPER_TABLE(ELEM)
}
#undef ELEM

void HhbcTranslator::MInstrTranslator::emitElem() {
  MemberCode mCode = m_ni.immVecM[m_mInd];
  MInstrAttr mia = MInstrAttr(m_mii.getAttr(mCode) & MIA_intermediate);
  SSATmp* key = getKey();

  // Fast path for the common/easy case
  const bool warn = mia & Warn;
  const bool unset = mia & Unset;
  const bool define = mia & Define;
  if (m_base->isA(Type::PtrToArr) &&
      !unset && !define &&
      (key->isA(Type::Int) || key->isA(Type::Str))) {
    constrainBase(DataTypeSpecific);
    emitElemArray(key, warn);
    return;
  }

  assert(!(define && unset));
  if (unset) {
    SSATmp* uninit = m_tb.genPtrToUninit();
    Type baseType = m_base->type().strip();
    constrainBase(DataTypeSpecific);
    if (baseType <= Type::Str) {
      m_ht.exceptionBarrier();
      gen(
        RaiseError,
        cns(makeStaticString(Strings::OP_NOT_SUPPORTED_STRING))
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
  BUILD_OPTAB(getKeyType(key), mia);
  if (define || unset) {
    m_base = genStk(define ? ElemDX : ElemUX, makeCatch(),
                    cns((TCA)opFunc), m_base, key, genMisPtr());
  } else {
    m_base = gen(ElemX, makeCatch(),
                 cns((TCA)opFunc), m_base, key, genMisPtr());
  }
}
#undef HELPER_TABLE

template<bool warn>
NEVER_INLINE
static TypedValue* elemArrayNotFound(int64_t k) {
  if (warn) {
    raise_notice("Undefined index: %" PRId64, k);
  }
  return (TypedValue*)&null_variant;
}

template<bool warn>
NEVER_INLINE
static TypedValue* elemArrayNotFound(const StringData* k) {
  if (warn) {
    raise_notice("Undefined index: %s", k->data());
  }
  return (TypedValue*)&null_variant;
}

static inline TypedValue* checkedGet(ArrayData* a, StringData* key) {
  int64_t i;
  return UNLIKELY(key->isStrictlyInteger(i)) ? a->nvGet(i) : a->nvGet(key);
}

static inline TypedValue* checkedGet(ArrayData* a, int64_t key) {
  not_reached();
}

template<KeyType keyType, bool checkForInt, bool warn>
static inline TypedValue* elemArrayImpl(
  TypedValue* a, typename KeyTypeTraits<keyType>::rawType key) {
  assert(a->m_type == KindOfArray);
  ArrayData* ad = a->m_data.parr;
  TypedValue* ret = checkForInt ? checkedGet(ad, key)
                                : ad->nvGet(key);
  return ret ? ret : elemArrayNotFound<warn>(key);
}

#define HELPER_TABLE(m)                                 \
  /* name               keyType  checkForInt   warn */  \
  m(elemArrayS,    KeyType::Str,       false, false)    \
  m(elemArraySi,   KeyType::Str,        true, false)    \
  m(elemArrayI,    KeyType::Int,       false, false)    \
  m(elemArraySW,   KeyType::Str,       false,  true)    \
  m(elemArraySiW,  KeyType::Str,        true,  true)    \
  m(elemArrayIW,   KeyType::Int,       false,  true)

#define ELEM(nm, keyType, checkForInt, warn)            \
  TypedValue* nm(TypedValue* a, TypedValue* key) {      \
    return elemArrayImpl<keyType, checkForInt, warn>(   \
      a, keyAsRaw<keyType>(key));                       \
  }
namespace MInstrHelpers {
HELPER_TABLE(ELEM)
}
#undef ELEM

void HhbcTranslator::MInstrTranslator::emitElemArray(SSATmp* key, bool warn) {
  KeyType keyType;
  bool checkForInt;
  m_ht.checkStrictlyInteger(key, keyType, checkForInt);

  typedef TypedValue* (*OpFunc)(ArrayData*, TypedValue*);
  BUILD_OPTAB(keyType, checkForInt, warn);
  m_base = gen(ElemArray, makeCatch(), cns((TCA)opFunc), m_base, key);
}
#undef HELPER_TABLE

void HhbcTranslator::MInstrTranslator::emitNewElem() {
  PUNT(emitNewElem);
}

void HhbcTranslator::MInstrTranslator::emitRatchetRefs() {
  if (ratchetInd() < 0 || ratchetInd() >= int(nLogicalRatchets())) {
    return;
  }

  m_base = m_tb.cond(
    [&] (Block* taken) {
      gen(CheckInitMem, taken, m_misBase, cns(MISOFF(tvRef)));
    },
    [&] { // Next: tvRef isn't Uninit. Ratchet the refs
      // Clean up tvRef2 before overwriting it.
      if (ratchetInd() > 0) {
        gen(DecRefMem, Type::Gen, m_misBase, cns(MISOFF(tvRef2)));
      }
      // Copy tvRef to tvRef2. Use mmx at some point
      SSATmp* tvRef = gen(
        LdMem, Type::Gen, m_misBase, cns(MISOFF(tvRef))
      );
      gen(StMem, m_misBase, cns(MISOFF(tvRef2)), tvRef);

      // Reset tvRef.
      gen(StMem, m_misBase, cns(MISOFF(tvRef)), m_tb.genDefUninit());

      // Adjust base pointer.
      assert(m_base->type().isPtr());
      return gen(LdAddr, m_misBase, cns(MISOFF(tvRef2)));
    },
    [&] { // Taken: tvRef is Uninit. Do nothing.
      return m_base;
    }
  );
}

void HhbcTranslator::MInstrTranslator::emitFinalMOp() {
  typedef void (HhbcTranslator::MInstrTranslator::*MemFun)();

  switch (m_ni.immVecM[m_mInd]) {
  case MEC: case MEL: case MET: case MEI:
    static MemFun elemOps[] = {
#   define MII(instr, ...) &HhbcTranslator::MInstrTranslator::emit##instr##Elem,
    MINSTRS
#   undef MII
    };
    (this->*elemOps[m_mii.instr()])();
    break;

  case MPC: case MPL: case MPT:
    static MemFun propOps[] = {
#   define MII(instr, ...) &HhbcTranslator::MInstrTranslator::emit##instr##Prop,
    MINSTRS
#   undef MII
    };
    (this->*propOps[m_mii.instr()])();
    break;

  case MW:
    assert(m_mii.getAttr(MW) & MIA_final);
    static MemFun newOps[] = {
#   define MII(instr, attrs, bS, iS, vC, fN) \
      &HhbcTranslator::MInstrTranslator::emit##fN,
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
  TypedValue* key = keyPtr<keyType>(keyVal);
  TypedValue scratch;
  TypedValue* result = Prop<true, false, false, isObj, keyType>(
    scratch, mis->tvRef, ctx, base, key);

  if (result->m_type == KindOfRef) {
    result = result->m_data.pref->tv();
  }
  tvRefcountedIncRef(result);
  return *result;
}

#define HELPER_TABLE(m)                   \
  /* name         keyType       isObj */  \
  m(cGetPropC,    KeyType::Any, false)    \
  m(cGetPropCO,   KeyType::Any,  true)    \
  m(cGetPropS,    KeyType::Str, false)    \
  m(cGetPropSO,   KeyType::Str,  true)

#define PROP(nm, ...)                                              \
TypedValue nm(Class* ctx, TypedValue* base, TypedValue key,        \
                     MInstrState* mis) {                           \
  return cGetPropImpl<__VA_ARGS__>(ctx, base, key, mis);           \
}
namespace MInstrHelpers {
HELPER_TABLE(PROP)
}
#undef PROP

void HhbcTranslator::MInstrTranslator::emitCGetProp() {
  assert(!m_ni.outLocal);

  const Class* knownCls = nullptr;
  const auto propInfo   = getPropertyOffset(m_ni, contextClass(), knownCls,
                                            m_mii, m_mInd, m_iInd);
  if (propInfo.offset != -1) {
    emitPropSpecialized(MIA_warn, propInfo);
    SSATmp* cellPtr = gen(UnboxPtr, m_base);
    m_result = gen(LdMem, Type::Cell, cellPtr, cns(0));
    gen(IncRef, m_result);
    return;
  }

  typedef TypedValue (*OpFunc)(Class*, TypedValue*, TypedValue, MInstrState*);
  SSATmp* key = getKey();
  auto keyType = getKeyTypeNoInt(key);
  BUILD_OPTAB(keyType, m_base->isA(Type::Obj));
  m_result = gen(CGetProp, makeCatch(),
                 cns((TCA)opFunc), CTX(), m_base, key, genMisPtr());
}
#undef HELPER_TABLE

template <KeyType keyType, bool isObj>
static inline RefData* vGetPropImpl(Class* ctx, TypedValue* base,
                                    TypedValue keyVal, MInstrState* mis) {
  TypedValue* key = keyPtr<keyType>(keyVal);
  TypedValue* result = HPHP::Prop<false, true, false, isObj, keyType>(
    mis->tvScratch, mis->tvRef, ctx, base, key);

  if (result->m_type != KindOfRef) {
    tvBox(result);
  }
  RefData* ref = result->m_data.pref;
  ref->incRefCount();
  return ref;
}

#define HELPER_TABLE(m)                \
  /* name        keyType       isObj */\
  m(vGetPropC,   KeyType::Any, false)  \
  m(vGetPropCO,  KeyType::Any,  true)  \
  m(vGetPropS,   KeyType::Str, false)  \
  m(vGetPropSO,  KeyType::Str,  true)

#define PROP(nm, ...)                                              \
RefData* nm(Class* ctx, TypedValue* base, TypedValue key,          \
                     MInstrState* mis) {                           \
  return vGetPropImpl<__VA_ARGS__>(ctx, base, key, mis);           \
}
namespace MInstrHelpers {
HELPER_TABLE(PROP)
}
#undef PROP

void HhbcTranslator::MInstrTranslator::emitVGetProp() {
  SSATmp* key = getKey();
  typedef RefData* (*OpFunc)(Class*, TypedValue*, TypedValue, MInstrState*);
  BUILD_OPTAB(getKeyTypeNoInt(key), m_base->isA(Type::Obj));
  m_result = genStk(VGetProp, makeCatch(), cns((TCA)opFunc), CTX(),
                    m_base, key, genMisPtr());
}
#undef HELPER_TABLE

template <bool useEmpty, bool isObj>
static inline bool issetEmptyPropImpl(Class* ctx, TypedValue* base,
                                      TypedValue keyVal) {
  return HPHP::IssetEmptyProp<useEmpty, isObj>(ctx, base, &keyVal);
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
namespace MInstrHelpers {
HELPER_TABLE(ISSET)
}
#undef ISSET

void HhbcTranslator::MInstrTranslator::emitIssetEmptyProp(bool isEmpty) {
  SSATmp* key = getKey();
  typedef uint64_t (*OpFunc)(Class*, TypedValue*, TypedValue);
  BUILD_OPTAB(isEmpty, m_base->isA(Type::Obj));
  m_result = gen(isEmpty ? EmptyProp : IssetProp, makeCatch(),
                 cns((TCA)opFunc), CTX(), m_base, key);
}
#undef HELPER_TABLE

void HhbcTranslator::MInstrTranslator::emitIssetProp() {
  emitIssetEmptyProp(false);
}

void HhbcTranslator::MInstrTranslator::emitEmptyProp() {
  emitIssetEmptyProp(true);
}

template <bool isObj>
static inline void setPropImpl(Class* ctx, TypedValue* base,
                               TypedValue keyVal, Cell val) {
  HPHP::SetProp<false, isObj>(ctx, base, &keyVal, &val);
}

#define HELPER_TABLE(m)                     \
  /* name        isObj */                   \
  m(setPropC,    false)                     \
  m(setPropCO,    true)

#define PROP(nm, ...)                                                   \
void nm(Class* ctx, TypedValue* base, TypedValue key, Cell val) {       \
  setPropImpl<__VA_ARGS__>(ctx, base, key, val);                        \
}
namespace MInstrHelpers {
HELPER_TABLE(PROP)
}
#undef PROP

void HhbcTranslator::MInstrTranslator::emitSetProp() {
  SSATmp* value = getValue();

  /* If we know the class for the current base, emit a direct property set. */
  const Class* knownCls = nullptr;
  const auto propInfo   = getPropertyOffset(m_ni, contextClass(), knownCls,
                                            m_mii, m_mInd, m_iInd);
  if (propInfo.offset != -1) {
    emitPropSpecialized(MIA_define, propInfo);
    SSATmp* cellPtr = gen(UnboxPtr, m_base);
    SSATmp* oldVal = gen(LdMem, Type::Cell, cellPtr, cns(0));

    gen(IncRef, value);
    gen(StMem, cellPtr, cns(0), value);
    gen(DecRef, oldVal);
    m_result = value;
    return;
  }

  // Emit the appropriate helper call.
  typedef void (*OpFunc)(Class*, TypedValue*, TypedValue, Cell);
  SSATmp* key = getKey();
  BUILD_OPTAB(m_base->isA(Type::Obj));
  genStk(SetProp, makeCatchSet(), cns((TCA)opFunc), CTX(),
         m_base, key, value);
  m_result = value;
}
#undef HELPER_TABLE

template <bool isObj>
static inline TypedValue setOpPropImpl(TypedValue* base, TypedValue keyVal,
                                       Cell val, MInstrState* mis, SetOpOp op) {
  TypedValue* result = HPHP::SetOpProp<isObj>(
    mis->tvScratch, mis->tvRef, mis->ctx, op, base, &keyVal, &val);

  Cell ret;
  cellDup(*tvToCell(result), ret);
  return ret;
}

#define HELPER_TABLE(m)                                  \
  /* name         isObj */                               \
  m(setOpPropC,    false)                                \
  m(setOpPropCO,    true)

#define SETOP(nm, ...)                                                 \
TypedValue nm(TypedValue* base, TypedValue key,                        \
              Cell val, MInstrState* mis, SetOpOp op) {                \
  return setOpPropImpl<__VA_ARGS__>(base, key, val, mis, op);          \
}
namespace MInstrHelpers {
HELPER_TABLE(SETOP)
}
#undef SETOP

void HhbcTranslator::MInstrTranslator::emitSetOpProp() {
  SetOpOp op = SetOpOp(m_ni.imm[0].u_OA);
  SSATmp* key = getKey();
  SSATmp* value = getValue();
  typedef TypedValue (*OpFunc)(TypedValue*, TypedValue,
                               Cell, MInstrState*, SetOpOp);
  BUILD_OPTAB(m_base->isA(Type::Obj));
  m_tb.gen(StRaw, m_misBase, cns(RawMemSlot::MisCtx), CTX());
  m_result = genStk(SetOpProp, makeCatch(), cns((TCA)opFunc),
                    m_base, key, value, genMisPtr(), cns(op));
}
#undef HELPER_TABLE

template <bool isObj>
static inline TypedValue incDecPropImpl(TypedValue* base, TypedValue keyVal,
                                        MInstrState* mis, IncDecOp op) {
  TypedValue result;
  result.m_type = KindOfUninit;
  HPHP::IncDecProp<true, isObj>(
    mis->tvScratch, mis->tvRef, mis->ctx, op, base, &keyVal, result);
  assert(result.m_type != KindOfRef);
  return result;
}


#define HELPER_TABLE(m)                         \
  /* name         isObj */                      \
  m(incDecPropC,   false)                       \
  m(incDecPropCO,   true)

#define INCDEC(nm, ...)                                                 \
TypedValue nm(TypedValue* base, TypedValue key,                         \
              MInstrState* mis, IncDecOp op) {                          \
  return incDecPropImpl<__VA_ARGS__>(base, key, mis, op);               \
}
namespace MInstrHelpers {
HELPER_TABLE(INCDEC)
}
#undef INCDEC

void HhbcTranslator::MInstrTranslator::emitIncDecProp() {
  IncDecOp op = static_cast<IncDecOp>(m_ni.imm[0].u_OA);
  SSATmp* key = getKey();
  typedef TypedValue (*OpFunc)(TypedValue*, TypedValue,
                               MInstrState*, IncDecOp);
  BUILD_OPTAB(m_base->isA(Type::Obj));
  m_tb.gen(StRaw, m_misBase, cns(RawMemSlot::MisCtx), CTX());
  m_result = genStk(IncDecProp, makeCatch(), cns((TCA)opFunc),
                    m_base, key, genMisPtr(), cns(op));
}
#undef HELPER_TABLE

template <bool isObj>
static inline void bindPropImpl(Class* ctx, TypedValue* base, TypedValue keyVal,
                                RefData* val, MInstrState* mis) {
  TypedValue* prop = HPHP::Prop<false, true, false, isObj>(
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
namespace MInstrHelpers {
HELPER_TABLE(PROP)
}
#undef PROP

void HhbcTranslator::MInstrTranslator::emitBindProp() {
  SSATmp* key = getKey();
  SSATmp* box = getValue();
  typedef void (*OpFunc)(Class*, TypedValue*, TypedValue, RefData*,
                         MInstrState*);
  BUILD_OPTAB(m_base->isA(Type::Obj));
  genStk(BindProp, makeCatch(), cns((TCA)opFunc), CTX(),
         m_base, key, box, genMisPtr());
  m_result = box;
}
#undef HELPER_TABLE

template <bool isObj>
static inline void unsetPropImpl(Class* ctx, TypedValue* base,
                                 TypedValue keyVal) {
  HPHP::UnsetProp<isObj>(ctx, base, &keyVal);
}

#define HELPER_TABLE(m)            \
  /* name        isObj */          \
  m(unsetPropC,  false)            \
  m(unsetPropCO,  true)

#define PROP(nm, ...)                                                   \
  static void nm(Class* ctx, TypedValue* base, TypedValue key) {        \
    unsetPropImpl<__VA_ARGS__>(ctx, base, key);                         \
  }
namespace MInstrHelpers {
HELPER_TABLE(PROP)
}
#undef PROP

void HhbcTranslator::MInstrTranslator::emitUnsetProp() {
  SSATmp* key = getKey();

  if (m_base->type().strip().not(Type::Obj)) {
    // Noop
    constrainBase(DataTypeSpecific);
    return;
  }

  typedef void (*OpFunc)(Class*, TypedValue*, TypedValue);
  BUILD_OPTAB(m_base->isA(Type::Obj));
  gen(UnsetProp, makeCatch(), cns((TCA)opFunc), CTX(), m_base, key);
}
#undef HELPER_TABLE

// Keep these error handlers in sync with ArrayData::getNotFound();
NEVER_INLINE
static TypedValue arrayGetNotFound(int64_t k) {
  raise_notice("Undefined index: %" PRId64, k);
  TypedValue v;
  tvWriteNull(&v);
  return v;
}

NEVER_INLINE
static TypedValue arrayGetNotFound(const StringData* k) {
  raise_notice("Undefined index: %s", k->data());
  TypedValue v;
  tvWriteNull(&v);
  return v;
}

void HhbcTranslator::MInstrTranslator::emitPackedArrayGet(SSATmp* key) {
  assert(m_base->isA(Type::Arr) &&
         m_base->type().getArrayKind() == ArrayData::kPackedKind);
  m_result = m_tb.cond(
    [&] (Block* taken) {
      gen(CheckPackedArrayBounds, taken, m_base, key);
    },
    [&] { // Next:
      auto res = gen(LdPackedArrayElem, m_base, key);
      auto unboxed = gen(Unbox, res);
      gen(IncRef, unboxed);
      return unboxed;
    },
    [&] { // Taken:
      m_tb.hint(Block::Hint::Unlikely);
      gen(RaiseArrayIndexNotice, key);
      return m_tb.genDefInitNull();
    }
  );
}

template<KeyType keyType, bool checkForInt>
static inline TypedValue arrayGetImpl(
  ArrayData* a, typename KeyTypeTraits<keyType>::rawType key) {
  TypedValue* ret = checkForInt ? checkedGet(a, key)
                                : a->nvGet(key);
  if (ret) {
    ret = tvToCell(ret);
    tvRefcountedIncRef(ret);
    return *ret;
  }
  return arrayGetNotFound(key);
}

#define HELPER_TABLE(m)                    \
  /* name        keyType     checkForInt */\
  m(arrayGetS,   KeyType::Str,   false)    \
  m(arrayGetSi,  KeyType::Str,    true)    \
  m(arrayGetI,   KeyType::Int,   false)

#define ELEM(nm, keyType, checkForInt)                                  \
TypedValue nm(ArrayData* a, TypedValue* key) {                          \
  return arrayGetImpl<keyType, checkForInt>(a, keyAsRaw<keyType>(key)); \
}
namespace MInstrHelpers {
HELPER_TABLE(ELEM)
}
#undef ELEM

void HhbcTranslator::MInstrTranslator::emitArrayGet(SSATmp* key) {
  KeyType keyType;
  bool checkForInt;
  m_ht.checkStrictlyInteger(key, keyType, checkForInt);

  typedef TypedValue (*OpFunc)(ArrayData*, TypedValue*);
  BUILD_OPTAB(keyType, checkForInt);
  assert(m_base->isA(Type::Arr));
  m_result = gen(ArrayGet, makeCatch(), cns((TCA)opFunc), m_base, key);
}
#undef HELPER_TABLE

namespace MInstrHelpers {
StringData* stringGetI(StringData* str, uint64_t x) {
  if (LIKELY(x < str->size())) {
    return str->getChar(x);
  }
  if (RuntimeOption::EnableHipHopSyntax) {
    raise_warning("Out of bounds");
  }
  return makeStaticString("");
}
}

void HhbcTranslator::MInstrTranslator::emitStringGet(SSATmp* key) {
  assert(key->isA(Type::Int));
  m_result = gen(StringGet, makeCatch(),
                 cns((TCA)MInstrHelpers::stringGetI), m_base, key);
}

void HhbcTranslator::MInstrTranslator::emitVectorGet(SSATmp* key) {
  assert(key->isA(Type::Int));
  if (key->isConst() && key->getValInt() < 0) {
    PUNT(emitVectorGet);
  }
  SSATmp* size = gen(LdVectorSize, m_base);
  gen(CheckBounds, makeCatch(), key, size);
  SSATmp* base = gen(LdVectorBase, m_base);
  static_assert(sizeof(TypedValue) == 16,
                "TypedValue size expected to be 16 bytes");
  auto idx = gen(Shl, key, cns(4));
  m_result = gen(LdElem, base, idx);
  gen(IncRef, m_result);
}

void HhbcTranslator::MInstrTranslator::emitPairGet(SSATmp* key) {
  assert(key->isA(Type::Int));
  static_assert(sizeof(TypedValue) == 16,
                "TypedValue size expected to be 16 bytes");
  if (key->isConst()) {
    auto idx = key->getValInt();
    if (idx < 0 || idx > 1) {
      PUNT(emitPairGet);
    }
    // no reason to check bounds
    SSATmp* base = gen(LdPairBase, m_base);
    auto index = cns(key->getValInt() << 4);
    m_result = gen(LdElem, base, index);
  } else {
    gen(CheckBounds, makeCatch(), key, cns(1));
    SSATmp* base = gen(LdPairBase, m_base);
    auto idx = gen(Shl, key, cns(4));
    m_result = gen(LdElem, base, idx);
  }
  gen(IncRef, m_result);
}

template<KeyType keyType>
static inline TypedValue mapGetImpl(
    c_Map* map, typename KeyTypeTraits<keyType>::rawType key) {
  TypedValue* ret = map->at(key);
  tvRefcountedIncRef(ret);
  return *ret;
}

#define HELPER_TABLE(m)        \
  /* name      keyType  */     \
  m(mapGetS,   KeyType::Str)   \
  m(mapGetI,   KeyType::Int)

#define ELEM(nm, keyType)                             \
TypedValue nm(c_Map* map, TypedValue* key) {          \
  return mapGetImpl<keyType>(map, keyAsRaw<keyType>(key)); \
}
namespace MInstrHelpers {
HELPER_TABLE(ELEM)
}
#undef ELEM

void HhbcTranslator::MInstrTranslator::emitMapGet(SSATmp* key) {
  assert(key->isA(Type::Int) || key->isA(Type::Str));
  KeyType keyType = key->isA(Type::Int) ? KeyType::Int : KeyType::Str;

  typedef TypedValue (*OpFunc)(c_Map*, TypedValue*);
  BUILD_OPTAB(keyType);
  m_result = gen(MapGet, makeCatch(), cns((TCA)opFunc), m_base, key);
}
#undef HELPER_TABLE

template<KeyType keyType>
static inline TypedValue stableMapGetImpl(
    c_StableMap* map, typename KeyTypeTraits<keyType>::rawType key) {
  TypedValue* ret = map->at(key);
  tvRefcountedIncRef(ret);
  return *ret;
}

#define HELPER_TABLE(m)              \
  /* name            keyType  */     \
  m(stableMapGetS,   KeyType::Str)   \
  m(stableMapGetI,   KeyType::Int)

#define ELEM(nm, keyType)                                        \
TypedValue nm(c_StableMap* map, TypedValue* key) {               \
  return stableMapGetImpl<keyType>(map, keyAsRaw<keyType>(key)); \
}
namespace MInstrHelpers {
HELPER_TABLE(ELEM)
}
#undef ELEM

void HhbcTranslator::MInstrTranslator::emitStableMapGet(SSATmp* key) {
  assert(key->isA(Type::Int) || key->isA(Type::Str));
  KeyType keyType = key->isA(Type::Int) ? KeyType::Int : KeyType::Str;

  typedef TypedValue (*OpFunc)(c_StableMap*, TypedValue*);
  BUILD_OPTAB(keyType);
  m_result = gen(StableMapGet, makeCatch(), cns((TCA)opFunc), m_base, key);
}
#undef HELPER_TABLE

template <KeyType keyType>
static inline TypedValue cGetElemImpl(TypedValue* base, TypedValue keyVal,
                                      MInstrState* mis) {
  TypedValue* key = keyPtr<keyType>(keyVal);
  TypedValue scratch;
  TypedValue* result = Elem<true, keyType>(scratch, mis->tvRef, base, key);

  if (result->m_type == KindOfRef) {
    result = result->m_data.pref->tv();
  }
  tvRefcountedIncRef(result);
  return *result;
}

#define HELPER_TABLE(m)                    \
  /* name       key  */                    \
  m(cGetElemC,  KeyType::Any)              \
  m(cGetElemI,  KeyType::Int)              \
  m(cGetElemS,  KeyType::Str)

#define ELEM(nm, ...)                                                   \
TypedValue nm(TypedValue* base, TypedValue key, MInstrState* mis) {     \
  return cGetElemImpl<__VA_ARGS__>(base, key, mis);                     \
}
namespace MInstrHelpers {
HELPER_TABLE(ELEM)
}
#undef ELEM

void HhbcTranslator::MInstrTranslator::emitCGetElem() {
  SSATmp* key = getKey();

  SimpleOp simpleOpType = simpleCollectionOp();
  switch (simpleOpType) {
  case SimpleOp::Array:
    emitArrayGet(key);
    break;
  case SimpleOp::PackedArray:
    emitPackedArrayGet(key);
    break;
  case SimpleOp::String:
    emitStringGet(key);
    break;
  case SimpleOp::Vector:
    emitVectorGet(key);
    break;
  case SimpleOp::Pair:
    emitPairGet(key);
    break;
  case SimpleOp::Map:
    emitMapGet(key);
    break;
  case SimpleOp::StableMap:
    emitStableMapGet(key);
    break;
  case SimpleOp::None:
    typedef TypedValue (*OpFunc)(TypedValue*, TypedValue, MInstrState*);
    BUILD_OPTAB(getKeyType(key));
    m_result = gen(CGetElem, makeCatch(), cns((TCA)opFunc),
                   m_base, key, genMisPtr());
    break;
  }
}
#undef HELPER_TABLE

template <KeyType keyType>
static inline RefData* vGetElemImpl(TypedValue* base, TypedValue keyVal,
                                    MInstrState* mis) {
  TypedValue* key = keyPtr<keyType>(keyVal);
  TypedValue* result = HPHP::ElemD<false, true, keyType>(
    mis->tvScratch, mis->tvRef, base, key);

  if (result->m_type != KindOfRef) {
    tvBox(result);
  }
  RefData* ref = result->m_data.pref;
  ref->incRefCount();
  return ref;
}

#define HELPER_TABLE(m)                      \
  /* name         keyType */                 \
  m(vGetElemC,    KeyType::Any)              \
  m(vGetElemI,    KeyType::Int)              \
  m(vGetElemS,    KeyType::Str)

#define ELEM(nm, ...)                                                   \
RefData* nm(TypedValue* base, TypedValue key, MInstrState* mis) {       \
  return vGetElemImpl<__VA_ARGS__>(base, key,  mis);                    \
}
namespace MInstrHelpers {
HELPER_TABLE(ELEM)
}
#undef ELEM

void HhbcTranslator::MInstrTranslator::emitVGetElem() {
  SSATmp* key = getKey();
  typedef RefData* (*OpFunc)(TypedValue*, TypedValue, MInstrState*);
  BUILD_OPTAB(getKeyType(key));
  m_result = genStk(VGetElem, makeCatch(), cns((TCA)opFunc),
                    m_base, key, genMisPtr());
}
#undef HELPER_TABLE

template <KeyType keyType, bool isEmpty>
static inline bool issetEmptyElemImpl(TypedValue* base, TypedValue keyVal,
                                      MInstrState* mis) {
  TypedValue* key = keyPtr<keyType>(keyVal);
  // mis == nullptr if we proved that it won't be used. mis->tvScratch and
  // mis->tvRef are ok because those params are passed by
  // reference.
  return HPHP::IssetEmptyElem<isEmpty, keyType>(
    mis->tvScratch, mis->tvRef, base, key);
}

#define HELPER_TABLE(m)                 \
  /* name         keyType     isEmpty */\
  m(issetElemC,   KeyType::Any, false)  \
  m(issetElemCE,  KeyType::Any,  true)  \
  m(issetElemI,   KeyType::Int, false)  \
  m(issetElemIE,  KeyType::Int,  true)  \
  m(issetElemS,   KeyType::Str, false)  \
  m(issetElemSE,  KeyType::Str,  true)

#define ISSET(nm, ...)                                             \
uint64_t nm(TypedValue* base, TypedValue key, MInstrState* mis) {  \
  return issetEmptyElemImpl<__VA_ARGS__>(base, key, mis);          \
}
namespace MInstrHelpers {
HELPER_TABLE(ISSET)
}
#undef ISSET

void HhbcTranslator::MInstrTranslator::emitIssetEmptyElem(bool isEmpty) {
  SSATmp* key = getKey();

  typedef uint64_t (*OpFunc)(TypedValue*, TypedValue, MInstrState*);
  BUILD_OPTAB(getKeyType(key), isEmpty);
  m_result = gen(isEmpty ? EmptyElem : IssetElem, makeCatch(),
                 cns((TCA)opFunc), m_base, key, genMisPtr());
}
#undef HELPER_TABLE

void HhbcTranslator::MInstrTranslator::emitPackedArrayIsset() {
  assert(m_base->type().getArrayKind() == ArrayData::kPackedKind);
  SSATmp* key = getKey();
  m_result = m_tb.cond(
    [&] (Block* taken) {
      gen(CheckPackedArrayBounds, taken, m_base, key);
    },
    [&] { // Next:
      return gen(CheckPackedArrayElemNull, m_base, key);
    },
    [&] { // Taken:
      return cns(false);
    }
  );
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
  /* name           keyType       checkForInt */\
  m(arrayIssetS,    KeyType::Str,   false)      \
  m(arrayIssetSi,   KeyType::Str,    true)      \
  m(arrayIssetI,    KeyType::Int,   false)

#define ISSET(nm, keyType, checkForInt)                                 \
  uint64_t nm(ArrayData* a, TypedValue* key) {                          \
    return arrayIssetImpl<keyType, checkForInt>(a, keyAsRaw<keyType>(key)); \
  }
namespace MInstrHelpers {
HELPER_TABLE(ISSET)
}
#undef ISSET

void HhbcTranslator::MInstrTranslator::emitArrayIsset() {
  SSATmp* key = getKey();
  KeyType keyType;
  bool checkForInt;
  m_ht.checkStrictlyInteger(key, keyType, checkForInt);

  typedef uint64_t (*OpFunc)(ArrayData*, TypedValue*);
  BUILD_OPTAB(keyType, checkForInt);
  assert(m_base->isA(Type::Arr));
  m_result = gen(ArrayIsset, makeCatch(), cns((TCA)opFunc), m_base, key);
}
#undef HELPER_TABLE

void HhbcTranslator::MInstrTranslator::emitStringIsset() {
  auto key = getKey();
  m_result = gen(StringIsset, m_base, key);
}

namespace MInstrHelpers {
uint64_t vectorIsset(c_Vector* vec, int64_t index) {
  auto result = vec->get(index);
  return result ? !cellIsNull(result) : false;
}
}

void HhbcTranslator::MInstrTranslator::emitVectorIsset() {
  SSATmp* key = getKey();
  assert(key->isA(Type::Int));
  m_result = gen(VectorIsset,
                 cns((TCA)MInstrHelpers::vectorIsset),
                 m_base,
                 key);
}

namespace MInstrHelpers {
uint64_t pairIsset(c_Pair* pair, int64_t index) {
  auto result = pair->get(index);
  return result ? !cellIsNull(result) : false;
}
}

void HhbcTranslator::MInstrTranslator::emitPairIsset() {
  SSATmp* key = getKey();
  assert(key->isA(Type::Int));
  m_result = gen(PairIsset,
                 cns((TCA)MInstrHelpers::pairIsset),
                 m_base,
                 key);
}

template<KeyType keyType>
static inline uint64_t mapIssetImpl(
  c_Map* map, typename KeyTypeTraits<keyType>::rawType key) {
  auto result = map->get(key);
  return result ? !cellIsNull(result) : false;
}

#define HELPER_TABLE(m)          \
  /* name        keyType  */     \
  m(mapIssetS,   KeyType::Str)   \
  m(mapIssetI,   KeyType::Int)

#define ELEM(nm, keyType)                                    \
uint64_t nm(c_Map* map, TypedValue* key) {                   \
  return mapIssetImpl<keyType>(map, keyAsRaw<keyType>(key)); \
}
namespace MInstrHelpers {
HELPER_TABLE(ELEM)
}
#undef ELEM

void HhbcTranslator::MInstrTranslator::emitMapIsset() {
  SSATmp* key = getKey();
  assert(key->isA(Type::Int) || key->isA(Type::Str));
  KeyType keyType = key->isA(Type::Int) ? KeyType::Int : KeyType::Str;

  typedef TypedValue (*OpFunc)(c_Map*, TypedValue*);
  BUILD_OPTAB(keyType);
  m_result = gen(MapIsset, cns((TCA)opFunc), m_base, key);
}
#undef HELPER_TABLE

template<KeyType keyType>
static inline uint64_t stableMapIssetImpl(
  c_StableMap* map, typename KeyTypeTraits<keyType>::rawType key) {
  auto result = map->get(key);
  return result ? !cellIsNull(result) : false;
}

#define HELPER_TABLE(m)                \
  /* name              keyType  */     \
  m(stableMapIssetS,   KeyType::Str)   \
  m(stableMapIssetI,   KeyType::Int)

#define ELEM(nm, keyType)                                          \
uint64_t nm(c_StableMap* map, TypedValue* key) {                   \
  return stableMapIssetImpl<keyType>(map, keyAsRaw<keyType>(key)); \
}
namespace MInstrHelpers {
HELPER_TABLE(ELEM)
}
#undef ELEM

void HhbcTranslator::MInstrTranslator::emitStableMapIsset() {
  SSATmp* key = getKey();
  assert(key->isA(Type::Int) || key->isA(Type::Str));
  KeyType keyType = key->isA(Type::Int) ? KeyType::Int : KeyType::Str;

  typedef TypedValue (*OpFunc)(c_StableMap*, TypedValue*);
  BUILD_OPTAB(keyType);
  m_result = gen(StableMapIsset, cns((TCA)opFunc), m_base, key);
}
#undef HELPER_TABLE

void HhbcTranslator::MInstrTranslator::emitIssetElem() {
  SimpleOp simpleOpType = simpleCollectionOp();
  switch (simpleOpType) {
  case SimpleOp::Array:
    emitArrayIsset();
    break;
  case SimpleOp::PackedArray:
    emitPackedArrayIsset();
    break;
  case SimpleOp::String:
    emitStringIsset();
    break;
  case SimpleOp::Vector:
    emitVectorIsset();
    break;
  case SimpleOp::Pair:
    emitPairIsset();
    break;
  case SimpleOp::Map:
    emitMapIsset();
    break;
  case SimpleOp::StableMap:
    emitStableMapIsset();
    break;
  case SimpleOp::None:
    emitIssetEmptyElem(false);
    break;
  }
}

void HhbcTranslator::MInstrTranslator::emitEmptyElem() {
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
  static_assert(keyType != KeyType::Any,
                "KeyType::Any is not supported in arraySetMImpl");
  const bool copy = a->hasMultipleRefs();
  ArrayData* ret = checkForInt ? checkedSet(a, key, value, copy)
                               : a->set(key, value, copy);

  return arrayRefShuffle<setRef>(a, ret, setRef ? ref->tv() : nullptr);
}

#define HELPER_TABLE(m)                                    \
  /* name        keyType        checkForInt setRef */      \
  m(arraySetS,   KeyType::Str,   false,     false)         \
  m(arraySetSi,  KeyType::Str,    true,     false)         \
  m(arraySetI,   KeyType::Int,   false,     false)         \
  m(arraySetSR,  KeyType::Str,   false,      true)         \
  m(arraySetSiR, KeyType::Str,    true,      true)         \
  m(arraySetIR,  KeyType::Int,   false,      true)

#define ELEM(nm, keyType, checkForInt, setRef)                          \
typename ShuffleReturn<setRef>::return_type                             \
nm(ArrayData* a, TypedValue* key, TypedValue value, RefData* ref) {     \
  return arraySetImpl<keyType, checkForInt, setRef>(                    \
    a, keyAsRaw<keyType>(key), tvAsCVarRef(&value), ref);               \
}
namespace MInstrHelpers {
HELPER_TABLE(ELEM)
}
#undef ELEM

void HhbcTranslator::MInstrTranslator::emitArraySet(SSATmp* key,
                                                    SSATmp* value) {
  assert(m_iInd == m_mii.valCount() + 1);
  const int baseStkIdx = m_mii.valCount();
  assert(key->type().notBoxed());
  assert(value->type().notBoxed());
  KeyType keyType;
  bool checkForInt;
  m_ht.checkStrictlyInteger(key, keyType, checkForInt);
  const DynLocation& base = *m_ni.inputs[m_mii.valCount()];
  bool setRef = base.outerType() == KindOfRef;
  typedef ArrayData* (*OpFunc)(ArrayData*, TypedValue*, TypedValue, RefData*);
  BUILD_OPTAB(keyType, checkForInt, setRef);

  // No catch trace below because the helper can't throw. It may reenter to
  // call destructors so it has a sync point in nativecalls.cpp, but exceptions
  // are swallowed at destructor boundaries right now: #2182869.
  if (setRef) {
    assert(base.location.space == Location::Local ||
           base.location.space == Location::Stack);
    SSATmp* box = getInput(baseStkIdx, DataTypeSpecific);
    gen(ArraySetRef, cns((TCA)opFunc), m_base, key, value, box);
    // Unlike the non-ref case, we don't need to do anything to the stack
    // because any load of the box will be guarded.
  } else {
    SSATmp* newArr = gen(ArraySet, cns((TCA)opFunc), m_base, key, value);

    // Update the base's value with the new array
    if (base.location.space == Location::Local) {
      // We know it's not boxed (setRef above handles that), and
      // newArr has already been incref'd in the helper.
      gen(StLoc, LocalId(base.location.offset), m_tb.fp(), newArr);
    } else if (base.location.space == Location::Stack) {
      MInstrEffects effects(newArr->inst());
      assert(effects.baseValChanged);
      assert(effects.baseType <= Type::Arr);
      m_ht.extendStack(baseStkIdx, Type::Gen);
      m_ht.replace(baseStkIdx, newArr);
    } else {
      not_reached();
    }
  }

  m_result = value;
}
#undef HELPER_TABLE

namespace MInstrHelpers {
void setWithRefElemC(TypedValue* base, TypedValue keyVal, TypedValue* val,
                     MInstrState* mis) {
  base = HPHP::ElemD<false, false>(mis->tvScratch, mis->tvRef, base, &keyVal);
  if (base != &mis->tvScratch) {
    tvDup(*val, *base);
  } else {
    assert(base->m_type == KindOfUninit);
  }
}

void setWithRefNewElem(TypedValue* base, TypedValue* val,
                       MInstrState* mis) {
  base = NewElem(mis->tvScratch, mis->tvRef, base);
  if (base != &mis->tvScratch) {
    tvDup(*val, *base);
  } else {
    assert(base->m_type == KindOfUninit);
  }
}
}

void HhbcTranslator::MInstrTranslator::emitSetWithRefLElem() {
  SSATmp* key = getKey();
  SSATmp* locAddr = getValAddr();
  if (m_base->type().strip() <= Type::Arr &&
      !locAddr->type().deref().maybeBoxed()) {
    constrainBase(DataTypeSpecific);
    emitSetElem();
    assert(m_strTestResult == nullptr);
  } else {
    genStk(SetWithRefElem, makeCatch(),
           cns((TCA)MInstrHelpers::setWithRefElemC),
           m_base, key, locAddr, genMisPtr());
  }
  m_result = nullptr;
}

void HhbcTranslator::MInstrTranslator::emitSetWithRefLProp() {
  SPUNT(__func__);
}

void HhbcTranslator::MInstrTranslator::emitSetWithRefRElem() {
  emitSetWithRefLElem();
}

void HhbcTranslator::MInstrTranslator::emitSetWithRefRProp() {
  emitSetWithRefLProp();
}

void HhbcTranslator::MInstrTranslator::emitSetWithRefNewElem() {
  if (m_base->type().strip() <= Type::Arr &&
      getValue()->type().notBoxed()) {
    constrainBase(DataTypeSpecific);
    emitSetNewElem();
  } else {
    genStk(SetWithRefNewElem, makeCatch(),
           cns((TCA)MInstrHelpers::setWithRefNewElem),
           m_base, getValAddr(), genMisPtr());
  }
  m_result = nullptr;
}

void HhbcTranslator::MInstrTranslator::emitVectorSet(
    SSATmp* key, SSATmp* value) {
  assert(key->isA(Type::Int));
  if (key->isConst() && key->getValInt() < 0) {
    PUNT(emitVectorSet); // will throw
  }
  SSATmp* size = gen(LdVectorSize, m_base);
  gen(CheckBounds, makeCatch(), key, size);

  m_tb.ifThen([&](Block* taken) {
          gen(VectorHasFrozenCopy, taken, m_base);
        },
        [&] {
          m_tb.hint(Block::Hint::Unlikely);
          gen(VectorDoCow, m_base);
        });

  gen(IncRef, value);
  SSATmp* vecBase = gen(LdVectorBase, m_base);
  SSATmp* oldVal;
  static_assert(sizeof(TypedValue) == 16,
                "TypedValue size expected to be 16 bytes");
  auto idx = gen(Shl, key, cns(4));
  oldVal = gen(LdElem, vecBase, idx);
  gen(StElem, vecBase, idx, value);
  gen(DecRef, oldVal);

  m_result = value;
}

template<KeyType keyType>
static inline void mapSetImpl(
    c_Map* map,
    typename KeyTypeTraits<keyType>::rawType key,
    Cell value) {
  map->set(key, &value);
}

#define HELPER_TABLE(m)        \
  /* name      keyType  */     \
  m(mapSetS,   KeyType::Str)   \
  m(mapSetI,   KeyType::Int)

#define ELEM(nm, keyType)                                       \
void nm(c_Map* map, TypedValue* key, Cell value) {              \
  mapSetImpl<keyType>(map, keyAsRaw<keyType>(key), value);      \
}
namespace MInstrHelpers {
HELPER_TABLE(ELEM)
}
#undef ELEM

void HhbcTranslator::MInstrTranslator::emitMapSet(
    SSATmp* key, SSATmp* value) {
  assert(key->isA(Type::Int) || key->isA(Type::Str));
  KeyType keyType = key->isA(Type::Int) ? KeyType::Int : KeyType::Str;

  typedef TypedValue (*OpFunc)(c_Map*, TypedValue*, TypedValue*);
  BUILD_OPTAB(keyType);
  gen(MapSet, makeCatch(), cns((TCA)opFunc), m_base, key, value);
  m_result = value;
}
#undef HELPER_TABLE

template<KeyType keyType>
static inline void stableMapSetImpl(
    c_StableMap* map,
    typename KeyTypeTraits<keyType>::rawType key,
    Cell value) {
  map->set(key, &value);
}

#define HELPER_TABLE(m)              \
  /* name            keyType  */     \
  m(stableMapSetS,   KeyType::Str)   \
  m(stableMapSetI,   KeyType::Int)

#define ELEM(nm, keyType)                                           \
void nm(c_StableMap* map, TypedValue* key, Cell value) {            \
  stableMapSetImpl<keyType>(map, keyAsRaw<keyType>(key), value);    \
}
namespace MInstrHelpers {
HELPER_TABLE(ELEM)
}
#undef ELEM

void HhbcTranslator::MInstrTranslator::emitStableMapSet(
    SSATmp* key, SSATmp* value) {
  assert(key->isA(Type::Int) || key->isA(Type::Str));
  KeyType keyType = key->isA(Type::Int) ? KeyType::Int : KeyType::Str;

  typedef TypedValue (*OpFunc)(c_StableMap*, TypedValue*, TypedValue*);
  BUILD_OPTAB(keyType);
  gen(StableMapSet, makeCatch(), cns((TCA)opFunc), m_base, key, value);
  m_result = value;
}
#undef HELPER_TABLE

template <KeyType keyType>
static inline StringData* setElemImpl(TypedValue* base, TypedValue keyVal,
                                      Cell val) {
  TypedValue* key = keyPtr<keyType>(keyVal);
  return HPHP::SetElem<false, keyType>(base, key, &val);
}

#define HELPER_TABLE(m)                    \
  /* name       keyType    */              \
  m(setElemC,   KeyType::Any)              \
  m(setElemI,   KeyType::Int)              \
  m(setElemS,   KeyType::Str)

#define ELEM(nm, ...)                                              \
StringData* nm(TypedValue* base, TypedValue key, Cell val) {       \
  return setElemImpl<__VA_ARGS__>(base, key, val);                 \
}
namespace MInstrHelpers {
HELPER_TABLE(ELEM)
}
#undef ELEM

void HhbcTranslator::MInstrTranslator::emitSetElem() {
  SSATmp* value = getValue();
  SSATmp* key = getKey();

  SimpleOp simpleOpType = simpleCollectionOp();
  assert(simpleOpType != SimpleOp::PackedArray);
  switch (simpleOpType) {
  case SimpleOp::Array:
    emitArraySet(key, value);
    break;
  case SimpleOp::PackedArray:
    not_reached();
    break;
  case SimpleOp::String:
    not_reached();
    break;
  case SimpleOp::Vector:
    emitVectorSet(key, value);
    break;
  case SimpleOp::Map:
    emitMapSet(key, value);
    break;
  case SimpleOp::StableMap:
    emitStableMapSet(key, value);
    break;
  case SimpleOp::Pair:
  case SimpleOp::None:
    // Emit the appropriate helper call.
    typedef StringData* (*OpFunc)(TypedValue*, TypedValue, Cell);
    BUILD_OPTAB(getKeyType(key));
    constrainBase(DataTypeSpecific);
    SSATmp* result = genStk(SetElem, makeCatchSet(), cns((TCA)opFunc),
                            m_base, key, value);
    auto t = result->type();
    if (t.equals(Type::Nullptr)) {
      // Base is not a string. Result is always value.
      m_result = value;
    } else if (t.equals(Type::CountedStr)) {
      // Base is a string. Stack result is a new string so we're responsible for
      // decreffing value.
      m_result = result;
      gen(DecRef, value);
    } else {
      assert(t.equals(Type::CountedStr | Type::Nullptr));
      // Base might be a string. Assume the result is value, then inform
      // emitMPost that it needs to test the actual result.
      m_result = value;
      m_strTestResult = result;
    }
    break;
  }
}
#undef HELPER_TABLE

template <SetOpOp op>
static inline TypedValue setOpElemImpl(TypedValue* base, TypedValue keyVal,
                                       Cell val, MInstrState* mis) {
  TypedValue* result =
    HPHP::SetOpElem(mis->tvScratch, mis->tvRef, op, base, &keyVal, &val);

  Cell ret;
  cellDup(*tvToCell(result), ret);
  return ret;
}

#define OPELEM_TABLE(m, nm, pfx, op)            \
  /* name          op */                        \
  m(nm##op##ElemC, pfx op)

#define HELPER_TABLE(m, op) OPELEM_TABLE(m, setOp, SetOpOp::, op)
#define SETOP(nm, ...)                                                  \
TypedValue nm(TypedValue* base, TypedValue key, Cell val,               \
                     MInstrState* mis) {                                \
  return setOpElemImpl<__VA_ARGS__>(base, key, val, mis);               \
}
#define SETOP_OP(op, bcOp) HELPER_TABLE(SETOP, op)
namespace MInstrHelpers {
SETOP_OPS
}
#undef SETOP_OP
#undef SETOP

void HhbcTranslator::MInstrTranslator::emitSetOpElem() {
  SetOpOp op = SetOpOp(m_ni.imm[0].u_OA);
  SSATmp* key = getKey();
  typedef TypedValue (*OpFunc)(TypedValue*, TypedValue, Cell, MInstrState*);
# define SETOP_OP(op, bcOp) HELPER_TABLE(FILL_ROW, op)
  BUILD_OPTAB_ARG(SETOP_OPS, op);
# undef SETOP_OP
  m_result = genStk(SetOpElem, makeCatch(), cns((TCA)opFunc),
                    m_base, key, getValue(), genMisPtr());
}
#undef HELPER_TABLE

template <IncDecOp op>
static inline TypedValue incDecElemImpl(TypedValue* base, TypedValue keyVal,
                                        MInstrState* mis) {
  TypedValue result;
  HPHP::IncDecElem<true>(
    mis->tvScratch, mis->tvRef, op, base, &keyVal, result);
  assert(result.m_type != KindOfRef);
  return result;
}

#define HELPER_TABLE(m, op) OPELEM_TABLE(m, incDec, IncDecOp::, op)
#define INCDEC(nm, ...)                                                 \
TypedValue nm(TypedValue* base, TypedValue key, MInstrState* mis) {     \
  return incDecElemImpl<__VA_ARGS__>(base, key, mis);                   \
}
#define INCDEC_OP(op) HELPER_TABLE(INCDEC, op)
namespace MInstrHelpers {
INCDEC_OPS
}
#undef INCDEC_OP
#undef INCDEC

void HhbcTranslator::MInstrTranslator::emitIncDecElem() {
  IncDecOp op = static_cast<IncDecOp>(m_ni.imm[0].u_OA);
  SSATmp* key = getKey();
  typedef TypedValue (*OpFunc)(TypedValue*, TypedValue, MInstrState*);
# define INCDEC_OP(op) HELPER_TABLE(FILL_ROW, op)
  BUILD_OPTAB_ARG(INCDEC_OPS, op);
# undef INCDEC_OP
  m_result = genStk(IncDecElem, makeCatch(), cns((TCA)opFunc),
                    m_base, key, genMisPtr());
}
#undef HELPER_TABLE

namespace MInstrHelpers {
void bindElemC(TypedValue* base, TypedValue keyVal, RefData* val,
               MInstrState* mis) {
  base = HPHP::ElemD<false, true>(mis->tvScratch, mis->tvRef, base, &keyVal);
  if (!(base == &mis->tvScratch && base->m_type == KindOfUninit)) {
    tvBindRef(val, base);
  }
}
}

void HhbcTranslator::MInstrTranslator::emitBindElem() {
  SSATmp* key = getKey();
  SSATmp* box = getValue();
  genStk(BindElem, makeCatch(), cns((TCA)MInstrHelpers::bindElemC),
         m_base, key, box, genMisPtr());
  m_result = box;
}

template <KeyType keyType>
static inline void unsetElemImpl(TypedValue* base, TypedValue keyVal) {
  TypedValue* key = keyPtr<keyType>(keyVal);
  HPHP::UnsetElem<keyType>(base, key);
}

#define HELPER_TABLE(m)                 \
  /* name         keyType */            \
  m(unsetElemC,   KeyType::Any)         \
  m(unsetElemI,   KeyType::Int)         \
  m(unsetElemS,   KeyType::Str)

#define ELEM(nm, ...)                                      \
void nm(TypedValue* base, TypedValue key) {                \
  unsetElemImpl<__VA_ARGS__>(base, key);                   \
}
namespace MInstrHelpers {
HELPER_TABLE(ELEM)
}
#undef ELEM

void HhbcTranslator::MInstrTranslator::emitUnsetElem() {
  SSATmp* key = getKey();

  Type baseType = m_base->type().strip();
  constrainBase(DataTypeSpecific);
  if (baseType <= Type::Str) {
    gen(RaiseError, makeCatch(),
        cns(makeStaticString(Strings::CANT_UNSET_STRING)));
    return;
  }
  if (baseType.not(Type::Arr | Type::Obj)) {
    // Noop
    return;
  }

  typedef void (*OpFunc)(TypedValue*, TypedValue);
  BUILD_OPTAB(getKeyType(key));
  genStk(UnsetElem, makeCatch(), cns((TCA)opFunc), m_base, key);
}
#undef HELPER_TABLE

void HhbcTranslator::MInstrTranslator::emitNotSuppNewElem() {
  not_reached();
}

void HhbcTranslator::MInstrTranslator::emitVGetNewElem() {
  SPUNT(__func__);
}

void HhbcTranslator::MInstrTranslator::emitSetNewElem() {
  SSATmp* value = getValue();
  if (m_base->type() <= Type::PtrToArr) {
    constrainBase(DataTypeSpecific);
    gen(SetNewElemArray, makeCatchSet(), m_base, value);
  } else {
    gen(SetNewElem, makeCatchSet(), m_base, value);
  }
  m_result = value;
}

void HhbcTranslator::MInstrTranslator::emitSetOpNewElem() {
  SPUNT(__func__);
}

void HhbcTranslator::MInstrTranslator::emitIncDecNewElem() {
  SPUNT(__func__);
}

void HhbcTranslator::MInstrTranslator::emitBindNewElem() {
  SSATmp* box = getValue();
  genStk(BindNewElem, makeCatch(), m_base, box, genMisPtr());
  m_result = box;
}

void HhbcTranslator::MInstrTranslator::emitMPost() {
  SSATmp* catchSp = nullptr;
  if (m_failedSetBlock) {
    auto* endCatch = &m_failedSetBlock->trace()->back()->back();
    assert(endCatch->is(EndCatch, TryEndCatch));
    catchSp = endCatch->src(1);
    assert(catchSp->isA(Type::StkPtr));
  }

  // Decref stack inputs. If we're translating a SetM or BindM, then input 0 is
  // both our input and output so leave its refcount alone. If m_failedSetBlock
  // is non-null, the final helper call may throw an InvalidSetMException. We
  // need to add instructions to m_failedSetBlock to finish the vector
  // instruction in case this happens, so any DecRefs emitted here are also
  // added to m_failedSetBlock.
  unsigned nStack =
    (m_ni.mInstrOp() == OpSetM || m_ni.mInstrOp() == OpBindM) ? 1 : 0;
  for (unsigned i = nStack; i < m_ni.inputs.size(); ++i) {
    const DynLocation& input = *m_ni.inputs[i];
    switch (input.location.space) {
    case Location::Stack: {
      ++nStack;
      auto input = getInput(i, DataTypeCountness); // just going to decref it
      if (input->isA(Type::Gen)) {
        gen(DecRef, input);
        if (m_failedSetBlock) {
          TracePusher tp(m_tb, m_failedSetBlock->trace(), m_marker);
          gen(DecRefStack, StackOffset(m_stackInputs[i]), Type::Cell, catchSp);
        }
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

  // Push result, if one was produced. If we have a predicted result use that
  // instead of the real result; its validity will be guarded later in this
  // function.
  if (m_result) {
    m_ht.push(m_result);
  } else {
    assert(m_ni.mInstrOp() == OpUnsetM ||
           m_ni.mInstrOp() == OpSetWithRefLM ||
           m_ni.mInstrOp() == OpSetWithRefRM);
  }

  // Clean up tvRef(2): during exception handling any objects required only
  // during vector expansion need to be DecRef'd.  There may be either one
  // or two such scratch objects, in the case of a Set the first of which will
  // always be tvRef2, in all other cases if only one scratch value is present
  // it will be stored in tvRef.
  static const size_t refOffs[] = { MISOFF(tvRef), MISOFF(tvRef2) };
  for (unsigned i = 0; i < std::min(nLogicalRatchets(), 2U); ++i) {
    IRInstruction* inst = m_irf.gen(DecRefMem, m_marker, Type::Gen, m_misBase,
                                    cns(refOffs[m_failedSetBlock ? 1 - i : i]));
    m_tb.add(inst);
    prependToTraces(inst);
  }

  emitSideExits(catchSp, nStack);
}

void HhbcTranslator::MInstrTranslator::emitSideExits(SSATmp* catchSp,
                                                     int nStack) {
  auto const nextOff = m_ht.nextBcOff();
  auto const op = m_ni.mInstrOp();
  const bool isSetWithRef = op == OpSetWithRefLM || op == OpSetWithRefRM;

  if (m_failedSetBlock) {
    assert(bool(m_result) ^ isSetWithRef);
    // We need to emit code to clean up and side exit if the TryEndCatch falls
    // through because of an InvalidSetMException. If we're translating a
    // SetWithRef* bytecode we don't have to do anything special to the stack
    // since they have no stack output. Otherwise we need to pop our input
    // value and push the value from the exception to the stack (done with a
    // DecRefStack followed by a SpillStack).

    std::vector<SSATmp*> args{
        catchSp,     // sp from the previous SpillStack
        cns(nStack), // cells popped since the last SpillStack
    };

    TracePusher tp(m_tb, m_failedSetBlock->trace(), m_marker);
    if (!isSetWithRef) {
      gen(DecRefStack, StackOffset(0), Type::Cell, catchSp);
      args.push_back(m_ht.gen(LdUnwinderValue, Type::Cell));
    }

    SSATmp* sp = gen(SpillStack, std::make_pair(args.size(), &args[0]));
    gen(DeleteUnwinderException);
    gen(SyncABIRegs, m_tb.fp(), sp);
    gen(ReqBindJmp, BCOffset(nextOff));
  }

  if (m_strTestResult) {
    assert(!isSetWithRef);
    // We expected SetElem's base to not be a Str but might be wrong. Make an
    // exit trace to side exit to the next instruction, replacing our guess
    // with the correct stack output.

    auto toSpill = m_ht.peekSpillValues();
    assert(toSpill.size());
    assert(toSpill[0] == m_result);
    SSATmp* str = m_irf.gen(AssertNonNull, m_marker, m_strTestResult)->dst();
    toSpill[0] = str;

    auto exit = m_ht.makeExit(nextOff, toSpill);
    {
      TracePusher tp(m_tb, exit->trace(), m_marker, exit->trace()->back(),
                     exit->trace()->back()->skipHeader());
      gen(IncStat, cns(Stats::TC_SetMStrGuess_Miss), cns(1), cns(false));
      gen(DecRef, m_result);
      m_tb.add(str->inst());
    }
    gen(CheckNullptr, exit, m_strTestResult);
    gen(IncStat, cns(Stats::TC_SetMStrGuess_Hit), cns(1), cns(false));
  }
}

bool HhbcTranslator::MInstrTranslator::needFirstRatchet() const {
  if (m_ni.inputs[m_mii.valCount()]->valueType() == KindOfArray) {
    switch (m_ni.immVecM[0]) {
    case MEC: case MEL: case MET: case MEI: return false;
    case MPC: case MPL: case MPT: case MW:  return true;
    default: not_reached();
    }
  }
  return true;
}

bool HhbcTranslator::MInstrTranslator::needFinalRatchet() const {
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
unsigned HhbcTranslator::MInstrTranslator::nLogicalRatchets() const {
  // If we've proven elsewhere that we don't need an MInstrState struct, we
  // know this translation won't need any ratchets
  if (!m_needMIS) return 0;

  unsigned ratchets = m_ni.immVecM.size();
  if (!needFirstRatchet()) --ratchets;
  if (!needFinalRatchet()) --ratchets;
  return ratchets;
}

int HhbcTranslator::MInstrTranslator::ratchetInd() const {
  return needFirstRatchet() ? int(m_mInd) : int(m_mInd) - 1;
}

} }
