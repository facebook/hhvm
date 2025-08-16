/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
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

#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <thrift/lib/cpp2/server/DecoratorDataRuntime.h>

namespace apache::thrift::server {

using namespace testing;

namespace {

class DecoratorDataRuntimeTest : public ::testing::Test {
 public:
  DecoratorDataRuntimeTest()
      : state_{DecoratorDataPerRequestBlueprint::Setup()} {}

  DecoratorDataHandleFactory handleFactory() {
    return std::get<DecoratorDataPerRequestBlueprint::Setup>(state_)
        .getHandleFactory();
  }

  void finishSetup() {
    state_ =
        std::move(std::get<DecoratorDataPerRequestBlueprint::Setup>(state_))
            .finalize();
    util::AllocationColocator<void> alloc;
    auto& decoratorDataRuntime =
        std::get<DecoratorDataPerRequestBlueprint>(state_);
    storagePtr_ = alloc.allocate(
        [this,
         &decoratorDataRuntime,
         locator = decoratorDataRuntime.planStorage(alloc)](auto make) mutable {
          storage_ = decoratorDataRuntime.initStorageForRequest(
              make, std::move(locator));
        });
  }

  DecoratorData getDecoratorData() {
    return DecoratorData::fromStorage(storage_);
  }

 private:
  std::variant<
      DecoratorDataPerRequestBlueprint::Setup,
      DecoratorDataPerRequestBlueprint>
      state_;
  util::AllocationColocator<void>::Ptr storagePtr_;
  DecoratorDataStorage storage_;
};

} // namespace

TEST_F(DecoratorDataRuntimeTest, Basic) {
  DecoratorDataKey<int64_t> i64DataKey{};
  DecoratorDataHandle<int64_t> handle =
      DecoratorDataHandle<int64_t>::uninitialized();

  {
    DecoratorDataHandleFactory factory = handleFactory();
    handle = factory.makeHandleForKey(i64DataKey);
    finishSetup();
  }
  {
    DecoratorData writeableData = getDecoratorData();
    writeableData.put(handle, 42);
    const DecoratorData readOnlyData = getDecoratorData();
    auto* observed = readOnlyData.get(handle);
    EXPECT_NE(observed, nullptr);
    EXPECT_EQ(*observed, 42);
  }
}

TEST_F(DecoratorDataRuntimeTest, ThrowsIfHandleUninitialized) {
  DecoratorDataHandle<int64_t> handle =
      DecoratorDataHandle<int64_t>::uninitialized();
  finishSetup();

  {
    DecoratorData writeableData = getDecoratorData();
    EXPECT_THROW(writeableData.put(handle, 42), std::logic_error);
    const DecoratorData readOnlyData = getDecoratorData();
    EXPECT_THROW(readOnlyData.get(handle), std::logic_error);
  }
}

TEST_F(DecoratorDataRuntimeTest, HandleAlias) {
  DecoratorDataKey<int64_t> i64DataKey{};
  DecoratorDataHandle<int64_t> handle1 =
      DecoratorDataHandle<int64_t>::uninitialized();
  DecoratorDataHandle<int64_t> handle2 =
      DecoratorDataHandle<int64_t>::uninitialized();

  {
    DecoratorDataHandleFactory factory = handleFactory();
    handle1 = factory.makeHandleForKey(i64DataKey);
    handle2 = factory.makeHandleForKey(i64DataKey);
    finishSetup();
  }
  {
    DecoratorData writeableData = getDecoratorData();
    writeableData.put(handle1, 42);
    const DecoratorData readOnlyData = getDecoratorData();
    auto* observed = readOnlyData.get(handle2);
    EXPECT_NE(observed, nullptr);
    EXPECT_EQ(*observed, 42);
  }
}

TEST_F(DecoratorDataRuntimeTest, ComplexTypes) {
  DecoratorDataKey<std::vector<std::string>> strVecKey{};
  DecoratorDataKey<std::string> strKey{};
  DecoratorDataHandle<std::vector<std::string>> strVecHandle =
      DecoratorDataHandle<std::vector<std::string>>::uninitialized();
  DecoratorDataHandle<std::string> strHandle =
      DecoratorDataHandle<std::string>::uninitialized();

  {
    DecoratorDataHandleFactory factory = handleFactory();
    strVecHandle = factory.makeHandleForKey(strVecKey);
    strHandle = factory.makeHandleForKey(strKey);
    finishSetup();
  }
  {
    DecoratorData writeableData = getDecoratorData();
    std::vector<std::string> vec = {"hello", "world"};
    writeableData.put(strVecHandle, std::move(vec));
    writeableData.put(strHandle, "foo");

    const DecoratorData readOnlyData = getDecoratorData();
    auto* observedStrVec = readOnlyData.get(strVecHandle);
    EXPECT_NE(observedStrVec, nullptr);
    EXPECT_THAT(*observedStrVec, ElementsAre("hello", "world"));
    auto* observedStr = readOnlyData.get(strHandle);
    EXPECT_NE(observedStr, nullptr);
    EXPECT_EQ(*observedStr, "foo");
  }
}

} // namespace apache::thrift::server
