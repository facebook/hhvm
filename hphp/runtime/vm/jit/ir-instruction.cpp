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

#include "hphp/runtime/vm/jit/ir-instruction.h"

#include "hphp/runtime/base/bespoke-array.h"
#include "hphp/runtime/base/implicit-context.h"
#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/base/type-structure-helpers-defs.h"
#include "hphp/runtime/vm/func.h"

#include "hphp/runtime/base/bespoke/logging-array.h"
#include "hphp/runtime/base/bespoke/type-structure.h"

#include "hphp/runtime/vm/jit/analysis.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/edge.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/irgen-builtin.h"
#include "hphp/runtime/vm/jit/irgen-call.h"
#include "hphp/runtime/vm/jit/ir-instr-table.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/simplify.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/type-array-elem.h"
#include "hphp/runtime/vm/jit/type.h"

#include "hphp/runtime/ext/asio/ext_async-function-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_async-generator.h"
#include "hphp/runtime/ext/asio/ext_async-generator-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_concurrent-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_static-wait-handle.h"
#include "hphp/runtime/ext/functioncredential/ext_functioncredential.h"
#include "hphp/runtime/ext/generator/ext_generator.h"

#include "hphp/util/arena.h"

#include <folly/Range.h>

#include <algorithm>
#include <sstream>

