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

#pragma once

#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include <thrift/lib/cpp/transport/THeader.h>
#include <thrift/lib/cpp2/async/AsyncProcessor.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>

namespace apache::thrift::test {

/*
 * Contains various utility functions helpful when testing
 * RoundRobinRequestPile. These functions allow you to construct a ServerRequest
 * and control which priority / bucket it ends up being assigned to. On dequeue
 * you can then see what bucket that request is coming from. This is necessary
 * to test round robin dequeue logic.
 *
 * RequestPileTestState::makeServerRequestForBucket() constructs ServerRequest
 * and sets headers extra header PRIORITY and BUCKET.
 *
 * RequestPileTestUtils::makePileSelectionFunction() constructs a
 * PileSelectionFunction that assigns a request to a given priority and bucket
 * based on values extracted from the headers.
 *
 * RequestPileTestUtils::extractRequestBucketFromRequestData() allows to inspect
 * a request's ServerRequestData to inspect which bucket it was dequeued from.
 */
class RequestPileTestUtils {
 public:
  using Priority = uint32_t;
  using Bucket = uint32_t;
  using PileSelectionFunction =
      std::function<std::pair<Priority, Bucket>(const ServerRequest&)>;

  static std::pair<Priority, Bucket> extractRequestBucketFromHeaders(
      const ServerRequest& request) {
    auto ctx = request.requestContext();
    auto headers = ctx->getHeaders();

    auto priority = folly::to<uint32_t>(headers.find("PRIORITY")->second);
    auto bucket = folly::to<uint32_t>(headers.find("BUCKET")->second);

    return {priority, bucket};
  }

  static std::pair<Priority, Bucket> extractRequestBucketFromRequestData(
      const ServerRequest& request) {
    return request.requestData().bucket.value();
  }

  static PileSelectionFunction makePileSelectionFunction() {
    return [](const ServerRequest& request) {
      return extractRequestBucketFromHeaders(request);
    };
  }

  static void expectRequestToBelongToBucket(
      const ServerRequest& request, Priority priority, Bucket bucket) {
    auto [thePriority, theBucket] =
        extractRequestBucketFromRequestData(request);
    EXPECT_EQ(thePriority, priority);
    EXPECT_EQ(theBucket, bucket);
  }
};

/*
 * Allocate one instance of RequestPileTestState per test case. This allows you
 * to construct ServerRequests and control un-managed resources. such as THeader
 * and Cpp2RequestContext.
 */
class RequestPileTestState {
 public:
  using Priority = uint32_t;
  using Bucket = uint32_t;
  using PileSelectionFunction =
      std::function<std::pair<Priority, Bucket>(const ServerRequest&)>;

  ServerRequest makeServerRequestForBucket(Priority priority, Bucket bucket) {
    std::unique_lock lock{mutex_};
    tHeaderStorage.emplace_back(new transport::THeader);
    auto* header = tHeaderStorage[tHeaderStorage.size() - 1].get();

    contextStorage.emplace_back(new Cpp2RequestContext(nullptr, header));
    auto* ctx = contextStorage[contextStorage.size() - 1].get();

    header->setReadHeaders(
        {{"PRIORITY", folly::to<std::string>(priority)},
         {"BUCKET", folly::to<std::string>(bucket)}});

    return ServerRequest(
        nullptr /* ResponseChannelRequest::UniquePtr  */,
        SerializedCompressedRequest(std::unique_ptr<folly::IOBuf>{}),
        ctx,
        static_cast<protocol::PROTOCOL_TYPES>(0),
        nullptr, /* requestContext  */
        nullptr, /* asyncProcessor  */
        nullptr /* methodMetadata  */);
  }

 private:
  std::mutex mutex_;
  std::vector<std::unique_ptr<transport::THeader>> tHeaderStorage;
  std::vector<std::unique_ptr<Cpp2RequestContext>> contextStorage;
};

} // namespace apache::thrift::test
