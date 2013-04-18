/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010- Facebook, Inc. (http://www.facebook.com)         |
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

#include "runtime/vm/translator/physreg.h"

namespace HPHP { namespace VM { namespace Transl {

PhysRegSaverParity::PhysRegSaverParity(int parity, X64Assembler& as,
                                       RegSet regs)
    : m_as(as)
    , m_regs(regs)
    , m_adjust((parity & 0x1) == (regs.size() & 0x1) ? 8 : 0)
{
  m_regs.forEach([&] (PhysReg pr) {
    m_as.    push   (pr);
  });
  if (m_adjust) {
    // Maintain stack evenness for SIMD compatibility.
    m_as.    subq   (m_adjust, reg::rsp);
  }
}

PhysRegSaverParity::~PhysRegSaverParity() {
  if (m_adjust) {
    // See above; stack parity.
    m_as.    addq   (m_adjust, reg::rsp);
  }
  m_regs.forEachR([&] (PhysReg pr) {
    m_as.    pop    (pr);
  });
}

int PhysRegSaverParity::rspAdjustment() const {
  return m_regs.size() + m_adjust / sizeof(int64_t);
}

void PhysRegSaverParity::bytesPushed(int64_t bytes) {
  m_adjust += bytes;
}

} } }
