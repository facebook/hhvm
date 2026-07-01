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

// Unit test for TLSConnectionAdapter, the tail endpoint of the TLS pipeline.
// Verifies its two dispatch contracts to the owner that drives the pipeline:
// a resolved TLSResponseMessage arriving on the read path is handed to the
// owner's resolved trampoline, and an exception is handed to the owner's
// exception trampoline rather than swallowed, so the owner can propagate it
// onto the outer pipeline.

#include <stdexcept>

#include <gtest/gtest.h>

#include <folly/ExceptionWrapper.h>
#include <folly/SocketAddress.h>
#include <folly/io/async/AsyncTransport.h>

#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/Common.h>
#include <thrift/lib/cpp2/fast_thrift/channel_pipeline/TypeErasedBox.h>
#include <thrift/lib/cpp2/fast_thrift/connection/security/common/Messages.h>
#include <thrift/lib/cpp2/fast_thrift/connection/security/handler/TLSConnectionAdapter.h>

namespace apache::thrift::fast_thrift::connection::security::handler::test {

namespace {

// Stand-in owner: records what the adapter dispatches through its trampolines.
struct RecordingOwner {
  bool resolvedCalled{false};
  folly::SocketAddress resolvedAddr;
  bool exceptionCalled{false};
  folly::exception_wrapper caught;

  static channel_pipeline::Result onResolved(
      void* self,
      folly::AsyncTransport::UniquePtr /*transport*/,
      folly::SocketAddress clientAddr) noexcept {
    auto* owner = static_cast<RecordingOwner*>(self);
    owner->resolvedCalled = true;
    owner->resolvedAddr = std::move(clientAddr);
    return channel_pipeline::Result::Success;
  }

  static void onException(void* self, folly::exception_wrapper&& e) noexcept {
    auto* owner = static_cast<RecordingOwner*>(self);
    owner->exceptionCalled = true;
    owner->caught = std::move(e);
  }
};

} // namespace

// A resolved response arriving on the read path is dispatched to the owner's
// resolved trampoline with the peer address intact.
TEST(TLSConnectionAdapterTest, DispatchesResolvedToOwner) {
  RecordingOwner owner;
  TLSConnectionAdapter adapter;
  adapter.setOwner(
      &owner, &RecordingOwner::onResolved, &RecordingOwner::onException);

  const folly::SocketAddress addr{"127.0.0.1", 4321};
  TLSResponseMessage resp{.transport = nullptr, .clientAddr = addr};
  const auto result =
      adapter.onRead(channel_pipeline::erase_and_box(std::move(resp)));

  EXPECT_EQ(result, channel_pipeline::Result::Success);
  EXPECT_TRUE(owner.resolvedCalled);
  EXPECT_FALSE(owner.exceptionCalled);
  EXPECT_EQ(owner.resolvedAddr, addr);
}

// An exception is dispatched to the owner's exception trampoline rather than
// swallowed, so the owner can re-fire it onto the outer pipeline. Guards the
// regression where the adapter only logged and dropped the exception.
TEST(TLSConnectionAdapterTest, DispatchesExceptionToOwner) {
  RecordingOwner owner;
  TLSConnectionAdapter adapter;
  adapter.setOwner(
      &owner, &RecordingOwner::onResolved, &RecordingOwner::onException);

  adapter.onException(
      folly::make_exception_wrapper<std::runtime_error>("tls boom"));

  EXPECT_TRUE(owner.exceptionCalled);
  EXPECT_FALSE(owner.resolvedCalled);
  ASSERT_NE(owner.caught.get_exception<std::runtime_error>(), nullptr);
}

} // namespace apache::thrift::fast_thrift::connection::security::handler::test

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
