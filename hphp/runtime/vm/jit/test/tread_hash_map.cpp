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
#include "hphp/runtime/vm/tread-hash-map.h"

#include <vector>
#include <algorithm>
#include <iostream>

#include <gtest/gtest.h>

#include "hphp/util/base.h"

namespace HPHP {

TEST(TreadHashMap, Iteration) {
  TreadHashMap<int64_t,int64_t,int64_hash> thm(64);

  auto testExpect = [&](int size) {
    std::vector<int> contents;
    for (auto& k : thm) {
      contents.push_back(k.first);
    }
    EXPECT_EQ(int(contents.size()), size);
    std::sort(contents.begin(), contents.end());
    for (size_t i = 0; i < contents.size(); ++i) {
      EXPECT_EQ(contents[i], int(i + 1));
    }
  };

  for (int i = 1 /* 0 is invalid key */; i < 1024; ++i) {
    thm.insert(i, i);
    testExpect(i);
  }
}

}
