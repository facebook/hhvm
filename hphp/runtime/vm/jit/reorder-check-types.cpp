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

#include "hphp/runtime/vm/jit/opt.h"

#include "hphp/runtime/vm/jit/cfg.h"
#include "hphp/runtime/vm/jit/ir-opcode.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/mutation.h"
#include "hphp/runtime/vm/jit/timer.h"

#include <folly/Lazy.h>

#include <iterator>

namespace HPHP { namespace jit {

TRACE_SET_MOD(hhir);

//////////////////////////////////////////////////////////////////////

/*
 * This optimization reorders chained, back-to-back CheckType instructions where
 * the second check is for a subtype of the first one.  More specifically, this
 * optimization transforms this pattern:
 *
 *   [block]
 *     tmp1:type1 = CheckType<type1> tmp0 -> taken1  [ct1]
 *   [fallthru1]
 *     tmp2:type2 = CheckType<type2> tmp1 -> taken2  [ct2]
 *   [fallthru2]
 *     ...
 *
 *   vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
 *   under the condition that type2 < type1, into the following one:
 *   vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
 *
 *   [block]
 *     tmp2:type2 = CheckType<type2> tmp0 -> fallthru1 [ct1]
 *   [fallthru2]
 *     ...
 *
 *   [fallthru1]
 *     tmp1:type1 = CheckType<type1> tmp0 -> taken1  [ct2]
 *   [taken2]
 *
 */
void reorderCheckTypes(IRUnit& unit) {
  Timer timer(Timer::optimize_reorderCheckTypes, unit.logEntry().get_pointer());

  FTRACE(5, "ReorderCheckTypes:vvvvvvvvvvvvvvvvvvvvv\n");
  SCOPE_EXIT { FTRACE(5, "ReorderCheckTypes:^^^^^^^^^^^^^^^^^^^^^\n"); };

  auto const blocks = rpoSortCfg(unit);

  // This transformation assumes that any use of tmp1 that is dominated by tmp2
  // actually uses tmp2 (which provides a more refined type) and not tmp1. Thus
  // we run refineTmps to make sure that this invariant applies.
  auto const idoms = findDominators(unit, blocks,
                                    numberBlocks(unit, blocks));
  refineTmps(unit, blocks, idoms);

  for (auto& block : blocks) {
    auto& ct1 = block->back();
    if (!ct1.is(CheckType)) continue;
    auto fallthru1 = ct1.next();
    auto& ct2 = fallthru1->front();
    if (!ct2.is(CheckType)) continue;
    if (ct1.dst() != ct2.src(0)) continue;
    auto const type1 = ct1.typeParam();
    auto const type2 = ct2.typeParam();
    if (!(type2 < type1)) continue;

    FTRACE(5, "  - reordering these 2 instructions:\n   - {}\n   - {}\n",
           ct1, ct2);

    auto tmp0 = ct1.src(0);
    auto tmp1 = ct1.dst(0);
    auto tmp2 = ct2.dst(0);
    auto taken1 = ct1.taken();
    auto taken2 = ct2.taken();
    auto fallthru2 = ct2.next();

    // Fix block, ct1 and tmp2.
    ct1.setTaken(fallthru1);
    ct1.setNext(fallthru2);
    ct1.setDst(tmp2);
    ct1.setTypeParam(type2);
    ct1.setSrc(0, tmp0);
    tmp2->setInstruction(&ct1);

    // Fix fallthru1, ct2 and tmp1.
    ct2.setTaken(taken1);
    ct2.setNext(taken2);
    ct2.setDst(tmp1);
    ct2.setTypeParam(type1);
    ct2.setSrc(0, tmp0);
    tmp1->setInstruction(&ct2);
    fallthru1->setProfCount(taken2->profCount());
    fallthru1->setHint(taken2->hint());
  }
}

//////////////////////////////////////////////////////////////////////

}}
