/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-2015 Facebook, Inc. (http://www.facebook.com)     |
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

#include "hphp/runtime/vm/jit/abi.h"
#include "hphp/runtime/vm/jit/mc-generator.h"

namespace HPHP { namespace jit {

///////////////////////////////////////////////////////////////////////////////

int PhysReg::getNumGP() {
  return abi().gp().size();
}

int PhysReg::getNumSIMD() {
  return abi().simd().size();
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

///////////////////////////////////////////////////////////////////////////////

}}
