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
  Status getExtensions(
      std::vector<Extension>& ret,
      Error& /* err */,
      const ClientHello& /* unused */) override {
    Extension ext;
    ret.push_back(std::move(ext));
    return Status::Success;
  }
};

class OtherExampleServerExtension : public ServerExtensions {
 public:
  Status getExtensions(
      std::vector<Extension>& ret,
      Error& /* err */,
      const ClientHello& /* unused */) override {
    Extension ext;
    ret.push_back(std::move(ext));
    return Status::Success;
  }
};

class MultiServerExtensionTest : public Test {
 public:
  ClientHello chlo_;
};

TEST_F(MultiServerExtensionTest, TestNoExtensions) {
  MultiServerExtensions multi(std::vector<std::shared_ptr<ServerExtensions>>{});
  std::vector<Extension> exts;
  Error err;
  EXPECT_EQ(multi.getExtensions(exts, err, chlo_), Status::Success);
  EXPECT_EQ(exts.size(), 0);
}

TEST_F(MultiServerExtensionTest, TestSingleExtension) {
  auto firstExtension = std::make_shared<ExampleServerExtension>();
  MultiServerExtensions multi({firstExtension});
  std::vector<Extension> exts;
  Error err;
  EXPECT_EQ(multi.getExtensions(exts, err, chlo_), Status::Success);
  EXPECT_EQ(exts.size(), 1);
}

TEST_F(MultiServerExtensionTest, TestDuplicateExtensions) {
  auto firstExtension = std::make_shared<ExampleServerExtension>();
  auto secondExtension = std::make_shared<ExampleServerExtension>();
  MultiServerExtensions multi({firstExtension, secondExtension});
  std::vector<Extension> exts;
  Error err;
  EXPECT_EQ(multi.getExtensions(exts, err, chlo_), Status::Success);
  EXPECT_EQ(exts.size(), 2);
}

TEST_F(MultiServerExtensionTest, TestDifferentTypeExtensions) {
  auto firstExtension = std::make_shared<ExampleServerExtension>();
  auto secondExtension = std::make_shared<OtherExampleServerExtension>();
  MultiServerExtensions multi({firstExtension, secondExtension});
  std::vector<Extension> exts;
  Error err;
  EXPECT_EQ(multi.getExtensions(exts, err, chlo_), Status::Success);
  EXPECT_EQ(exts.size(), 2);
}

} // namespace test
} // namespace server
} // namespace fizz
