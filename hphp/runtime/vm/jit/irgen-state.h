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

#include <memory>
#include <vector>
#include <stack>
#include <utility>
#include <string>
#include <functional>

#include "hphp/runtime/vm/jit/bc-marker.h"
#include "hphp/runtime/vm/jit/inline-state.h"
#include "hphp/runtime/vm/jit/ir-builder.h"
#include "hphp/runtime/vm/jit/ir-unit.h"
#include "hphp/runtime/vm/jit/irgen-iter-spec.h"
#include "hphp/runtime/vm/jit/translator.h"
#include "hphp/runtime/vm/jit/types.h"

namespace HPHP { namespace jit {

struct NormalizedInstruction;
struct SSATmp;
struct TranslateRetryContext;

namespace irgen {

//////////////////////////////////////////////////////////////////////

/*
 * IR-Generation State.
 *
 * This structure contains the main state bag for the HHIR frontend, which
 * translates HHBC into HHIR.  The parse-time state in HHIR is relatively
 * non-trivial, for two reasons: one is that we perform a number of
 * optimizations during parse time, and two is that since the IR can not
 * represent all operations on generic types some simple type analysis is
 * required to determine high-level compilation strategy.
 */
struct IRGS {
  explicit IRGS(IRUnit& unit, const RegionDesc* region, int32_t budgetBCInstrs,
                TranslateRetryContext* retryContext);

  TransContext context;
  const RegionDesc* region;
  IRUnit& unit;
  std::unique_ptr<IRBuilder> irb;

  /*
   * Tracks information about the current bytecode offset and which function we
   * are in.
   */
  SrcKey bcState;

  /*
   * Tracks information about the state of inlining.
   */
  InlineState inlineState;

  /*
   * The ids of the profiling translations for the code we're currently
   * generating (may be empty).
   */
  TransIDSet profTransIDs;

  /*
   * True if we're on the first HHBC instruction that will be executed
   * for this instruction.  This is the first bytecode instruction in
   * either the region entry block or any other block in its
   * retranslation chain (i.e. that can be reached due to guard
   * failures before advancing VM state for any bytecode instruction).
   */
  bool firstBcInst{true};

  /*
   * True if we are just forming a region. Used to pessimize return values of
   * function calls that may have been inferred based on specialized type
   * information that won't be available when the region is translated.
   */
  bool formingRegion{false};

  /*
   * Profile-weight factor, to be multiplied by the region blocks'
   * profile-translation counters in PGO mode.
   */
  double profFactor{1};

  /*
   * The remaining bytecode instruction budget for this region translation.
   */
  int32_t budgetBCInstrs{0};

  /*
   * A surprise check has already been emitted and does not need to be generated
   * again for the current instruction.
   */
  bool skipSurpriseCheck{false};

  /*
   * Context for translation retries.
   */
  TranslateRetryContext* retryContext;

  /*
   * Used to reuse blocks of code between specialized IterInits and IterNexts.
   * See irgen-iter-spec for details.
   */
  jit::fast_map<Block*, std::unique_ptr<SpecializedIterator>> iters;
};

//////////////////////////////////////////////////////////////////////

/*
 * Debug-printable string.
 */
std::string show(const IRGS&);

//////////////////////////////////////////////////////////////////////

}}}
