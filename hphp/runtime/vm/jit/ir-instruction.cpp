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
  , m_marker(inst->m_marker)
  , m_id(id)
  , m_srcs(m_numSrcs ? new (arena) SSATmp*[m_numSrcs] : nullptr)
  , m_dst(nullptr)
  , m_block(nullptr)
  , m_extra(inst->m_extra ? cloneExtra(op(), inst->m_extra, arena)
                          : nullptr)
{
  assert(!isTransient());
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
  m_op        = nop.m_op;
  m_typeParam = nop.m_typeParam;
  m_numSrcs   = nop.m_numSrcs;
  m_srcs      = nop.m_srcs;
  m_numDsts   = nop.m_numDsts;
  m_dst       = nop.m_dst;
  m_extra     = nullptr;
}

void IRInstruction::become(IRUnit& unit, IRInstruction* other) {
  assert(other->isTransient() || m_numDsts == other->m_numDsts);
  auto& arena = unit.arena();

  if (hasEdges()) clearEdges();

  m_op = other->m_op;
  m_typeParam = other->m_typeParam;
  m_numSrcs = other->m_numSrcs;
  m_extra = other->m_extra ? cloneExtra(m_op, other->m_extra, arena) : nullptr;
  m_srcs = new (arena) SSATmp*[m_numSrcs];
  std::copy(other->m_srcs, other->m_srcs + m_numSrcs, m_srcs);

  if (hasEdges()) {
    assert(other->hasEdges());  // m_op is from other now
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
    case ConcatCellCell:
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
    case StLocNT:
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

    case ColAddElemC:
      // value at index 2
      return srcNo == 2;

    case ColAddNewElemC:
      // value at index 1
      return srcNo == 1;

    case CheckNullptr:
      return srcNo == 0;

    case CreateAFWH:
      return srcNo == 4;

    case InitPackedArray:
      return srcNo == 1;

    case InitPackedArrayLoop:
      return srcNo > 0;

    default:
      return true;
  }
}

bool IRInstruction::killsSource(int idx) const {
  if (!killsSources()) return false;
  switch (m_op) {
    case DecRef:
    case ConvObjToArr:
    case ConvCellToArr:
    case ConvCellToObj:
      assert(idx == 0);
      return true;
    case ArraySet:
    case ArraySetRef:
      return idx == 1;
    default:
      not_reached();
      break;
  }
}

///////////////////////////////////////////////////////////////////////////////

void IRInstruction::setOpcode(Opcode newOpc) {
  assert(hasEdges() || !jit::hasEdges(newOpc)); // cannot allocate new edges
  if (hasEdges() && !jit::hasEdges(newOpc)) {
    clearEdges();
  }
  m_op = newOpc;
}

SSATmp* IRInstruction::dst(unsigned i) const {
  if (i == 0 && m_numDsts == 0) return nullptr;
  assert(i < m_numDsts);
  assert(naryDst() || i == 0);
  return hasDst() ? dst() : &m_dst[i];
}

///////////////////////////////////////////////////////////////////////////////
// outputType().

