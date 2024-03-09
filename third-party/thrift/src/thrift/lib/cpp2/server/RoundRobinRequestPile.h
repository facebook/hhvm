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

#include <atomic>
#include <cstdint>
#include <limits>
#include <memory>

#include <folly/concurrency/UnboundedQueue.h>
#include <folly/io/async/AtomicNotificationQueue.h>
#include <thrift/lib/cpp2/async/AsyncProcessor.h>
#include <thrift/lib/cpp2/server/RequestPileBase.h>
#include <thrift/lib/cpp2/server/WeightedRequestPileQueue.h>

namespace apache::thrift {

class RoundRobinRequestPile : public RequestPileBase {
 public:
  using Priority = uint32_t;
  using Bucket = uint32_t;
  // Function to specify priority and bucket for a request. The priority must be
  // <= the number of priorities (lower numbers have higher priority). The
  // bucket must be <= the number of buckets available at the specified
  // priority.
  using PileSelectionFunction =
      std::function<std::pair<Priority, Bucket>(const ServerRequest&)>;

  struct Options {
    inline constexpr static unsigned int kDefaultNumPriorities = 1;
    inline constexpr static unsigned int kDefaultNumBuckets = 1;

    std::string name;

    // numBucket = numBuckets_[priority]
    std::vector<unsigned int> numBucketsPerPriority;

    // 0 means no limit
    // this is a per bucket limit
    uint64_t numMaxRequests{0};

    // Function to route requests to priority/bucket
    PileSelectionFunction pileSelectionFunction;

    Options() {
      numBucketsPerPriority.reserve(kDefaultNumPriorities);
      for (unsigned i = 0; i < kDefaultNumPriorities; ++i) {
        numBucketsPerPriority.emplace_back(kDefaultNumBuckets);
      }
    }

    void setName(std::string rName) { name = std::move(rName); }

    // Set the number of priority levels to provide. By default the number of
    // buckets per priority will be set to 1.
    void setNumPriorities(unsigned int numPri) {
      numBucketsPerPriority.clear();
      numBucketsPerPriority.resize(numPri, kDefaultNumBuckets);
    }

    void setNumBucketsPerPriority(Priority priority, unsigned int numBucket) {
      numBucketsPerPriority.at(priority) = numBucket;
    }

    // This function sets the shape of the RequestPile
    // e.g. This is setting the number of buckets per priority
    // If your RequestPile has 3 priorities and each has 2 buckets
    // you should pass in {2,2,2}
    void setShape(std::vector<unsigned int> shape) {
      numBucketsPerPriority = std::move(shape);
    }

    void setPileSelectionFunction(PileSelectionFunction func) {
      pileSelectionFunction = std::move(func);
    }

    // emulating PriorityQueueThreadManager
    PileSelectionFunction getDefaultPileSelectionFunc(
        unsigned defaultPriority = static_cast<unsigned>(concurrency::NORMAL));

    std::string describe() const {
      auto numBucketsPerPriorityString = [this]() -> std::string {
        std::string result = "{";
        for (std::size_t i = 0; i < numBucketsPerPriority.size(); i++) {
          result += std::to_string(numBucketsPerPriority[i]);
        }
        result += "}";
        return result;
      };
      return fmt::format(
          "{{Options name={} numBucketsPerPriority={} numMaxRequests={}}}",
          name,
          numBucketsPerPriorityString(),
          numMaxRequests);
    }
  };

  // The default number of buckets for each priority is 1
  explicit RoundRobinRequestPile(Options opts);

  ~RoundRobinRequestPile() override = default;

  std::optional<ServerRequestRejection> enqueue(
      ServerRequest&& request) override;

  std::optional<ServerRequest> dequeue() override;

  uint64_t requestCount() const override;

  void onRequestFinished(ServerRequestData&) override {}

  std::string describe() const override;

  serverdbginfo::RequestPileDbgInfo getDbgInfo() const override;

 private:
  // we choose 1KB as segment size to minimize allocations
  using RetrievalIndexQueue = folly::UMPMCQueue<
      unsigned,
      /* MayBlock  */ false,
      /* log2(SegmentSize=1024) */ 10>;

  // The reason why we chose AtomicNotificationQueue instead of two UMPMCQueue
  // is that UMPMCQueue does not provide an enqueue() interface that indicates
  // which one is the first enqueue to the empty queue
  using RequestQueue = folly::AtomicNotificationQueue<ServerRequest>;

  // This is a temporary solution to single bucket case
  // because ReqeustQueue is a MPSC queue
  using SingleBucketRequestQueue =
      server::WeightedRequestPileQueue<ServerRequest>;

  // the consumer class used by the AtomicNotificationQueue
  class Consumer {
   public:
    // this operation simply put the retrieved item into the temporary
    // carrier for the caller to extract
    void operator()(
        ServerRequest&& req, std::shared_ptr<folly::RequestContext>&&);

    ServerRequest carrier_;
  };

  Options opts_;
  // The reason why we chose unique_ptr managed array
  // instead of std containers like vector is that
  // the queues are not movable / move constructable
  // which is necessary for standard container
  std::unique_ptr<std::unique_ptr<RequestQueue[]>[]> requestQueues_;
  // we use std::nullopt to signal that a certain priority is
  // using only one bucket - in that case we don't need retrieval queue
  std::unique_ptr<std::optional<RetrievalIndexQueue>[]> retrievalIndexQueues_;

  std::unique_ptr<std::unique_ptr<SingleBucketRequestQueue>[]>
      singleBucketRequestQueues_;

  ServerRequest dequeueImpl(unsigned pri, unsigned bucket);
};

// RoundRobinRequestPile is the default request pile used by most of
// the thrift servers
using StandardRequestPile = RoundRobinRequestPile;

} // namespace apache::thrift
