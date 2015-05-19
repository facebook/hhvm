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

#include "hphp/runtime/vm/jit/ir-unit.h"

#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/frame-state.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/timer.h"

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(hhir);

///////////////////////////////////////////////////////////////////////////////

IRUnit::IRUnit(TransContext context)
  : m_context(context)
  , m_entry(defBlock())
{}

IRInstruction* IRUnit::defLabel(unsigned numDst, BCMarker marker) {
  IRInstruction inst(DefLabel, marker);
  auto const label = clone(&inst);
  if (numDst > 0) {
    auto const dstsPtr = new (m_arena) SSATmp*[numDst];
    for (unsigned i = 0; i < numDst; ++i) {
      dstsPtr[i] = newSSATmp(label);
    }
    label->setDsts(numDst, dstsPtr);
  }
  return label;
}

void IRUnit::expandLabel(IRInstruction* label, unsigned extraDst) {
  assertx(label->is(DefLabel));
  assertx(extraDst > 0);
  auto const dstsPtr = new (m_arena) SSATmp*[extraDst + label->numDsts()];
  unsigned i = 0;
  for (auto dst : label->dsts()) {
    dstsPtr[i++] = dst;
  }
  for (unsigned j = 0; j < extraDst; j++) {
    dstsPtr[i++] = newSSATmp(label);
  }
  label->setDsts(i, dstsPtr);
}

void IRUnit::expandJmp(IRInstruction* jmp, SSATmp* value) {
  assertx(jmp->is(Jmp));
  std::vector<SSATmp*> newSrcs(jmp->numSrcs() + 1);
  size_t i = 0;
  for (auto src : jmp->srcs()) {
    newSrcs[i++] = src;
  }
  newSrcs[i++] = value;
  replace(jmp, Jmp, jmp->taken(), std::make_pair(i, &newSrcs[0]));
}

Block* IRUnit::defBlock(Block::Hint hint) {
  FTRACE(2, "IRUnit defining B{}\n", m_nextBlockId);
  auto const block = new (m_arena) Block(m_nextBlockId++);
  block->setHint(hint);
  return block;
}

SSATmp* IRUnit::cns(Type type) {
  assertx(type.hasConstVal() ||
         type.subtypeOfAny(TUninit, TInitNull, TNullptr));
  IRInstruction inst(DefConst, BCMarker{});
  inst.setTypeParam(type);
  if (SSATmp* tmp = m_constTable.lookup(&inst)) {
    assertx(tmp->type() == type);
    return tmp;
  }
  return m_constTable.insert(clone(&inst)->dst());
}

///////////////////////////////////////////////////////////////////////////////

/*
 * Returns true iff `block' ends the IR unit after finishing execution
 * of the bytecode instruction at `sk'.
 */
static bool endsUnitAtSrcKey(const Block* block, SrcKey sk) {
  if (!block->isExitNoThrow()) return false;

  const auto& inst = block->back();
  const auto  instSk = inst.marker().sk();

  switch (inst.op()) {
    // These instructions end a unit after executing the bytecode
    // instruction they correspond to.
    case InterpOneCF:
    case JmpSSwitchDest:
    case JmpSwitchDest:
    case RaiseError:
      return instSk == sk;;

    // The RetCtrl is generally ending a bytecode instruction, with the
    // exception being in an Await bytecode instruction, where we consider the
    // end of the bytecode instruction to be the non-suspending path.
    case RetCtrl:
    case AsyncRetCtrl:
      return inst.marker().sk().op() != Op::Await;

    // A ReqBindJmp ends a unit and it jumps to the next instruction
    // to execute.
    case ReqBindJmp: {
      auto destOffset = inst.extra<ReqBindJmp>()->dest.offset();
      return sk.succOffsets().count(destOffset);
    }

    default:
      return false;
  }
}

Block* findMainExitBlock(const IRUnit& unit, SrcKey lastSk) {
  Block* mainExit = nullptr;

  FTRACE(5, "findMainExitBlock: starting on unit:\n{}\n", show(unit));

  for (auto block : rpoSortCfg(unit)) {
    if (endsUnitAtSrcKey(block, lastSk)) {
      if (mainExit == nullptr) {
        mainExit = block;
        continue;
      }

      always_assert_flog(
        mainExit->hint() == Block::Hint::Unlikely ||
        block->hint() == Block::Hint::Unlikely,
        "findMainExit: 2 likely exits found: B{} and B{}\nlastSk = {}",
        mainExit->id(), block->id(), showShort(lastSk));

      if (mainExit->hint() == Block::Hint::Unlikely) mainExit = block;
    }
  }

  always_assert_flog(mainExit, "findMainExit: no exit found for lastSk = {}",
                     showShort(lastSk));

  FTRACE(5, "findMainExitBlock: mainExit = B{}\n", mainExit->id());

  return mainExit;
}

///////////////////////////////////////////////////////////////////////////////

}}
