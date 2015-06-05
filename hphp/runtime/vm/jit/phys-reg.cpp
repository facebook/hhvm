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

#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/abi-x64.h"
#include "hphp/runtime/vm/jit/mc-generator.h"
#include "hphp/runtime/vm/jit/vasm-emit.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"

namespace HPHP { namespace jit {
using namespace x64;

int PhysReg::getNumGP() {
  return mcg->backEnd().abi().gp().size();
}

int PhysReg::getNumSIMD() {
  return mcg->backEnd().abi().simd().size();
}

std::string show(RegSet regs) {
  auto& backEnd = mcg->backEnd();
  std::ostringstream out;
  auto sep = "";

  out << '{';
  regs.forEach([&](PhysReg r) {
    out << sep;
    backEnd.streamPhysReg(out, r);
    sep = ", ";
  });
  out << '}';

  return out.str();
}

PhysRegSaverParity::PhysRegSaverParity(int parity, Vout& v,
                                       RegSet regs)
    : m_as(nullptr)
    , m_v(&v)
    , m_regs(regs)
{
  auto xmm = regs & x64::kXMMRegs;
  auto gpr = regs - xmm;
  m_adjust = (parity & 0x1) == (gpr.size() & 0x1) ? 8 : 0;
  if (!xmm.empty()) {
    v << subqi{16 * xmm.size(), reg::rsp, reg::rsp, v.makeReg()};
    int offset = 0;
    xmm.forEach([&](PhysReg pr) {
      v << storeups{pr, reg::rsp[offset]};
      offset += 16;
    });
  }
  gpr.forEach([&] (PhysReg pr) {
    v << push{pr};
  });
  if (m_adjust) {
    // Maintain stack evenness for SIMD compatibility.
    v << subqi{m_adjust, reg::rsp, reg::rsp, v.makeReg()};
  }
}

PhysRegSaverParity::PhysRegSaverParity(int parity, X64Assembler& as,
                                       RegSet regs)
  : PhysRegSaverParity{parity, Vauto(as.code()).main(), regs} {
  m_v = nullptr;
  m_as = &as;
}

PhysRegSaverParity::~PhysRegSaverParity() {
  auto finish = [&](Vout& v) {
    if (m_adjust) {
      // See above; stack parity.
      auto const sf = v.makeReg();
      v << addqi{m_adjust, reg::rsp, reg::rsp, sf};
    }
    emitPops(v, m_regs);
  };
  if (m_as) {
    finish(Vauto(m_as->code()).main());
  } else {
    finish(*m_v);
  }
}

void PhysRegSaverParity::emitPops(Vout& v, RegSet regs) {
  auto xmm = regs & x64::kXMMRegs;
  auto gpr = regs - xmm;
  gpr.forEachR([&] (PhysReg pr) {
    v << pop{pr};
  });
  if (!xmm.empty()) {
    int offset = 0;
    xmm.forEach([&](PhysReg pr) {
      v << loadups{reg::rsp[offset], pr};
      offset += 16;
    });
    auto const sf = v.makeReg();
    v << addqi{offset, reg::rsp, reg::rsp, sf};
  }
}

void PhysRegSaverParity::emitPops(X64Assembler& as, RegSet regs) {
  emitPops(Vauto(as.code()).main(), regs);
}

int PhysRegSaverParity::rspAdjustment() const {
  return m_adjust;
}

int PhysRegSaverParity::dwordsPushed() const {
  assert((m_adjust % sizeof(int64_t)) == 0);
  return m_regs.size() + m_adjust / sizeof(int64_t);
}

void PhysRegSaverParity::bytesPushed(int bytes) {
  m_adjust += bytes;
}

} }
