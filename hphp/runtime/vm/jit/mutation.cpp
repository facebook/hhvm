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

#include "hphp/runtime/vm/jit/mutation.h"

#include "hphp/runtime/vm/jit/guard-relaxation.h"
#include "hphp/runtime/vm/jit/simplifier.h"
#include "hphp/runtime/vm/jit/state-vector.h"

namespace HPHP { namespace JIT {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

void cloneToBlock(const BlockList& rpoBlocks,
                  IRFactory& irFactory,
                  Block::iterator const first,
                  Block::iterator const last,
                  Block* const target) {
  assert(isRPOSorted(rpoBlocks));

  StateVector<SSATmp,SSATmp*> rewriteMap(irFactory, nullptr);

  auto rewriteSources = [&] (IRInstruction* inst) {
    for (int i = 0; i < inst->numSrcs(); ++i) {
      if (auto newTmp = rewriteMap[inst->src(i)]) {
        FTRACE(5, "  rewrite: {} -> {}\n",
               inst->src(i)->toString(),
               newTmp->toString());
        inst->setSrc(i, newTmp);
      }
    }
  };

  auto targetIt = target->skipHeader();
  for (auto it = first; it != last; ++it) {
    assert(!it->isControlFlow());

    FTRACE(5, "cloneToBlock({}): {}\n", target->id(), it->toString());
    auto const newInst = irFactory.cloneInstruction(&*it);

    if (auto const numDests = newInst->numDsts()) {
      for (int i = 0; i < numDests; ++i) {
        FTRACE(5, "  add rewrite: {} -> {}\n",
               it->dst(i)->toString(),
               newInst->dst(i)->toString());
        rewriteMap[it->dst(i)] = newInst->dst(i);
      }
    }

    target->insert(targetIt, newInst);
    targetIt = ++target->iteratorTo(newInst);
  }

  auto it = rpoIteratorTo(rpoBlocks, target);
  for (; it != rpoBlocks.end(); ++it) {
    FTRACE(5, "cloneToBlock: rewriting block {}\n", (*it)->id());
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

  auto const srcBlock = first->block();

  auto targetIt = target->skipHeader();
  for (auto it = first; it != last;) {
    auto const inst = &*it;
    assert(!inst->isControlFlow());

    FTRACE(5, "moveToBlock({}): {}\n",
           target->id(),
           inst->toString());

    it = srcBlock->erase(it);
    target->insert(targetIt, inst);
    targetIt = ++target->iteratorTo(inst);
  }
}

namespace {
/*
 * Given a load and the new type of that load's guard, update the type
 * of the load to match the relaxed type of the guard.
 */
void retypeLoad(IRInstruction* load, Type newType) {
  newType = load->is(LdLocAddr, LdStackAddr) ? newType.ptr() : newType;

  if (!newType.equals(load->typeParam())) {
    FTRACE(2, "retypeLoad changing type param of {} to {}\n", *load, newType);
    load->setTypeParam(newType);
  }
}


void visitLoad(IRInstruction* inst) {
  // Loads from locals and the stack are special: they get their type from a
  // guard instruction but have no direct reference to that guard. This block
  // only changes the load's type param; the loop afterwards will retype the
  // dest if needed.
  switch (inst->op()) {
    case LdLoc:
    case LdLocAddr: {
      auto const extra = inst->extra<LdLocData>();
      auto valSrc = extra->valSrc;

      // Unknown value that's new in this trace, so there's no guard to find.
      if (!valSrc) break;

      // If valSrc is a FramePtr, the load is of the local's original value so
      // we just have to find its guard. Otherwise, we know that the type of
      // valSrc has already been updated appropriately.
      Type newType;
      if (valSrc->isA(Type::FramePtr)) {
        auto guard = guardForLocal(extra->locId, valSrc);
        if (!guard) break;
        newType = guard->typeParam();
      } else {
        newType = valSrc->type();
      }
      retypeLoad(inst, newType);
      break;
    }

    case LdStack:
    case LdStackAddr: {
      auto idx = inst->extra<StackOffset>()->offset;
      auto typeSrc = getStackValue(inst->src(0), idx).typeSrc;
      if (typeSrc->is(GuardStk, CheckStk, AssertStk)) {
        retypeLoad(inst, typeSrc->typeParam());
      }
      break;
    }

    case LdRef: {
      auto inner = inst->src(0)->type().innerType();
      auto param = inst->typeParam();
      assert(inner.maybe(param));

      // If the type of the src has been relaxed past the LdRef's type param,
      // update the type param.
      if (inner > param) {
        inst->setTypeParam(inner);
      }
      break;
    }

    default: break;
  }
}

void retypeDst(IRInstruction* inst, int num) {
  auto ssa = inst->dst(num);

  /*
   * The type of a tmp defined by DefLabel is the union of the types of the
   * tmps at each incoming Jmp.
   */
  if (inst->op() == DefLabel) {
    Type type = Type::Bottom;
    inst->block()->forEachSrc(num, [&](IRInstruction*, SSATmp* tmp) {
        type = Type::unionOf(type, tmp->type());
      });
    ssa->setType(type);
    return;
  }

  ssa->setType(outputType(inst, num));
}

void visitInstruction(IRInstruction* inst) {
  visitLoad(inst);

  for (int i = 0; i < inst->numDsts(); ++i) {
    auto const ssa = inst->dst(i);
    auto const oldType = ssa->type();
    retypeDst(inst, i);
    if (!ssa->type().equals(oldType)) {
      FTRACE(5, "reflowTypes: retyped {} in {}\n", oldType.toString(),
             inst->toString());
    }
  }

  assertOperandTypes(inst);
}
}

void reflowTypes(Block* const changed, const BlockList& blocks) {
  assert(isRPOSorted(blocks));

  auto it = rpoIteratorTo(blocks, changed);
  assert(it != blocks.end());
  for (; it != blocks.end(); ++it) {
    FTRACE(5, "reflowTypes: visiting block {}\n", (*it)->id());
    for (auto& inst : **it) visitInstruction(&inst);
  }
}

//////////////////////////////////////////////////////////////////////

}}
