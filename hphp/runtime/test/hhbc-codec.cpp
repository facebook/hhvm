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
#include <gtest/gtest.h>

#include "hphp/runtime/vm/hhbc-codec.h"

namespace HPHP {

TEST(HHBCCodec, Basic) {
  std::vector<uint8_t> bc;

  for (size_t i = 0; i <= 0x1fd; ++i) {
    auto const op = static_cast<Op>(i);
    auto const before_sz = bc.size();
    encode_op(op, [&](uint8_t byte) { bc.emplace_back(byte); });
    auto const size = bc.size() - before_sz;

    // Verify that the encoded size is as expected.
    if (i < 0xff) {
      EXPECT_TRUE(size == 1);
    } else {
      EXPECT_TRUE(size == 2);
    }
  }

  const uint8_t* it = bc.data();
  auto const end = bc.data() + bc.size();
  for (size_t i = 0; i <= 0x1fd; ++i) {
    auto const op = decode_op_unchecked(it);
    if (i == 0x1fd) {
      EXPECT_EQ(it, end);
    } else {
      EXPECT_LT(it, end);
    }
    EXPECT_EQ(op, static_cast<Op>(i));
  }
}

}
