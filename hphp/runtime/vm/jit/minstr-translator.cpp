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

#include "hphp/runtime/base/mixed-array-defs.h"

#include <algorithm>
#include <type_traits>
#include <vector>

#include "hphp/runtime/base/strings.h"
#include "hphp/runtime/vm/member-operations.h"
#include "hphp/runtime/vm/repo-global-data.h"
#include "hphp/runtime/vm/repo.h"
#include "hphp/runtime/vm/jit/frame-state.h"
#include "hphp/runtime/vm/jit/hhbc-translator.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/punt.h"
#include "hphp/runtime/vm/jit/target-profile.h"
#include "hphp/runtime/vm/jit/translator-runtime.h"

// These files do ugly things with macros so include them last
#include "hphp/util/assert-throw.h"
#include "hphp/runtime/vm/jit/minstr-translator-internal.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir);

using HPHP::jit::Location;

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

void MInstrEffects::get(const IRInstruction* inst,
                        const FrameStateMgr& frame,
                        LocalStateHook& hook) {
  // If the base for this instruction is a local address, the helper call might
  // have side effects on the local's value
  auto const base = inst->src(minstrBaseIdx(inst));
  auto const locInstr = base->inst();

  // Right now we require that the address of any affected local is the
  // immediate source of the base tmp.  This isn't actually specified in the ir
  // spec right now but will intend to make it more general soon.
  if (locInstr->op() != LdLocAddr) return;

  auto const locId = locInstr->extra<LdLocAddr>()->locId;
  auto const baseType = frame.localType(locId);

  MInstrEffects effects(inst->op(), baseType.ptr(Ptr::Frame));
  if (effects.baseTypeChanged || effects.baseValChanged) {
    auto const ty = effects.baseType.derefIfPtr();
    if (ty.isBoxed()) {
      hook.setLocalType(locId, Type::BoxedInitCell);
      hook.setBoxedLocalPrediction(locId, ty);
    } else {
      hook.setLocalType(locId, ty);
    }
  }
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
       : opcodeHasFlags(op, MInstrElem) ? SetElem
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
      baseType = baseType <= Type::Str ? Type::Str : newBase;
    } else if (baseType <= Type::Str &&
               (rawOp == SetElem || rawOp == SetElemStk)) {
      /* If the base is known to be a string and the operation is exactly
       * SetElem, we're guaranteed that either the base will end as a
       * CountedStr or the instruction will throw an exception and side
       * exit. */
      baseType = Type::CountedStr;
    } else if (baseType <= Type::Str &&
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

MInstrEffects::MInstrEffects(const Opcode rawOp, const Type origBase) {
  baseType = origBase;
  // Note: MInstrEffects wants to manipulate pointer types in some situations
  // for historical reasons.  We'll eventually change that.
  always_assert(baseType.isPtr() ^ baseType.notPtr());
  auto const basePtr = baseType.isPtr();
  auto const basePtrKind = basePtr ? baseType.ptrKind() : Ptr::Unk;
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
  baseType = basePtr ? baseType.ptr(basePtrKind) : baseType;

  baseTypeChanged = baseType != origBase;

  /* Boxed bases may have their inner value changed but the value of the box
   * will never change. */
  baseValChanged = !origBase.isBoxed() && (baseValChanged || baseTypeChanged);
}

// minstrBaseIdx returns the src index for inst's base operand.
int minstrBaseIdx(Opcode opc) {
  return opcodeHasFlags(opc, MInstrProp) ? 0 :
         opcodeHasFlags(opc, MInstrElem) ? 0 :
         bad_value<int>();
}
int minstrBaseIdx(const IRInstruction* inst) {
  return minstrBaseIdx(inst->op());
}

HhbcTranslator::MInstrTranslator::MInstrTranslator(
    const NormalizedInstruction& ni,
    HhbcTranslator& ht)
  : m_ni(ni)
  , m_ht(ht)
  , m_irb(*m_ht.m_irb)
  , m_unit(m_ht.m_unit)
  , m_mii(getMInstrInfo(ni.mInstrOp()))
  , m_marker(ht.makeMarker(ht.bcOff()))
  , m_iInd(m_mii.valCount())
  , m_needMIS(true)
  , m_misBase(nullptr)
  , m_result(nullptr)
  , m_strTestResult(nullptr)
  , m_failedSetBlock(nullptr)
{
}

namespace { struct NoExtraData {}; }

template<class E> struct HhbcTranslator::MInstrTranslator::genStkImpl {
  template<class... Srcs>
  static SSATmp* go(MInstrTranslator& minst, Opcode opc, Block* taken,
                    const E& extra, Srcs... srcs) {
    return minst.gen(opc, taken, extra, srcs...);
  }
};

template<> struct HhbcTranslator::MInstrTranslator::genStkImpl<NoExtraData> {
  template<class... Srcs>
  static SSATmp* go(MInstrTranslator& minst, Opcode opc, Block* taken,
                    const NoExtraData&, Srcs... srcs) {
    return minst.gen(opc, taken, srcs...);
  }
};

template<class ExtraData, class... Srcs>
SSATmp* HhbcTranslator::MInstrTranslator::genStk(Opcode opc,
                                                 Block* taken,
                                                 const ExtraData& extra,
                                                 Srcs... srcs) {
  static_assert(!std::is_same<ExtraData,SSATmp*>::value,
                "Pass NoExtraData{} if you don't need extra data in genStk");
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
    auto const prev = getStackValue(
      base->inst()->src(0),
      base->inst()->extra<LdStackAddr>()->offset
    );
    MInstrEffects effects(opc, prev.knownType.ptr(Ptr::Stk));
    if (effects.baseTypeChanged || effects.baseValChanged) {
      return genStkImpl<ExtraData>::go(
        *this, getStackModifyingOpcode(opc), taken, extra, srcs..., m_irb.sp());
    }
  }

  return genStkImpl<ExtraData>::go(*this, opc, taken, extra, srcs...);
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
  assert(m_base.value && "genMisPtr called before emitBaseOp");

  if (m_needMIS) {
    return gen(LdMIStateAddr, m_misBase, cns(RDS::kVmMInstrStateOff));
  }
  return cns(Type::cns(nullptr, Type::PtrToMISUninit));
}


