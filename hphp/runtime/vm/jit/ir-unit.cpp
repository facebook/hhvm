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
#include "hphp/runtime/vm/jit/frame-state.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/simplifier.h"
#include "hphp/runtime/vm/jit/timer.h"

namespace HPHP {  namespace JIT {

TRACE_SET_MOD(hhir);

IRUnit::IRUnit(TransContext context)
  : m_context(context)
  , m_entry(defBlock())
{}

IRInstruction* IRUnit::defLabel(unsigned numDst, BCMarker marker,
                                const smart::vector<unsigned>& producedRefs) {
  IRInstruction inst(DefLabel, marker);
  IRInstruction* label = cloneInstruction(&inst);
  always_assert(producedRefs.size() == numDst);
  m_labelRefs[label] = producedRefs;
  if (numDst > 0) {
    SSATmp* dsts = (SSATmp*) m_arena.alloc(numDst * sizeof(SSATmp));
    for (unsigned i = 0; i < numDst; ++i) {
      new (&dsts[i]) SSATmp(m_nextOpndId++, label);
    }
    label->setDsts(numDst, dsts);
  }
  return label;
}

Block* IRUnit::defBlock() {
  FTRACE(2, "IRUnit defining B{}\n", m_nextBlockId);
  return new (m_arena) Block(m_nextBlockId++);
}

IRInstruction* IRUnit::mov(SSATmp* dst, SSATmp* src, BCMarker marker) {
  IRInstruction* inst = gen(Mov, marker, src);
  dst->setInstruction(inst);
  inst->setDst(dst);
  return inst;
}

SSATmp* IRUnit::findConst(Type type) {
  assert(type.isConst());
  IRInstruction inst(DefConst, BCMarker{});
  inst.setTypeParam(type);
  if (SSATmp* tmp = m_constTable.lookup(&inst)) {
    assert(tmp->type().equals(type));
    return tmp;
  }
  return m_constTable.insert(cloneInstruction(&inst)->dst());
}

/*
 * Whether the block is a part of the main code path.
 */
static bool isMainBlock(const Block* b) {
  auto const hint = b->hint();
  return hint != Block::Hint::Unlikely && hint != Block::Hint::Unused;
}

/*
 * Whether the block appears to be the exit block of the main code
 * path of a region. This is conservative, so it may return false on
 * all blocks in a region.
 */
static bool isMainExit(const Block* b) {
  if (!isMainBlock(b)) return false;

  if (b->next()) return false;

  // The Await bytecode instruction does a RetCtrl to the scheduler,
  // which is in a likely block.  We don't want to consider this as
  // the main exit.
  auto const& back = b->back();
  if (back.op() == RetCtrl && back.marker().sk().op() == OpAwait) return false;

  auto const taken = b->taken();
  return !taken || taken->isCatch();
}

/*
 * Intended to be called after all optimizations are finished on a
 * single-entry, single-exit tracelet, this collects the types of all stack
 * slots and locals at the end of the main exit.
 */
void IRUnit::collectPostConditions() {
  // This function is only correct when given a single-exit region, as in
  // TransKind::Profile.  Furthermore, its output is only used to guide
  // formation of profile-driven regions.
  assert(mcg->tx().mode() == TransKind::Profile);
  assert(m_postConds.empty());
  Timer _t(Timer::collectPostConditions);

  // We want the state for the last block on the "main trace".  Figure
  // out which that is.
  Block* mainExit = nullptr;
  Block* lastMainBlock = nullptr;

  FrameState state{*this, entry()->front().marker()};
  ITRACE(2, "collectPostConditions starting\n");
  Trace::Indent _i;

  for (auto* block : rpoSortCfg(*this)) {
    state.startBlock(block);

    for (auto& inst : *block) {
      state.setMarker(inst.marker());
      state.update(&inst);
    }

    if (isMainBlock(block)) lastMainBlock = block;

    if (isMainExit(block)) {
      mainExit = block;
      break;
    }

    state.finishBlock(block);
  }

  // If we didn't find an obvious exit, then use the last block in the region.
  always_assert(lastMainBlock != nullptr);
  if (mainExit == nullptr) mainExit = lastMainBlock;

  FTRACE(1, "mainExit: B{}\n", mainExit->id());

  // state currently holds the state at the end of mainExit
  auto const curFunc  = state.func();
  auto const sp       = state.sp();
  auto const spOffset = state.spOffset();

  for (unsigned i = 0; i < spOffset; ++i) {
    auto t = getStackValue(sp, i).knownType;
    if (!t.equals(Type::StackElem)) {
      m_postConds.push_back({ RegionDesc::Location::Stack{i, spOffset - i},
                              t });
    }
  }

  for (unsigned i = 0; i < curFunc->numLocals(); ++i) {
    auto t = state.localType(i);
    if (!t.equals(Type::Gen)) {
      FTRACE(1, "Local {}: {}\n", i, t.toString());
      m_postConds.push_back({ RegionDesc::Location::Local{i}, t });
    }
  }
}

const PostConditions& IRUnit::postConditions() const {
  return m_postConds;
}

}}
