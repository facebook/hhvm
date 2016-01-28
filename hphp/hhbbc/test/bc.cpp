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
  bc::FPassC { 0 },
  bc::FPushFunc { 2 },
};

TEST(Bytecode, EqualityComparable) {
  Bytecode x = bc::Nop {};
  Bytecode y = bc::FPassC { 1 };
  EXPECT_FALSE(x == y);
  EXPECT_TRUE(bc::Nop {} == x);
  EXPECT_FALSE(bc::FPassC { 2 } == y);
  EXPECT_TRUE(y == bc::FPassC { 1 });

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

  std::unordered_map<Bytecode,Bytecode,bc_hash> map {
    { bc::Nop {}, bc::Int { 3 } },
    { bc::FPassC { 0 }, bc::Int { 4 } },
    { bc::FPassC { 1 }, bc::Int { 5 } },
  };

  EXPECT_EQ(map[bc::Nop{}], bc::Int { 3 });
  EXPECT_EQ(map[bc::FPassC { 0 }], bc::Int { 4 });
  EXPECT_EQ(map[bc::FPassC { 1 }], bc::Int { 5 });
}

}}
