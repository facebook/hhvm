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

#pragma once

#include "hphp/runtime/vm/jit/abi.h"
#ifdef __aarch64__
#include "hphp/runtime/vm/jit/abi-arm.h"
#else
#include "hphp/runtime/vm/jit/abi-x64.h"
#endif
#include "hphp/runtime/vm/jit/vasm-gen.h"
#include "hphp/runtime/vm/jit/vasm-instr.h"
#include "hphp/runtime/vm/jit/vasm-print.h"
#include "hphp/runtime/vm/jit/vasm-unit.h"

#include <folly/portability/GTest.h>

namespace HPHP::jit {

inline const Abi& regAllocTestAbi() {
#ifdef __aarch64__
  return arm::abi();
#else
  return x64::abi();
#endif
}

inline Vreg64 regAllocTestArg0() {
#ifdef __aarch64__
  return Vreg64{arm::rarg(0)};
#else
  return Vreg64{x64::rarg(0)};
#endif
}

inline void emitLdimm128CopyToStore(Vunit& unit) {
  unit.entry = unit.makeBlock(AreaIndex::Main, 1);
  Vout v(unit, unit.entry);

  auto const d0 = Vreg128{v.makeReg()};
  auto const d1 = Vreg128{v.makeReg()};

  v << ldimm128{0xdeadbeef, 0xcafef00d, d0};
  v << copy{d0, d1};
  v << storeups{d1, Vptr128{regAllocTestArg0()[0]}};
  v << ret{};
}

inline void expectPhysSIMD(Vreg reg) {
  EXPECT_TRUE(reg.isPhys());
  EXPECT_TRUE(reg.isSIMD());
}

inline void expectLdimm128CopyToStoreAllocated(const Vunit& unit) {
  auto const& code = unit.blocks[unit.entry].code;
  auto const out = show(unit);
  auto sawLdimm128 = false;
  auto sawStoreups = false;
  for (auto const& inst : code) {
    switch (inst.op) {
      case Vinstr::ldimm128:
        sawLdimm128 = true;
        expectPhysSIMD(Vreg{inst.ldimm128_.d});
        break;
      case Vinstr::copy:
        expectPhysSIMD(inst.copy_.s);
        expectPhysSIMD(inst.copy_.d);
        break;
      case Vinstr::storeups:
        sawStoreups = true;
        expectPhysSIMD(Vreg{inst.storeups_.s});
        break;
      default:
        break;
    }
  }
  EXPECT_TRUE(sawLdimm128) << out;
  EXPECT_TRUE(sawStoreups) << out;
}

}
