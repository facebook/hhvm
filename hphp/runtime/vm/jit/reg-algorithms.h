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

#include "hphp/runtime/base/smart-containers.h"
#include "hphp/runtime/vm/jit/phys-reg.h"

namespace HPHP { namespace JIT {


struct CycleInfo {
  PhysReg node;
  int length;
};

struct MoveInfo {
  enum class Kind { Move, Xchg };

  MoveInfo(Kind kind, PhysReg s, PhysReg d):
    m_kind(kind), m_src(s), m_dst(d) {}

  Kind m_kind;
  PhysReg m_src, m_dst;
};


bool cycleHasSIMDReg(const CycleInfo& cycle,
                     PhysReg::Map<PhysReg>& moves);

// Compute a sequence of moves and swaps that will fill the dest registers in
// the moves map with their correct source values, even if some sources are
// also destinations. The moves map provides one source for each dest.  rTmp
// will be used when necessary to break copy-cycles, so it is illegal to
// specify a source for rTmp (rTmp cannot be a desination).  However, it
// is legal for rTmp to be a source for some other destination. Since rTmp
// cannot be a destination, it cannot be in a copy-cycle, so its value will
// be read before we deal with cycles.
smart::vector<MoveInfo> doRegMoves(PhysReg::Map<PhysReg>& moves, PhysReg rTmp);

}}

#endif
