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

#include "hphp/util/union-find.h"

#include <folly/portability/GTest.h>

#include <unordered_map>
#include <vector>

namespace HPHP {

//////////////////////////////////////////////////////////////////////////////

namespace {

using Groups = std::vector<std::vector<std::string>>;

Groups sortedGroups(UnionFind<std::string>& uf) {
  Groups result;
  uf.forEachGroup([&](auto& group) {
    std::sort(group.begin(), group.end());
    result.push_back(group);
  });
  std::sort(result.begin(), result.end());
  return result;
}
}

TEST(UnionFind, basic) {
  auto uf = UnionFind<std::string>{};
  uf.insert("a");
  uf.insert("b");
  uf.insert("c");
  uf.insert("d");
  uf.insert("e");

  EXPECT_EQ(uf.countGroups(), 5);
  EXPECT_EQ(sortedGroups(uf), Groups({{"a"}, {"b"}, {"c"}, {"d"}, {"e"}}));

  uf.merge("a", "b");
  uf.merge("c", "d");

  EXPECT_EQ(uf.countGroups(), 3);
  EXPECT_EQ(sortedGroups(uf), Groups({{"a", "b"}, {"c", "d"}, {"e"}}));

  uf.merge("b", "c");

  EXPECT_EQ(uf.countGroups(), 2);
  EXPECT_EQ(sortedGroups(uf), Groups({{"a", "b", "c", "d"}, {"e"}}));

  uf.merge("a", "d");

  EXPECT_EQ(uf.countGroups(), 2);
  EXPECT_EQ(sortedGroups(uf), Groups({{"a", "b", "c", "d"}, {"e"}}));
}

//////////////////////////////////////////////////////////////////////////////

}
