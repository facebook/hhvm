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

namespace detail {

/*
 * Computes the dominating vreg constraint for each block. When lowering, this
 * will be used to determine if a lowerer can use a new vreg as a scratch, or
 * if it must rely on platform specific assembler scratch registers.
 * The dominating restriction is used initially for the block and then updated
 * whenever a vregrestrict/vregunrestrict is encountered.
 *
 *   INT_MAX: uninitialized
 *      >= 0: new vregs allowed
 *      <  0: new vregs restricted
 *
 * vregrestrict and vregunrestrict can be nested for scoping purposes. Each
 * nested vregrestrict decreases the constraint. vregunrestrict increases it.
 */
jit::vector<int> computeDominatingConstraints(Vunit& unit);

}

///////////////////////////////////////////////////////////////////////////////

template<class Vlower>
void vasm_lower(Vunit& unit, Vlower lower_impl) {
  Timer timer(Timer::vasm_lower, unit.log_entry);

  // This pass relies on having no critical edges in the unit.
  splitCriticalEdges(unit);

  // Scratch block can change blocks allocation, hence cannot use regular
  // iterators.
  auto& blocks = unit.blocks;

  VLS env { unit };
  auto const vregConstraints = detail::computeDominatingConstraints(unit);

  // The vlower() implementations, and `lower', may allocate scratch blocks and
  // modify instruction streams, so we cannot use standard iterators here.
  PostorderWalker{unit}.dfs([&] (Vlabel b) {
    assertx(!blocks[b].code.empty());

    env.vreg_restrict_level = vregConstraints[b];
    for (size_t i = 0; i < blocks[b].code.size(); ++i) {
      DEBUG_ONLY auto const allow = env.allow_vreg();
      DEBUG_ONLY auto const next = unit.next_vr;

      vlower(env, b, i);
      lower_impl(env, blocks[b].code[i], b, i);

      assertx(allow || unit.next_vr == next);
    }
  });

  printUnit(kVasmLowerLevel, "after vasm lowering", unit);
}

///////////////////////////////////////////////////////////////////////////////

}}
