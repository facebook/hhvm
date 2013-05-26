/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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
#include "hphp/runtime/vm/translator/hopt/mutation.h"
#include "hphp/runtime/vm/translator/hopt/state_vector.h"

namespace HPHP { namespace JIT {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

void cloneToBlock(const BlockList& rpoBlocks,
                  IRFactory* const irFactory,
                  Block::iterator const first,
                  Block::iterator const last,
                  Block* const target) {
  assert(isRPOSorted(rpoBlocks));

  StateVector<SSATmp,SSATmp*> rewriteMap(irFactory, nullptr);

  auto rewriteSources = [&] (IRInstruction* inst) {
    for (int i = 0; i < inst->getNumSrcs(); ++i) {
      if (auto newTmp = rewriteMap[inst->getSrc(i)]) {
        FTRACE(5, "  rewrite: {} -> {}\n",
               inst->getSrc(i)->toString(),
               newTmp->toString());
        inst->setSrc(i, newTmp);
      }
    }
  };

  auto targetIt = target->skipLabel();
  for (auto it = first; it != last; ++it) {
    assert(!it->isControlFlowInstruction());

    FTRACE(5, "cloneToBlock({}): {}\n", target->getId(), it->toString());
    auto const newInst = irFactory->cloneInstruction(&*it);

    if (auto const numDests = newInst->getNumDsts()) {
      for (int i = 0; i < numDests; ++i) {
        FTRACE(5, "  add rewrite: {} -> {}\n",
               it->getDst(i)->toString(),
               newInst->getDst(i)->toString());
        rewriteMap[it->getDst(i)] = newInst->getDst(i);
      }
    }

    target->insert(targetIt, newInst);
    targetIt = ++target->iteratorTo(newInst);
  }

  auto it = rpoIteratorTo(rpoBlocks, target);
  for (; it != rpoBlocks.end(); ++it) {
    FTRACE(5, "cloneToBlock: rewriting block {}\n", (*it)->getId());
    for (auto& inst : **it) {
      FTRACE(5, " rewriting {}\n", inst.toString());
      rewriteSources(&inst);
    }
  }
}

void moveToBlock(Block::iterator const first,
                 Block::iterator const last,
                 Block* const target) {
  if (first == last) return;

  auto const srcBlock = first->getBlock();

  auto targetIt = target->skipLabel();
  for (auto it = first; it != last;) {
    auto const inst = &*it;
    assert(!inst->isControlFlowInstruction());

    FTRACE(5, "moveToBlock({}): {}\n",
           target->getId(),
           inst->toString());

    it = srcBlock->erase(it);
    target->insert(targetIt, inst);
    targetIt = ++target->iteratorTo(inst);
  }
}

void reflowTypes(Block* const changed, const BlockList& blocks) {
  assert(isRPOSorted(blocks));

  auto retypeDst = [&] (IRInstruction* inst, int num) {
    auto ssa = inst->getDst(num);

    /*
     * The type of a tmp defined by DefLabel is the union of the
     * types of the tmps at each incoming Jmp.
     */
    if (inst->op() == DefLabel) {
      Type type = Type::Bottom;
      inst->getBlock()->forEachSrc(num, [&](IRInstruction*, SSATmp* tmp) {
        type = Type::unionOf(type, tmp->type());
      });
      ssa->setType(type);
      return;
    }

    ssa->setType(outputType(inst, num));
  };

  auto visit = [&] (IRInstruction* inst) {
    for (int i = 0; i < inst->getNumDsts(); ++i) {
      auto const ssa = inst->getDst(i);
      auto const oldType = ssa->type();
      retypeDst(inst, i);
      if (ssa->type() != oldType) {
        FTRACE(5, "reflowTypes: retyped {} in {}\n", oldType.toString(),
               inst->toString());
      }
    }
  };

  auto it = rpoIteratorTo(blocks, changed);
  assert(it != blocks.end());
  for (; it != blocks.end(); ++it) {
    FTRACE(5, "reflowTypes: visiting block {}\n", (*it)->getId());
    for (auto& inst : **it) visit(&inst);
  }
}

//////////////////////////////////////////////////////////////////////

}}
