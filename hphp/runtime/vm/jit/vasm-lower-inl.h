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

#include "hphp/runtime/vm/jit/timer.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm-util.h"
#include "hphp/runtime/vm/jit/vasm-visit.h"

#include "hphp/util/assertions.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

template<class Vlower>
void vasm_lower(Vunit& unit, Vlower lower_impl) {
  Timer timer(Timer::vasm_lower, unit.log_entry);

  // This pass relies on having no critical edges in the unit.
  splitCriticalEdges(unit);

  // Scratch block can change blocks allocation, hence cannot use regular
  // iterators.
  auto& blocks = unit.blocks;

  // The vlower() implementations, and `lower', may allocate scratch blocks and
  // modify instruction streams, so we cannot use standard iterators here.
  PostorderWalker{unit}.dfs([&] (Vlabel b) {
    assertx(!blocks[b].code.empty());

    for (size_t i = 0; i < blocks[b].code.size(); ++i) {
      vlower(unit, b, i);
      lower_impl(blocks[b].code[i], b, i);
    }
  });

  printUnit(kVasmLowerLevel, "after vasm lowering", unit);
}

///////////////////////////////////////////////////////////////////////////////

}}
