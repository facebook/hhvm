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

#include "hphp/runtime/vm/jit/ir-instruction.h"
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
  , m_marker(inst->m_marker)
  , m_extra(inst->m_extra ? cloneExtra(op(), inst->m_extra, arena)
                          : nullptr)
{
  std::copy(inst->m_srcs, inst->m_srcs + inst->m_numSrcs, m_srcs);
  setTaken(inst->taken());
}

IRTrace* IRInstruction::trace() const {
  return block()->trace();
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

bool IRInstruction::producesReference() const {
  return opcodeHasFlags(op(), ProducesRC);
}

bool IRInstruction::hasMemEffects() const {
  return opcodeHasFlags(op(), MemEffects) || mayReenterHelper();
}

bool IRInstruction::canCSE() const {
  auto canCSE = opcodeHasFlags(op(), CanCSE);
  // Make sure that instructions that are CSE'able can't produce a reference
  // count or consume reference counts. CheckType/AssertType are special
  // because they can refine a maybeCounted type to a notCounted type, so they
  // logically consume and produce a reference without doing any work.
  assert(!canCSE || !consumesReferences() ||
         m_op == CheckType || m_op == AssertType);
  return canCSE && !mayReenterHelper();
}

bool IRInstruction::consumesReferences() const {
  return opcodeHasFlags(op(), ConsumesRC);
}

bool IRInstruction::consumesReference(int srcNo) const {
  if (!consumesReferences()) {
    return false;
  }
  // CheckType/AssertType consume a reference if we're guarding from a
  // maybeCounted type to a notCounted type.
  if (m_op == CheckType || m_op == AssertType) {
    assert(srcNo == 0);
    return src(0)->type().maybeCounted() && typeParam().notCounted();
  }
  // SpillStack consumes inputs 2 and onward
  if (m_op == SpillStack) return srcNo >= 2;
  // Call consumes inputs 3 and onward
  if (m_op == Call) return srcNo >= 3;
  // StRetVal only consumes input 1
  if (m_op == StRetVal) return srcNo == 1;

  if (m_op == StLoc || m_op == StLocNT) {
    // StLoc[NT] <stkptr>, <value>
    return srcNo == 1;
  }
  if (m_op == StProp || m_op == StPropNT || m_op == StMem || m_op == StMemNT) {
    // StProp[NT]|StMem[NT] <base>, <offset>, <value>
    return srcNo == 2;
  }
  if (m_op == ArraySet || m_op == ArraySetRef) {
    // Only consumes the reference to its input array
    return srcNo == 1;
  }
  return true;
}

bool IRInstruction::mayModifyRefs() const {
  Opcode opc = op();
  // DecRefNZ does not have side effects other than decrementing the ref
  // count. Therefore, its MayModifyRefs should be false.
  if (opc == DecRef) {
    auto type = src(0)->type();
    if (isControlFlow()) {
      // If the decref has a target label, then it exits if the destructor
      // has to be called, so it does not have any side effects on the main
      // trace.
      return false;
    }
    if (!type.canRunDtor()) {
      return false;
    }
  }
  return opcodeHasFlags(opc, MayModifyRefs) || mayReenterHelper();
}

bool IRInstruction::mayRaiseError() const {
  return opcodeHasFlags(op(), MayRaiseError) || mayReenterHelper();
}

bool IRInstruction::isEssential() const {
  Opcode opc = op();
  if (opc == DecRefNZ) {
    // If the source of a DecRefNZ is not an IncRef, mark it as essential
    // because we won't remove its source as well as itself.
    // If the ref count optimization is turned off, mark all DecRefNZ as
    // essential.
    if (!RuntimeOption::EvalHHIREnableRefCountOpt ||
        src(0)->inst()->op() != IncRef) {
      return true;
    }
  }
  return isControlFlow() ||
         opcodeHasFlags(opc, Essential) ||
         mayReenterHelper();
}

bool IRInstruction::isTerminal() const {
  return opcodeHasFlags(op(), Terminal);
}

bool IRInstruction::isPassthrough() const {
  return opcodeHasFlags(op(), Passthrough);
}

/*
 * Returns true if the instruction loads into a SSATmp representing a
 * PHP value (a subtype of Gen).  Note that this function returns
 * false for instructions that load internal meta-data, such as Func*,
 * Class*, etc.
 */
bool IRInstruction::isLoad() const {
  switch (m_op) {
    case LdStack:
    case LdLoc:
    case LdMem:
    case LdProp:
    case LdElem:
    case LdRef:
    case LdThis:
    case LdStaticLocCached:
    case LookupCns:
    case LookupClsCns:
    case CGetProp:
    case VGetProp:
    case VGetPropStk:
    case ArrayGet:
    case VectorGet:
    case PairGet:
    case MapGet:
    case StableMapGet:
    case CGetElem:
    case VGetElem:
    case VGetElemStk:
    case ArrayIdx:
      return true;

    default:
      return false;
  }
}

bool IRInstruction::storesCell(uint32_t srcIdx) const {
  switch (m_op) {
    case StRetVal:
    case StLoc:
    case StLocNT:
      return srcIdx == 1;

    case StMem:
    case StMemNT:
    case StProp:
    case StPropNT:
    case StElem:
      return srcIdx == 2;

    case ArraySet:
    case VectorSet:
    case MapSet:
    case StableMapSet:
      return srcIdx == 3;

    case SpillStack:
      return srcIdx >= 2 && srcIdx < numSrcs();

    case Call:
      return srcIdx >= 3 && srcIdx < numSrcs();

    case CallBuiltin:
      return srcIdx >= 1 && srcIdx < numSrcs();

    case FunctionExitSurpriseHook:
      return srcIdx == 2;

    default:
      return false;
  }
}

SSATmp* IRInstruction::getPassthroughValue() const {
  assert(isPassthrough());
  assert(m_op == IncRef || m_op == PassFP || m_op == PassSP ||
         m_op == CheckType || m_op == AssertType ||
         m_op == Mov);
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
  assert(MInstrEffects::supported(this));
  SSATmp* sp = dst(hasMainDst() ? 1 : 0);
  assert(sp->isA(Type::StkPtr));
  return sp;
}

bool IRInstruction::hasMainDst() const {
  return opcodeHasFlags(op(), HasDest);
}

bool IRInstruction::mayReenterHelper() const {
  if (isCmpOp(op())) {
    return cmpOpTypesMayReenter(op(),
                                src(0)->type(),
                                src(1)->type());
  }
  // Not necessarily actually false; this is just a helper for other
  // bits.
  return false;
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
  IRInstruction nop(Nop, marker());
  // copy all but m_id, m_taken, m_listNode
  m_op = nop.m_op;
  m_typeParam = nop.m_typeParam;
  m_numSrcs = nop.m_numSrcs;
  m_srcs = nop.m_srcs;
  m_numDsts = nop.m_numDsts;
  m_dst = nop.m_dst;
  setTaken(nullptr);
  m_extra = nullptr;
}

void IRInstruction::convertToJmp() {
  assert(isControlFlow());
  assert(IMPLIES(block(), block()->back() == this));
  m_op = Jmp_;
  m_typeParam = Type::None;
  m_numSrcs = 0;
  m_numDsts = 0;
  m_srcs = nullptr;
  m_dst = nullptr;
  // Instructions in the simplifier don't have blocks yet.
  if (block()) block()->setNext(nullptr);
}

void IRInstruction::convertToMov() {
  assert(!isControlFlow());
  m_op = Mov;
  m_typeParam = Type::None;
  m_extra = nullptr;
  assert(m_numSrcs == 1);
  // Instructions in the simplifier don't have dests yet
  assert((m_numDsts == 1) != isTransient());
}

void IRInstruction::become(IRFactory& factory, IRInstruction* other) {
  assert(other->isTransient() || m_numDsts == other->m_numDsts);
  auto& arena = factory.arena();

  // Copy all but m_id, m_taken.from, m_listNode, m_marker, and don't clone
  // dests---the whole point of become() is things still point to us.
  m_op = other->m_op;
  m_typeParam = other->m_typeParam;
  m_numSrcs = other->m_numSrcs;
  m_extra = other->m_extra ? cloneExtra(m_op, other->m_extra, arena) : nullptr;
  m_srcs = new (arena) SSATmp*[m_numSrcs];
  std::copy(other->m_srcs, other->m_srcs + m_numSrcs, m_srcs);
  setTaken(other->taken());
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
   * Don't CSE on m_taken---it's ok to use the destination of some
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
  return CSEHash::hashCombine(srcHash, m_op, m_typeParam);
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
  return
    func != nullptr &&
    bcOff >= func->base() && bcOff < func->past() &&
    spOff <= func->numSlotsInFrame() + func->maxStackCells();
}

}}

