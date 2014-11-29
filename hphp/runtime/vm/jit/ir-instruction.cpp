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

#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/cse.h"
#include "hphp/runtime/vm/jit/print.h"
#include "hphp/runtime/vm/jit/ssa-tmp.h"

namespace HPHP { namespace jit {

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

    case StProp:
    case StMem:
      // StProp|StMem <base>, <offset>, <value>
      return srcNo == 2;

    case ArraySet:
    case ArraySetRef:
      // Only consumes the reference to its input array
      return srcNo == 0;

    case SpillStack:
      // Inputs 2+ are values to store
      return srcNo >= 2;

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

bool IRInstruction::mayRaiseError() const {
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
    case LdContField:
    case LdPackedArrayElem:
    case LdLocPseudoMain:
      return true;

    default:
      return false;
  }
}

SSATmp* IRInstruction::getPassthroughValue() const {
  assert(isPassthrough());
  assert(is(IncRef,
            CheckType, AssertType, AssertNonNull,
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
  return src(numSrcs() - 1);
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
  return DstRange(m_dst, m_numDsts);
}

folly::Range<const SSATmp*> IRInstruction::dsts() const {
  return folly::Range<const SSATmp*>(m_dst, m_numDsts);
}

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

void IRInstruction::setOpcode(Opcode newOpc) {
  assert(hasEdges() || !jit::hasEdges(newOpc)); // cannot allocate new edges
  if (hasEdges() && !jit::hasEdges(newOpc)) {
    clearEdges();
  }
  m_op = newOpc;
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
   * Don't CSE on the edges--it's ok to use the destination of some earlier
   * branching instruction even though the instruction we may have generated
   * here would've exited to a different block.
   *
   * This is currently only used for CSE'ing some instructions that can take a
   * branch deterministically, based on thier inputs, like DivDbl. If we CSE
   * the result, it's safe because the place we would have had second one is
   * dominated by the first one, so it can't exit.
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

}}
