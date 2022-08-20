/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include <fizz/crypto/Hkdf.h>
#include <fizz/crypto/Sha256.h>
#include <fizz/crypto/test/TestUtil.h>

namespace fizz {
namespace test {

struct HashParams {
  std::string ikm;
  std::string salt;
  std::string info;
  size_t outputBytes;
  std::string okm;
};

class HkdfTest : public ::testing::TestWithParam<HashParams> {};

TEST_P(HkdfTest, TestHkdfSha256Expand) {
  auto ikm = toIOBuf(GetParam().ikm);
  auto salt = toIOBuf(GetParam().salt);
  auto info = toIOBuf(GetParam().info);
  size_t outputBytes = GetParam().outputBytes;
  auto expectedOkm = toIOBuf(GetParam().okm);
  CHECK_EQ(outputBytes, expectedOkm->length());

  auto actualOkm = HkdfImpl::create<Sha256>().hkdf(
      ikm->coalesce(), salt->coalesce(), *info, outputBytes);
  EXPECT_FALSE(actualOkm->isChained());
  EXPECT_EQ(outputBytes, actualOkm->length());
  EXPECT_FALSE(memcmp(actualOkm->data(), expectedOkm->data(), outputBytes));
}

// Test cases from https://tools.ietf.org/html/rfc5869
INSTANTIATE_TEST_SUITE_P(
    TestVectors,
    HkdfTest,
    ::testing::Values(
        HashParams{
            "0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b",
            "000102030405060708090a0b0c",
            "f0f1f2f3f4f5f6f7f8f9",
            42,
            "3cb25f25faacd57a90434f64d0362f2a"
            "2d2d0a90cf1a5a4c5db02d56ecc4c5bf"
            "34007208d5b887185865"},
        HashParams{
            "000102030405060708090a0b0c0d0e0f"
            "101112131415161718191a1b1c1d1e1f"
            "202122232425262728292a2b2c2d2e2f"
            "303132333435363738393a3b3c3d3e3f"
            "404142434445464748494a4b4c4d4e4f",
            "606162636465666768696a6b6c6d6e6f"
            "707172737475767778797a7b7c7d7e7f"
            "808182838485868788898a8b8c8d8e8f"
            "909192939495969798999a9b9c9d9e9f"
            "a0a1a2a3a4a5a6a7a8a9aaabacadaeaf",
            "b0b1b2b3b4b5b6b7b8b9babbbcbdbebf"
            "c0c1c2c3c4c5c6c7c8c9cacbcccdcecf"
            "d0d1d2d3d4d5d6d7d8d9dadbdcdddedf"
            "e0e1e2e3e4e5e6e7e8e9eaebecedeeef"
            "f0f1f2f3f4f5f6f7f8f9fafbfcfdfeff",
            82,
            "b11e398dc80327a1c8e7f78c596a4934"
            "4f012eda2d4efad8a050cc4c19afa97c"
            "59045a99cac7827271cb41c65e590e09"
            "da3275600c2f09b8367793a9aca3db71"
            "cc30c58179ec3e87c14c01d5c1f3434f"
            "1d87"},
        HashParams{
            "0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b0b",
            "",
            "",
            42,
            "8da4e775a563c18f715f802a063c5a31"
            "b8a11f5c5ee1879ec3454e5f3c738d2d"
            "9d201395faa4b61a96c8"}));
} // namespace test
} // namespace fizz
