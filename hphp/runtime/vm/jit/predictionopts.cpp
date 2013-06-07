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

#include <boost/next_prior.hpp>

#include "folly/Optional.h"
#include "folly/Lazy.h"

#include "hphp/runtime/vm/jit/irfactory.h"
#include "hphp/runtime/vm/jit/state_vector.h"
#include "hphp/runtime/vm/jit/ir.h"
#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/mutation.h"

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
void optimizePredictions(IRTrace* const trace, IRFactory* const irFactory) {
  FTRACE(5, "PredOpts:vvvvvvvvvvvvvvvvvvvvv\n");
  SCOPE_EXIT { FTRACE(5, "PredOpts:^^^^^^^^^^^^^^^^^^^^^\n"); };

  auto const sortedBlocks = folly::lazy([&]{
    return rpoSortCfg(trace, *irFactory);
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
  auto optLdMem = [&] (IRInstruction* checkType,
                       IRInstruction* lastMarker) -> bool {
    auto const incRef = checkType->src(0)->inst();
    if (incRef->op() != IncRef) return false;
    auto const ldMem = incRef->src(0)->inst();
    if (ldMem->op() != LdMem) return false;
    if (ldMem->src(1)->getValInt() != 0) return false;
    if (!ldMem->typeParam().equals(Type::Cell)) return false;

    FTRACE(5, "candidate: {}\n", ldMem->toString());

    auto const mainBlock   = ldMem->block();
    auto const exit        = checkType->taken();
    auto const specialized = checkType->block()->next();

    if (mainBlock != checkType->block()) return false;
    if (exit->numPreds() != 1) return false;
    if (exit->isMain()) return false;

    auto const sinkFirst = mainBlock->iteratorTo(ldMem);
    auto const sinkLast  = mainBlock->iteratorTo(checkType);
    if (!instructionsAreSinkable(sinkFirst, sinkLast)) return false;

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
      checkType->typeParam(),
      checkType->taken(),
      ldMem->src(0)
    );
    mainBlock->insert(mainBlock->iteratorTo(ldMem), newCheckType);

    // Clone the instructions to the exit before specializing.
    cloneToBlock(rpoSort, irFactory, sinkFirst, sinkLast, exit);
    exit->insert(exit->skipHeader(), irFactory->cloneInstruction(lastMarker));

    /*
     * Specialize the LdMem left on the main trace after cloning the
     * generic version to the exit.  We'll reflowTypes after we're
     * done with all of this to get everything downstream specialized.
     */
    ldMem->setTypeParam(checkType->typeParam());

    /*
     * Replace the old CheckType with a Mov from the result of the
     * IncRef so that any uses of its dest point to the correct new
     * value.  We'll copyProp and get rid of this in a later pass.
     */
    irFactory->replace(
      checkType,
      Mov,
      incRef->dst()
    );

    // Move the fallthrough case to specialized.
    moveToBlock(sinkFirst, boost::next(sinkLast), specialized);
    specialized->insert(specialized->skipHeader(),
                        irFactory->cloneInstruction(lastMarker));

    return true;
  };

  /*
   * When we have a type prediction for a LdClsCns that is followed
   * immediately by CheckInit, we can merge both checks into the
   * LdClsCns and change it to exit to the same location the CheckInit
   * would've exited to.
   */
  auto optLdClsCns = [&] (IRInstruction* checkType,
                          IRInstruction* lastMarker) -> bool {
    auto const ldClsCns = checkType->src(0)->inst();
    if (ldClsCns->op() != LdClsCns) return false;
    if (ldClsCns->taken()) return false;

    auto const mainBlock = ldClsCns->block();
    auto const nextIt    = boost::next(mainBlock->iteratorTo(ldClsCns));
    if (nextIt == mainBlock->end()) return false;
    auto const checkInit = &*nextIt;
    if (checkInit->op() != CheckInit) return false;
    auto const exit      = checkInit->taken();
    if (exit->numPreds() != 1) return false;

    FTRACE(5, "candidate: {}\n", ldClsCns->toString());

    // Change the LdClsCns to do the check on the more refined type,
    // exiting to the trace we would've exited to, and get rid of the
    // CheckInit.
    checkInit->setTaken(nullptr);
    mainBlock->erase(mainBlock->iteratorTo(checkInit));
    ldClsCns->setTaken(exit);
    ldClsCns->setTypeParam(checkType->typeParam());

    // We don't need the checkType anymore.
    irFactory->replace(
      checkType,
      Mov,
      ldClsCns->dst()
    );

    return true;
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
  bool needsReflow = false;
  for (Block* b : trace->blocks()) {
    IRInstruction* lastMarker = nullptr;
    for (auto& inst : *b) {
      if (inst.op() == Marker) {
        lastMarker = &inst;
        continue;
      }

      if (inst.op() == CheckType &&
          inst.src(0)->type().equals(Type::Cell)) {
        assert(lastMarker);
        if (optLdMem(&inst, lastMarker) ||
            optLdClsCns(&inst, lastMarker)) {
          needsReflow = true;
          break;
        }
      }
    }
  }

  if (needsReflow) {
    auto& cfg = sortedBlocks();
    reflowTypes(cfg.front(), cfg);
  }
}

//////////////////////////////////////////////////////////////////////

}}
