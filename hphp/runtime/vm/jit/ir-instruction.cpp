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

#include "hphp/runtime/vm/jit/ir-instruction.h"

#include "hphp/runtime/base/repo-auth-type.h"
#include "hphp/runtime/base/repo-auth-type-array.h"
#include "hphp/runtime/vm/func.h"
#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/edge.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/ir-instr-table.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/minstr-effects.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/type.h"

#include "hphp/util/arena.h"
#include "hphp/runtime/ext/asio/ext_async-function-wait-handle.h"
#include "hphp/runtime/ext/asio/ext_static-wait-handle.h"

#include <folly/Range.h>

#include <algorithm>
#include <sstream>

namespace HPHP { namespace jit {
//////////////////////////////////////////////////////////////////////

IRInstruction::IRInstruction(Arena& arena, const IRInstruction* inst, Id id)
  : m_typeParam(inst->m_typeParam)
  , m_op(inst->m_op)
  , m_numSrcs(inst->m_numSrcs)
  , m_numDsts(inst->m_numDsts)
  , m_hasTypeParam{inst->m_hasTypeParam}
  , m_marker(inst->m_marker)
  , m_id(id)
  , m_srcs(m_numSrcs ? new (arena) SSATmp*[m_numSrcs] : nullptr)
  , m_dest(nullptr)
  , m_block(nullptr)
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
  IRInstruction nop(Nop, marker());
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

bool IRInstruction::consumesReference(int srcNo) const {
  if (!consumesReferences()) {
    return false;
  }

  switch (op()) {
    case ConcatStrStr:
    case ConcatStrInt:
    case ConcatStr3:
    case ConcatStr4:
      // Call a helper that decrefs the first argument
      return srcNo == 0;

    case StRef:
    case StClosureArg:
    case StClosureCtx:
    case StContArValue:
    case StContArKey:
    case StRetVal:
    case StLoc:
    case AFWHBlockOn:
      // Consume the value being stored, not the thing it's being stored into
      return srcNo == 1;

    case StMem:
      // StMem <base>, <value>
      return srcNo == 1;

    case ArraySet:
    case ArraySetRef:
      // Only consumes the reference to its input array
      return srcNo == 0;

    case SpillFrame:
      // Consumes the $this/Class field of the ActRec
      return srcNo == 2;

    case MapAddElemC:
      // value at index 2
      return srcNo == 2;

    case ColAddNewElemC:
      // value at index 1
      return srcNo == 1;

    case CheckNullptr:
      return srcNo == 0;

    case CreateAFWH:
    case CreateAFWHNoVV:
      return srcNo == 4;

    case InitPackedArray:
      return srcNo == 1;

    case InitPackedArrayLoop:
      return srcNo > 0;

    default:
      return true;
  }
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
// outputType().

namespace {

Type unboxPtr(Type t) {
  auto const pcell = t & TPtrToCell;
  auto const pref = t & TPtrToBoxedInitCell;
  return pref.deref().inner().ptr(Ptr::Ref) | pcell;
}

Type boxPtr(Type t) {
  auto const rawBoxed = t.deref().unbox().box();
  auto const noNull = rawBoxed - TBoxedUninit;
  return noNull.ptr(remove_ref(t.ptrKind()));
}

Type allocObjReturn(const IRInstruction* inst) {
  switch (inst->op()) {
    case ConstructInstance:
      return Type::ExactObj(inst->extra<ConstructInstance>()->cls);

    case NewInstanceRaw:
      return Type::ExactObj(inst->extra<NewInstanceRaw>()->cls);

    case AllocObj:
      return inst->src(0)->hasConstVal()
        ? Type::ExactObj(inst->src(0)->clsVal())
        : TObj;

    case CreateSSWH:
      return Type::ExactObj(c_StaticWaitHandle::classof());

    case CreateAFWH:
    case CreateAFWHNoVV:
      return Type::ExactObj(c_AsyncFunctionWaitHandle::classof());

    default:
      always_assert(false && "Invalid opcode returning AllocObj");
  }
}

Type arrElemReturn(const IRInstruction* inst) {
  assertx(inst->is(LdStructArrayElem, ArrayGet));
  assertx(!inst->hasTypeParam() || inst->typeParam() <= TGen);

  auto resultType = inst->hasTypeParam() ? inst->typeParam() : TGen;
  if (inst->is(ArrayGet)) {
    resultType &= TInit;
  }

  // Elements of a static array are uncounted
  if (inst->src(0)->isA(TStaticArr)) {
    resultType &= TUncountedInit;
  }

  auto const arrTy = inst->src(0)->type().arrSpec().type();
  if (!arrTy) return resultType;

  using T = RepoAuthType::Array::Tag;
  using E = RepoAuthType::Array::Empty;

  switch (arrTy->tag()) {
    case T::Packed:
    {
      auto const idx = inst->src(1);
      if (idx->hasConstVal(TInt) &&
          idx->intVal() >= 0 &&
          idx->intVal() < arrTy->size()) {
        resultType &= typeFromRAT(arrTy->packedElem(idx->intVal()));
      }
      break;
    }
    case T::PackedN:
      resultType &= typeFromRAT(arrTy->elemType());
      break;
  }

  if (arrTy->emptiness() == E::Maybe) {
    resultType |= TInitNull;
  }
  return resultType;
}

Type thisReturn(const IRInstruction* inst) {
  auto const func = inst->marker().func();

  // If the function is a cloned closure which may have a re-bound $this which
  // is not a subclass of the context return an unspecialized type.
  if (func->hasForeignThis()) return TObj;

  if (auto const cls = func->cls()) {
    return Type::SubObj(cls);
  }
  return TObj;
}

Type ctxReturn(const IRInstruction* inst) {
  return thisReturn(inst) | TCctx;
}

Type setElemReturn(const IRInstruction* inst) {
  assertx(inst->op() == SetElem);
  auto baseType = inst->src(minstrBaseIdx(inst->op()))->type().strip();

  // If the base is a Str, the result will always be a CountedStr (or
  // an exception). If the base might be a str, the result wil be
  // CountedStr or Nullptr. Otherwise, the result is always Nullptr.
  if (baseType <= TStr) {
    return TCountedStr;
  } else if (baseType.maybe(TStr)) {
    return TCountedStr | TNullptr;
  }
  return TNullptr;
}

Type newColReturn(const IRInstruction* inst) {
  assertx(inst->is(NewCol, NewColFromArray));
  auto getColClassType = [&](CollectionType ct) -> Type {
    auto name = collections::typeToString(ct);
    auto cls = Unit::lookupClassOrUniqueClass(name);
    if (cls == nullptr) return TObj;
    return Type::ExactObj(cls);
  };

  if (inst->is(NewCol)) {
    return getColClassType(inst->extra<NewCol>()->type);
  }
  return getColClassType(inst->extra<NewColFromArray>()->type);
}

Type builtinReturn(const IRInstruction* inst) {
  assertx(inst->op() == CallBuiltin);

  Type t = inst->typeParam();
  if (t.isSimpleType() || t == TCell) {
    return t;
  }
  if (t.isReferenceType() || t == TBoxedCell) {
    return t | TInitNull;
  }
  not_reached();
}

} // namespace

Type outputType(const IRInstruction* inst, int dstId) {
  using namespace TypeNames;
  using TypeNames::TCA;
#define ND              assertx(0 && "outputType requires HasDest or NaryDest");
#define D(type)         return type;
#define DofS(n)         return inst->src(n)->type();
#define DRefineS(n)     return inst->src(n)->type() & inst->typeParam();
#define DParamMayRelax  return inst->typeParam();
#define DParam          return inst->typeParam();
#define DParamPtr(k)    assertx(inst->typeParam() <= TGen.ptr(Ptr::k)); \
                        return inst->typeParam();
#define DUnboxPtr       return unboxPtr(inst->src(0)->type());
#define DBoxPtr         return boxPtr(inst->src(0)->type());
#define DAllocObj       return allocObjReturn(inst);
#define DArrElem        return arrElemReturn(inst);
#define DArrPacked      return Type::Array(ArrayData::kPackedKind);
#define DCol            return newColReturn(inst);
#define DThis           return thisReturn(inst);
#define DCtx            return ctxReturn(inst);
#define DMulti          return TBottom;
#define DSetElem        return setElemReturn(inst);
#define DBuiltin        return builtinReturn(inst);
#define DSubtract(n, t) return inst->src(n)->type() - t;
#define DCns            return TUninit | TInitNull | TBool | \
                               TInt | TDbl | TStr | TRes;

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
#undef DParamMayRelax
#undef DParam
#undef DParamPtr
#undef DUnboxPtr
#undef DBoxPtr
#undef DAllocObj
#undef DArrElem
#undef DArrPacked
#undef DCol
#undef DThis
#undef DCtx
#undef DMulti
#undef DSetElem
#undef DBuiltin
#undef DSubtract
#undef DCns
}

}}
