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

#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/abi-x64.h"

namespace HPHP { namespace JIT {

int PhysReg::kNumGP = 0;
int PhysReg::kNumSIMD = 0;

PhysRegSaverParity::PhysRegSaverParity(int parity, X64Assembler& as,
                                       RegSet regs)
    : m_as(as)
    , m_regs(regs)
{
  auto xmm = regs & X64::kXMMRegs;
  auto gpr = regs - xmm;
  m_adjust = (parity & 0x1) == (gpr.size() & 0x1) ? 8 : 0;
  if (!xmm.empty()) {
    m_as.      subq   (16 * xmm.size(), reg::rsp);
    int offset = 0;
    xmm.forEach([&](PhysReg pr) {
      m_as.    movdqu (pr, reg::rsp[offset]);
      offset += 16;
    });
  }
  gpr.forEach([&] (PhysReg pr) {
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
  emitPops(m_as, m_regs);
}

void PhysRegSaverParity::emitPops(X64Assembler& as, RegSet regs) {
  auto xmm = regs & X64::kXMMRegs;
  auto gpr = regs - xmm;
  gpr.forEachR([&] (PhysReg pr) {
    as.    pop    (pr);
  });
  if (!xmm.empty()) {
    int offset = 0;
    xmm.forEach([&](PhysReg pr) {
      as.  movdqu (reg::rsp[offset], pr);
      offset += 16;
    });
    as.    addq   (offset, reg::rsp);
  }
}

int PhysRegSaverParity::rspAdjustment() const {
  return m_adjust;
}

int PhysRegSaverParity::rspTotalAdjustmentRegs() const {
  return m_regs.size() + m_adjust / sizeof(int64_t);
}

void PhysRegSaverParity::bytesPushed(int64_t bytes) {
  m_adjust += bytes;
}

} }
