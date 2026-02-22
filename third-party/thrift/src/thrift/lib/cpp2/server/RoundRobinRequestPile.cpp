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

#include <folly/executors/CPUThreadPoolExecutor.h>

#include <thrift/lib/cpp/concurrency/Thread.h>
#include <thrift/lib/cpp2/server/RoundRobinRequestPile.h>

namespace apache::thrift {

RoundRobinRequestPile::PileSelectionFunction
RoundRobinRequestPile::Options::getDefaultPileSelectionFunc(
    unsigned defaultPriority) {
  DCHECK(numBucketsPerPriority.size());
  unsigned priorityLimit = numBucketsPerPriority.size() - 1;
  return [defaultPriority, priorityLimit](
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
  };
}

folly::AtomicNotificationQueueTaskStatus
RoundRobinRequestPile::Consumer::operator()(
    ServerRequest&& request, std::shared_ptr<folly::RequestContext>&&) {
  // If Executor for expiring requests is NOT provided, we return even expired
  // requests to the user
  if (requestExpirationDelegate_ == nullptr) {
    carrier_ = std::move(request);
    return folly::AtomicNotificationQueueTaskStatus::CONSUMED;
  }

  // If Executor to expiring requests IS provided, we schedule expiration of
  // expired requests on that Executor and return only not-yet-expired requests
  // to the user

  using namespace apache::thrift::detail;

  if (request.request() != nullptr && !request.request()->isActive()) {
    // NOTE that if request()->isActive() returns false we know *for sure* that
    // request has expired, but if it returns true then request MIGHT be
    // expired or not.
    requestExpirationDelegate_->processExpiredRequest(std::move(request));
    return folly::AtomicNotificationQueueTaskStatus::DISCARD;
  }

  carrier_ = std::move(request);
  return folly::AtomicNotificationQueueTaskStatus::CONSUMED;
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

  DCHECK(
      opts_.numMaxRequestsPerPriority.empty() ||
      opts_.numMaxRequestsPerPriority.size() == numBucketsPerPriority.size());

  for (unsigned i = 0; i < numBucketsPerPriority.size(); ++i) {
    DCHECK(numBucketsPerPriority.at(i));

    if (auto limit = opts_.getNumMaxRequestsForPriority(i)) {
      singleBucketRequestQueues_[i] =
          std::make_unique<SingleBucketRequestQueue>(true);
      singleBucketRequestQueues_[i]->setLimit(limit);
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

std::optional<ServerRequest> RoundRobinRequestPile::dequeueImpl(
    unsigned pri, unsigned bucket) {
  Consumer consumer(requestExpirationDelegate_);
  auto& queue = requestQueues_[pri][bucket];
  queue.drive(consumer);

  // Schedule a callback from the consumer thread iff
  // there is something remaining in the queue
  // we only store the index of next queue
  if (!queue.arm()) {
    retrievalIndexQueues_[pri]->enqueue(bucket);
  }

  if (consumer.carrier_) {
    RequestPileBase::onDequeued(*consumer.carrier_);
    return std::move(*consumer.carrier_);
  } else {
    return std::nullopt;
  }
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
      if (onRequestAcceptedFunction_) {
        onRequestAcceptedFunction_(pri, bucket);
      }
      return std::nullopt;
    }
    if (onRequestRejectedFunction_) {
      onRequestRejectedFunction_(pri, bucket);
    }
    return std::make_optional<ServerRequestRejection>(AppServerException(
        "AppServerException", "RequestPile enqueue error : reached max limit"));
  }

  if (auto maxRequest = opts_.getNumMaxRequestsForPriority(pri)) {
    auto res =
        requestQueues_[pri][bucket].tryPush(std::move(request), maxRequest);
    switch (res) {
      case RequestQueue::TryPushResult::FAILED_LIMIT_REACHED: {
        if (onRequestRejectedFunction_) {
          onRequestRejectedFunction_(pri, bucket);
        }
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

  if (onRequestAcceptedFunction_) {
    onRequestAcceptedFunction_(pri, bucket);
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

serverdbginfo::RequestPileDbgInfo RoundRobinRequestPile::getDbgInfo() const {
  serverdbginfo::RequestPileDbgInfo info;
  info.name() = folly::demangle(typeid(*this));
  info.prioritiesCount() = opts_.numBucketsPerPriority.size();

  for (size_t i = 0; i < opts_.numBucketsPerPriority.size(); ++i) {
    info.bucketsPerPriority()->push_back(opts_.numBucketsPerPriority[i]);
  }

  info.perBucketRequestLimit() = opts_.numMaxRequests;
  info.queuedRequestsCount() = requestCount();

  return info;
}

std::vector<std::vector<uint64_t>> RoundRobinRequestPile::getRequestCounts()
    const {
  std::vector<std::vector<uint64_t>> res;
  res.reserve(opts_.numBucketsPerPriority.size());
  for (size_t i = 0; i < opts_.numBucketsPerPriority.size(); ++i) {
    if (!retrievalIndexQueues_[i]) {
      res.emplace_back(1, singleBucketRequestQueues_[i]->size());
    } else {
      std::vector<uint64_t> subRes;
      if (!retrievalIndexQueues_[i]->empty()) {
        subRes.reserve(opts_.numBucketsPerPriority[i]);
        for (size_t j = 0; j < opts_.numBucketsPerPriority[i]; ++j) {
          subRes.emplace_back(requestQueues_[i][j].size());
        }
      } else {
        subRes.resize(opts_.numBucketsPerPriority[i]);
      }
      res.push_back(std::move(subRes));
    }
  }

  return res;
}

namespace {
/*
 * augmentWithInternalPriorities takes a PileSelectionFunction and updates it to
 * work with internal priorities. The updated PileSelectionFunction will return
 * a priority that is twice as large as the original priority, and will add 1 to
 * the priority if the internal priority is LO_PRI.
 *
 * The updated function will return the same bucket as the original.
 */
RoundRobinRequestPile::PileSelectionFunction augmentWithInternalPriorities(
    RoundRobinRequestPile::PileSelectionFunction&& func) {
  return [originalFunc = std::move(func)](const ServerRequest& req)
             -> std::pair<
                 RoundRobinRequestPile::Priority,
                 RoundRobinRequestPile::Bucket> {
    unsigned pri = 0, bucket = 0;
    if (originalFunc) {
      std::tie(pri, bucket) = originalFunc(req);
    }
    pri *= 2;
    if (detail::ServerRequestHelper::internalPriority(req) ==
        folly::Executor::LO_PRI) {
      pri += 1;
    }
    return {pri, bucket};
  };
}
} // namespace

/*static*/ RoundRobinRequestPile::Options
RoundRobinRequestPile::addInternalPriorities(
    RoundRobinRequestPile::Options opts) {
  if (opts.numBucketsPerPriority.size() >
      std::numeric_limits<RoundRobinRequestPile::Priority>::max() / 2) {
    LOG(WARNING) << "Too many priorities, additional internal priorities "
                 << "will not be added.";
    return opts;
  }

  // Double the number of priorities
  std::vector<unsigned int> numBucketsPerPriority;
  numBucketsPerPriority.reserve(opts.numBucketsPerPriority.size() * 2);
  for (auto numBuckets : opts.numBucketsPerPriority) {
    // See comment on addInternalPriorities declaration for why we do this
    numBucketsPerPriority.push_back(numBuckets);
    numBucketsPerPriority.push_back(numBuckets);
  }
  opts.numBucketsPerPriority = std::move(numBucketsPerPriority);

  // Double the number of priorities also for per-priority limits.
  if (!opts.numMaxRequestsPerPriority.empty()) {
    std::vector<uint32_t> numMaxRequestsPerPriority;
    numMaxRequestsPerPriority.reserve(
        opts.numMaxRequestsPerPriority.size() * 2);
    for (auto limit : opts.numMaxRequestsPerPriority) {
      numMaxRequestsPerPriority.push_back(limit);
      numMaxRequestsPerPriority.push_back(limit);
    }
    opts.numMaxRequestsPerPriority = std::move(numMaxRequestsPerPriority);
  }

  // Update pileSelectionFunction
  opts.pileSelectionFunction =
      augmentWithInternalPriorities(std::move(opts.pileSelectionFunction));
  return opts;
}

} // namespace apache::thrift
