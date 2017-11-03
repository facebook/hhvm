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

#include "hphp/runtime/vm/jit/ir-unit.h"

#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/frame-state.h"
#include "hphp/util/timer.h"

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

TRACE_SET_MOD(hhir);

///////////////////////////////////////////////////////////////////////////////

IRUnit::IRUnit(TransContext context) : m_context(context)
{
  // Setup m_entry after property initialization, since it depends on
  // the value of m_defHint.
  // For Optimize translations, the entry block's profCount is
  // adjusted later in translateRegion.
  m_entry = defBlock();
  m_startNanos = HPHP::Timer::GetThreadCPUTimeNanos();
}

void IRUnit::initLogEntry(const Func* func) {
  if (func ? func->shouldSampleJit() :
      StructuredLog::coinflip(RuntimeOption::EvalJitSampleRate)) {
    m_logEntry.emplace();
  }
}

IRInstruction* IRUnit::defLabel(unsigned numDst, BCContext bcctx) {
  IRInstruction inst(DefLabel, bcctx);
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
  auto const newSrcs = new (m_arena) SSATmp*[jmp->numSrcs() + 1];
  size_t i = 0;
  for (auto src : jmp->srcs()) {
    newSrcs[i++] = src;
  }
  newSrcs[i++] = value;
  jmp->setSrcs(i, newSrcs);
}

Block* IRUnit::defBlock(uint64_t profCount /* =1 */,
                        Block::Hint hint   /* =Neither */ ) {
  FTRACE(2, "IRUnit defining B{}\n", m_nextBlockId);
  auto const block = new (m_arena) Block(m_nextBlockId++, profCount);
  if (hint == Block::Hint::Neither) {
    hint = m_defHint;
  }
  block->setHint(hint);
  return block;
}

SSATmp* IRUnit::cns(Type type) {
  assertx(type.hasConstVal() ||
         type.subtypeOfAny(TUninit, TInitNull, TNullptr));
  IRInstruction inst(DefConst, BCContext{});
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
    case ThrowOutOfBounds:
    case ThrowInvalidArrayKey:
    case ThrowInvalidOperation:
    case ThrowArithmeticError:
    case ThrowDivisionByZeroError:
    case VerifyParamFailHard:
    case VerifyRetFailHard:
    case Unreachable:
    case EndBlock:
    case FatalMissingThis:
      return instSk == sk;

    // The RetCtrl is generally ending a bytecode instruction, with the
    // exception being in an Await bytecode instruction, where we consider the
    // end of the bytecode instruction to be the non-suspending path.
    case RetCtrl: {
      auto const op = inst.marker().sk().op();
      return op != Op::Await && op != Op::AwaitAll && op != Op::FCallAwait;
    }

    case AsyncRetCtrl:
    case AsyncRetFast:
      return true;

    // A ReqBindJmp ends a unit and it jumps to the next instruction to
    // execute.
    case ReqBindJmp: {
      auto destOffset = inst.extra<ReqBindJmp>()->target.offset();
      return sk.succOffsets().count(destOffset);
    }

    default:
      return false;
  }
}

Block* findMainExitBlock(const IRUnit& unit, SrcKey lastSk) {
  bool unreachable = false;
  Block* mainExit = nullptr;

  FTRACE(5, "findMainExitBlock: looking for exit at {} in unit:\n{}\n",
         showShort(lastSk), show(unit));

  for (auto block : rpoSortCfg(unit)) {
    if (block->back().is(Unreachable)) unreachable = true;

    if (endsUnitAtSrcKey(block, lastSk)) {
      if (mainExit == nullptr) {
        mainExit = block;
        continue;
      }

      always_assert_flog(
        mainExit->hint() == Block::Hint::Unlikely ||
        block->hint() == Block::Hint::Unlikely,
        "findMainExit: 2 likely exits found: B{} and B{}\nlastSk = {}",
        mainExit->id(), block->id(), showShort(lastSk)
      );

      if (mainExit->hint() == Block::Hint::Unlikely) mainExit = block;
    }
  }

  always_assert_flog(
    mainExit || unreachable,
    "findMainExit: no exit found for lastSk = {}",
    showShort(lastSk)
  );

  FTRACE(5, "findMainExitBlock: mainExit = B{}\n", mainExit->id());

  return mainExit;
}

///////////////////////////////////////////////////////////////////////////////

}}
