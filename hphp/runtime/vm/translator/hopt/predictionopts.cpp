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

#include <boost/next_prior.hpp>

#include "folly/Optional.h"
#include "folly/Lazy.h"

#include "hphp/runtime/vm/translator/hopt/irfactory.h"
#include "hphp/runtime/vm/translator/hopt/state_vector.h"
#include "hphp/runtime/vm/translator/hopt/ir.h"
#include "hphp/runtime/vm/translator/hopt/cfg.h"
#include "hphp/runtime/vm/translator/hopt/mutation.h"

namespace HPHP { namespace JIT {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

namespace {

template<class InputIterator>
bool instructionsAreSinkable(InputIterator first, InputIterator last) {
  for (; first != last; ++first) {
    switch (first->op()) {
    case ReDefSP:
    case ReDefGeneratorSP:
    case DecRef:
    case DecRefNZ:
    case Marker:
    case IncRef:
    case LdMem:
      return true;
    default:
      FTRACE(5, "unsinkable: {}\n", first->toString());
      return false;
    }
  }
  not_reached();
}

}

//////////////////////////////////////////////////////////////////////

/*
 * Optimizations that try to hoist CheckType instructions so that we
 * can specialize code earlier and avoid generic operations.
 */
void optimizePredictions(Trace* const trace, IRFactory* const irFactory) {
  FTRACE(5, "PredOpts:vvvvvvvvvvvvvvvvvvvvv\n");
  SCOPE_EXIT { FTRACE(5, "PredOpts:^^^^^^^^^^^^^^^^^^^^^\n"); };

  auto const sortedBlocks = folly::lazy([&]{
    return sortCfg(trace, *irFactory);
  });
  auto const predecessors = folly::lazy([&]{
    return computePredecessors(sortedBlocks());
  });

  /*
   * We frequently generate a generic LdMem, followed by a generic
   * IncRef, followed by a CheckType that refines that temporary or
   * otherwise branches to a block that's going to do a ReqBindJmp.
   * (This happens for e.g. in the case of object property accesses.)
   *
   * As long as the intervening instructions can be sunk into the exit
   * block this optimization will change these sequences to do the
   * type check before loading the value.  If it fails, we'll do the
   * generic LdMem/IncRef on the exit block, otherwise we do
   * type-specialized versions.
   */
  auto optLdMem = [&] (IRInstruction* checkType, IRInstruction* lastMarker) {
    auto const incRef = checkType->getSrc(0)->inst();
    if (incRef->op() != IncRef) return;
    auto const ldMem = incRef->getSrc(0)->inst();
    if (ldMem->op() != LdMem) return;
    if (ldMem->getSrc(1)->getValInt() != 0) return;
    if (!ldMem->getTypeParam().equals(Type::Cell)) return;

    FTRACE(5, "candidate: {}\n", ldMem->toString());

    auto const mainBlock   = ldMem->getBlock();
    auto const exit        = checkType->getTaken();
    auto const specialized = checkType->getBlock()->getNext();

    if (mainBlock != checkType->getBlock()) return;
    if (predecessors()[exit->postId()].size() != 1) return;
    if (exit->isMain()) return;

    auto const sinkFirst = mainBlock->iteratorTo(ldMem);
    auto const sinkLast  = mainBlock->iteratorTo(checkType);
    if (!instructionsAreSinkable(sinkFirst, sinkLast)) return;

    FTRACE(5, "all sinkable\n");
    auto const& rpoSort = sortedBlocks();

    /*
     * We are going to add a new CheckTypeMem instruction in front of
     * the LdMem.  Since CheckTypeMem is a control flow instruction,
     * it needs to end the block, so all the code after it has to move
     * to either the taken block (exit) or the fallthrough block
     * (specialized).
     */
    auto const newCheckType = irFactory->gen(
      CheckTypeMem,
      checkType->getTypeParam(),
      checkType->getTaken(),
      ldMem->getSrc(0)
    );
    mainBlock->insert(mainBlock->iteratorTo(ldMem), newCheckType);

    // Clone the instructions to the exit before specializing.
    cloneToBlock(rpoSort, irFactory, sinkFirst, sinkLast, exit);
    exit->insert(exit->skipLabel(), irFactory->cloneInstruction(lastMarker));

    /*
     * Specialize the LdMem left on the main trace after cloning the
     * generic version to the exit.  We'll reflowTypes in a sec to get
     * everything downstream specialized.
     */
    ldMem->setTypeParam(checkType->getTypeParam());

    /*
     * Replace the old CheckType with a Mov from the result of the
     * IncRef so that any uses of its dest point to the correct new
     * value.  We'll copyProp and get rid of this in a later pass.
     */
    irFactory->replace(
      checkType,
      Mov,
      incRef->getDst()
    );

    // Move the fallthrough case to specialized.
    moveToBlock(sinkFirst, boost::next(sinkLast), specialized);
    specialized->insert(specialized->skipLabel(),
                        irFactory->cloneInstruction(lastMarker));

    reflowTypes(specialized, rpoSort);
  };

  /*
   * The main loop for this pass.
   *
   * The individual optimizations called here are expected to change
   * block boundaries and potentially add new blocks.  They may not
   * unlink the block containing the CheckType instruction they are
   * visiting.
   */
  if (!trace->isMain()) return;
  for (Block* b : trace->getBlocks()) {
    IRInstruction* lastMarker = nullptr;
    for (auto& inst : *b) {
      if (inst.op() == Marker) {
        lastMarker = &inst;
        continue;
      }

      if (inst.op() == CheckType &&
          inst.getSrc(0)->type().equals(Type::Cell)) {
        assert(lastMarker);
        optLdMem(&inst, lastMarker);
        break;
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////

}}
