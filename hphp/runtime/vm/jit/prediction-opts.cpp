/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2016 Facebook, Inc. (http://www.facebook.com)     |
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

#include <iterator>

#include <folly/Lazy.h>
#include <folly/Optional.h>

#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/mutation.h"
#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/timer.h"

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

namespace {

template<class InputIterator>
bool instructionsAreSinkable(InputIterator first, InputIterator last) {
  for (; first != last; ++first) {
    switch (first->op()) {
    case DecRef:
    case DecRefNZ:
    case IncRef:
    case LdMem:
      continue;
    default:
      FTRACE(5, "unsinkable: {}\n", first->toString());
      return false;
    }
  }
  return true;
}

/*
 * Returns whether a type is generic enough to try hoisting a prediction in
 * order to specialize it.
 */
bool typeSufficientlyGeneric(Type t) {
  return t.needsReg();
}

}

//////////////////////////////////////////////////////////////////////

/*
 * Optimization that tries to hoist CheckType instructions so that we can
 * specialize code earlier and avoid generic operations.
 */
void optimizePredictions(IRUnit& unit) {
  Timer timer(Timer::optimize_predictionOpts);

  FTRACE(5, "PredOpts:vvvvvvvvvvvvvvvvvvvvv\n");
  SCOPE_EXIT { FTRACE(5, "PredOpts:^^^^^^^^^^^^^^^^^^^^^\n"); };

  auto const sortedBlocks = folly::lazy([&]{
    return rpoSortCfg(unit);
  });

  /*
   * We frequently generate a generic load, followed by a generic IncRef,
   * followed by a CheckType that refines that temporary or otherwise branches
   * to a block that's going to do a ReqBindJmp.  (This happens for e.g. in the
   * case of object property accesses.)
   *
   * As long as the intervening instructions can be sunk into the exit block
   * this optimization will change these sequences to do the type check before
   * loading the value.  If it fails, we'll do the generic load/IncRef on the
   * exit block, otherwise we do type-specialized versions.
   */
  auto doOpt = [&] (IRInstruction* checkType) -> bool {
    auto const load = checkType->src(0)->inst();
    if (!load->is(LdMem)) return false;
    if (!typeSufficientlyGeneric(load->dst()->type())) return false;

    FTRACE(5, "candidate: {}\n", load->toString());

    auto const mainBlock   = load->block();
    auto const exit        = checkType->taken();
    auto const specialized = checkType->next();

    if (mainBlock != checkType->block()) return false;
    if (exit->numPreds() != 1) return false;
    if (!exit->isExit()) return false;

    auto const sinkFirst = mainBlock->iteratorTo(load);
    auto const sinkLast  = mainBlock->iteratorTo(checkType);
    if (!instructionsAreSinkable(sinkFirst, sinkLast)) return false;

    FTRACE(5, "all sinkable\n");
    auto const& rpoSort = sortedBlocks();

    /*
     * We are going to add a new check instruction in front of the Load.  Since
     * check is a control flow instruction, it needs to end the block, so all
     * the code after it has to move to either the taken block (exit) or the
     * fallthrough block (specialized).
     */
    auto const newCheckType = unit.gen(CheckTypeMem,
                                       checkType->marker(),
                                       checkType->typeParam(),
                                       exit,
                                       load->src(0)
                                      );
    newCheckType->setNext(specialized);
    mainBlock->insert(mainBlock->iteratorTo(load), newCheckType);

    // Clone the instructions to the exit before specializing.
    cloneToBlock(rpoSort, unit, sinkFirst, sinkLast, exit);

    /*
     * Specialize the Load left on the main trace after cloning the
     * generic version to the exit---give it the type the old
     * CheckType would've produced.  We'll reflowTypes after we're
     * done with all of this to get everything downstream specialized.
     */
    load->setTypeParam(outputType(checkType));

    /*
     * Replace the old CheckType with a Mov from the result of the
     * Load so that any uses of its dest point to the correct new
     * value.  We'll copyProp and get rid of this in a later pass.
     */
    unit.replace(
      checkType,
      Mov,
      load->dst()
    );

    // Move the fallthrough case to specialized.
    moveToBlock(sinkFirst, std::next(sinkLast), specialized);

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
  bool needsReflow = false;
  postorderWalk(unit, [&](Block* b) {
    for (auto& inst : *b) {
      if (inst.op() == CheckType &&
          typeSufficientlyGeneric(inst.src(0)->type())) {
        if (doOpt(&inst)) {
          needsReflow = true;
          break;
        }
      }
    }
  });

  if (needsReflow) {
    reflowTypes(unit);
  }
}

//////////////////////////////////////////////////////////////////////

}}
