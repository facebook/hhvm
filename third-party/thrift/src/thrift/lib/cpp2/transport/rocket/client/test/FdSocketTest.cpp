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
#include <folly/testing/TestUtil.h>

#include <thrift/lib/cpp2/test/gen-cpp2/TestServiceAsyncClient.h>
#include <thrift/lib/cpp2/test/util/TestHandler.h>
#include <thrift/lib/cpp2/transport/rocket/test/util/FdUtils.h>
#include <thrift/lib/cpp2/util/ScopedServerInterfaceThread.h>

using namespace apache::thrift;
using namespace apache::thrift::rocket::test;
using namespace std::literals;

struct EchoInterface : public TestHandler {
  folly::coro::Task<std::unique_ptr<std::string>> co_echoRequest(
      apache::thrift::RequestParams params,
      std::unique_ptr<std::string> s) override {
    auto& fds = params.getRequestContext()->getHeader()->fds;
    LOG(INFO) << "Server echoing data: " << renderData(*s) << ". Got FDs: "
              << (fds.empty() ? 0 : fds.releaseReceived().size());
    // It's tempting to also test that the received FDs have the same file
    // description object in the kernel, but this sort of thing is
    // separately covered in end-to-end / round-trip tests.  The current
    // test focused on client transport behavior: batching and fragments.
    co_return std::move(s);
  }
};

struct FdClientBatchingTest : public testing::Test {
  folly::test::TemporaryDirectory tempDir_;

  MessageQueue checkQueue_{1000}; // never fills up

  folly::EventBase evb_;

  std::unique_ptr<ScopedServerInterfaceThread> runner_;
  std::unique_ptr<Client<test::TestService>> client_;

  void init() {
    folly::SocketAddress sockAddr;
    sockAddr.setFromPath((tempDir_.path() / "sock").string());

    runner_ = std::make_unique<ScopedServerInterfaceThread>(
        std::make_shared<EchoInterface>(), sockAddr);
    client_ = std::make_unique<Client<test::TestService>>(
        RocketClientChannel::newChannel(
            folly::AsyncSocket::UniquePtr{
                new InterceptedAsyncFdSocket(&checkQueue_, &evb_, sockAddr)}));
  }

  auto newFd() {
    return std::make_shared<folly::File>(
        folly::File{2, /*ownsFd*/ false}.dupCloseOnExec());
  }

  folly::SemiFuture<int> testEchoRequest(
      std::string msg, folly::SocketFds::ToSend fds, std::string msgRe) {
    // In `BatchWithoutFDs`, e.g., we have fewer "check" events than
    // requests due to batching.
    if (!msgRe.empty()) {
      checkQueue_.blockingWrite(
          std::make_pair(std::make_unique<std::string>(msgRe), fds));
    }

    apache::thrift::RpcOptions rpcOptions;
    rpcOptions.setFdsToSend(fds);
    return client_->semifuture_echoRequest(rpcOptions, msg)
        .via(&evb_)
        .thenValue([msg](auto&& res) {
          EXPECT_EQ(res, msg);
          return 1;
        });
  }

  using Requests = std::vector<
      std::tuple<std::string, folly::SocketFds::ToSend, std::string>>;

  void sendAndVerifyRequestBatch(
      const Requests& requests, size_t numSocketCalls = 0) {
    std::vector<folly::SemiFuture<int>> futures;
    for (const auto& [data, fds, dataRe] : requests) {
      futures.emplace_back(testEchoRequest(data, fds, dataRe));
    }

    // Requests are enqueued on EventBase, to be processed as a batch.
    EXPECT_EQ(0, checkQueue_.readCount());
    for (const auto& [_data, fds, _dataRe] : requests) {
      expectUseCount(3, fds); // `RpcOptions` + `requests` + `checkQueue_`
    }

    EXPECT_EQ(
        requests.size(),
        folly::collectAllUnsafe(std::move(futures))
            .thenValue([](auto&& vs) {
              int successfulRequests = 0;
              for (auto v : vs) {
                successfulRequests += *v;
              }
              return successfulRequests;
            })
            .getVia(&evb_));
    EXPECT_EQ(
        numSocketCalls ? numSocketCalls : requests.size(),
        checkQueue_.readCount());
    for (const auto& [_data, fds, _dataRe] : requests) {
      expectUseCount(1, fds);
    }
  }
};

// Unbatched processing of a single short request.
TEST_F(FdClientBatchingTest, SingleRequest) {
  init();
  Requests requests{
      {"ShortMessage", folly::SocketFds::ToSend{newFd()}, "ShortMessage"}};
  sendAndVerifyRequestBatch(requests);
}

// Cover some core "client sends FDs" functionality:
//  - The right data matches with the right FD
//  - Unbatching: each set of FDs gets a separate socket call.
//  - Fragment frame handling: if a request is split across multiple frames,
//    the socket gets the FDs together with the last frame -- actually, in
//    the current implementation, it gets all the frames at once.
TEST_F(FdClientBatchingTest, SimpleBatch) {
  CHECK_GE(apache::thrift::rocket::kMaxFragmentedPayloadSize, 10000);
  init();
  Requests requests{
      {"ShortMsg", folly::SocketFds::ToSend{newFd(), newFd()}, "ShortMsg"},
      {// Make this request large enough that it HAS to be split into two
       // fragments.
       "StartLong" +
           std::string(apache::thrift::rocket::kMaxFragmentedPayloadSize, '%') +
           "EndLong",
       {newFd(), newFd(), newFd()},
       // The regex needs to match some protocol bytes between fragments.
       folly::to<std::string>(
           "StartLong"
           // Almost all of the payload (% chars) should be in the first frame.
           // Add `+` to make the quantifier greedy for faster matching.
           "%{",
           apache::thrift::rocket::kMaxFragmentedPayloadSize - 1000,
           ",}+"
           // Match some non-% protocol bytes between fragments
           ".{1,100}[^%].{1,100}"
           // Second frame with the tail end of the message
           "%{1,1000}+EndLong")}};
  sendAndVerifyRequestBatch(requests);
}

// Covers two behaviors:
//  - Requests without FDs are batched into the next request that does
//    have FDs.
//  - We get data-only batching when the last request in a batch does not
//    carry FDs.
TEST_F(FdClientBatchingTest, BatchWithoutFds) {
  init();
  Requests requests{
      {"Batch1M1", folly::SocketFds::ToSend{}, ""},
      {"Batch1M2", folly::SocketFds::ToSend{}, ""},
      {"Batch1M3",
       folly::SocketFds::ToSend{newFd()},
       "Batch1M1.+Batch1M2.+Batch1M3"},
      {"Batch2", folly::SocketFds::ToSend{newFd()}, "Batch2"},
      {"Batch3M1", folly::SocketFds::ToSend{}, ""},
      {"Batch3M2", folly::SocketFds::ToSend{newFd()}, "Batch3M1.+Batch3M2"},
      // Cover "final batch has no FDs" case, SimpleBatch covers the other.
      {"Batch4M1", folly::SocketFds::ToSend{}, ""},
      {"Batch4M2", folly::SocketFds::ToSend{}, "Batch4M1.+Batch4M2"},
  };
  sendAndVerifyRequestBatch(requests, 4);
}
