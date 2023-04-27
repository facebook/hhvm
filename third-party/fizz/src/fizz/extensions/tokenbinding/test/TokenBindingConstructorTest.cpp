/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/extensions/tokenbinding/TokenBindingConstructor.h>
#include <fizz/extensions/tokenbinding/Validator.h>

using namespace folly;
using namespace folly::ssl;

using namespace testing;

namespace fizz {
namespace extensions {
namespace test {

StringPiece ekm{
    "1234567890012345689012345678901234567890123456789012345678901234"};

class TokenBindingConstructorTest : public Test {
 public:
  void SetUp() override {
    OpenSSL_add_all_algorithms();
    EcKeyUniquePtr ecKey(EC_KEY_new_by_curve_name(NID_X9_62_prime256v1));
    key_ = EvpPkeyUniquePtr(EVP_PKEY_new());
    if (!ecKey || !key_) {
      throw std::runtime_error("Unable to initialize key");
    }
    if (EC_KEY_generate_key(ecKey.get()) != 1) {
      throw std::runtime_error("Unable to generate EC Key");
    }
    if (EVP_PKEY_set1_EC_KEY(key_.get(), ecKey.get()) != 1) {
      throw std::runtime_error("Unable to set EC key");
    }
  }

  Buf getBuf(StringPiece hex) {
    auto data = unhexlify(hex);
    return folly::IOBuf::copyBuffer(data.data(), data.size());
  }

  EvpPkeyUniquePtr key_;
};

TEST_F(TokenBindingConstructorTest, TestSignAndValidate) {
  auto ekmBuf = getBuf(ekm);
  auto binding = TokenBindingConstructor::createTokenBinding(
      *key_.get(),
      ekmBuf,
      TokenBindingKeyParameters::ecdsap256,
      TokenBindingType::provided_token_binding);
  EXPECT_TRUE(
      Validator::validateTokenBinding(
          std::move(binding), ekmBuf, TokenBindingKeyParameters::ecdsap256)
          .has_value());
}

TEST_F(TokenBindingConstructorTest, TestBadEcKey) {
  auto badKey = EvpPkeyUniquePtr(EVP_PKEY_new());
  auto ekmBuf = getBuf(ekm);
  EXPECT_THROW(
      TokenBindingConstructor::createTokenBinding(
          *badKey.get(),
          ekmBuf,
          TokenBindingKeyParameters::ecdsap256,
          TokenBindingType::provided_token_binding),
      std::runtime_error);
}
} // namespace test
} // namespace extensions
} // namespace fizz