namespace {

Type unboxPtr(Type t) {
  t = t - Type::PtrToBoxedCell;
  return t.deref().ptr(add_ref(t.ptrKind()));
}

Type boxPtr(Type t) {
  auto const rawBoxed = t.deref().unbox().box();
  auto const noNull = rawBoxed - Type::BoxedUninit;
  return noNull.ptr(remove_ref(t.ptrKind()));
}

Type allocObjReturn(const IRInstruction* inst) {
  switch (inst->op()) {
    case ConstructInstance:
      return Type::SubObj(inst->extra<ConstructInstance>()->cls);

    case NewInstanceRaw:
      return Type::ExactObj(inst->extra<NewInstanceRaw>()->cls);

    case AllocObj:
      return inst->src(0)->isConst()
        ? Type::ExactObj(inst->src(0)->clsVal())
        : Type::Obj;

    default:
      always_assert(false && "Invalid opcode returning AllocObj");
  }
}

Type arrElemReturn(const IRInstruction* inst) {
  assert(inst->op() == LdPackedArrayElem || inst->op() == LdStructArrayElem);
  assert(!inst->hasTypeParam() || inst->typeParam() <= Type::Gen);
  auto const tyParam = inst->hasTypeParam() ? inst->typeParam() : Type::Gen;

  auto const arrTy = inst->src(0)->type().arrSpec().type();
  if (!arrTy) return tyParam;

  using T = RepoAuthType::Array::Tag;

  switch (arrTy->tag()) {
    case T::Packed:
      {
        auto const idx = inst->src(1);
        if (!idx->isConst()) return Type::Gen;
        if (idx->intVal() >= 0 && idx->intVal() < arrTy->size()) {
          return typeFromRAT(arrTy->packedElem(idx->intVal())) & tyParam;
        }
      }
      return Type::Gen;
    case T::PackedN:
      return typeFromRAT(arrTy->elemType()) & tyParam;
  }

  return tyParam;
}

Type thisReturn(const IRInstruction* inst) {
  auto const func = inst->marker().func();

  // If the function is a cloned closure which may have a re-bound $this which
  // is not a subclass of the context return an unspecialized type.
  if (func->hasForeignThis()) return Type::Obj;

  if (auto const cls = func->cls()) {
    return Type::SubObj(cls);
  }
  return Type::Obj;
}

Type ctxReturn(const IRInstruction* inst) {
  return thisReturn(inst) | Type::Cctx;
}

Type setElemReturn(const IRInstruction* inst) {
  assert(inst->op() == SetElem);
  auto baseType = inst->src(minstrBaseIdx(inst->op()))->type().strip();

  // If the base is a Str, the result will always be a CountedStr (or
  // an exception). If the base might be a str, the result wil be
  // CountedStr or Nullptr. Otherwise, the result is always Nullptr.
  if (baseType <= Type::Str) {
    return Type::CountedStr;
  } else if (baseType.maybe(Type::Str)) {
    return Type::CountedStr | Type::Nullptr;
  }
  return Type::Nullptr;
}

Type builtinReturn(const IRInstruction* inst) {
  assert(inst->op() == CallBuiltin);

  Type t = inst->typeParam();
  if (t.isSimpleType() || t == Type::Cell) {
    return t;
  }
  if (t.isReferenceType() || t == Type::BoxedCell) {
    return t | Type::InitNull;
  }
  not_reached();
}

} // namespace

namespace TypeNames {
#define IRT(name, ...) UNUSED const Type name = Type::name;
#define IRTP(name, ...) IRT(name)
  IR_TYPES
#undef IRT
#undef IRTP
};

Type outputType(const IRInstruction* inst, int dstId) {
  using namespace TypeNames;
  using TypeNames::TCA;
#define ND              assert(0 && "outputType requires HasDest or NaryDest");
#define D(type)         return type;
#define DofS(n)         return inst->src(n)->type();
#define DRefineS(n)     return inst->src(n)->type() & inst->typeParam();
#define DParamMayRelax  return inst->typeParam();
#define DParam          return inst->typeParam();
#define DParamPtr(k)    assert(inst->typeParam() <= Type::Gen.ptr(Ptr::k)); \
                        return inst->typeParam();
#define DUnboxPtr       return unboxPtr(inst->src(0)->type());
#define DBoxPtr         return boxPtr(inst->src(0)->type());
#define DAllocObj       return allocObjReturn(inst);
#define DArrElem        return arrElemReturn(inst);
#define DArrPacked      return Type::Array(ArrayData::kPackedKind);
#define DThis           return thisReturn(inst);
#define DCtx            return ctxReturn(inst);
#define DMulti          return Type::Bottom;
#define DSetElem        return setElemReturn(inst);
#define DBuiltin        return builtinReturn(inst);
#define DSubtract(n, t) return inst->src(n)->type() - t;
#define DCns            return Type::Uninit | Type::InitNull | Type::Bool | \
                               Type::Int | Type::Dbl | Type::Str | Type::Res;

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
#undef DThis
#undef DCtx
#undef DMulti
#undef DSetElem
#undef DBuiltin
#undef DSubtract
#undef DCns

}

}}
