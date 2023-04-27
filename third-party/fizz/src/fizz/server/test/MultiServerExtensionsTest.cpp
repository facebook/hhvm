/*
 *  Copyright (c) 2018-present, Facebook, Inc.
 *  All rights reserved.
 *
 *  This source code is licensed under the BSD-style license found in the
 *  LICENSE file in the root directory of this source tree.
 */

#include <folly/portability/GMock.h>
#include <folly/portability/GTest.h>

#include <fizz/server/MultiServerExtensions.h>

using namespace testing;

namespace fizz {
namespace server {
namespace test {

// Example classes to provide other types of ServerExtensions for testing.
class ExampleServerExtension : public ServerExtensions {
 public:
  std::vector<Extension> getExtensions(
      const ClientHello& /* unused */) override {
    std::vector<Extension> exts;
    Extension ext;
    exts.push_back(std::move(ext));
    return exts;
  }
};

class OtherExampleServerExtension : public ServerExtensions {
 public:
  std::vector<Extension> getExtensions(
      const ClientHello& /* unused */) override {
    std::vector<Extension> exts;
    Extension ext;
    exts.push_back(std::move(ext));
    return exts;
  }
};

class MultiServerExtensionTest : public Test {
 public:
  ClientHello chlo_;
};

TEST_F(MultiServerExtensionTest, TestNoExtensions) {
  MultiServerExtensions multi(std::vector<std::shared_ptr<ServerExtensions>>{});
  auto exts = multi.getExtensions(chlo_);
  EXPECT_EQ(exts.size(), 0);
}

TEST_F(MultiServerExtensionTest, TestSingleExtension) {
  auto firstExtension = std::make_shared<ExampleServerExtension>();
  MultiServerExtensions multi({firstExtension});
  auto exts = multi.getExtensions(chlo_);
  EXPECT_EQ(exts.size(), 1);
}

TEST_F(MultiServerExtensionTest, TestDuplicateExtensions) {
  auto firstExtension = std::make_shared<ExampleServerExtension>();
  auto secondExtension = std::make_shared<ExampleServerExtension>();
  MultiServerExtensions multi({firstExtension, secondExtension});
  auto exts = multi.getExtensions(chlo_);
  EXPECT_EQ(exts.size(), 2);
}

TEST_F(MultiServerExtensionTest, TestDifferentTypeExtensions) {
  auto firstExtension = std::make_shared<ExampleServerExtension>();
  auto secondExtension = std::make_shared<OtherExampleServerExtension>();
  MultiServerExtensions multi({firstExtension, secondExtension});
  auto exts = multi.getExtensions(chlo_);
  EXPECT_EQ(exts.size(), 2);
}

} // namespace test
} // namespace server
} // namespace fizz
