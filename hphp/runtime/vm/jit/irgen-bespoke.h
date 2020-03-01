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

#ifndef incl_HPHP_IRGEN_BESPOKE_H
#define incl_HPHP_IRGEN_BESPOKE_H

#include "hphp/runtime/vm/jit/irgen-state.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"

namespace HPHP { namespace jit { namespace irgen {

///////////////////////////////////////////////////////////////////////////////

/*
 * If this bytecode makes use of the layout of any of its array-like inputs,
 * check that we have a specific enough type to continue a tracelet.
 */
bool checkBespokeInputs(IRGS&, const NormalizedInstruction&);

/*
 * If this bytecode makes use of the layout of any of its array-like inputs,
 * handle the cases where they have some non-vanilla "bespoke" layout. The rest
 * of irgen may assume that these cases never occur.
 *
 * Having this hook allows us to handle these cases in wildly different ways
 * based on runtime flags, profiling vs. optimizing, etc.
 */
void handleBespokeInputs(IRGS&, const NormalizedInstruction&);

///////////////////////////////////////////////////////////////////////////////

}}}

#endif
