/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2013 Facebook, Inc. (http://www.facebook.com)     |
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

  MoveInfo(Kind kind, PhysReg reg1, PhysReg reg2):
    m_kind(kind), m_reg1(reg1), m_reg2(reg2) {}

  Kind m_kind;
  PhysReg m_reg1, m_reg2;
};


bool cycleHasSIMDReg(const CycleInfo& cycle,
                     PhysReg::Map<PhysReg>& moves);
smart::vector<MoveInfo> doRegMoves(PhysReg::Map<PhysReg>& moves,
                                   PhysReg rTmp);

}}

#endif
