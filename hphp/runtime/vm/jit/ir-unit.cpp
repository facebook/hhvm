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

IRInstruction* IRUnit::defLabel(unsigned numDst, BCMarker marker,
                                const jit::vector<uint32_t>& producedRefs) {
  IRInstruction inst(DefLabel, marker);
  IRInstruction* label = clone(&inst);
  always_assert(producedRefs.size() == numDst);
  m_labelRefs[label] = producedRefs;
  if (numDst > 0) {
    SSATmp* dsts = (SSATmp*) m_arena.alloc(numDst * sizeof(SSATmp));
    for (unsigned i = 0; i < numDst; ++i) {
      new (&dsts[i]) SSATmp(m_nextTmpId++, label);
    }
    label->setDsts(numDst, dsts);
  }
  return label;
}

Block* IRUnit::defBlock(Block::Hint hint) {
  FTRACE(2, "IRUnit defining B{}\n", m_nextBlockId);
  auto const block = new (m_arena) Block(m_nextBlockId++);
  block->setHint(hint);
  return block;
}

SSATmp* IRUnit::cns(Type type) {
  assert(type.isConst());
  IRInstruction inst(DefConst, BCMarker{});
  inst.setTypeParam(type);
  if (SSATmp* tmp = m_constTable.lookup(&inst)) {
    assert(tmp->type() == type);
    return tmp;
  }
  return m_constTable.insert(clone(&inst)->dst());
}

///////////////////////////////////////////////////////////////////////////////

static bool isMainExit(const Block* block, SrcKey lastSk) {
  if (!block->isExit()) return false;

  auto& lastInst = block->back();

  // This captures cases where we end the region with a terminal
  // instruction, e.g. RetCtrl, RaiseError, InterpOneCF.
  if (lastInst.isTerminal() && lastInst.marker().sk() == lastSk) return true;

  // Otherwise, the region must contain a ReqBindJmp to a bytecode
  // offset than will follow the execution of lastSk.
  if (lastInst.op() != ReqBindJmp) return false;

  auto succOffsets = lastSk.succOffsets();

  FTRACE(6, "isMainExit: instrSuccOffsets({}): {}\n",
         show(lastSk), folly::join(", ", succOffsets));

  return succOffsets.count(lastInst.marker().bcOff());
}

Block* findMainExitBlock(const IRUnit& unit, SrcKey lastSk) {
  for (auto block : rpoSortCfg(unit)) {
    if (isMainExit(block, lastSk)) return block;
  }
  return nullptr;
}

///////////////////////////////////////////////////////////////////////////////

}}
