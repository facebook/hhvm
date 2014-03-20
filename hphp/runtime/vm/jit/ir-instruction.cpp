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
#include <algorithm>
#include "hphp/runtime/vm/jit/ssa-tmp.h"
#include "hphp/runtime/vm/jit/cse.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/block.h"

namespace HPHP {  namespace JIT {

IRInstruction::IRInstruction(Arena& arena, const IRInstruction* inst, Id id)
  : m_op(inst->m_op)
  , m_typeParam(inst->m_typeParam)
  , m_numSrcs(inst->m_numSrcs)
  , m_numDsts(inst->m_numDsts)
  , m_id(id)
  , m_srcs(m_numSrcs ? new (arena) SSATmp*[m_numSrcs] : nullptr)
  , m_dst(nullptr)
  , m_block(nullptr)
  , m_marker(inst->m_marker)
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

bool IRInstruction::hasExtra() const {
  return m_extra;
}

// Instructions with ModifiesStack are always naryDst regardless of
// the inner dest.

bool IRInstruction::hasDst() const {
  return opcodeHasFlags(op(), HasDest) &&
    !opcodeHasFlags(op(), ModifiesStack);
}

bool IRInstruction::naryDst() const {
  return opcodeHasFlags(op(), NaryDest | ModifiesStack);
}

bool IRInstruction::isNative() const {
  return opcodeHasFlags(op(), CallsNative);
}

bool IRInstruction::producesReference(int dstNo) const {
  return opcodeHasFlags(op(), ProducesRC);
}

bool IRInstruction::canCSE() const {
  auto canCSE = opcodeHasFlags(op(), CanCSE);
  // Make sure that instructions that are CSE'able can't consume reference
  // counts.
  assert(!canCSE || !consumesReferences());
  return canCSE;
}

bool IRInstruction::consumesReferences() const {
  return opcodeHasFlags(op(), ConsumesRC);
}

bool IRInstruction::consumesReference(int srcNo) const {
  if (!consumesReferences()) {
    return false;
  }

  switch (op()) {
    case ConcatStrStr:
    case ConcatStrInt:
    case ConcatCellCell:
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
      // Consume the value being stored, not the thing it's being stored into
      return srcNo == 1;

    case StProp:
    case StMem:
      // StProp|StMem <base>, <offset>, <value>
      return srcNo == 2;

    case ArraySet:
    case ArraySetRef:
      // Only consumes the reference to its input array
      return srcNo == 1;

    case SpillStack:
      // Inputs 2+ are values to store
      return srcNo >= 2;

    case SpillFrame:
      // Consumes the $this/Class field of the ActRec
      return srcNo == 3;

    case Call:
      // Inputs 3+ are arguments to the function
      return srcNo >= 3;

    case ColAddElemC:
      // value at index 2
      return srcNo == 2;

    case ColAddNewElemC:
      // value at index 1
      return srcNo == 1;

    case CheckNullptr:
      return srcNo == 0;

    case CreateAFWHFunc:
      return srcNo == 1;

    case CreateAFWHMeth:
      return srcNo == 2;

    default:
      return true;
  }
}

bool IRInstruction::mayRaiseError() const {
  if (!opcodeHasFlags(op(), MayRaiseError)) return false;

  // Some instructions only throw if they do not have a non-catch label.
  if (is(LdClsPropAddrCached, LdClsPropAddr) &&
      taken() && !taken()->isCatch()) {
    return false;
  }

  return opcodeHasFlags(op(), MayRaiseError);
}

bool IRInstruction::isEssential() const {
  return isControlFlow() ||
         opcodeHasFlags(op(), Essential);
}

bool IRInstruction::isTerminal() const {
  return opcodeHasFlags(op(), Terminal);
}

bool IRInstruction::isPassthrough() const {
  return opcodeHasFlags(op(), Passthrough);
}

/*
 * Returns true if the instruction does nothing but load a PHP value from
 * memory, possibly with some straightforward computation beforehand to decide
 * where the load should come from. This specifically excludes opcodes such as
 * CGetProp and ArrayGet that incref their return value.
 */
bool IRInstruction::isRawLoad() const {
  switch (m_op) {
    case LdMem:
    case LdRef:
    case LdStack:
    case LdElem:
    case LdProp:
    case LdPackedArrayElem:
    case Unbox:
      return true;

    default:
      return false;
  }
}

SSATmp* IRInstruction::getPassthroughValue() const {
  assert(isPassthrough());
  assert(is(IncRef, PassFP, PassSP,
            CheckType, AssertType, AssertNonNull,
            StRef,
            ColAddElemC, ColAddNewElemC,
            Mov));
  return src(0);
}

bool IRInstruction::killsSources() const {
  return opcodeHasFlags(op(), KillsSources);
}

bool IRInstruction::killsSource(int idx) const {
  if (!killsSources()) return false;
  switch (m_op) {
    case DecRef:
    case ConvObjToArr:
    case ConvCellToArr:
    case ConvCellToBool:
    case ConvObjToDbl:
    case ConvStrToDbl:
    case ConvCellToDbl:
    case ConvObjToInt:
    case ConvCellToInt:
    case ConvCellToObj:
    case ConvObjToStr:
    case ConvResToStr:
    case ConvCellToStr:
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

bool IRInstruction::modifiesStack() const {
  return opcodeHasFlags(op(), ModifiesStack);
}

SSATmp* IRInstruction::modifiedStkPtr() const {
  assert(modifiesStack());
  SSATmp* sp = dst(hasMainDst() ? 1 : 0);
  assert(sp->isA(Type::StkPtr));
  return sp;
}

SSATmp* IRInstruction::previousStkPtr() const {
  assert(modifiesStack());
  assert(MInstrEffects::supported(this));
  auto base = src(minstrBaseIdx(this));
  assert(base->inst()->is(LdStackAddr));
  return base->inst()->src(0);
}

bool IRInstruction::hasMainDst() const {
  return opcodeHasFlags(op(), HasDest);
}

SSATmp* IRInstruction::dst(unsigned i) const {
  if (i == 0 && m_numDsts == 0) return nullptr;
  assert(i < m_numDsts);
  assert(naryDst() || i == 0);
  return hasDst() ? dst() : &m_dst[i];
}

DstRange IRInstruction::dsts() {
  return Range<SSATmp*>(m_dst, m_numDsts);
}

Range<const SSATmp*> IRInstruction::dsts() const {
  return Range<const SSATmp*>(m_dst, m_numDsts);
}

void IRInstruction::convertToNop() {
  if (hasEdges()) clearEdges();
  IRInstruction nop(Nop, marker());
  // copy all but m_id, m_edges, m_listNode
  m_op = nop.m_op;
  m_typeParam = nop.m_typeParam;
  m_numSrcs = nop.m_numSrcs;
  m_srcs = nop.m_srcs;
  m_numDsts = nop.m_numDsts;
  m_dst = nop.m_dst;
  m_extra = nullptr;
}

void IRInstruction::convertToJmp() {
  assert(isControlFlow());
  assert(IMPLIES(block(), &block()->back() == this));
  m_op = Jmp;
  m_typeParam.clear();
  m_numSrcs = 0;
  m_numDsts = 0;
  m_srcs = nullptr;
  m_dst = nullptr;
  m_extra = nullptr;
  // Instructions in the simplifier don't have blocks yet.
  setNext(nullptr);
}

void IRInstruction::convertToJmp(Block* target) {
  convertToJmp();
  setTaken(target);
}

void IRInstruction::convertToMov() {
  assert(!isControlFlow());
  m_op = Mov;
  m_typeParam.clear();
  m_extra = nullptr;
  if (m_numDsts == 1) m_dst->setInstruction(this); // recompute type
  assert(m_numSrcs == 1);
  // Instructions in the simplifier don't have dests yet
  assert((m_numDsts == 1) != isTransient());
}

void IRInstruction::become(IRUnit& unit, IRInstruction* other) {
  assert(other->isTransient() || m_numDsts == other->m_numDsts);
  auto& arena = unit.arena();

  // Copy all but m_id, m_edges[].from, m_listNode, m_marker, and don't clone
  // dests---the whole point of become() is things still point to us.
  if (hasEdges() && !other->hasEdges()) {
    clearEdges();
  } else if (!hasEdges() && other->hasEdges()) {
    m_edges = new (arena) Edge[2];
    setNext(other->next());
    setTaken(other->taken());
  }
  m_op = other->m_op;
  m_typeParam = other->m_typeParam;
  m_numSrcs = other->m_numSrcs;
  m_extra = other->m_extra ? cloneExtra(m_op, other->m_extra, arena) : nullptr;
  m_srcs = new (arena) SSATmp*[m_numSrcs];
  std::copy(other->m_srcs, other->m_srcs + m_numSrcs, m_srcs);
}

void IRInstruction::setOpcode(Opcode newOpc) {
  assert(hasEdges() || !JIT::hasEdges(newOpc)); // cannot allocate new edges
  if (hasEdges() && !JIT::hasEdges(newOpc)) {
    clearEdges();
  }
  m_op = newOpc;
}

void IRInstruction::addCopy(IRUnit& unit, SSATmp* src, const PhysLoc& dest) {
  assert(op() == Shuffle);
  auto data = extra<Shuffle>();
  auto n = numSrcs();
  assert(n == data->size && n <= data->cap);
  if (n == data->cap) {
    auto cap = data->cap * 2;
    auto srcs = new (unit.arena()) SSATmp*[cap];
    auto dests = new (unit.arena()) PhysLoc[cap];
    for (unsigned i = 0; i < n; i++) {
      srcs[i] = m_srcs[i];
      dests[i] = data->dests[i];
    }
    m_srcs = srcs;
    data->dests = dests;
    data->cap = cap;
  }
  m_numSrcs = n + 1;
  m_srcs[n] = src;
  data->size = n + 1;
  data->dests[n] = dest;
}

SSATmp* IRInstruction::src(uint32_t i) const {
  always_assert(i < numSrcs());
  return m_srcs[i];
}

void IRInstruction::setSrc(uint32_t i, SSATmp* newSrc) {
  always_assert(i < numSrcs());
  m_srcs[i] = newSrc;
}

bool IRInstruction::cseEquals(IRInstruction* inst) const {
  assert(canCSE());

  if (m_op != inst->m_op ||
      m_typeParam != inst->m_typeParam ||
      m_numSrcs != inst->m_numSrcs) {
    return false;
  }
  for (uint32_t i = 0; i < numSrcs(); i++) {
    if (src(i) != inst->src(i)) {
      return false;
    }
  }
  if (hasExtra() && !cseEqualsExtra(op(), m_extra, inst->m_extra)) {
    return false;
  }
  /*
   * Don't CSE on the edges--it's ok to use the destination of some
   * earlier guarded load even though the instruction we may have
   * generated here would've exited to a different trace.
   *
   * For example, we use this to cse LdThis regardless of its label.
   */
  return true;
}

size_t IRInstruction::cseHash() const {
  assert(canCSE());

  size_t srcHash = 0;
  for (unsigned i = 0; i < numSrcs(); ++i) {
    srcHash = CSEHash::hashCombine(srcHash, src(i));
  }
  if (hasExtra()) {
    srcHash = CSEHash::hashCombine(srcHash,
      cseHashExtra(op(), m_extra));
  }
  if (hasTypeParam()) {
    srcHash = CSEHash::hashCombine(srcHash, m_typeParam.value());
  }
  return CSEHash::hashCombine(srcHash, m_op);
}

std::string IRInstruction::toString() const {
  std::ostringstream str;
  print(str, this);
  return str.str();
}

std::string BCMarker::show() const {
  assert(valid());
  return folly::format("--- bc {}, spOff {} ({})",
                       bcOff,
                       spOff,
                       func->fullName()->data()).str();
}

bool BCMarker::valid() const {
  if (reinterpret_cast<uintptr_t>(func) == 1) return true;
  return
    func != nullptr &&
    bcOff >= func->base() && bcOff < func->past() &&
    (RuntimeOption::EvalHHIREnableGenTimeInlining ||
     spOff <= func->numSlotsInFrame() + func->maxStackCells());
  // When inlining is on, we may modify markers to weird values in case reentry
  // happens.
}

}}
