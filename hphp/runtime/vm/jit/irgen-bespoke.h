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

#include "hphp/runtime/vm/jit/irgen-state.h"
#include "hphp/runtime/vm/jit/normalized-instruction.h"

#include "hphp/runtime/vm/srckey.h"

namespace HPHP { namespace jit { namespace irgen {

///////////////////////////////////////////////////////////////////////////////

/*
 * Any extra locations that we should guard for a layout-sensitive bytecode.
 * Right now, this method only considers FCall bytecodes, because all other
 * layout-sensitive bytecodes already guard their inputs to DataTypeSpecific.
 */
jit::vector<Location> guardsForBespoke(const IRGS& env, SrcKey sk);

/*
 * If this bytecode is a bespoke array source or sink, this dispatch method
 * will hijack irgen and emit bespoke IR (possibly in addition to vanilla IR).
 *
 * In general, for a source or sink, this helper will emit profiling code in
 * profiling translations and use the chosen layout in optimized translations.
 * There are lots of special cases, though - see the implementation.
 */
void translateDispatchBespoke(IRGS&, const NormalizedInstruction&,
                              std::function<void(IRGS&)> emitVanilla);

///////////////////////////////////////////////////////////////////////////////

}}}
