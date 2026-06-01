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

#include <gtest/gtest.h>
#include <thrift/lib/cpp2/fast_thrift/test/IntegrationTestFixture.h>

namespace apache::thrift::fast_thrift::test {

using BasicPipelineIntegrationTest = IntegrationTestFixture;

TEST_F(BasicPipelineIntegrationTest, SendMessageToServer) {
  serverAppAdapter().setEchoEnabled(false);

  auto& client = connectClient();
  auto payload = folly::IOBuf::copyBuffer("Hello, Server!");
  (void)client.appAdapter.send(std::move(payload));

  ASSERT_TRUE(serverAppAdapter().waitForMessage());
  EXPECT_EQ(serverAppAdapter().messageCount(), 1);

  auto& messages = serverAppAdapter().messages();
  ASSERT_EQ(messages.size(), 1);

  auto& bytes =
      messages[0]
          .get<apache::thrift::fast_thrift::channel_pipeline::BytesPtr>();
  auto data = bytes->clone()->moveToFbString();
  EXPECT_EQ(data, "Hello, Server!");
}

TEST_F(BasicPipelineIntegrationTest, EchoResponse) {
  serverAppAdapter().setEchoEnabled(true);

  auto& client = connectClient();
  auto payload = folly::IOBuf::copyBuffer("Echo me!");
  (void)client.appAdapter.send(std::move(payload));

  ASSERT_TRUE(serverAppAdapter().waitForMessage());
  ASSERT_TRUE(client.appAdapter.waitForResponse());

  EXPECT_EQ(client.appAdapter.responseCount(), 1);

  auto& responses = client.appAdapter.responses();
  ASSERT_EQ(responses.size(), 1);

  auto& bytes =
      responses[0]
          .get<apache::thrift::fast_thrift::channel_pipeline::BytesPtr>();
  auto data = bytes->clone()->moveToFbString();
  EXPECT_EQ(data, "Echo me!");
}

TEST_F(BasicPipelineIntegrationTest, MultipleMessages) {
  serverAppAdapter().setEchoEnabled(false);

  auto& client = connectClient();

  auto payload1 = folly::IOBuf::copyBuffer("Message 1");
  (void)client.appAdapter.send(std::move(payload1));
  ASSERT_TRUE(serverAppAdapter().waitForMessage());
  EXPECT_EQ(serverAppAdapter().messageCount(), 1);

  serverAppAdapter().resetMessageBaton();
  auto payload2 = folly::IOBuf::copyBuffer("Message 2");
  (void)client.appAdapter.send(std::move(payload2));
  ASSERT_TRUE(serverAppAdapter().waitForMessage());
  EXPECT_EQ(serverAppAdapter().messageCount(), 2);

  serverAppAdapter().resetMessageBaton();
  auto payload3 = folly::IOBuf::copyBuffer("Message 3");
  (void)client.appAdapter.send(std::move(payload3));
  ASSERT_TRUE(serverAppAdapter().waitForMessage());
  EXPECT_EQ(serverAppAdapter().messageCount(), 3);
}

TEST_F(BasicPipelineIntegrationTest, MultipleEchoRoundTrips) {
  serverAppAdapter().setEchoEnabled(true);

  auto& client = connectClient();

  for (int i = 0; i < 5; ++i) {
    serverAppAdapter().resetMessageBaton();
    client.appAdapter.resetResponseBaton();

    auto msg = "Message " + std::to_string(i);
    auto payload = folly::IOBuf::copyBuffer(msg);
    (void)client.appAdapter.send(std::move(payload));

    ASSERT_TRUE(serverAppAdapter().waitForMessage());
    ASSERT_TRUE(client.appAdapter.waitForResponse());
  }

  EXPECT_EQ(serverAppAdapter().messageCount(), 5);
  EXPECT_EQ(client.appAdapter.responseCount(), 5);
}

TEST_F(BasicPipelineIntegrationTest, LargerPayload) {
  serverAppAdapter().setEchoEnabled(true);

  auto& client = connectClient();

  // Use a message size that fits in a single TCP segment
  std::string largeData(1024, 'X');
  auto payload = folly::IOBuf::copyBuffer(largeData);
  (void)client.appAdapter.send(std::move(payload));

  ASSERT_TRUE(serverAppAdapter().waitForMessage());
  ASSERT_TRUE(client.appAdapter.waitForResponse());

  auto& responses = client.appAdapter.responses();
  ASSERT_EQ(responses.size(), 1);

  auto& bytes =
      responses[0]
          .get<apache::thrift::fast_thrift::channel_pipeline::BytesPtr>();
  auto data = bytes->clone()->moveToFbString();
  EXPECT_EQ(data.size(), largeData.size());
  EXPECT_EQ(data, largeData);
}

} // namespace apache::thrift::fast_thrift::test
