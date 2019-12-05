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

#include "hphp/util/non-invalidating-vector.h"

#include <gtest/gtest.h>

#include <vector>

namespace HPHP {

namespace {

//////////////////////////////////////////////////////////////////////

struct Element {
  explicit Element(int i) : i{i} {}
  Element(const Element&) = default;
  Element(Element&& o) noexcept : i{o.i} { o.i = -1; }
  Element& operator=(const Element&) = default;
  Element& operator=(Element&& o) { i = o.i; o.i = -1; return *this; }
  ~Element() { i = -1; }
  int i;
};

using ElementVec = NonInvalidatingVector<Element>;

//////////////////////////////////////////////////////////////////////

}

TEST(NonInvalidatingVector, Basic) {
  ElementVec vec;
  vec.reserve(0);

  std::vector<const Element*> ptrs;
  for (auto i = 0; i < 100; ++i) {
    EXPECT_EQ(vec.size(), i);
    vec.emplace_back(i);
    EXPECT_EQ(vec.size(), i+1);
    ptrs.emplace_back(&vec[i]);
  }

  for (auto i = 0; i < ptrs.size(); ++i) {
    EXPECT_EQ(ptrs[i]->i, i);
  }
  auto moved = std::move(vec);
  for (auto i = 0; i < ptrs.size(); ++i) {
    EXPECT_EQ(ptrs[i]->i, i);
  }

  moved.reserve(10000);
  for (auto i = 0; i < ptrs.size(); ++i) {
    EXPECT_EQ(ptrs[i]->i, i);
  }
}

//////////////////////////////////////////////////////////////////////

}