namespace {
bool mightCallMagicPropMethod(MInstrAttr mia, const Class* cls,
                              PropInfo propInfo) {
  if (convertToType(propInfo.repoAuthType).not(Type::Uninit)) {
    return false;
  }
  if (!cls) return true;
  bool const no_override_magic =
    // NB: this function can't yet be used for unset or isset contexts.  Just
    // get and set.
    (mia & Define) ? cls->attrs() & AttrNoOverrideMagicSet
                   : cls->attrs() & AttrNoOverrideMagicGet;
  return !no_override_magic;
}

bool mInstrHasUnknownOffsets(const NormalizedInstruction& ni, Class* context) {
  const MInstrInfo& mii = getMInstrInfo(ni.mInstrOp());
  unsigned mi = 0;
  unsigned ii = mii.valCount() + 1;
  for (; mi < ni.immVecM.size(); ++mi) {
    MemberCode mc = ni.immVecM[mi];
    if (mcodeIsProp(mc)) {
      const Class* cls = nullptr;
      auto propInfo = getPropertyOffset(ni, context, cls, mii, mi, ii);
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
}

// Inspect the instruction we're about to translate and determine if
// it can be executed without using an MInstrState struct.
void HhbcTranslator::MInstrTranslator::checkMIState() {
  if (m_ni.immVec.locationCode() == LNL || m_ni.immVec.locationCode() == LNC) {
    // We're definitely going to punt in emitBaseN, so we might not
    // have guarded the base's type.
    return;
  }

  Type baseType             = m_base.type.derefIfPtr();
  const bool baseArr        = baseType <= Type::Arr;
  const bool isCGetM        = m_ni.mInstrOp() == OpCGetM;
  const bool isSetM         = m_ni.mInstrOp() == OpSetM;
  const bool isIssetM       = m_ni.mInstrOp() == OpIssetM;
  const bool isUnsetM       = m_ni.mInstrOp() == OpUnsetM;
  const bool isSingle       = m_ni.immVecM.size() == 1;
  const bool unknownOffsets = mInstrHasUnknownOffsets(m_ni, contextClass());

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
  const bool singleElem = isSingle && mcodeIsElem(m_ni.immVecM[0]);

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
    isSimpleBase() && mcodeMaybeArrayIntKey(m_ni.immVecM[0]) &&
    baseType <= Type::Str;

  if (simpleProp || singleSet ||
      simpleArrayGet || simpleCollectionGet ||
      simpleArrayUnset || simpleCollectionIsset ||
      simpleArrayIsset || simpleStringOp) {
    setNoMIState();
    if (simpleCollectionGet || simpleCollectionIsset) {
      constrainBase(TypeConstraint(baseType.getClass()));
    } else {
      constrainBase(DataTypeSpecific);
    }
  }
}

void HhbcTranslator::MInstrTranslator::emitMPre() {
  if (HPHP::Trace::moduleEnabled(HPHP::Trace::minstr, 1)) {
    emitMTrace();
  }

  // The base location is input 0 or 1, and the location code is stored
  // separately from m_ni.immVecM, so input indices (iInd) and member indices
  // (mInd) commonly differ.  Additionally, W members have no corresponding
  // inputs, so it is necessary to track the two indices separately.
  m_simpleOp = computeSimpleCollectionOp();
  emitBaseOp();
  ++m_iInd;

  checkMIState();
  if (m_needMIS) {
    m_misBase = gen(DefMIStateBase);
    SSATmp* uninit = cns(Type::Uninit);

    if (nLogicalRatchets() > 0) {
      gen(StMem, m_misBase, cns(MISOFF(tvRef)), uninit);
      gen(StMem, m_misBase, cns(MISOFF(tvRef2)), uninit);
    }
  }

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

void
HhbcTranslator::MInstrTranslator::setBase(SSATmp* tmp,
                                          folly::Optional<Type> baseType) {
  m_base.value = tmp;
  m_base.type = baseType ? *baseType : m_base.value->type();
  always_assert(m_base.type <= m_base.value->type());
}

SSATmp* HhbcTranslator::MInstrTranslator::getBase(TypeConstraint tc) {
  assert(m_iInd == m_mii.valCount());
  return getInput(m_iInd, tc);
}

SSATmp* HhbcTranslator::MInstrTranslator::getKey() {
  auto key = getInput(m_iInd, DataTypeSpecific);
  auto const keyType = key->type();
  assert(keyType.isBoxed() || keyType.notBoxed());
  if (keyType.isBoxed()) {
    key = gen(LdRef, Type::InitCell, key);
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
    assert(!m_stackInputs.count(0));
    return m_ht.ldLocAddr(l.offset);
  }

  assert(l.space == Location::Stack);
  assert(m_stackInputs.count(0));
  m_ht.spillStack();
  return m_ht.ldStackAddr(m_stackInputs[0]);
}

void HhbcTranslator::MInstrTranslator::constrainBase(TypeConstraint tc) {
  // Member operations only care about the inner type of the base if it's
  // boxed, so this handles the logic of using the inner constraint when
  // appropriate.
  if (m_base.type.maybeBoxed()) {
    //tc.innerCat = tc.category;
    tc.category = DataTypeCountness;
  }
  m_irb.constrainValue(m_base.value, tc);
}

SSATmp* HhbcTranslator::MInstrTranslator::getInput(unsigned i,
                                                   TypeConstraint tc) {
  const DynLocation& dl = *m_ni.inputs[i];
  const Location& l = dl.location;

  assert(!!m_stackInputs.count(i) == (l.space == Location::Stack));
  switch (l.space) {
    case Location::Stack:
      return m_ht.top(Type::StackElem, m_stackInputs[i], tc);

    case Location::Local:
      // N.B. Exit block for LdLocPseudoMain is nullptr because we always
      // InterpOne member instructions in pseudomains
      return m_ht.ldLoc(l.offset, nullptr, tc);

    case Location::Litstr:
      return cns(m_ht.lookupStringId(l.offset));

    case Location::Litint:
      return cns(l.offset);

    case Location::This:
      // If we don't have a current class context, this instruction will be
      // unreachable.
      if (!m_ht.curClass()) PUNT(Unreachable-LdThis);
      return m_ht.ldThis();

    default: not_reached();
  }
}

void HhbcTranslator::MInstrTranslator::emitBaseLCR() {
  const MInstrAttr& mia = m_mii.getAttr(m_ni.immVec.locationCode());
  const DynLocation& baseDL = *m_ni.inputs[m_iInd];
  // We use DataTypeGeneric here because we might not care about the type. If
  // we do, it's constrained further.
  auto base = getBase(DataTypeGeneric);
  auto baseType = base->type();
  assert(baseType.isBoxed() || baseType.notBoxed());

  if (baseDL.location.isLocal()) {
    // Check for Uninit and warn/promote to InitNull as appropriate
    if (baseType <= Type::Uninit) {
      if (mia & MIA_warn) {
        gen(RaiseUninitLoc, makeEmptyCatch(),
            cns(m_ht.curFunc()->localVarName(baseDL.location.offset)));
      }
      if (mia & MIA_define) {
        // We care whether or not the local is Uninit, and
        // CountnessInit will tell us that.
        m_irb.constrainLocal(baseDL.location.offset, DataTypeSpecific,
                            "emitBaseLCR: Uninit base local");
        base = cns(Type::InitNull);
        baseType = Type::InitNull;
        gen(
          StLoc,
          LocalId(baseDL.location.offset),
          m_irb.fp(),
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
  Block* failedRef = baseType.isBoxed() ? m_ht.makeExit() : nullptr;
  if (baseType.isBoxed() && baseDL.location.isLocal()) {
    auto const predTy = m_irb.predictedInnerType(baseDL.location.offset);
    gen(CheckRefInner, predTy, failedRef, base);
    base = gen(LdRef, predTy, base);
    baseType = base->type();
  }

  // Check for common cases where we can pass the base by value, we unboxed
  // above if it was needed.
  if ((baseType.subtypeOfAny(Type::Obj) && mcodeIsProp(m_ni.immVecM[0])) ||
      m_simpleOp != SimpleOp::None) {
    // Register that we care about the specific type of the base, though, and
    // might care about its specialized type.
    setBase(base, baseType);
    constrainBase(DataTypeSpecific);
    specializeBaseIfPossible(baseType);
    return;
  }

  // Everything else is passed by pointer. We don't have to worry about
  // unboxing, since all the generic helpers understand boxed bases. They still
  // may rely on the CheckRefInner guard above, though; the various emit*
  // functions may do smarter things based on the guarded type.
  if (baseDL.location.space == Location::Local) {
    setBase(
      m_ht.ldLocAddr(baseDL.location.offset),
      m_irb.localType(baseDL.location.offset, DataTypeSpecific).ptr(Ptr::Frame)
    );
  } else {
    assert(baseDL.location.space == Location::Stack);
    // Make sure the stack is clean before getting a pointer to one of its
    // elements.
    m_ht.spillStack();
    assert(m_stackInputs.count(m_iInd));
    auto const sinfo = getStackValue(m_irb.sp(), m_stackInputs[m_iInd]);
    setBase(
      m_ht.ldStackAddr(m_stackInputs[m_iInd]),
      sinfo.knownType.ptr(Ptr::Stk)
    );
  }
  assert(m_base.value->type().isPtr());
  assert(m_base.type.isPtr());

  // TODO(t2598894): We do this for consistency with the old guard relaxation
  // code, but may change it in the future.
  constrainBase(DataTypeSpecific);
}

// Compute whether the current instruction a 1-element simple collection
// (includes Array) operation.
HhbcTranslator::MInstrTranslator::SimpleOp
HhbcTranslator::MInstrTranslator::computeSimpleCollectionOp() {
  // DataTypeGeneric is used in here to avoid constraining the base in case we
  // end up not caring about the type. Consumers of the return value must
  // constrain the base as appropriate.
  auto const base = getBase(DataTypeGeneric); // XXX: generates unneeded instrs
  if (!base->type().isBoxed() && base->type().maybeBoxed()) {
    // We might be doing a Base NL or something similar.  Either way we can't
    // do a simple op if we have a mixed boxed/unboxed type.
    return SimpleOp::None;
  }

  auto const baseType = [&] {
    const DynLocation& baseDL = *m_ni.inputs[m_iInd];
    // Before we do any simpleCollectionOp on a local base, we will always emit
    // the appropriate CheckRefInner guard to allow us to use a predicted inner
    // type.  So when calculating the SimpleOp assume that type.
    if (base->type().maybeBoxed() && baseDL.location.isLocal()) {
      return m_irb.predictedInnerType(baseDL.location.offset);
    }
    return base->type();
  }();

  auto const op = m_ni.mInstrOp();
  bool const readInst = (op == OpCGetM || op == OpIssetM);
  if ((op == OpSetM || readInst) && isSimpleBase() && isSingleMember()) {
    if (baseType <= Type::Arr) {
      auto const isPacked =
        baseType.isSpecialized() &&
        baseType.hasArrayKind() &&
        baseType.getArrayKind() == ArrayData::kPackedKind;
      if (mcodeIsElem(m_ni.immVecM[0])) {
        SSATmp* key = getInput(m_mii.valCount() + 1, DataTypeGeneric);
        if (key->isA(Type::Int) || key->isA(Type::Str)) {
          if (readInst && key->isA(Type::Int)) {
            return isPacked ? SimpleOp::PackedArray
                            : SimpleOp::ProfiledArray;
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
      // TODO: Add Frozen collections

      if (isVector || isPair) {
        if (mcodeMaybeVectorKey(m_ni.immVecM[0])) {
          SSATmp* key = getInput(m_mii.valCount() + 1, DataTypeGeneric);
          if (key->isA(Type::Int)) {
            // We don't specialize setting pair elements.
            if (isPair && op == OpSetM) return SimpleOp::None;

            return isVector ? SimpleOp::Vector : SimpleOp::Pair;
          }
        }
      } else if (isMap) {
        if (mcodeIsElem(m_ni.immVecM[0])) {
          SSATmp* key = getInput(m_mii.valCount() + 1, DataTypeGeneric);
          if (key->isA(Type::Int) || key->isA(Type::Str)) {
            return SimpleOp::Map;
          }
        }
      }
    }
  }

  return SimpleOp::None;
}

void HhbcTranslator::MInstrTranslator::specializeBaseIfPossible(Type baseType) {
  if (constrainCollectionOpBase()) return;
  if (baseType >= Type::Obj) return;
  if (!baseType.isSpecialized()) return;

  constrainBase(TypeConstraint(baseType.getClass()));
}

bool HhbcTranslator::MInstrTranslator::constrainCollectionOpBase() {
  switch (m_simpleOp) {
    case SimpleOp::None:
      return false;

    case SimpleOp::Array:
    case SimpleOp::ProfiledArray:
    case SimpleOp::String:
      m_irb.constrainValue(m_base.value, DataTypeSpecific);
      return true;

    case SimpleOp::PackedArray:
      constrainBase(TypeConstraint(DataTypeSpecialized).setWantArrayKind());
      return true;

    case SimpleOp::Vector:
    case SimpleOp::Map:
    case SimpleOp::Pair:
      always_assert(m_base.type < Type::Obj &&
                    m_base.type.isSpecialized());
      constrainBase(TypeConstraint(m_base.type.getClass()));
      return true;
  }

  not_reached();
  return false;
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
  setBase(getBase(DataTypeSpecific));
}

void HhbcTranslator::MInstrTranslator::emitBaseN() {
  // If this is ever implemented, the check at the beginning of
  // checkMIState must be removed/adjusted as appropriate.
  PUNT(emitBaseN);
}

void HhbcTranslator::MInstrTranslator::emitBaseG() {
  auto const& mia = m_mii.getAttr(m_ni.immVec.locationCode());
  auto const gblName = getBase(DataTypeSpecific);
  if (!gblName->isA(Type::Str)) PUNT(BaseG-non-string-name);
  setBase(gen(BaseG, MInstrAttrData { mia }, makeEmptyCatch(), gblName));
}

void HhbcTranslator::MInstrTranslator::emitBaseS() {
  const int kClassIdx = m_ni.inputs.size() - 1;
  SSATmp* key = getKey();
  SSATmp* clsRef = getInput(kClassIdx, DataTypeGeneric /* will be a Cls */);

  /*
   * Note, the base may be a pointer to a boxed type after this.  We don't
   * unbox here, because we never are going to generate a special translation
   * unless we know it's not boxed, and the C++ helpers for generic dims
   * currently always conditionally unbox.
   */
  setBase(m_ht.ldClsPropAddr(makeEmptyCatch(), clsRef, key, true));
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

PropInfo HhbcTranslator::MInstrTranslator::getCurrentPropertyOffset(
    const Class*& knownCls) {
  auto const baseType = m_base.type.derefIfPtr();
  if (!knownCls) {
    if (baseType < (Type::Obj|Type::InitNull) && baseType.isSpecialized()) {
      knownCls = baseType.getClass();
    }
  }

  /*
   * TODO(#5616733): If we still don't have a knownCls, we can't do anything
   * good.  It's possible we still have the known information statically, and
   * it might be in m_ni.inputs, but right now we can't really trust that
   * because it's not very clear what it means.  See task for more information.
   */
  if (!knownCls) return PropInfo{};

  auto const info = getPropertyOffset(m_ni, contextClass(), knownCls,
                                      m_mii, m_mInd, m_iInd);
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

  if (m_irb.constrainValue(m_base.value, TypeConstraint(baseCls).setWeak())) {
    // We can't use this specialized class without making a guard more
    // expensive, so don't do it.
    knownCls = nullptr;
    return PropInfo{};
  }
  return info;
}

void HhbcTranslator::MInstrTranslator::emitProp() {
  const Class* knownCls = nullptr;
  const auto propInfo   = getCurrentPropertyOffset(knownCls);
  auto mia = m_mii.getAttr(m_ni.immVecM[m_mInd]);
  if (propInfo.offset == -1 || (mia & Unset) ||
      mightCallMagicPropMethod(mia, knownCls, propInfo)) {
    emitPropGeneric();
  } else {
    emitPropSpecialized(mia, propInfo);
  }
}

void HhbcTranslator::MInstrTranslator::emitPropGeneric() {
  MemberCode mCode = m_ni.immVecM[m_mInd];
  MInstrAttr mia = MInstrAttr(m_mii.getAttr(mCode) & MIA_intermediate_prop);

  if ((mia & Unset) && m_base.type.strip().not(Type::Obj)) {
    constrainBase(DataTypeSpecific);
    setBase(m_irb.genPtrToInitNull());
    return;
  }

  auto const key = getKey();
  if (mia & Define) {
    setBase(gen(PropDX, makeCatch(), MInstrAttrData { mia },
                        m_base.value, key, genMisPtr()));
  } else {
    setBase(gen(PropX, makeCatch(), MInstrAttrData { mia },
                       m_base.value, key, genMisPtr()));
  }
}

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
  auto const key = getKey();
  assert(key->isA(Type::StaticStr));
  assert(baseAsObj->isA(Type::Obj));
  assert(propAddr->type().isPtr());

  auto const needsCheck =
    Type::Uninit <= propAddr->type().deref() &&
    // The m_mInd check is to avoid initializing a property to
    // InitNull right before it's going to be set to something else.
    (doWarn || (doDefine && m_mInd < m_ni.immVecM.size() - 1));

  if (!needsCheck) return propAddr;

  return m_irb.cond(
    0,
    [&] (Block* taken) {
      gen(CheckInitMem, taken, propAddr, cns(0));
    },
    [&] { // Next: Property isn't Uninit. Do nothing.
      return propAddr;
    },
    [&] { // Taken: Property is Uninit. Raise a warning and return
          // a pointer to InitNull, either in the object or
          // init_null_variant.
      m_irb.hint(Block::Hint::Unlikely);
      if (doWarn && wantPropSpecializedWarnings()) {
        gen(RaiseUndefProp, m_ht.makeCatch(), baseAsObj, key);
      }
      if (doDefine) {
        gen(
          StProp,
          baseAsObj,
          cns(propInfo.offset),
          cns(Type::InitNull)
        );
        return propAddr;
      }
      return m_irb.genPtrToInitNull();
    });
}

Class* HhbcTranslator::MInstrTranslator::contextClass() const {
  return m_ht.curFunc()->cls();
}

void HhbcTranslator::MInstrTranslator::emitPropSpecialized(const MInstrAttr mia,
                                                           PropInfo propInfo) {
  assert(!(mia & MIA_warn) || !(mia & MIA_unset));
  const bool doWarn   = mia & MIA_warn;
  const bool doDefine = mia & MIA_define || mia & MIA_unset;

  auto const initNull = m_irb.genPtrToInitNull();

  SCOPE_EXIT {
    // After this function, m_base is either a pointer to init_null_variant or
    // a property in the object that we've verified isn't uninit.
    assert(m_base.type.isPtr());
  };

  /*
   * Normal case, where the base is an object (and not a pointer to
   * something)---just do a lea with the type information we got from static
   * analysis.  The caller of this function will use it to know whether it can
   * avoid a generic incref, unbox, etc.
   */
  if (m_base.type <= Type::Obj) {
    auto const propAddr = gen(
      LdPropAddr,
      convertToType(propInfo.repoAuthType).ptr(Ptr::Prop),
      m_base.value,
      cns(propInfo.offset)
    );
    setBase(checkInitProp(m_base.value, propAddr, propInfo, doWarn, doDefine));
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
  auto const newBase = m_irb.cond(
    0,
    [&] (Block* taken) {
      gen(CheckTypeMem, Type::Obj, taken, m_base.value);
    },
    [&] {
      // Next: Base is an object. Load property and check for uninit.
      auto const obj = gen(
        LdMem,
        m_base.type.deref() & Type::Obj,
        m_base.value,
        cns(0)
      );
      auto const propAddr = gen(
        LdPropAddr,
        convertToType(propInfo.repoAuthType).ptr(Ptr::Prop),
        obj,
        cns(propInfo.offset)
      );
      return checkInitProp(obj, propAddr, propInfo, doWarn, doDefine);
    },
    [&] { // Taken: Base is Null. Raise warnings/errors and return InitNull.
      m_irb.hint(Block::Hint::Unlikely);
      if (doWarn) {
        gen(WarnNonObjProp, makeCatch());
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
  setBase(newBase);
}

void HhbcTranslator::MInstrTranslator::emitElem() {
  MemberCode mCode = m_ni.immVecM[m_mInd];
  MInstrAttr mia = MInstrAttr(m_mii.getAttr(mCode) & MIA_intermediate);
  SSATmp* key = getKey();

  // Fast path for the common/easy case
  const bool warn = mia & Warn;
  const bool unset = mia & Unset;
  const bool define = mia & Define;
  if (m_base.type <= Type::PtrToArr &&
      !unset && !define &&
      (key->isA(Type::Int) || key->isA(Type::Str))) {
    setBase(gen(warn ? ElemArrayW : ElemArray, makeCatch(), m_base.value, key));
    return;
  }

  assert(!(define && unset));
  if (unset) {
    auto const uninit = m_irb.genPtrToUninit();
    auto const baseType = m_base.type.strip();
    constrainBase(DataTypeSpecific);
    if (baseType <= Type::Str) {
      m_ht.exceptionBarrier();
      gen(
        RaiseError,
        makeCatch(),
        cns(makeStaticString(Strings::OP_NOT_SUPPORTED_STRING))
      );
      setBase(uninit);
      return;
    }
    if (baseType.not(Type::Arr | Type::Obj)) {
      setBase(uninit);
      return;
    }
  }

  if (define || unset) {
    setBase(genStk(
      define ? ElemDX : ElemUX,
      makeCatch(),
      MInstrAttrData { mia },
      m_base.value,
      key,
      genMisPtr()
    ));
    return;
  }
  setBase(
    gen(ElemX, makeCatch(), MInstrAttrData { mia }, m_base.value, key,
      genMisPtr())
  );
}

void HhbcTranslator::MInstrTranslator::emitNewElem() {
  PUNT(emitNewElem);
}

void HhbcTranslator::MInstrTranslator::emitRatchetRefs() {
  if (ratchetInd() < 0 || ratchetInd() >= int(nLogicalRatchets())) {
    return;
  }

  setBase(m_irb.cond(
    0,
    [&] (Block* taken) {
      gen(CheckInitMem, taken, m_misBase, cns(MISOFF(tvRef)));
    },
    [&] { // Next: tvRef isn't Uninit. Ratchet the refs
      // Clean up tvRef2 before overwriting it.
      if (ratchetInd() > 0) {
        gen(DecRefMem, Type::Gen, m_misBase, cns(MISOFF(tvRef2)));
      }
      // Copy tvRef to tvRef2. Use mmx at some point
      SSATmp* tvRef = gen(LdMem, Type::Gen, m_misBase, cns(MISOFF(tvRef)));
      gen(StMem, m_misBase, cns(MISOFF(tvRef2)), tvRef);

      // Reset tvRef.
      gen(StMem, m_misBase, cns(MISOFF(tvRef)), cns(Type::Uninit));

      // Adjust base pointer.
      assert(m_base.type.isPtr());
      return gen(LdMIStateAddr, m_misBase, cns(MISOFF(tvRef2)));
    },
    [&] { // Taken: tvRef is Uninit. Do nothing.
      return m_base.value;
    }
  ));
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

void HhbcTranslator::MInstrTranslator::emitCGetProp() {
  const Class* knownCls = nullptr;
  const auto propInfo   = getCurrentPropertyOffset(knownCls);

  if (propInfo.offset != -1 &&
      !mightCallMagicPropMethod(None, knownCls, propInfo)) {
    emitPropSpecialized(MIA_warn, propInfo);

    if (!RuntimeOption::RepoAuthoritative) {
      auto const cellPtr = gen(UnboxPtr, m_base.value);
      m_result = gen(LdMem, Type::Cell, cellPtr, cns(0));
      gen(IncRef, m_result);
      return;
    }

    auto const ty      = m_base.type.deref();
    auto const cellPtr = ty.maybeBoxed() ? gen(UnboxPtr, m_base.value)
                                         : m_base.value;
    m_result = gen(LdMem, ty.unbox(), cellPtr, cns(0));
    gen(IncRef, m_result);
    return;
  }

  auto const key = getKey();
  m_result = gen(CGetProp, makeCatch(), m_base.value, key, genMisPtr());
}

void HhbcTranslator::MInstrTranslator::emitVGetProp() {
  auto const key = getKey();
  m_result = genStk(VGetProp, makeCatch(), NoExtraData{},
                    m_base.value, key, genMisPtr());
}

void HhbcTranslator::MInstrTranslator::emitIssetProp() {
  auto const key = getKey();
  m_result = gen(IssetProp, makeCatch(), m_base.value, key);
}

void HhbcTranslator::MInstrTranslator::emitEmptyProp() {
  auto const key = getKey();
  m_result = gen(EmptyProp, makeCatch(), m_base.value, key);
}

void HhbcTranslator::MInstrTranslator::emitSetProp() {
  SSATmp* value = getValue();

  /* If we know the class for the current base, emit a direct property set. */
  const Class* knownCls = nullptr;
  const auto propInfo   = getCurrentPropertyOffset(knownCls);

  if (propInfo.offset != -1 &&
      !mightCallMagicPropMethod(Define, knownCls, propInfo)) {
    emitPropSpecialized(MIA_define, propInfo);

    auto const propTy  = m_base.type.deref();
    auto const cellTy  = propTy.maybeBoxed() ? propTy.unbox() : propTy;
    auto const cellPtr = propTy.maybeBoxed() ? gen(UnboxPtr, m_base.value)
                                             : m_base.value;
    auto const oldVal  = gen(LdMem, cellTy, cellPtr, cns(0));

    gen(IncRef, value);
    gen(StMem, cellPtr, cns(0), value);
    gen(DecRef, oldVal);
    m_result = value;
    return;
  }

  // Emit the appropriate helper call.
  auto const key = getKey();
  genStk(SetProp, makeCatchSet(), NoExtraData{}, m_base.value, key, value);
  m_result = value;
}

void HhbcTranslator::MInstrTranslator::emitSetOpProp() {
  SetOpOp op = SetOpOp(m_ni.imm[0].u_OA);
  auto const key = getKey();
  auto const value = getValue();
  m_result = genStk(SetOpProp, makeCatch(), SetOpData { op },
                    m_base.value, key, value, genMisPtr());
}

void HhbcTranslator::MInstrTranslator::emitIncDecProp() {
  IncDecOp op = static_cast<IncDecOp>(m_ni.imm[0].u_OA);
  auto const key = getKey();
  m_result = genStk(IncDecProp, makeCatch(), IncDecData { op },
                    m_base.value, key, genMisPtr());
}

void HhbcTranslator::MInstrTranslator::emitBindProp() {
  auto const key = getKey();
  auto const box = getValue();
  genStk(BindProp, makeCatch(), NoExtraData{}, m_base.value, key, box,
    genMisPtr());
  m_result = box;
}

void HhbcTranslator::MInstrTranslator::emitUnsetProp() {
  SSATmp* key = getKey();

  if (m_base.type.strip().not(Type::Obj)) {
    // Noop
    constrainBase(DataTypeSpecific);
    return;
  }

  gen(UnsetProp, makeCatch(), m_base.value, key);
}

SSATmp* HhbcTranslator::MInstrTranslator::emitPackedArrayGet(SSATmp* base,
                                                             SSATmp* key) {
  assert(base->isA(Type::Arr) &&
         base->type().getArrayKind() == ArrayData::kPackedKind);

  auto doLdElem = [&] {
    auto res = gen(LdPackedArrayElem, base, key);
    auto unboxed = m_ht.unbox(res, nullptr);
    gen(IncRef, unboxed);
    return unboxed;
  };

  if (key->isConst() &&
      packedArrayBoundsCheckUnnecessary(base->type(), key->intVal())) {
    return doLdElem();
  }

  return m_irb.cond(
    1,
    [&] (Block* taken) {
      gen(CheckPackedArrayBounds, taken, base, key);
    },
    [&] { // Next:
      return doLdElem();
    },
    [&] { // Taken:
      m_irb.hint(Block::Hint::Unlikely);
      gen(RaiseArrayIndexNotice, makeCatch(), key);
      return cns(Type::InitNull);
    }
  );
}

SSATmp* HhbcTranslator::MInstrTranslator::emitArrayGet(SSATmp* key) {
  return gen(ArrayGet, makeCatch(), m_base.value, key);
}

const StaticString s_PackedArray("PackedArray");

void HhbcTranslator::MInstrTranslator::emitProfiledArrayGet(SSATmp* key) {
  TargetProfile<NonPackedArrayProfile> prof(m_ht.m_context,
                                            m_irb.nextMarker(),
                                            s_PackedArray.get());
  if (prof.profiling()) {
    gen(ProfileArray, RDSHandleData { prof.handle() }, m_base.value);
    m_result = emitArrayGet(key);
    return;
  }

  if (prof.optimizing()) {
    auto const data = prof.data(NonPackedArrayProfile::reduce);
    // NonPackedArrayProfile data counts how many times a non-packed array was
    // observed.  Zero means it was monomorphic (or never executed).
    auto const typePackedArr = Type::Arr.specialize(ArrayData::kPackedKind);
    if (m_base.type.maybe(typePackedArr) &&
        (data.count == 0 || RuntimeOption::EvalJitPGOArrayGetStress)) {
      // It's safe to side-exit still because we only do these profiled array
      // gets on the first element, with simple bases and single-element dims.
      // See computeSimpleCollectionOp.
      auto const exit = m_ht.makeExit();
      setBase(gen(CheckType, typePackedArr, exit, m_base.value));
      m_irb.constrainValue(
        m_base.value, TypeConstraint(DataTypeSpecialized).setWantArrayKind()
      );
      m_result = emitPackedArrayGet(m_base.value, key);
      return;
    }
  }

  // Fall back to a generic array get.
  m_result = emitArrayGet(key);
}

void HhbcTranslator::MInstrTranslator::emitStringGet(SSATmp* key) {
  assert(key->isA(Type::Int));
  m_result = gen(StringGet, makeCatch(), m_base.value, key);
}

void HhbcTranslator::MInstrTranslator::emitVectorGet(SSATmp* key) {
  assert(key->isA(Type::Int));
  if (key->isConst() && key->intVal() < 0) {
    PUNT(emitVectorGet);
  }
  auto const size = gen(LdVectorSize, m_base.value);
  gen(CheckBounds, makeCatch(), key, size);
  auto const base = gen(LdVectorBase, m_base.value);
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
    auto idx = key->intVal();
    if (idx < 0 || idx > 1) {
      PUNT(emitPairGet);
    }
    // no reason to check bounds
    auto const base = gen(LdPairBase, m_base.value);
    auto index = cns(key->intVal() << 4);
    m_result = gen(LdElem, base, index);
  } else {
    gen(CheckBounds, makeCatch(), key, cns(1));
    auto const base = gen(LdPairBase, m_base.value);
    auto idx = gen(Shl, key, cns(4));
    m_result = gen(LdElem, base, idx);
  }
  gen(IncRef, m_result);
}

void HhbcTranslator::MInstrTranslator::emitCGetElem() {
  auto const key = getKey();

  switch (m_simpleOp) {
  case SimpleOp::Array:
    m_result = emitArrayGet(key);
    break;
  case SimpleOp::PackedArray:
    m_result = emitPackedArrayGet(m_base.value, key);
    break;
  case SimpleOp::ProfiledArray:
    emitProfiledArrayGet(key);
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
    m_result = gen(MapGet, makeCatch(), m_base.value, key);
    break;
  case SimpleOp::None:
    m_result = gen(CGetElem, makeCatch(), m_base.value, key, genMisPtr());
    break;
  }
}

void HhbcTranslator::MInstrTranslator::emitVGetElem() {
  auto const key = getKey();
  m_result = genStk(VGetElem, makeCatch(), NoExtraData{},
                    m_base.value, key, genMisPtr());
}

void HhbcTranslator::MInstrTranslator::emitPackedArrayIsset() {
  assert(m_base.type.getArrayKind() == ArrayData::kPackedKind);
  auto const key = getKey();
  m_result = m_irb.cond(
    0,
    [&] (Block* taken) {
      gen(CheckPackedArrayBounds, taken, m_base.value, key);
    },
    [&] { // Next:
      return gen(IsPackedArrayElemNull, m_base.value, key);
    },
    [&] { // Taken:
      return cns(false);
    }
  );
}

void HhbcTranslator::MInstrTranslator::emitIssetElem() {
  switch (m_simpleOp) {
  case SimpleOp::Array:
  case SimpleOp::ProfiledArray:
    m_result = gen(ArrayIsset, makeCatch(), m_base.value, getKey());
    break;
  case SimpleOp::PackedArray:
    emitPackedArrayIsset();
    break;
  case SimpleOp::String:
    m_result = gen(StringIsset, m_base.value, getKey());
    break;
  case SimpleOp::Vector:
    m_result = gen(VectorIsset, m_base.value, getKey());
    break;
  case SimpleOp::Pair:
    m_result = gen(PairIsset, m_base.value, getKey());
    break;
  case SimpleOp::Map:
    m_result = gen(MapIsset, m_base.value, getKey());
    break;
  case SimpleOp::None:
    {
      auto const key = getKey();
      m_result = gen(IssetElem, makeCatch(), m_base.value, key, genMisPtr());
    }
    break;
  }
}

void HhbcTranslator::MInstrTranslator::emitEmptyElem() {
  auto const key = getKey();
  m_result = gen(EmptyElem, makeCatch(), m_base.value, key, genMisPtr());
}

void HhbcTranslator::MInstrTranslator::emitArraySet(SSATmp* key,
                                                    SSATmp* value) {
  assert(m_iInd == m_mii.valCount() + 1);
  const int baseStkIdx = m_mii.valCount();
  assert(key->type().notBoxed());
  assert(value->type().notBoxed());

  auto const& base = *m_ni.inputs[m_mii.valCount()];
  bool const setRef = base.rtt.isBoxed();

  // No catch trace below because the helper can't throw. It may reenter to
  // call destructors so it has a sync point in nativecalls.cpp, but exceptions
  // are swallowed at destructor boundaries right now: #2182869.
  if (setRef) {
    assert(base.location.space == Location::Local ||
           base.location.space == Location::Stack);
    auto const box = getInput(baseStkIdx, DataTypeSpecific);
    gen(ArraySetRef, makeCatch(), m_base.value, key, value, box);
    // Unlike the non-ref case, we don't need to do anything to the stack
    // because any load of the box will be guarded.
    m_result = value;
    return;
  }

  auto const newArr = gen(ArraySet, makeCatch(), m_base.value, key, value);

  // Update the base's value with the new array
  if (base.location.space == Location::Local) {
    // We know it's not boxed (setRef above handles that), and
    // newArr has already been incref'd in the helper.
    gen(StLoc, LocalId(base.location.offset), m_irb.fp(), newArr);
  } else if (base.location.space == Location::Stack) {
    m_ht.extendStack(baseStkIdx, Type::Gen);
    m_irb.evalStack().replace(baseStkIdx, newArr);
  } else {
    not_reached();
  }

  m_result = value;
}

void HhbcTranslator::MInstrTranslator::emitSetWithRefLElem() {
  SSATmp* key = getKey();
  SSATmp* locAddr = getValAddr();
  if (m_base.type.strip() <= Type::Arr &&
      !locAddr->type().deref().maybeBoxed()) {
    constrainBase(DataTypeSpecific);
    emitSetElem();
    assert(m_strTestResult == nullptr);
  } else {
    genStk(SetWithRefElem, makeCatch(), NoExtraData{},
           m_base.value, key, locAddr, genMisPtr());
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
  if (m_base.type.strip() <= Type::Arr && getValue()->type().notBoxed()) {
    constrainBase(DataTypeSpecific);
    emitSetNewElem();
  } else {
    genStk(SetWithRefNewElem, makeCatch(),
           NoExtraData{},
           m_base.value, getValAddr(), genMisPtr());
  }
  m_result = nullptr;
}

void HhbcTranslator::MInstrTranslator::emitVectorSet(
    SSATmp* key, SSATmp* value) {
  assert(key->isA(Type::Int));
  if (key->isConst() && key->intVal() < 0) {
    PUNT(emitVectorSet); // will throw
  }
  SSATmp* size = gen(LdVectorSize, m_base.value);
  gen(CheckBounds, makeCatch(), key, size);

  m_irb.ifThen([&](Block* taken) {
          gen(VectorHasImmCopy, taken, m_base.value);
        },
        [&] {
          m_irb.hint(Block::Hint::Unlikely);
          gen(VectorDoCow, m_base.value);
        });

  gen(IncRef, value);
  SSATmp* vecBase = gen(LdVectorBase, m_base.value);
  SSATmp* oldVal;
  static_assert(sizeof(TypedValue) == 16,
                "TypedValue size expected to be 16 bytes");
  auto idx = gen(Shl, key, cns(4));
  oldVal = gen(LdElem, vecBase, idx);
  gen(StElem, vecBase, idx, value);
  gen(DecRef, oldVal);

  m_result = value;
}

void HhbcTranslator::MInstrTranslator::emitSetElem() {
  SSATmp* value = getValue();
  SSATmp* key = getKey();

  switch (m_simpleOp) {
  case SimpleOp::Array:
  case SimpleOp::ProfiledArray:
    emitArraySet(key, value);
    break;
  case SimpleOp::PackedArray:
  case SimpleOp::String:
    always_assert(false && "Bad SimpleOp in emitSetElem");
    break;
  case SimpleOp::Vector:
    emitVectorSet(key, value);
    break;
  case SimpleOp::Map:
    gen(MapSet, makeCatch(), m_base.value, key, value);
    m_result = value;
    break;
  case SimpleOp::Pair:
  case SimpleOp::None:
    constrainBase(DataTypeSpecific);
    SSATmp* result = genStk(SetElem, makeCatchSet(), NoExtraData{},
                            m_base.value, key, value);
    auto t = result->type();
    if (t == Type::Nullptr) {
      // Base is not a string. Result is always value.
      m_result = value;
    } else if (t == Type::CountedStr) {
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

void HhbcTranslator::MInstrTranslator::emitSetOpElem() {
  SetOpOp op = static_cast<SetOpOp>(m_ni.imm[0].u_OA);
  m_result = genStk(SetOpElem, makeCatch(), SetOpData{op},
                    m_base.value, getKey(), getValue(), genMisPtr());
}

void HhbcTranslator::MInstrTranslator::emitIncDecElem() {
  IncDecOp op = static_cast<IncDecOp>(m_ni.imm[0].u_OA);
  m_result = genStk(IncDecElem, makeCatch(), IncDecData { op },
                    m_base.value, getKey(), genMisPtr());
}

void HhbcTranslator::MInstrTranslator::emitBindElem() {
  auto const key = getKey();
  auto const box = getValue();
  genStk(BindElem, makeCatch(), NoExtraData{},
         m_base.value, key, box, genMisPtr());
  m_result = box;
}

void HhbcTranslator::MInstrTranslator::emitUnsetElem() {
  auto const key = getKey();

  auto const baseType = m_base.type.strip();
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

  genStk(UnsetElem, makeCatch(), NoExtraData{}, m_base.value, key);
}

void HhbcTranslator::MInstrTranslator::emitNotSuppNewElem() {
  PUNT(NotSuppNewElem);
}

void HhbcTranslator::MInstrTranslator::emitVGetNewElem() {
  SPUNT(__func__);
}

void HhbcTranslator::MInstrTranslator::emitSetNewElem() {
  SSATmp* value = getValue();
  if (m_base.type <= Type::PtrToArr) {
    constrainBase(DataTypeSpecific);
    gen(SetNewElemArray, makeCatchSet(), m_base.value, value);
  } else {
    gen(SetNewElem, makeCatchSet(), m_base.value, value);
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
  genStk(BindNewElem, makeCatch(), NoExtraData{}, m_base.value, box,
    genMisPtr());
  m_result = box;
}

void HhbcTranslator::MInstrTranslator::emitMPost() {
  SSATmp* catchSp = nullptr;
  if (m_failedSetBlock) {
    auto* endCatch = &m_failedSetBlock->back();
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
      auto input = getInput(i, DataTypeSpecific);
      if (input->isA(Type::Gen)) {
        gen(DecRef, input);
        if (m_failedSetBlock) {
          BlockPauser bp(m_irb, m_marker, m_failedSetBlock);
          gen(DecRefStack, StackOffset(m_stackInputs[i]), Type::Gen, catchSp);
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
  static constexpr size_t refOffs[] = { MISOFF(tvRef), MISOFF(tvRef2) };
  for (unsigned i = 0; i < std::min(nLogicalRatchets(), 2U); ++i) {
    IRInstruction* inst = m_unit.gen(DecRefMem, m_marker, Type::Gen, m_misBase,
                                    cns(refOffs[m_failedSetBlock ? 1 - i : i]));
    m_irb.add(inst);
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

    // Need to save FP, we're switching to our side exit block, but it hasn't
    // had a predecessor propagate state to it via FrameStateMgr::finishBlock
    // yet.
    auto const fp = m_irb.fp();

    BlockPusher bp(m_irb, m_marker, m_failedSetBlock);
    if (!isSetWithRef) {
      gen(DecRefStack, StackOffset(0), Type::Cell, catchSp);
      args.push_back(m_ht.gen(LdUnwinderValue, Type::Cell));
    }

    SSATmp* sp = gen(SpillStack, std::make_pair(args.size(), &args[0]));
    gen(DeleteUnwinderException);
    gen(SyncABIRegs, fp, sp);
    gen(ReqBindJmp, ReqBindJmpData(nextOff));
  }

  if (m_strTestResult) {
    assert(!isSetWithRef);
    // We expected SetElem's base to not be a Str but might be wrong. Make an
    // exit trace to side exit to the next instruction, replacing our guess
    // with the correct stack output.

    auto toSpill = m_ht.peekSpillValues();
    assert(toSpill.size());
    assert(toSpill.back() == m_result);
    SSATmp* str = m_unit.gen(AssertNonNull, m_marker, m_strTestResult)->dst();
    toSpill.back() = str;

    auto exit = m_ht.makeExit(nextOff, toSpill);
    {
      BlockPauser tp(m_irb, m_marker, exit, exit->skipHeader());
      gen(IncStat, cns(Stats::TC_SetMStrGuess_Miss), cns(1), cns(false));
      gen(DecRef, m_result);
      m_irb.add(str->inst());
    }
    gen(CheckNullptr, exit, m_strTestResult);
    gen(IncStat, cns(Stats::TC_SetMStrGuess_Hit), cns(1), cns(false));
  }
}

Block* HhbcTranslator::MInstrTranslator::makeEmptyCatch() {
  return m_ht.makeCatch();
}

Block* HhbcTranslator::MInstrTranslator::makeCatch() {
  auto b = makeEmptyCatch();
  m_failedVec.push_back(b);
  return b;
}

Block* HhbcTranslator::MInstrTranslator::makeCatchSet() {
  assert(!m_failedSetBlock);
  m_failedSetBlock = makeCatch();

  // This catch trace will be modified in emitMPost to end with a side
  // exit, and TryEndCatch will fall through to that side exit if an
  // InvalidSetMException is thrown.
  m_failedSetBlock->back().setOpcode(TryEndCatch);
  return m_failedSetBlock;
}

void HhbcTranslator::MInstrTranslator::prependToTraces(IRInstruction* inst) {
  for (auto b : m_failedVec) {
    b->prepend(m_unit.cloneInstruction(inst));
  }
}

bool HhbcTranslator::MInstrTranslator::needFirstRatchet() const {
  auto const firstTy = m_ni.inputs[m_mii.valCount()]->rtt.unbox();
  if (firstTy <= Type::Arr) {
    if (mcodeIsElem(m_ni.immVecM[0])) return false;
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
    if (mcodeIsProp(m_ni.immVecM[0])) return false;
    return true;
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
