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
#include "hphp/util/tiny-vector.h"
#include <gtest/gtest.h>

namespace HPHP {

TEST(TinyVector, Basic) {
  TinyVector<int> v;

  auto fill = [&] {
    for (int i = 0; i < 10; ++i) {
      v.push_back(i);
    }
  };

  auto checkElems = [&] {
    for (auto& i : v) {
      EXPECT_EQ(i, v[i]);
    }
  };

  fill();
  EXPECT_EQ(10ul, v.size());
  checkElems();

  v.clear();
  EXPECT_EQ(0ul, v.size());

  fill();
  EXPECT_EQ(10ul, v.size());
  size_t size = v.size();
  for (size_t i = 0; i < size; ++i) {
    v.pop_back();
    checkElems();
    EXPECT_EQ(size - (i + 1), v.size());
  }

  EXPECT_EQ(0ul, v.size());
}

}
