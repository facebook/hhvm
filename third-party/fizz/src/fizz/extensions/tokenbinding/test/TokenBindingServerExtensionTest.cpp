/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/extensions/tokenbinding/TokenBindingContext.h>
#include <fizz/extensions/tokenbinding/TokenBindingServerExtension.h>
#include <fizz/extensions/tokenbinding/Types.h>
#include <fizz/record/Extensions.h>
#include <fizz/server/ServerExtensions.h>

using namespace testing;

namespace fizz {
namespace extensions {
namespace test {

class TokenBindingServerExtensionTest : public Test {
 public:
  void SetUp() override {
    tokenBindingContext_ = std::make_shared<TokenBindingContext>();
    extensions_ =
        std::make_shared<TokenBindingServerExtension>(tokenBindingContext_);
  }

  void setUpTokenBindingWithParameters(
      TokenBindingProtocolVersion version,
      TokenBindingKeyParameters keyParams) {
    params_.version = version;
    params_.key_parameters_list.push_back(keyParams);
    chlo_.extensions.push_back(encodeExtension(std::move(params_)));
  }

  void verifyExtensionFields(
      const folly::Optional<TokenBindingParameters>& actual,
      TokenBindingProtocolVersion expectedVersion,
      TokenBindingKeyParameters expectedParams) {
    EXPECT_TRUE(actual.has_value());
    EXPECT_EQ(actual->version, expectedVersion);
    EXPECT_EQ(actual->key_parameters_list.size(), 1);
    EXPECT_EQ(actual->key_parameters_list[0], expectedParams);
  }

  TokenBindingParameters params_;
  ClientHello chlo_;
  std::shared_ptr<TokenBindingServerExtension> extensions_;
  std::shared_ptr<TokenBindingContext> tokenBindingContext_;
};

TEST_F(TokenBindingServerExtensionTest, TestFullNegotiationFlow) {
  setUpTokenBindingWithParameters(
      TokenBindingProtocolVersion::token_binding_1_0,
      TokenBindingKeyParameters::ecdsap256);
  auto exts = extensions_->getExtensions(chlo_);
  EXPECT_EQ(exts.size(), 1);

  auto tokenBindingExtension = getExtension<TokenBindingParameters>(exts);
  verifyExtensionFields(
      tokenBindingExtension,
      TokenBindingProtocolVersion::token_binding_1_0,
      TokenBindingKeyParameters::ecdsap256);
}

TEST_F(TokenBindingServerExtensionTest, TestNoExtensions) {
  auto exts = extensions_->getExtensions(chlo_);
  EXPECT_EQ(exts.size(), 0);
}

TEST_F(TokenBindingServerExtensionTest, TestIncompatibleKeyParam) {
  setUpTokenBindingWithParameters(
      TokenBindingProtocolVersion::token_binding_1_0,
      TokenBindingKeyParameters::ecdsap256);
  std::vector<TokenBindingKeyParameters> keyParams = {
      TokenBindingKeyParameters::rsa2048_pss};

  tokenBindingContext_->setSupportedKeyParameters(std::move(keyParams));
  auto exts = extensions_->getExtensions(chlo_);
  EXPECT_EQ(exts.size(), 0);
}

} // namespace test
} // namespace extensions
} // namespace fizz
