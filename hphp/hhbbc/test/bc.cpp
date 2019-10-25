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
#include "hphp/hhbbc/bc.h"
#include <gtest/gtest.h>

#include <algorithm>
#include <vector>
#include <unordered_map>

#include <folly/Lazy.h>

namespace HPHP { namespace HHBBC {

std::vector<Bytecode> samples {
  bc::True {},
  bc::Int { 52 },
  bc::False {},
  bc::FCallFunc { FCallArgs(2) },
};

TEST(Bytecode, EqualityComparable) {
  Bytecode x = bc::Nop {};
  Bytecode y = bc::Int { 42 };
  EXPECT_FALSE(x == y);
  EXPECT_TRUE(bc::Nop {} == x);
  Bytecode q = bc::Int { 47 };
  EXPECT_FALSE(q == y);
  Bytecode r = bc::Int { 42 };
  EXPECT_TRUE(y == r);

  for (auto& b : samples) EXPECT_EQ(b, b);
  EXPECT_EQ(std::unique(begin(samples), end(samples)), end(samples));
}

TEST(Bytecode, Hash) {
  std::vector<size_t> hashes(samples.size());
  std::transform(
    begin(samples), end(samples), begin(hashes),
    [&] (const Bytecode& bc) { return hash(bc); }
  );
  EXPECT_EQ(std::unique(begin(hashes), end(hashes)), end(hashes));

  struct bc_hash {
    size_t operator()(const Bytecode& b) const { return hash(b); }
  };

  hphp_fast_map<Bytecode,Bytecode,bc_hash> map {
    { bc::Nop {}, bc::Int { 3 } },
    { bc::Int { 42 }, bc::Int { 4 } },
    { bc::Int { 47 }, bc::Int { 5 } },
  };

  Bytecode b1 = bc::Int { 42 };
  Bytecode b2 = bc::Int { 47 };
  EXPECT_EQ(map[bc::Nop{}], bc::Int { 3 });
  EXPECT_EQ(map[b1], bc::Int { 4 });
  EXPECT_EQ(map[b2], bc::Int { 5 });
}

}}
