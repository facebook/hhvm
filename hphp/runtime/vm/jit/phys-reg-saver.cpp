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

#include "hphp/runtime/vm/jit/phys-reg-saver.h"

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/phys-reg.h"
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"

#include "hphp/util/arch.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

PhysRegSaver::PhysRegSaver(Vout& v, RegSet regs)
  : m_v(v)
  , m_regs(regs)
{
  auto gpr = m_regs & abi().gp();
  auto xmm = m_regs & abi().simd();
  auto const sp = rsp();

  if (!xmm.empty()) {
    v << lea{sp[-16 * xmm.size()], sp};

    int offset = 0;
    xmm.forEach([&] (PhysReg r) {
      v << storeups{r, sp[offset]};
      offset += 16;
    });
  }

  gpr.forEachPair([&] (PhysReg r0, PhysReg r1) {
    v << pushp{r0, r1 == InvalidReg ? r0 : r1};
  });

  v << vregunrestrict{};
}

PhysRegSaver::~PhysRegSaver() {
  auto& v = m_v;
  auto const sp = rsp();

  auto gpr = m_regs & abi().gp();
  auto xmm = m_regs & abi().simd();

  v << vregrestrict{};

  gpr.forEachPairR([&] (PhysReg r0, PhysReg r1) {
    v << popp{r0 == InvalidReg ? v.makeReg() : Vreg{r0}, r1};
  });

  if (!xmm.empty()) {
    int offset = 0;
    xmm.forEach([&] (PhysReg r) {
      v << loadups{sp[offset], r};
      offset += 16;
    });
    v << lea{sp[offset], sp};
  }
}

size_t PhysRegSaver::dwordsPushed() const {
  auto const gpr = m_regs & abi().gp();
  auto const xmm = m_regs & abi().simd();
  return 2 * xmm.size() + gpr.size() + (gpr.size() & 0x1);
}

///////////////////////////////////////////////////////////////////////////////

}}
