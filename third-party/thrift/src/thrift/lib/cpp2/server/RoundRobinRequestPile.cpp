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

#include <utility>

#include <thrift/lib/cpp/concurrency/Thread.h>
#include <thrift/lib/cpp2/server/RoundRobinRequestPile.h>

namespace apache::thrift {

RoundRobinRequestPile::PileSelectionFunction
RoundRobinRequestPile::Options::getDefaultPileSelectionFunc(
    unsigned defaultPriority) {
  DCHECK(numBucketsPerPriority.size());
  unsigned priorityLimit = numBucketsPerPriority.size() - 1;
  PileSelectionFunction pileSelectionFunction{
      [defaultPriority, priorityLimit](
          const ServerRequest& request) -> std::pair<unsigned, unsigned> {
        unsigned priority =
            static_cast<unsigned>(request.requestContext()->getCallPriority());
        if (priority <= priorityLimit) {
          return std::make_pair(static_cast<uint64_t>(priority), 0);
        }
        if (request.methodMetadata()) {
          if (auto prio = request.methodMetadata()->priority) {
            priority = static_cast<unsigned>(*prio);
          } else {
            priority = defaultPriority;
          }

        } else {
          priority = defaultPriority;
        }
        return std::make_pair(static_cast<uint64_t>(priority), 0);
      }};
  return pileSelectionFunction;
}

void RoundRobinRequestPile::Consumer::operator()(
    ServerRequest&& req, std::shared_ptr<folly::RequestContext>&&) {
  carrier_ = std::move(req);
}

RoundRobinRequestPile::RoundRobinRequestPile(Options opts)
    : RequestPileBase(std::move(opts.name)) {
  opts_ = std::move(opts);
  const auto& numBucketsPerPriority = opts_.numBucketsPerPriority;

  requestQueues_ = std::make_unique<std::unique_ptr<RequestQueue[]>[]>(
      numBucketsPerPriority.size());
  retrievalIndexQueues_ =
      std::make_unique<std::optional<RetrievalIndexQueue>[]>(
          numBucketsPerPriority.size());
  singleBucketRequestQueues_ =
      std::make_unique<std::unique_ptr<SingleBucketRequestQueue>[]>(
          numBucketsPerPriority.size());

  for (unsigned i = 0; i < numBucketsPerPriority.size(); ++i) {
    DCHECK(numBucketsPerPriority.at(i));

    if (opts_.numMaxRequests) {
      singleBucketRequestQueues_[i] =
          std::make_unique<SingleBucketRequestQueue>(true);
      singleBucketRequestQueues_[i]->setLimit(opts_.numMaxRequests);
    } else {
      singleBucketRequestQueues_[i] =
          std::make_unique<SingleBucketRequestQueue>(false);
    }

    requestQueues_[i] =
        std::make_unique<RequestQueue[]>(numBucketsPerPriority.at(i));

    if (numBucketsPerPriority.at(i) == 1) {
      // explicitly set it to null
      retrievalIndexQueues_[i] = std::nullopt;
    } else {
      // copy constructor is deleted so we use placement new
      new (&retrievalIndexQueues_[i])
          std::optional<RetrievalIndexQueue>(std::in_place);
    }

    for (unsigned j = 0; j < numBucketsPerPriority.at(i); ++j) {
      // because we are doing round robin
      // for each round we only want to consumer
      // one item from the queue
      requestQueues_[i][j].setMaxReadAtOnce(1);
      requestQueues_[i][j].arm();
    }
  }
}

ServerRequest RoundRobinRequestPile::dequeueImpl(
    unsigned pri, unsigned bucket) {
  Consumer consumer;
  auto& queue = requestQueues_[pri][bucket];
  queue.drive(consumer);

  // Schedule a callback from the consumer thread iff
  // there is something remaining in the queue
  // we only store the index of next queue
  if (!queue.arm()) {
    retrievalIndexQueues_[pri]->enqueue(bucket);
  }

  RequestPileBase::onDequeued(consumer.carrier_);

  return std::move(consumer.carrier_);
}

std::optional<ServerRequestRejection> RoundRobinRequestPile::enqueue(
    ServerRequest&& request) {
  // record the enqueue time
  auto& info = request.requestData();
  info.queueBegin = std::chrono::steady_clock::now();

  unsigned pri = 0, bucket = 0;
  if (opts_.pileSelectionFunction) {
    std::tie(pri, bucket) = opts_.pileSelectionFunction(request);
  }
  DCHECK_LT(pri, opts_.numBucketsPerPriority.size());
  DCHECK_LT(bucket, opts_.numBucketsPerPriority[pri]);
  request.requestData().bucket = {pri, bucket};
  RequestPileBase::onEnqueued(request);

  // TODO(yichengfb): enforcing limit on single bucket queue
  // We temporarily disable limit for single bucket queue
  // because we currently rely on AtomicNotificationQueue to
  // achieve that.
  if (!retrievalIndexQueues_[pri]) {
    if (singleBucketRequestQueues_[pri]->enqueue(std::move(request))) {
      return std::nullopt;
    }
    return std::make_optional<ServerRequestRejection>(AppServerException(
        "AppServerException", "RequestPile enqueue error : reached max limit"));
  }

  if (opts_.numMaxRequests != 0) {
    auto res = requestQueues_[pri][bucket].tryPush(
        std::move(request), opts_.numMaxRequests);
    switch (res) {
      case RequestQueue::TryPushResult::FAILED_LIMIT_REACHED: {
        return std::make_optional<ServerRequestRejection>(AppServerException(
            "AppServerException",
            "RequestPile enqueue error : reached max limit"));
      }
      case RequestQueue::TryPushResult::SUCCESS_AND_ARMED: {
        retrievalIndexQueues_[pri]->enqueue(bucket);
        break;
      }
      default:
        break;
    }
  } else {
    if (requestQueues_[pri][bucket].push(std::move(request))) {
      // Schedule a pull iff we are the first one to push into
      // the request queue
      retrievalIndexQueues_[pri]->enqueue(bucket);
    }
  }
  return std::nullopt;
}

std::optional<ServerRequest> RoundRobinRequestPile::dequeue() {
  for (unsigned i = 0; i < opts_.numBucketsPerPriority.size(); ++i) {
    if (!retrievalIndexQueues_[i]) {
      if (auto req = singleBucketRequestQueues_[i]->tryDequeue()) {
        return std::move(*req);
      }
    } else {
      if (auto bucket = retrievalIndexQueues_[i]->try_dequeue()) {
        return dequeueImpl(i, *bucket);
      }
    }
  }
  return std::nullopt;
}

uint64_t RoundRobinRequestPile::requestCount() const {
  uint64_t res = 0;
  for (size_t i = 0; i < opts_.numBucketsPerPriority.size(); ++i) {
    if (!retrievalIndexQueues_[i]) {
      res += singleBucketRequestQueues_[i]->size();
    } else if (!retrievalIndexQueues_[i]->empty()) {
      for (size_t j = 0; j < opts_.numBucketsPerPriority[i]; ++j) {
        res += requestQueues_[i][j].size();
      }
    }
  }

  return res;
}

std::string RoundRobinRequestPile::describe() const {
  std::string result;
  result = fmt::format(
      "RoundRobinRequestPile priorities:{}",
      opts_.numBucketsPerPriority.size());
  for (size_t i = 0; i < opts_.numBucketsPerPriority.size(); ++i) {
    result +=
        fmt::format(" Pri:{} Buckets:{}", i, opts_.numBucketsPerPriority[i]);
  }
  return result;
}

} // namespace apache::thrift