namespace HPHP::jit {
//////////////////////////////////////////////////////////////////////

IRInstruction::IRInstruction(Arena& arena, const IRInstruction* inst, Id id)
  : m_typeParam(inst->m_typeParam)
  , m_op(inst->m_op)
  , m_iroff(inst->m_iroff)
  , m_numSrcs(inst->m_numSrcs)
  , m_numDsts(inst->m_numDsts)
  , m_hasTypeParam{inst->m_hasTypeParam}
  , m_marker(inst->m_marker)
  , m_id(id)
  , m_srcs(m_numSrcs ? new (arena) SSATmp*[m_numSrcs] : nullptr)
  , m_dest(nullptr)
  , m_extra(inst->m_extra ? cloneExtra(op(), inst->m_extra, arena)
                          : nullptr)
{
  assertx(!isTransient());
  std::copy(inst->m_srcs, inst->m_srcs + inst->m_numSrcs, m_srcs);
  if (hasEdges()) {
    m_edges = new (arena) Edge[2];
    m_edges[0].setInst(this);
    m_edges[0].setTo(inst->next());
    m_edges[1].setInst(this);
    m_edges[1].setTo(inst->taken());
  } else {
    m_edges = nullptr;
  }
}

std::string IRInstruction::toString() const {
  std::ostringstream str;
  print(str, this);
  return str.str();
}

///////////////////////////////////////////////////////////////////////////////

void IRInstruction::convertToNop() {
  if (hasEdges()) clearEdges();
  IRInstruction nop(Nop, bcctx());
  m_op           = nop.m_op;
  m_typeParam    = nop.m_typeParam;
  m_numSrcs      = nop.m_numSrcs;
  m_srcs         = nop.m_srcs;
  m_numDsts      = nop.m_numDsts;
  m_hasTypeParam = nop.m_hasTypeParam;
  m_dest         = nop.m_dest;
  m_extra        = nullptr;
}

void IRInstruction::become(IRUnit& unit, const IRInstruction* other) {
  assertx(other->isTransient() || m_numDsts == other->m_numDsts);
  auto& arena = unit.arena();

  if (hasEdges()) clearEdges();

  m_op = other->m_op;
  m_typeParam = other->m_typeParam;
  m_hasTypeParam = other->m_hasTypeParam;
  m_numSrcs = other->m_numSrcs;
  m_extra = other->m_extra ? cloneExtra(m_op, other->m_extra, arena) : nullptr;
  m_srcs = new (arena) SSATmp*[m_numSrcs];
  std::copy(other->m_srcs, other->m_srcs + m_numSrcs, m_srcs);

  if (hasEdges()) {
    assertx(other->hasEdges());  // m_op is from other now
    m_edges = new (arena) Edge[2];
    m_edges[0].setInst(this);
    m_edges[1].setInst(this);
    setNext(other->next());
    setTaken(other->taken());
  }
}

///////////////////////////////////////////////////////////////////////////////

enum MoveKind {
  Consume,
  MustMove,
  MayMove,
};

template<MoveKind move>
bool consumesRefImpl(const IRInstruction* inst, int srcNo) {
  if (!inst->consumesReferences()) {
    return false;
  }

  switch (inst->op()) {
    case ConcatStrStr:
    case ConcatStrInt:
    case ConcatStr3:
    case ConcatStr4:
      // Call a helper that decrefs the first argument
      return move == Consume && srcNo == 0;

    case ConstructClosure:
      return srcNo == 0;

    case StClosureArg:
    case StContArValue:
    case StContArKey:
      return srcNo == 1;

    case ContEnter:
      return move != MustMove && srcNo == 4;

    case Call:
      return move != MustMove && srcNo == 3;

    case CallFuncEntry:
      return move != MustMove && srcNo == 2;

    case EnterTCUnwind:
      return srcNo == 2;

    case RaiseTooManyArg:
      // RaiseTooManyArg decrefs the unpack arguments.
      return move == Consume && srcNo == 0;

    case AFWHBlockOn:
      // Consume the value being stored, not the thing it's being stored into
      return srcNo == 1;

    case DictSet:
    case BespokeSet:
      // Consumes the reference to its input array, and moves input value
      return move == Consume && (srcNo == 0 || srcNo == 2);

    case BespokeUnset:
    case StructDictUnset:
      // Only consumes the reference to its input array
      return move == Consume && srcNo == 0;

    case BespokeAppend:
      // Consumes the reference to its input array, and moves input value (both
      // parameters).
      return move == Consume;

    case MapSet:
    case VectorSet:
      // Moves input value
      return move == Consume && srcNo == 2;

    case AddNewElemKeyset:
    case AddNewElemVec:
      // Only consumes the reference to its input array
      return move == Consume && srcNo == 0;

    case CreateAFWH:
      return srcNo == 4;

    case CreateAGWH:
      return srcNo == 3;

    case CreateSSWH:
      return srcNo == 0;

    case InitDictElem:
    case InitVecElem:
    case InitStructElem:
      return srcNo == 1;

    case InitVecElemLoop:
      return srcNo > 0;

    case NewPair:
    case NewColFromArray:
      return true;

    case VerifyParamCoerce:
    case VerifyRetCoerce:
      return move != MustMove && srcNo == 0;

    case VerifyPropCoerce:
    case VerifyPropCoerceAll:
      return move != MustMove && srcNo == 2;

    default:
      return move != MustMove;
  }
}

bool IRInstruction::consumesReference(int srcNo) const {
  return consumesRefImpl<Consume>(this, srcNo);
}

bool IRInstruction::movesReference(int srcNo) const {
  return consumesRefImpl<MustMove>(this, srcNo);
}

bool IRInstruction::mayMoveReference(int srcNo) const {
  return consumesRefImpl<MayMove>(this, srcNo);
}

///////////////////////////////////////////////////////////////////////////////

void IRInstruction::setOpcode(Opcode newOpc) {
  assertx(hasEdges() || !jit::hasEdges(newOpc)); // cannot allocate new edges
  if (hasEdges() && !jit::hasEdges(newOpc)) {
    clearEdges();
  }
  m_op = newOpc;
}

SSATmp* IRInstruction::dst(unsigned i) const {
  if (i == 0 && m_numDsts == 0) return nullptr;
  assertx(i < m_numDsts);
  assertx(naryDst() || i == 0);
  return hasDst() ? dst() : m_dsts[i];
}

///////////////////////////////////////////////////////////////////////////////

Type thisTypeFromFunc(const Func* func) {
  assertx(func && func->cls());
  // If the function is a cloned closure which may have a re-bound $this which
  // is not a subclass of the context return an unspecialized type.
  return func->hasForeignThis() ? TObj : Type::SubObj(func->cls());
}

Type callCtxType(const Func* func) {
  assertx(func);
  assertx(func->isClosureBody() || func->cls());
  if (func->isClosureBody()) return Type::ExactObj(func->implCls());
  if (func->isStatic()) return Type::SubCls(func->cls());
  return thisTypeFromFunc(func);
}

///////////////////////////////////////////////////////////////////////////////
// outputType().

namespace {

Type allocObjReturn(const IRInstruction* inst) {
  switch (inst->op()) {
    case ConstructClosure:
    case ConstructInstance:
      return Type::ExactObj(inst->extra<ClassData>()->cls);

    case NewInstanceRaw:
      return Type::ExactObj(inst->extra<NewInstanceRaw>()->cls);

    case AllocObj:
      if (auto spec = inst->src(0)->type().clsSpec()) {
        auto const cls = spec.cls();
        return spec.exact() ? Type::ExactObj(cls) : Type::SubObj(cls);
      }
      return TObj;

    case CreateGen:
      return Type::ExactObj(Generator::classof());

    case CreateAGen:
      return Type::ExactObj(AsyncGenerator::classof());

    case CreateAFWH:
      return Type::ExactObj(c_AsyncFunctionWaitHandle::classof());

    case CreateAGWH:
      return Type::ExactObj(c_AsyncGeneratorWaitHandle::classof());

    case CreateCCWH:
      return Type::ExactObj(c_ConcurrentWaitHandle::classof());

    case CreateSSWH:
      return Type::ExactObj(c_StaticWaitHandle::classof());

    case FuncCred:
      return Type::ExactObj(FunctionCredential::classof());

    default:
      always_assert(false && "Invalid opcode returning AllocObj");
  }
}

/*
* Analyze the type of return element (key or value) for different container.
*/
Type arrFirstLastReturn(const IRInstruction* inst, bool first, bool isKey) {
  auto elem = arrLikeFirstLastType(
      inst->src(0)->type(), first, isKey, inst->ctx());
  if (!elem.second) elem.first |= TInitNull;
  return elem.first;
}

Type bespokeElemReturn(const IRInstruction* inst, bool present) {
  assertx(inst->src(0)->type() <= TArrLike);

  auto resultType = arrLikeElemType(
      inst->src(0)->type(), inst->src(1)->type(), inst->ctx());

  auto const knownPresent = [&] {
    if (present || resultType.second) return true;
    if (!inst->is(BespokeGet)) return false;
    auto const keyState = inst->extra<BespokeGet>()->state;
    return keyState == BespokeGetData::KeyState::Present;
  }();

  return knownPresent ? resultType.first : (resultType.first | TUninit);
}

Type elemLval(const IRInstruction* inst) {
  assertx(inst->is(ElemDictD, ElemDictU, BespokeElem));
  assertx(inst->typeParam() <= TArrLike);

  auto const elem = arrLikeElemType(
    inst->typeParam(),
    inst->src(1)->type(),
    inst->ctx()
  );
  auto lval = (elem.first <= TBottom) ? TBottom : TLvalToElem;

  if (!elem.second) {
    if (inst->is(ElemDictU) ||
        (inst->is(BespokeElem) && !inst->src(2)->boolVal())) {
      lval |= TLvalToConst;
    }
  }
  return lval;
}

Type elemLvalPos(const IRInstruction* inst) {
  assertx(inst->is(ElemDictK, LdVecElemAddr));

  auto const elem = [&] {
    if (inst->is(ElemDictK)){
      return arrLikePosType(
        inst->src(3)->type(),
        inst->src(2)->type(),
        false,
        inst->ctx()
      );
    }
    return arrLikeElemType(
      inst->src(2)->type(),
      inst->src(1)->type(),
      inst->ctx()
    ).first;
  }();
  if (elem <= TBottom) return TBottom;
  return TLvalToElem;
}

Type bespokePosReturn(const IRInstruction* inst, bool isKey) {
  assertx(inst->src(0)->type() <= TArrLike);

  auto resultType = arrLikePosType(
    inst->src(0)->type(),
    inst->src(1)->type(),
    isKey,
    inst->ctx()
  );
  return resultType;
}

Type vecElemReturn(const IRInstruction* inst) {
  assertx(inst->is(LdVecElem));
  assertx(inst->src(0)->isA(TVec));
  assertx(inst->src(1)->isA(TInt));

  auto resultType = arrLikeElemType(
      inst->src(0)->type(), inst->src(1)->type(), inst->ctx()).first;
  return resultType;
}

Type dictElemReturn(const IRInstruction* inst) {
  assertx(inst->is(DictGet, DictGetK, DictGetQuiet, DictIdx));
  assertx(inst->src(0)->isA(TDict));
  assertx(inst->src(1)->isA(TInt | TStr));

  auto elem = [&] {
    if (inst->is(DictGetK)) {
      auto const type = arrLikePosType(
        inst->src(0)->type(),
        inst->src(2)->type(),
        false,
        inst->ctx()
      );
      return std::make_pair(type, true);
    }
    return arrLikeElemType(
      inst->src(0)->type(),
      inst->src(1)->type(),
      inst->ctx()
    );
  }();

  if (!elem.second) {
    if (inst->is(DictGetQuiet)) elem.first |= TInitNull;
    if (inst->is(DictIdx)) elem.first |= inst->src(2)->type();
  }
  return elem.first;
}

Type keysetElemReturn(const IRInstruction* inst) {
  assertx(inst->is(KeysetGet, KeysetGetK, KeysetGetQuiet, KeysetIdx));
  assertx(inst->src(0)->isA(TKeyset));
  assertx(inst->src(1)->isA(TInt | TStr));

  auto elem = [&] {
    if (inst->is(KeysetGetK)) {
      auto const type = arrLikePosType(
        inst->src(0)->type(),
        inst->src(2)->type(),
        false,
        inst->ctx()
      );
      return std::make_pair(type, true);
    }
    return arrLikeElemType(
      inst->src(0)->type(),
      inst->src(1)->type(),
      inst->ctx()
    );
  }();

  if (!elem.second) {
    if (inst->is(KeysetGetQuiet)) elem.first |= TInitNull;
    if (inst->is(KeysetIdx)) elem.first |= inst->src(2)->type();
  }
  return elem.first;
}

Type setElemReturn(const IRInstruction* inst) {
  assertx(inst->op() == SetElem);
  auto const baseType = inst->typeParam();

  // If the base is a Str, the result will always be a StaticStr (or
  // an exception). If the base might be a str, the result wil be
  // StaticStr or Nullptr. Otherwise, the result is always Nullptr.
  if (baseType <= TStr) {
    return TStaticStr;
  } else if (baseType.maybe(TStr)) {
    return TStaticStr | TNullptr;
  }
  return TNullptr;
}

Type newColReturn(const IRInstruction* inst) {
  assertx(inst->is(NewCol, NewPair, NewColFromArray));
  auto getColClassType = [&](CollectionType ct) -> Type {
    auto name = collections::typeToString(ct);
    auto cls = Class::lookupUniqueInContext(name, inst->ctx(), nullptr);
    if (cls == nullptr) return TObj;
    return Type::ExactObj(cls);
  };

  if (inst->is(NewCol)) {
    return getColClassType(inst->extra<NewCol>()->type);
  } else if (inst->is(NewPair)) {
    return getColClassType(CollectionType::Pair);
  }
  return getColClassType(inst->extra<NewColFromArray>()->type);
}

Type builtinReturn(const IRInstruction* inst) {
  assertx(inst->is(CallBuiltin));
  return irgen::builtinReturnType(inst->extra<CallBuiltin>()->callee);
}

Type callReturn(const IRInstruction* inst) {
  assertx(inst->is(Call, CallFuncEntry, InlineSideExit));

  // Async eager return needs to load TVAux.
  //
  // Do not use the inferred Func* if we are forming a region. We may have
  // inferred the target of the call based on specialized type information
  // that won't be available when the region is translated. If we allow the
  // FCall to specialize using this information, we may infer narrower type
  // for the return value, erroneously preventing the region from breaking
  // on unknown type.

  switch (inst->op()) {
    case Call: {
      auto const extra = inst->extra<Call>();
      if (extra->asyncEagerReturn) return TInitCell;
      if (extra->formingRegion) return TInitCell;
      return inst->src(2)->hasConstVal(TFunc)
        ? irgen::callReturnType(inst->src(2)->funcVal()) : TInitCell;
    }
    case CallFuncEntry: {
      auto const extra = inst->extra<CallFuncEntry>();
      if (extra->asyncEagerReturn()) return TInitCell;
      if (extra->formingRegion) return TInitCell;
      return irgen::callReturnType(extra->target.func());
    }
    case InlineSideExit: {
      auto const extra = inst->extra<InlineSideExit>();
      return irgen::callReturnType(extra->callee);
    }
    default:
      not_reached();
  }
}

Type genIterReturn(const IRInstruction* inst) {
  assertx(inst->is(ContEnter));
  return inst->extra<ContEnter>()->isAsync
    ? Type::SubObj(c_Awaitable::classof())
    : TInitNull;
}

// Integers get mapped to integer memo keys, everything else gets mapped to
// strings.
Type memoKeyReturn(const IRInstruction* inst) {
  assertx(inst->is(GetMemoKey, GetMemoKeyScalar));
  auto const srcType = inst->src(0)->type();
  if (srcType <= TInt) return TInt;
  if (!srcType.maybe(TInt)) return TStr;
  return TInt | TStr;
}

Type ptrToLvalReturn(const IRInstruction* inst) {
  auto const ptr = inst->src(0)->type();
  assertx(ptr <= TPtr);
  return ptr.ptrToLval();
}

Type loggingArrLikeReturn(const IRInstruction* inst) {
  assertx(inst->is(NewLoggingArray));
  auto const arr = inst->src(0)->type();
  auto const isStatic = inst->extra<NewLoggingArray>()->isStatic;

  assertx(arr <= TArrLike);
  assertx(arr.isKnownDataType());
  assertx(IMPLIES(isStatic, arr.hasConstVal()));
  return isStatic ? arr.unspecialize() : arr.unspecialize().modified();
}

Type structDictReturn(const IRInstruction* inst) {
  assertx(inst->is(AllocBespokeStructDict, NewBespokeStructDict));
  auto const layout = inst->is(AllocBespokeStructDict)
    ? inst->extra<AllocBespokeStructDict>()->layout
    : inst->extra<NewBespokeStructDict>()->layout;
  return TDict.narrowToLayout(layout);
}

Type typeStructElemReturn(const IRInstruction* inst) {
  assertx(inst->is(LdTypeStructureValCns));
  auto const key = inst->extra<KeyedData>()->key;
  auto const dt = bespoke::TypeStructure::getFieldPair(key).first;
  if (dt == KindOfBoolean) return TBool;
  if (dt == KindOfInt64) return TInt;
  if (dt == KindOfVec) return TVec|TNullptr;
  if (dt == KindOfDict) return TDict|TNullptr;
  if (isStringType(dt)) return TStr|TNullptr;
  assertx(dt == KindOfUninit);
  return TNullptr;
}

Type arrLikeSetReturn(const IRInstruction* inst) {
  assertx(inst->is(BespokeSet, DictSet));
  auto const arr = inst->src(0)->type();
  auto const key = inst->src(1)->type();
  auto const val = inst->src(2)->type();

  assertx(arr <= TArrLike);
  assertx(arr.isKnownDataType());
  assertx(key.subtypeOfAny(TInt, TStr));
  auto const base = arr.modified() & TCounted;
  auto const layout = arr.arrSpec().layout().setType(key, val);
  return base.narrowToLayout(layout);
}

Type arrLikeUnsetReturn(const IRInstruction* inst) {
  assertx(inst->is(BespokeUnset, StructDictUnset));
  auto const arr = inst->src(0)->type();
  auto const key = inst->src(1)->type();

  assertx(arr <= TArrLike);
  assertx(arr.isKnownDataType());
  assertx(key.subtypeOfAny(TInt, TStr));
  auto const base = arr | (TCounted & arr.modified());
  auto const layout = arr.arrSpec().layout().removeType(key);
  return base.narrowToLayout(layout);
}

Type arrLikeAppendReturn(const IRInstruction* inst) {
  assertx(inst->is(BespokeAppend));
  auto const arr = inst->src(0)->type();
  auto const val = inst->src(1)->type();

  assertx(arr <= TArrLike);
  assertx(arr.isKnownDataType());
  auto const base = arr <= TDict
    ? (arr | TCountedDict)
    : (arr.modified() & TCounted);
  auto const layout = arr.arrSpec().layout().appendType(val);
  return base.narrowToLayout(layout);
}

Type typeCnsClsName(const IRInstruction* inst) {
  assertx(inst->is(LdResolvedTypeCnsClsName));
  auto const clsTmp = inst->src(0);
  auto const clsSpec = clsTmp->type().clsSpec();
  if (!clsSpec) return TStaticStr | TNullptr;
  auto const cls = clsSpec.cls();
  auto const extra = inst->extra<LdResolvedTypeCnsClsName>();

  assertx(cls->hasTypeConstant(extra->cnsName, true));
  assertx(extra->slot < cls->numConstants());
  auto const& typeCns = cls->constants()[extra->slot];
  auto const& resolved = typeCns.preConst->resolvedTypeStructure();

  auto const classname = [&] {
    auto const name = resolved->get(s_classname);
    if (tvIsString(name)) return Type::cns(name);
    return TNullptr;
  };

  if (clsSpec.exact()) {
    if (typeCns.isAbstractAndUninit()) return TNullptr;
    if (resolved.isNull()) return TStaticStr | TNullptr;
    return classname();
  }
  if (resolved.isNull()) return TStaticStr | TNullptr;

  switch (typeCns.preConst->invariance()) {
    case PreClass::Const::Invariance::None:
    case PreClass::Const::Invariance::Present:
      return TStaticStr | TNullptr;
    case PreClass::Const::Invariance::ClassnamePresent:
      return TStaticStr;
    case PreClass::Const::Invariance::Same:
      return classname();
  }
  always_assert(false);
}

Type verifyCoerceReturn(const IRInstruction* inst) {
  auto const [tc, srcTy] = [&]() {
    if (inst->is(VerifyParamCoerce, VerifyRetCoerce)) {
      return std::make_tuple(
        inst->extra<FuncParamWithTCData>()->tc,
        inst->src(0)->type()
      );
    }
    if (inst->is(VerifyPropCoerce)) {
      return std::make_tuple(
        inst->extra<TypeConstraintData>()->tc,
        inst->src(2)->type()
      );
    }
    always_assert(false);
  }();

  auto dstTy = srcTy;
  if (tc->maybeStringCompatible() &&
      (srcTy.maybe(TCls) || srcTy.maybe(TLazyCls))) {
    dstTy |= TPersistentStr;
  }
  return dstTy;
}

Type propLval(const IRInstruction* inst) {
  assertx(inst->is(PropX, PropDX, PropQ));
  auto const tvRef = inst->src(2);
  if (tvRef->isA(TNullptr)) return TLvalToProp|TLvalToConst;
  return TLvalToProp|TLvalToConst|TLvalToMISTemp;
}

Type cowReturn(const IRInstruction* inst) {
  assertx(inst->is(CopyArray, CheckArrayCOW));
  auto const arr = inst->src(0)->type();
  assertx(arr <= TArrLike);
  auto const modified = arr.modified() & TCounted;
  return modified.narrowToLayout(arr.arrSpec().layout());
}

Type structDictTypeBoundCheckReturn(const IRInstruction* inst) {
  assertx(inst->is(StructDictTypeBoundCheck));

  auto const type = [&] {
    auto const arr = inst->src(1);
    auto const& layout = arr->type().arrSpec().layout();
    assertx(layout.is_struct());
    return layout.getTypeBound(inst->src(2)->type());
  }();
  return inst->src(0)->type() & type;
}

Type specialICReturn(const IRInstruction* inst) {
  assertx(inst->is(CreateSpecialImplicitContext));

  auto const type = inst->src(0)->type();
  auto const memoKey = inst->src(1)->type();
  auto const func = inst->src(2)->type();

  if (!type.hasConstVal(TInt)) return TObj|TInitNull;

  switch (static_cast<ImplicitContext::State>(type.intVal())) {
    case ImplicitContext::State::Value:
      return TObj|TInitNull;
    case ImplicitContext::State::SoftInaccessible: {
      auto const sampleRate = [&] () -> uint32_t {
        if (!memoKey.maybe(TInitNull)) return 1;
        if (!func.hasConstVal(TFunc)) return 0;
        auto const f = func.funcVal();
        assertx(f->isMemoizeWrapper() || f->isMemoizeWrapperLSB());
        assertx(f->isSoftMakeICInaccessibleMemoize());
        return f->softMakeICInaccessibleSampleRate();
      }();
      return sampleRate == 1 ? TObj : (TObj | TInitNull);
    }
    case ImplicitContext::State::Inaccessible:
    case ImplicitContext::State::SoftSet:
      return TObj;
  }
  always_assert(false);
}

// Is this instruction an array cast that always modifies the type of the
// input array? Such casts are guaranteed to return vanilla arrays.
bool isNontrivialArrayCast(const IRInstruction* inst) {
  auto const& type = inst->src(0)->type();
  if (inst->is(ConvArrLikeToVec))    return !type.maybe(TVec);
  if (inst->is(ConvArrLikeToDict))   return !type.maybe(TDict);
  if (inst->is(ConvArrLikeToKeyset)) return !type.maybe(TKeyset);
  return false;
}

template <uint32_t...> struct IdxSeq {};

inline Type unionReturn(const IRInstruction* /*inst*/, IdxSeq<>) {
  return TBottom;
}

template <uint32_t Idx, uint32_t... Rest>
inline Type unionReturn(const IRInstruction* inst, IdxSeq<Idx, Rest...>) {
  assertx(Idx < inst->numSrcs());
  return inst->src(Idx)->type() | unionReturn(inst, IdxSeq<Rest...>{});
}

} // namespace

Type outputType(const IRInstruction* inst, int /*dstId*/) {
  for (auto const src : inst->srcs()) {
    if (src->type() == TBottom) return TBottom;
  }

  // Don't produce vanilla types if the bespoke runtime checks flag is off,
  // because we never use these types. Otherwise, apply layout-dependence.
  auto const checkLayoutFlags = [&](Type t) {
    if (!allowBespokeArrayLikes()) return t;
    if (inst->isLayoutPreserving()) {
      assertx(inst->src(0)->type() <= TArrLike);
      auto const vanilla = inst->src(0)->type().arrSpec().vanilla() ||
                           isNontrivialArrayCast(inst);
      return vanilla ? t.narrowToVanilla() : t;
    } else if (inst->isLayoutAgnostic()) {
      return t;
    } else {
      return t.narrowToVanilla();
    }
  };
  using namespace TypeNames;
  using TypeNames::TCA;
#define ND              assertx(0 && "outputType requires HasDest or NaryDest");
#define D(type)         return checkLayoutFlags(type);
#define DofS(n)         return inst->src(n)->type();
#define DRefineS(n)     return inst->src(n)->type() & inst->typeParam();
#define DParam(t)       return inst->typeParam();
#define DEscalateToVanilla return inst->src(0)->type().modified().\
                               narrowToVanilla();
#define DLdObjCls {                                                \
  if (auto spec = inst->src(0)->type().clsSpec()) {                \
    auto const cls = spec.cls();                                   \
    return spec.exact() ? Type::ExactCls(cls) : Type::SubCls(cls); \
  }                                                                \
  return TCls;                                                     \
}
#define DAllocObj       return allocObjReturn(inst);
#define DBespokeElem          return bespokeElemReturn(inst, true);
#define DBespokeElemUninit    return bespokeElemReturn(inst, false);
#define DBespokePosKey        return bespokePosReturn(inst, true);
#define DBespokePosVal        return bespokePosReturn(inst, false);
#define DVecElem        return vecElemReturn(inst);
#define DDictElem       return dictElemReturn(inst);
#define DKeysetElem     return keysetElemReturn(inst);
// Get the type of first or last element for different array type
#define DFirstElem        return arrFirstLastReturn(inst, true, false);
#define DLastElem         return arrFirstLastReturn(inst, false, false);
#define DFirstKey         return arrFirstLastReturn(inst, true, true);
#define DLastKey          return arrFirstLastReturn(inst, false, true);
#define DLoggingArrLike   return loggingArrLikeReturn(inst);
#define DModified(n)      return inst->src(n)->type().modified();
#define DArrLikeSet       return arrLikeSetReturn(inst);
#define DArrLikeUnset     return arrLikeUnsetReturn(inst);
#define DArrLikeAppend    return arrLikeAppendReturn(inst);
#define DStructDict     return structDictReturn(inst);
#define DTypeStructElem return typeStructElemReturn(inst);
#define DCol            return newColReturn(inst);
#define DMulti          return TBottom;
#define DSetElem        return setElemReturn(inst);
#define DBuiltin        return builtinReturn(inst);
#define DCall           return callReturn(inst);
#define DGenIter        return genIterReturn(inst);
#define DSubtract(n, t) return inst->src(n)->type() - t;
#define DUnion(...)     return unionReturn(inst, IdxSeq<__VA_ARGS__>{});
#define DMemoKey        return memoKeyReturn(inst);
#define DLvalOfPtr      return ptrToLvalReturn(inst);
#define DTypeCnsClsName return typeCnsClsName(inst);
#define DVerifyCoerce   return verifyCoerceReturn(inst);
#define DPropLval       return propLval(inst);
#define DElemLval       return elemLval(inst);
#define DElemLvalPos    return elemLvalPos(inst);
#define DCOW            return cowReturn(inst);
#define DStructTypeBound return structDictTypeBoundCheckReturn(inst);
#define DSpecialIC      return specialICReturn(inst);

#define O(name, dstinfo, srcinfo, flags) case name: dstinfo not_reached();

