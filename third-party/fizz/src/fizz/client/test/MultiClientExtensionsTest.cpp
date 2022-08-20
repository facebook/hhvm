/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/client/MultiClientExtensions.h>

using namespace testing;

namespace fizz {
namespace client {
namespace test {

class MultiClientExtensionsTest : public Test {};

// Example classes to provide other types of ClientExtensions for testing.
class ExampleClientExtension : public ClientExtensions {
 public:
  std::vector<Extension> getClientHelloExtensions() const override {
    std::vector<Extension> exts;
    Extension ext;
    exts.push_back(std::move(ext));
    return exts;
  }

  void onEncryptedExtensions(
      const std::vector<Extension>& /* unused */) override {}
};

class OtherExampleClientExtension : public ClientExtensions {
 public:
  std::vector<Extension> getClientHelloExtensions() const override {
    std::vector<Extension> exts;
    Extension ext;
    exts.push_back(std::move(ext));
    return exts;
  }

  void onEncryptedExtensions(
      const std::vector<Extension>& /* unused */) override {}
};

TEST_F(MultiClientExtensionsTest, TestNoExtensions) {
  MultiClientExtensions multi(std::vector<std::shared_ptr<ClientExtensions>>{});
  auto exts = multi.getClientHelloExtensions();
  EXPECT_TRUE(exts.empty());
}

TEST_F(MultiClientExtensionsTest, TestSingleExtension) {
  auto firstExtension = std::make_shared<ExampleClientExtension>();
  MultiClientExtensions multi({firstExtension});
  auto exts = multi.getClientHelloExtensions();
  EXPECT_EQ(exts.size(), 1);
}

TEST_F(MultiClientExtensionsTest, TestDuplicateExtensions) {
  auto firstExtension = std::make_shared<ExampleClientExtension>();
  auto secondExtension = std::make_shared<ExampleClientExtension>();
  MultiClientExtensions multi({firstExtension, secondExtension});
  auto exts = multi.getClientHelloExtensions();
  EXPECT_EQ(exts.size(), 2);
}

TEST_F(MultiClientExtensionsTest, TestDifferentTypeExtensions) {
  auto firstExtension = std::make_shared<ExampleClientExtension>();
  auto secondExtension = std::make_shared<OtherExampleClientExtension>();
  MultiClientExtensions multi({firstExtension, secondExtension});
  auto exts = multi.getClientHelloExtensions();
  EXPECT_EQ(exts.size(), 2);
}

} // namespace test
} // namespace client
} // namespace fizz
