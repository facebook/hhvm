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

#include "hphp/runtime/vm/jit/vasm-print.h"

#include <gtest/gtest.h>

namespace HPHP { namespace jit {

TEST(Vasm, PrintVptr) {
  auto v0 = Vreg{Vreg::V0};
  auto v1 = Vreg{Vreg::V0 + 1};
  Vptr p{Vreg{v0}, Vreg{}, 1, 0};
  EXPECT_EQ("[%128]", show(p));
  p.index = v1;
  EXPECT_EQ("[%128 + %129]", show(p));
  p.scale = 4;
  EXPECT_EQ("[%128 + %129 * 4]", show(p));
  p.disp = 0xbeef;
  EXPECT_EQ("[%128 + 0xbeef + %129 * 4]", show(p));
  p.disp = -16;
  p.index = Vreg{};
  EXPECT_EQ("[%128 - 0x10]", show(p));
  p.seg = Vptr::FS;
  EXPECT_EQ("[%fs + %128 - 0x10]", show(p));
  p.base = Vreg{};
  EXPECT_EQ("[%fs - 0x10]", show(p));
}

} }