  switch (inst->op()) {
    IR_OPCODES
    default: not_reached();
  }

#undef O

#undef ND
#undef D
#undef DofS
#undef DRefineS
#undef DParam
#undef DEscalateToVanilla
#undef DLdObjCls
#undef DAllocObj
#undef DBespokeElem
#undef DBespokeElemUninit
#undef DBespokePosKey
#undef DBespokePosVal
#undef DVecElem
#undef DDictElem
#undef DKeysetElem
#undef DVecKey
#undef DFirstElem
#undef DLastElem
#undef DFirstKey
#undef DLastKey
#undef DLoggingArrLike
#undef DStructDict
#undef DTypeStructElem
#undef DCol
#undef DMulti
#undef DSetElem
#undef DBuiltin
#undef DCall
#undef DGenIter
#undef DSubtract
#undef DUnion
#undef DMemoKey
#undef DLvalOfPtr
#undef DTypeCnsClsName
#undef DVerifyParamFail
#undef DPropLval
#undef DElemLval
#undef DElemLvalPos
#undef DCOW
#undef DStructTypeBound
#undef DSpecialIC

}

bool IRInstruction::maySyncVMRegsWithSources() const {
  if (mayRaiseError()) return true;
  switch (op()) {
    // Profiling
    case DebugBacktrace:
    case LogArrayReach:
    case LogGuardFailure:
    case NewLoggingArray:
    case ProfileArrLikeProps:
    case ProfileDictAccess:
    case ProfileKeysetAccess:
      return true;

    // CPGO profiling
    case BespokeEscalateToVanilla:
    case BespokeGet: {
      auto const loggingLayout =
        ArrayLayout(bespoke::LoggingArray::GetLayoutIndex());
      auto const loggingArray = TArrLike.narrowToLayout(loggingLayout);
      // bespoke type structures can be created in HHBBC and bespoke arrays
      // should not be compared in profiling mode
      if (src(0)->type().arrSpec().is_type_structure()) return false;
      return src(0)->type().maybe(loggingArray);
    }

    // Intended Access
    case InitThrowableFileAndLine:
      return true;

    // Likely fixable
    case EqStr:
    case NeqStr:
    case IsTypeStruct:
    case IsTypeStructShallow:
    case StructDictUnset:
      return true;

    default:
      return false;
  }

}

}
