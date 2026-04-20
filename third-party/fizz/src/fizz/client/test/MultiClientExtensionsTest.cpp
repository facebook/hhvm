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
  Status getClientHelloExtensions(std::vector<Extension>& ret, Error& /* err */)
      const override {
    ret.clear();
    Extension ext;
    ret.push_back(std::move(ext));
    return Status::Success;
  }

  Status onEncryptedExtensions(
      Error& /* err */,
      const std::vector<Extension>& /* unused */) override {
    return Status::Success;
  }
};

class OtherExampleClientExtension : public ClientExtensions {
 public:
  Status getClientHelloExtensions(std::vector<Extension>& ret, Error& /* err */)
      const override {
    ret.clear();
    Extension ext;
    ret.push_back(std::move(ext));
    return Status::Success;
  }

  Status onEncryptedExtensions(
      Error& /* err */,
      const std::vector<Extension>& /* unused */) override {
    return Status::Success;
  }
};

TEST_F(MultiClientExtensionsTest, TestNoExtensions) {
  MultiClientExtensions multi(std::vector<std::shared_ptr<ClientExtensions>>{});
  std::vector<Extension> exts;
  Error err;
  FIZZ_THROW_ON_ERROR(multi.getClientHelloExtensions(exts, err), err);
  EXPECT_TRUE(exts.empty());
}

TEST_F(MultiClientExtensionsTest, TestSingleExtension) {
  auto firstExtension = std::make_shared<ExampleClientExtension>();
  MultiClientExtensions multi({firstExtension});
  std::vector<Extension> exts;
  Error err;
  FIZZ_THROW_ON_ERROR(multi.getClientHelloExtensions(exts, err), err);
  EXPECT_EQ(exts.size(), 1);
}

TEST_F(MultiClientExtensionsTest, TestDuplicateExtensions) {
  auto firstExtension = std::make_shared<ExampleClientExtension>();
  auto secondExtension = std::make_shared<ExampleClientExtension>();
  MultiClientExtensions multi({firstExtension, secondExtension});
  std::vector<Extension> exts;
  Error err;
  FIZZ_THROW_ON_ERROR(multi.getClientHelloExtensions(exts, err), err);
  EXPECT_EQ(exts.size(), 2);
}

TEST_F(MultiClientExtensionsTest, TestDifferentTypeExtensions) {
  auto firstExtension = std::make_shared<ExampleClientExtension>();
  auto secondExtension = std::make_shared<OtherExampleClientExtension>();
  MultiClientExtensions multi({firstExtension, secondExtension});
  std::vector<Extension> exts;
  Error err;
  FIZZ_THROW_ON_ERROR(multi.getClientHelloExtensions(exts, err), err);
  EXPECT_EQ(exts.size(), 2);
}

} // namespace test
} // namespace client
} // namespace fizz
