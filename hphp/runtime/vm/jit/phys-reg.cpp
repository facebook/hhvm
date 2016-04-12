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

#include "hphp/runtime/vm/jit/phys-reg.h"

#include "hphp/runtime/base/arch.h"

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/mc-generator.h"

#include "hphp/util/asm-x64.h"
#include "hphp/vixl/a64/macro-assembler-a64.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

int PhysReg::getNumGP() {
  return abi().gp().size();
}

int PhysReg::getNumSIMD() {
  return abi().simd().size();
}

std::string show(PhysReg r) {
  switch (arch()) {
    case Arch::X64:
      return r.type() == PhysReg::GP   ? reg::regname(Reg64(r)) :
             r.type() == PhysReg::SIMD ? reg::regname(RegXMM(r)) :
          /* r.type() == PhysReg::SF)  ? */ reg::regname(RegSF(r));

    case Arch::ARM:
      if (r.isSF()) return "SF";

      return folly::to<std::string>(
        r.isGP() ? (vixl::Register(r).size() == vixl::kXRegSize ? 'x' : 'w')
                 : (vixl::FPRegister(r).size() == vixl::kSRegSize ? 's' : 'd'),
        ((vixl::CPURegister)r).code()
      );

    case Arch::PPC64:
      not_implemented();
  }
  not_reached();
}

///////////////////////////////////////////////////////////////////////////////

std::string show(RegSet regs) {
  std::ostringstream out;
  auto sep = "";

  out << '{';
  regs.forEach([&](PhysReg r) {
    out << sep << show(r);
    sep = ", ";
  });
  out << '}';

  return out.str();
}

///////////////////////////////////////////////////////////////////////////////

}}
