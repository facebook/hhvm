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

#include <thrift/lib/cpp2/test/gen-cpp2/MyLeaf.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

#include <gtest/gtest.h>

using namespace std;
using namespace folly;
using namespace apache::thrift;
using namespace apache::thrift::test;

/**
 *  The processor needs to jump through some hoops to handle inheritance, so we
 *  really ought to have a test for it.
 */

class Handler : public apache::thrift::ServiceHandler<MyLeaf> {
 public:
  Future<string> future_doRoot() override { return makeFuture(string("root")); }
  Future<string> future_doNode() override { return makeFuture(string("node")); }
  Future<string> future_doLeaf() override { return makeFuture(string("leaf")); }
};

class ThriftServerInheritanceTest : public testing::Test {};

TEST_F(ThriftServerInheritanceTest, example) {
  auto handler = make_shared<Handler>();
  ScopedServerInterfaceThread runner(handler);

  EventBase eb;
  auto client = runner.newClient<MyLeafAsyncClient>(&eb);

  EXPECT_EQ("root", client->future_doRoot().waitVia(&eb).result().value());
  EXPECT_EQ("node", client->future_doNode().waitVia(&eb).result().value());
  EXPECT_EQ("leaf", client->future_doLeaf().waitVia(&eb).result().value());
}
