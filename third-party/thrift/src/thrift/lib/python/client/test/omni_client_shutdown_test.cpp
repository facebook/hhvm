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

#include <memory>

#include <gtest/gtest.h>

#include <folly/Singleton.h>
#include <folly/executors/GlobalExecutor.h>
#include <folly/executors/IOThreadPoolExecutor.h>
#include <folly/io/async/AsyncSocket.h>
#include <folly/io/async/EventBase.h>

#include <thrift/lib/cpp2/async/RocketClientChannel.h>
#include <thrift/lib/python/client/OmniClient.h> // @manual=//thrift/lib/python/client:omni_client__cython-lib

using apache::thrift::RocketClientChannel;
using apache::thrift::python::client::OmniClient;
using apache::thrift::python::client::RequestChannelUnique;

// Regression tests for a use-after-free in ~OmniClient(): a sync client's
// channel is bound to an IO executor's EventBase, which can be destroyed before
// the client at shutdown. ~OmniClient's death probe detects that and leaks the
// channel instead of dereferencing the freed EventBase.

// Here the EventBase is freed by its own IOThreadPoolExecutor's worker-thread
// teardown while the global IO executor stays alive -- so pinning the global IO
// executor cannot detect the freed EventBase; only the death probe can.
// Hermetic (no process-wide teardown), so it is registered first, before the
// irreversible destroyInstances() case below.
TEST(OmniClientShutdownTest, EventBaseDestroyedByItsExecutorBeforeClient) {
  std::unique_ptr<OmniClient> client;
  {
    auto io = std::make_unique<folly::IOThreadPoolExecutor>(1);
    folly::EventBase* eb = io->getEventBase();

    // A DelayedDestruction channel must be built on its EventBase thread.
    RocketClientChannel::Ptr channel;
    eb->runInEventBaseThreadAndWait([&] {
      channel = RocketClientChannel::newChannel(
          folly::AsyncSocket::UniquePtr(new folly::AsyncSocket(eb)));
    });
    client =
        std::make_unique<OmniClient>(RequestChannelUnique(std::move(channel)));

    // Destroying `io` joins its worker thread and frees `eb`; `client` is left
    // holding a channel bound to a dead `eb`.
  }

  EXPECT_NO_FATAL_FAILURE(client.reset());
}

// The global IO executor's EventBase is freed before the client, matching the
// real shutdown path where the executor singleton is torn down during
// Py_FinalizeEx. destroyInstances() is process-global and irreversible, so this
// must be the last test to run in the binary.
TEST(OmniClientShutdownTest, GlobalIOExecutorDestroyedBeforeClient) {
  // Bind a channel to the global IO executor's EventBase, like the sync client
  // does. A DelayedDestruction channel must be built on its EventBase thread.
  folly::EventBase* eb = folly::getGlobalIOExecutor()->getEventBase();
  RocketClientChannel::Ptr channel;
  eb->runInEventBaseThreadAndWait([&] {
    channel = RocketClientChannel::newChannel(
        folly::AsyncSocket::UniquePtr(new folly::AsyncSocket(eb)));
  });

  auto client =
      std::make_unique<OmniClient>(RequestChannelUnique(std::move(channel)));

  // Free the global IO executor (and its EventBases) before the client.
  folly::SingletonVault::singleton()->destroyInstances();

  EXPECT_NO_FATAL_FAILURE(client.reset());
}
