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
#pragma once

#include "hphp/runtime/vm/jit/block.h"
#include "hphp/runtime/vm/jit/ir-instruction.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/state-vector.h"

namespace HPHP::jit {

enum class KnownRegState {
  Dead,
  MaybeLive,
  Live
};

/*
 * Traverses the unit and examines the status of regState() to determine
 * whether the VMRegs are clean or dirty at a given point. Clean VMRegs are
 * considered to be live, as they may be freely loaded (but not stored) by
 * accessors in the runtime. Dirty VMRegs are considered to be dead, as they
 * must be synced (and therefore written to) to before being loaded in the
 * runtime.
 *
 * The state of the VMRegs is necessary to produce the best memory effects for
 * IR instructions that maySync. This routine produces a mapping from each
 * instruction to a known liveness state, indicating whether the VMRegs are
 * Live, Dead, or MaybeLive. The results of the analysis are consumed in store
 * and load elimination.
 */
StateVector<IRInstruction,KnownRegState> analyzeVMRegLiveness(
    IRUnit& unit, const BlockList& poBlockList);

}
