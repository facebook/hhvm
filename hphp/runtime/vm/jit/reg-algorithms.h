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

#ifndef incl_HPHP_VM_REGALGORITHMS_H_
#define incl_HPHP_VM_REGALGORITHMS_H_

#include <vector>

#include "hphp/runtime/vm/jit/containers.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/vasm-reg.h"

namespace HPHP { namespace jit {
///////////////////////////////////////////////////////////////////////////////

struct Vunit;

///////////////////////////////////////////////////////////////////////////////

/*
 * Compute a sequence of moves and swaps that will fill the dest registers in
 * the moves map with their correct source values, even if some sources are
 * also destinations. The moves map provides one source for each dest.  rTmp
 * will be used when necessary to break copy-cycles, so it is illegal to
 * specify a source for rTmp (rTmp cannot be a destination).  However, it is
 * legal for rTmp to be a source for some other destination. Since rTmp cannot
 * be a destination, it cannot be in a copy-cycle, so its value will be read
 * before we deal with cycles.
 */

struct VMoveInfo {
  enum class Kind { Move, Xchg };
  Kind m_kind;
  Vreg m_src, m_dst;
};

struct MoveInfo {
  enum class Kind { Move, Xchg };
  Kind m_kind;
  PhysReg m_src, m_dst;
};

using MovePlan = PhysReg::Map<PhysReg>;
jit::vector<VMoveInfo> doVregMoves(Vunit&, MovePlan& moves);
jit::vector<MoveInfo> doRegMoves(MovePlan& moves, PhysReg rTmp);

///////////////////////////////////////////////////////////////////////////////
}}

#endif
