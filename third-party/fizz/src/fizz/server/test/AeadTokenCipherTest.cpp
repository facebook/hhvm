/*
 * Copyright (c) Facebook, Inc. and its affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <fizz/server/AeadTokenCipher.h>
#include <folly/Random.h>
#include <folly/io/IOBuf.h>
#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

using namespace folly;
using namespace testing;

namespace fizz::server::test {

class AeadTokenCipherTest : public Test {
 public:
  void SetUp() override {
    std::vector<std::string> contexts = {"AeadTokenCipherTest"};
    cipher_ = std::make_unique<Aead128GCMTokenCipher>(contexts);

    std::array<uint8_t, 32> secret;
    folly::Random::secureRandom(secret.data(), secret.size());
    secrets.emplace_back(folly::range(secret));

    cipher_->setSecrets(secrets);
  }

  std::unique_ptr<TokenCipher> cipher_;
  std::vector<folly::ByteRange> secrets;
};

TEST_F(AeadTokenCipherTest, MatchingAssocDataTest) {
  auto assocDataBuf = IOBuf::copyBuffer("plaintext_assoc_data");
  auto cipherText =
      cipher_->encrypt(IOBuf::copyBuffer("plaintext"), assocDataBuf.get());

  EXPECT_TRUE(cipherText.hasValue());

  auto plainText = cipher_->decrypt(std::move(*cipherText), assocDataBuf.get());

  EXPECT_TRUE(plainText.hasValue());

  EXPECT_TRUE(IOBufEqualTo{}(*plainText, IOBuf::copyBuffer("plaintext")));
}

TEST_F(AeadTokenCipherTest, UnmatchedAssocDataTest) {
  auto assocDataBuf = IOBuf::copyBuffer("plaintext_assoc_data");
  auto otherAssocDataBuf = IOBuf::copyBuffer("other_plaintext_assoc_data");

  auto cipherText =
      cipher_->encrypt(IOBuf::copyBuffer("plaintext"), assocDataBuf.get());

  EXPECT_TRUE(cipherText.hasValue());

  auto plainText =
      cipher_->decrypt(std::move(*cipherText), otherAssocDataBuf.get());

  EXPECT_FALSE(plainText.hasValue());
}

} // namespace fizz::server::test
