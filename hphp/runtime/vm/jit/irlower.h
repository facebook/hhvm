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

#ifndef incl_HPHP_JIT_IRLOWER_H_
#define incl_HPHP_JIT_IRLOWER_H_

#include "hphp/runtime/vm/jit/types.h"
#include "hphp/runtime/vm/jit/state-vector.h"
#include "hphp/runtime/vm/jit/vasm.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

namespace HPHP { namespace jit {

struct IRUnit;
struct Vout;

///////////////////////////////////////////////////////////////////////////////

namespace irlower {

///////////////////////////////////////////////////////////////////////////////

enum class SyncOptions {
  None,
  Sync,
  SyncAdjustOne,
};

enum class CatchCall {
  Uninit,
  PHP,
  CPP,
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
    , catch_calls(unit, CatchCall::Uninit)
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

  /*
   * Metadata used to handle catch blocks that are targets of calls.
   *
   * This StateVector is used to propagate information from the cg* function
   * which produces the call, to cgBeginCatch(), which encodes the information
   * in the landingpad{} instruction.
   */
  StateVector<Block,CatchCall> catch_calls;
};

/*
 * Generate machine code.
 *
 * Lower HHIR to vasm, optionally lower vasm to LLIR, run optimization passes,
 * emit code into main/cold/frozen sections, allocate RDS and global data, and
 * add fixup metadata.
 */
void genCode(IRUnit& unit, CodeKind kind = CodeKind::Trace);

///////////////////////////////////////////////////////////////////////////////

}}}

#endif
