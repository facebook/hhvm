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

#include "hphp/runtime/vm/jit/operand.h"

namespace HPHP {
namespace JIT{

using namespace Transl::reg;

TRACE_SET_MOD(hhir);

int RegisterInfo::numAllocatedRegs() const {
  // Return the number of register slots that actually have an allocated
  // register or spill slot.  We may not have allocated a full numNeededRegs()
  // worth of registers in some cases (if the value of this tmp wasn't used).
  // We rely on InvalidReg (-1) never being equal to a spill slot number.
  int i = 0;
  while (i < kMaxNumRegs && m_regs[i] != InvalidReg) {
    ++i;
  }
  return i;
}

RegSet RegisterInfo::regs() const {
  RegSet regs;
  for (int i = 0, n = numAllocatedRegs(); i < n; ++i) {
    if (hasReg(i)) regs.add(reg(i));
  }
  return regs;
}

}} // HPHP::JIT
