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

#include <gtest/gtest.h>

#include "hphp/runtime/base/comparisons.h"
#include "hphp/runtime/base/type-variant.h"

namespace HPHP {

TEST(Variant, Conversions) {
  Variant v("123");
  EXPECT_TRUE(v.toInt32() == 123);
}

TEST(Variant, References) {
  {
    Variant v1("original");
    Variant v2 = v1;
    v2 = String("changed");
    EXPECT_TRUE(equal(v1, String("original")));
  }
  {
    Variant v1("original");
    Variant v2(Variant::StrongBind{}, v1);
    v2 = String("changed");
    EXPECT_TRUE(equal(v1, String("changed")));
  }
}

}
