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

#ifndef incl_HPHP_JIT_IRLOWER_H_
#define incl_HPHP_JIT_IRLOWER_H_

#include "hphp/runtime/vm/jit/code-cache.h"
#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/vasm-emit.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"
#include "hphp/runtime/vm/jit/vasm.h"

namespace HPHP { namespace jit {

struct IRUnit;
struct Vout;

///////////////////////////////////////////////////////////////////////////////

namespace irlower {

///////////////////////////////////////////////////////////////////////////////

enum class SyncOptions {
  None,
  Sync,
  SyncStublogue,
};

/*
 * State updated and tracked across vasm generation for individual instructions
 * and blocks.
 */
struct IRLS {
  explicit IRLS(const IRUnit& unit)
    : unit(unit)
    , labels(unit, Vlabel())
    , locs(unit, Vloc{})
  {}

  /*
   * The unit being lowered to vasm.
   */
  const IRUnit& unit;

  /*
   * The vasm output streams.
   *
   * These may be updated during codegen of an instruction to point to new
   * Vblocks.
   */
  Vout* vmain{nullptr};
  Vout* vcold{nullptr};

  /*
   * Vasm block labels, one for each HHIR block.
   */
  StateVector<Block,Vlabel> labels;

  /*
   * Vlocs for each SSATmp used or defined in a reachable block.
   */
  StateVector<SSATmp,Vloc> locs;
};

/*
 * Estimate the cost of unit.
 */
Vcost computeIRUnitCost(const IRUnit& unit);

/*
 * Lower the given HHIR unit to a Vunit, then optimize, regalloc, and return
 * the Vunit. Returns nullptr on failure.
 */
std::unique_ptr<Vunit> lowerUnit(const IRUnit&,
                                 CodeKind kind = CodeKind::Trace,
                                 bool regAlloc = true) noexcept;

///////////////////////////////////////////////////////////////////////////////

}}}

#endif
