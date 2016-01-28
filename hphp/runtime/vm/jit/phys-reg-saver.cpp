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

#include "hphp/runtime/vm/jit/phys-reg-saver.h"

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

PhysRegSaver::PhysRegSaver(Vout& v, RegSet regs)
  : m_v(v)
  , m_regs(regs)
{
  auto gpr = m_regs & abi().gp();
  auto xmm = m_regs & abi().simd();
  auto const sp = rsp();

  m_adjust = gpr.size() & 0x1 ? 8 : 0;

  if (!xmm.empty()) {
    v << subqi{16 * xmm.size(), sp, sp, v.makeReg()};

    int offset = 0;
    xmm.forEach([&] (PhysReg r) {
      v << storeups{r, sp[offset]};
      offset += 16;
    });
  }

  gpr.forEach([&] (PhysReg r) {
    v << push{r};
  });

  if (m_adjust) {
    v << subqi{m_adjust, sp, sp, v.makeReg()};
  }
}

PhysRegSaver::~PhysRegSaver() {
  auto& v = m_v;
  auto const sp = rsp();

  if (m_adjust) {
    v << addqi{m_adjust, sp, sp, v.makeReg()};
  }

  auto gpr = m_regs & abi().gp();
  auto xmm = m_regs & abi().simd();

  gpr.forEachR([&] (PhysReg r) {
    v << pop{r};
  });

  if (!xmm.empty()) {
    int offset = 0;
    xmm.forEach([&] (PhysReg r) {
      v << loadups{sp[offset], r};
      offset += 16;
    });
    v << addqi{offset, sp, sp, v.makeReg()};
  }
}

size_t PhysRegSaver::rspAdjustment() const {
  return m_adjust;
}

size_t PhysRegSaver::dwordsPushed() const {
  assertx((m_adjust % sizeof(int64_t)) == 0);
  return m_regs.size() + m_adjust / sizeof(int64_t);
}

///////////////////////////////////////////////////////////////////////////////

}}
