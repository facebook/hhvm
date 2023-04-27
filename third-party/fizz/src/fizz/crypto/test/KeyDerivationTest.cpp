/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GTest.h>

#include <fizz/crypto/KeyDerivation.h>
#include <fizz/crypto/Sha256.h>
#include <folly/String.h>
#include <folly/io/IOBuf.h>

using namespace folly;

namespace fizz {
namespace testing {

struct KdfParams {
  std::string secret;
  std::string label;
  std::string hashValue;
  std::string result;
};

class KeyDerivationTest : public ::testing::TestWithParam<KdfParams> {};

TEST_P(KeyDerivationTest, ExpandLabel) {
  auto prk = unhexlify(GetParam().secret);
  auto hashValue = unhexlify(GetParam().hashValue);

  auto hash = IOBuf::wrapBuffer(range(hashValue));

  auto secret = std::vector<uint8_t>(prk.begin(), prk.end());

  auto deriver = KeyDerivationImpl::create<Sha256>(kHkdfLabelPrefix.str());
  auto out = deriver.expandLabel(
      range(secret),
      GetParam().label,
      hash->clone(),
      GetParam().result.size() / 2);
  std::string hexOut = hexlify(out->coalesce());
  EXPECT_EQ(GetParam().result, hexOut);
}

TEST(KeyDerivation, DeriveSecret) {
  // dummy prk
  std::vector<uint8_t> secret(
      KeyDerivationImpl::create<Sha256>(kHkdfLabelPrefix.str()).hashLength());
  std::vector<uint8_t> messageHash(
      KeyDerivationImpl::create<Sha256>(kHkdfLabelPrefix.str()).hashLength());
  auto deriver = KeyDerivationImpl::create<Sha256>(kHkdfLabelPrefix.str());
  deriver.deriveSecret(
      range(secret), "hey", range(messageHash), deriver.hashLength());
}

TEST(KeyDerivation, Sha256BlankHash) {
  std::vector<uint8_t> computed(
      KeyDerivationImpl::create<Sha256>(kHkdfLabelPrefix.str()).hashLength());
  folly::IOBuf blankBuf;
  Sha256::hash(blankBuf, MutableByteRange(computed.data(), computed.size()));
  EXPECT_EQ(
      StringPiece(KeyDerivationImpl::create<Sha256>(kHkdfLabelPrefix.str())
                      .blankHash()),
      StringPiece(folly::range(computed)));
}

// These are taken by dumping mint's internal state
INSTANTIATE_TEST_SUITE_P(
    KeyDerivation,
    KeyDerivationTest,
    ::testing::Values(
        KdfParams{
            "09cf566b0a8cb6910fd56947fd030027f8195fc6c5b18ded76d758b436f5db2b",
            "handshake key expansion, client write iv",
            "b89ff73312c94b89fadc3320689fd2316a9fd04116b6742e5920a35aaed072a5",
            "0788c228aca5f4b6873b7b57"},
        KdfParams{
            "6174b457f0282fcad096afd50eb92d172638b6fa9baaf8e23ef6b23fe551df73",
            "server finished",
            "",
            "e1ad4d2331428327317047fbf73f3545a8c46fc17b1db445586e9e9b4249c00d"},
        KdfParams{
            "09cf566b0a8cb6910fd56947fd030027f8195fc6c5b18ded76d758b436f5db2b",
            "handshake key expansion, client write key",
            "b89ff73312c94b89fadc3320689fd2316a9fd04116b6742e5920a35aaed072a5",
            "93f640530d77b1d386e1d1089ec86382"}));
} // namespace testing
} // namespace fizz
