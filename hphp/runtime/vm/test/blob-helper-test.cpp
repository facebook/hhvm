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
#include "hphp/runtime/vm/blob-helper.h"
#include <gtest/gtest.h>
#include <limits>
#include <iostream>
#include <utility>

namespace HPHP {

template<class T>
void testSerializationExactEquality(const T& val) {
  BlobEncoder encoder;
  T decodedVal;

  encoder(val);

  BlobDecoder decoder(encoder.data(), encoder.size());

  decoder(decodedVal);

  EXPECT_EQ(decodedVal, val);
}

template <typename T>
class IntegerSerializationTest : public ::testing::Test {};

typedef ::testing::Types<
                int8_t,
                uint8_t,
                int16_t,
                uint16_t,
                int32_t,
                uint32_t,
                int64_t,
                uint64_t
                >
                IntImplementations;

TYPED_TEST_CASE(IntegerSerializationTest,IntImplementations);

TYPED_TEST(IntegerSerializationTest, DoTest) {
  testSerializationExactEquality(std::numeric_limits<TypeParam>::min());
  testSerializationExactEquality(std::numeric_limits<TypeParam>::max());
  testSerializationExactEquality(TypeParam());
  for (int i = 0; i < std::numeric_limits<TypeParam>::digits; i ++) {
    testSerializationExactEquality(TypeParam(1) << i);
    testSerializationExactEquality(~(TypeParam(1) << i));
  }
}

TEST(BlobHelperTest, TestInputs) {
  testSerializationExactEquality(true);
  testSerializationExactEquality(false);
  testSerializationExactEquality(std::make_pair(false, 1));
  testSerializationExactEquality(std::make_pair(0xdeadbeef, 0xfaceb00c));
  // Can't get StringData to link
  //testSerializationExactEquality((const StringData*) nullptr);

}

}
