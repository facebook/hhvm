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
#ifndef incl_HPHP_JIT_IRGEN_EXIT_H_
#define incl_HPHP_JIT_IRGEN_EXIT_H_

#include <vector>
#include <functional>

#include "hphp/runtime/vm/hhbc.h"
#include "hphp/runtime/vm/jit/types.h"  // TransFlags

#include "hphp/runtime/vm/jit/ir-builder.h"

// This header has to include internal, because it uses things like
// peekSpillValues.
#include "hphp/runtime/vm/jit/irgen-internal.h"

namespace HPHP { struct StringData; }
namespace HPHP { namespace jit {
struct HTS;
struct Block;
}}

namespace HPHP { namespace jit { namespace irgen {

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
Block* makeExit(HTS&, Offset targetBcOff = -1);
Block* makeExit(HTS&, TransFlags trflags);

/*
 * Has the effects of makeExit(env) if the current function is a psuedomain,
 * and otherwise returns nullptr.
 */
Block* makePseudoMainExit(HTS&);

/*
 * Create a block that exits the current region by making a retranslate opt
 * service request.  Must not be used inside of an inlined function.
 */
Block* makeExitOpt(HTS&, TransID);

/*
 * Create a block that side exits the current region, after first calling the
 * interpreter to do an interp one of the current instruction.  "Slow" means
 * interpreter.
 *
 * The block is created with the current state.
 */
Block* makeExitSlow(HTS&);

//////////////////////////////////////////////////////////////////////

}}}

#endif
