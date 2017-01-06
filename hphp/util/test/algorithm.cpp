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

#include "hphp/util/algorithm.h"

#include <folly/portability/GTest.h>

#include <string>
#include <unordered_map>
#include <vector>

namespace HPHP {

///////////////////////////////////////////////////////////////////////////////

TEST(Algorithm, sort_keys_by_value) {
  auto const input_vec = std::vector<int> { 3, 1, 7, 5 };
  auto const input_vec_order =
    sort_keys_by_value(input_vec, [] (int a, int b) { return a > b; });

  EXPECT_EQ(input_vec_order.size(), 4);
  EXPECT_EQ(input_vec_order[0], 2);
  EXPECT_EQ(input_vec_order[1], 3);
  EXPECT_EQ(input_vec_order[2], 0);
  EXPECT_EQ(input_vec_order[3], 1);

  auto input_map = std::unordered_map<int,int>{};
  input_map[0] = 3;
  input_map[1] = 1;
  input_map[3] = 7;
  input_map[4] = 5;

  auto const input_map_order =
    sort_keys_by_value(input_map, [] (int a, int b) { return a > b; });

  EXPECT_EQ(input_map_order.size(), 4);
  EXPECT_EQ(input_map_order[0], 3);
  EXPECT_EQ(input_map_order[1], 4);
  EXPECT_EQ(input_map_order[2], 0);
  EXPECT_EQ(input_vec_order[3], 1);

  auto input_hash = std::unordered_map<std::string,std::string>{};
  input_hash["apple"] = "plompkeen";
  input_hash["banana"] = "fraboli";
  input_hash["cherry"] = "onon";
  input_hash["durian"] = "shramp";

  auto const input_hash_order = sort_keys_by_value(
    input_hash,
    [] (const std::string& a, const std::string& b) { return a[0] < b[0]; }
  );

  EXPECT_EQ(input_hash_order.size(), 4);
  EXPECT_EQ(input_hash_order[0], "banana");
  EXPECT_EQ(input_hash_order[1], "cherry");
  EXPECT_EQ(input_hash_order[2], "apple");
  EXPECT_EQ(input_hash_order[3], "durian");
}

///////////////////////////////////////////////////////////////////////////////

}
