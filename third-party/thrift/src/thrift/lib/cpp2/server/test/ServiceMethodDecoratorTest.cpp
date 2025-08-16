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
#include <thrift/lib/cpp2/server/test/gen-cpp2/ServiceMethodDecorator_handlers.h>

using namespace ::testing;

namespace apache::thrift::test {

namespace {

class MockMethodDecorator
    : public ServiceMethodDecorator<ServiceMethodDecoratorTest> {
 public:
  MOCK_METHOD(std::string_view, getName, (), (const, override));
};

class Handler : public ServiceHandler<ServiceMethodDecoratorTest> {
 public:
  folly::coro::Task<void> co_noop() override { co_return; }
};

} // namespace

TEST(ServiceMethodDecoratorTest, MethodDecoratorsCompile) {
  MockMethodDecorator decorator;
  EXPECT_CALL(decorator, getName()).WillOnce(Return("MockMethodDecorator"));
  EXPECT_EQ("MockMethodDecorator", decorator.getName());

  ServiceMethodDecoratorList<ServiceMethodDecoratorTest> decorators;
  decorators.push_back(std::make_shared<MockMethodDecorator>());

  auto handler = std::make_shared<Handler>();
  apache::thrift::decorate(*handler, std::move(decorators));
}

// TODO
} // namespace apache::thrift::test
