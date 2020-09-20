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

#include <vector>
#include <functional>

#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/jit/extra-data.h"
#include "hphp/runtime/vm/jit/types.h"  // TransFlags

#include "hphp/runtime/vm/jit/ir-builder.h"

// This header has to include internal, because it uses things like
// peekSpillValues.
#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { namespace jit {

struct Block;

namespace irgen {

struct IRGS;

//////////////////////////////////////////////////////////////////////

/*
 * Create a block that side exits the current region, going to the supplied
 * target offset (if targetBcOff is -1, it goes to the current instruction's
 * offset).  The `trflags' version side exits to the current offset, and passes
 * extra flags to the service request which can be used while JITing to disable
 * certain optimizations.
 *
 * Both functions use the current state to create the block.
 */
Block* makeExit(IRGS&, Offset targetBcOff = -1);
Block* makeExit(IRGS&, TransFlags trflags);

/*
 * Identical to makeExit(IRGS, TransTrags), but only to be used in situations
 * where the exit is known to be from a guard failure. This will generate a
 * ReqRetranslate for bytecode instructions that could branch to themselves,
 * rather than punting.
 */
Block* makeGuardExit(IRGS&, TransFlags);

/*
 * Create a block that exits the current region by making a retranslate opt
 * service request.  Must not be used inside of an inlined function.
 */
Block* makeExitOpt(IRGS&);

/*
 * Create a block that side exits the current region, after first calling the
 * interpreter to do an interp one of the current instruction.  "Slow" means
 * interpreter.
 *
 * The block is created with the current state.
 */
Block* makeExitSlow(IRGS&);
Block* makeExitSurprise(IRGS&, Offset);

/*
 * Create a block that should never be reached. Useful for debug assertions.
 */
Block* makeUnreachable(IRGS&, AssertReason);

//////////////////////////////////////////////////////////////////////

}}}

