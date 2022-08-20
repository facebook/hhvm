/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/extensions/tokenbinding/TokenBindingClientExtension.h>

using namespace testing;

namespace fizz {
namespace extensions {
namespace test {

class TokenBindingClientExtensionTest : public Test {
 public:
  void SetUp() override {
    context_ = std::make_shared<TokenBindingContext>();
    extensions_ = std::make_shared<TokenBindingClientExtension>(context_);
  }

  void setUpServerHelloExtensions(
      TokenBindingProtocolVersion version,
      TokenBindingKeyParameters keyParam) {
    TokenBindingParameters params;
    params.version = version;
    params.key_parameters_list.push_back(keyParam);
    serverExtensions_.push_back(encodeExtension(params));
  }

  std::vector<Extension> serverExtensions_;
  std::shared_ptr<TokenBindingClientExtension> extensions_;
  std::shared_ptr<TokenBindingContext> context_;
};

TEST_F(TokenBindingClientExtensionTest, TestValidCheckExtensions) {
  setUpServerHelloExtensions(
      TokenBindingProtocolVersion::token_binding_1_0,
      TokenBindingKeyParameters::ecdsap256);
  extensions_->onEncryptedExtensions(serverExtensions_);
  EXPECT_TRUE(extensions_->getVersion().has_value());
  EXPECT_EQ(
      extensions_->getVersion(),
      TokenBindingProtocolVersion::token_binding_1_0);
  EXPECT_TRUE(extensions_->getNegotiatedKeyParam().has_value());
  EXPECT_EQ(
      extensions_->getNegotiatedKeyParam(),
      TokenBindingKeyParameters::ecdsap256);
}

TEST_F(TokenBindingClientExtensionTest, TestNoExtensions) {
  extensions_->onEncryptedExtensions(serverExtensions_);
  EXPECT_FALSE(extensions_->getVersion().has_value());
  EXPECT_FALSE(extensions_->getNegotiatedKeyParam().has_value());
}

TEST_F(TokenBindingClientExtensionTest, TestServerBadKeyParam) {
  setUpServerHelloExtensions(
      TokenBindingProtocolVersion::token_binding_1_0,
      TokenBindingKeyParameters::rsa2048_pss);
  context_->setSupportedVersions(std::vector<TokenBindingProtocolVersion>{
      TokenBindingProtocolVersion::token_binding_1_0});
  context_->setSupportedKeyParameters(std::vector<TokenBindingKeyParameters>{
      TokenBindingKeyParameters::rsa2048_pkcs1_5});

  EXPECT_THROW(
      extensions_->onEncryptedExtensions(serverExtensions_), FizzException);
  EXPECT_FALSE(extensions_->getVersion().has_value());
  EXPECT_FALSE(extensions_->getNegotiatedKeyParam().has_value());
}
} // namespace test
} // namespace extensions
} // namespace fizz
