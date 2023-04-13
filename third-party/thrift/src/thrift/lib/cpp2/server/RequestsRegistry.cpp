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

#include <thrift/lib/cpp2/server/RequestsRegistry.h>

#include <atomic>
#include <fmt/format.h>
#include <thrift/lib/cpp2/server/Cpp2ConnContext.h>

namespace apache {
namespace thrift {

namespace {
// RequestId storage.
// Reserve some high bits for future use. Currently the maximum id supported
// is 2^52 (on 64-bit systems), so thrift servers theoretically can generate
// unique request id for ~12 years, assuming the QPS is ~10 million.
constexpr size_t kReserveBits = sizeof(uintptr_t) >= 8 ? 12 : 6;
constexpr size_t kLsbBits = 8 * sizeof(uintptr_t) - kReserveBits;
constexpr uintptr_t kLsbMask = (1ull << kLsbBits) - 1;

struct RegistryIdManager {
 public:
  uint32_t getId() {
    if (!freeIds_.empty()) {
      auto id = *freeIds_.begin();
      freeIds_.erase(freeIds_.begin());
      return id;
    }

    auto id = nextId_++;
    CHECK_LT(id, 1ull << kReserveBits); // 4096, if pointers are 64 bits
    return id;
  }

  void returnId(uint32_t id) {
    freeIds_.insert(id);
    while (!freeIds_.empty()) {
      auto largestId = *(--freeIds_.end());
      if (largestId < nextId_ - 1) {
        return;
      }
      DCHECK(largestId == nextId_ - 1);
      --nextId_;
      freeIds_.erase(largestId);
    }
  }

 private:
  std::set<uint32_t> freeIds_;
  uint32_t nextId_;
};

folly::Synchronized<RegistryIdManager>& registryIdManager() {
  static auto* registryIdManagerPtr =
      new folly::Synchronized<RegistryIdManager>();
  return *registryIdManagerPtr;
}
} // namespace

namespace detail {
THRIFT_PLUGGABLE_FUNC_REGISTER(uint64_t, getCurrentServerTick) {
  return 0;
}

THRIFT_PLUGGABLE_FUNC_REGISTER(
    std::unique_ptr<folly::WorkerProvider>,
    createIOWorkerProvider,
    folly::Executor*,
    RequestsRegistry*) {
  return nullptr;
}
} // namespace detail

void RecentRequestCounter::increment() {
  auto currBucket = getCurrentBucket();
  counts_[currBucket].arrivalCount += 1;
  counts_[currBucket].activeCount = ++currActiveCount_;
}

void RecentRequestCounter::decrement() {
  if (currActiveCount_ > 0) {
    auto currBucket = getCurrentBucket();
    counts_[currBucket].activeCount = --currActiveCount_;
  }
}

RecentRequestCounter::Values RecentRequestCounter::get() const {
  Values ret;
  uint64_t currentBucket = getCurrentBucket();
  uint64_t i = currentBucket + kBuckets;

  for (auto& val : ret) {
    val = counts_[i-- % kBuckets];
  }

  return ret;
}

void RecentRequestCounter::incrementOverloadCount() {
  auto currBucket = getCurrentBucket();
  counts_[currBucket].overloadCount += 1;
}

uint64_t RecentRequestCounter::getCurrentBucket() const {
  // Remove old request counts from counts_ and update lastTick_
  uint64_t currentTick = detail::getCurrentServerTick();

  if (lastTick_ < currentTick) {
    uint64_t tickDiff = currentTick - lastTick_;
    uint64_t ticksToClear = tickDiff < kBuckets ? tickDiff : kBuckets;

    while (ticksToClear) {
      auto index = (lastTick_ + ticksToClear--) % kBuckets;
      counts_[index].arrivalCount = 0;
      counts_[index].activeCount = currActiveCount_;
      counts_[index].overloadCount = 0;
    }
    lastTick_ = currentTick;
    currentBucket_ = lastTick_ % kBuckets;
  }

  return currentBucket_;
}

RequestsRegistry::RequestsRegistry(
    uint64_t requestPayloadMem,
    uint64_t totalPayloadMem,
    uint16_t finishedRequestsLimit)
    : registryId_(registryIdManager().wlock()->getId()),
      payloadMemoryLimitPerRequest_(requestPayloadMem),
      payloadMemoryLimitTotal_(totalPayloadMem),
      finishedRequestsLimit_(finishedRequestsLimit) {}

RequestsRegistry::~RequestsRegistry() {
  while (!reqFinishedList_.empty()) {
    --finishedRequestsCount_;
    auto& front = reqFinishedList_.front();
    reqFinishedList_.pop_front();
    front.decRef();
  }
  DCHECK(finishedRequestsCount_ == 0);
  registryIdManager().wlock()->returnId(registryId_);
}

/* static */ std::string RequestsRegistry::getRequestId(intptr_t rootid) {
  return fmt::format("{:016x}", static_cast<uintptr_t>(rootid));
}

bool RequestsRegistry::isThriftRootId(intptr_t rootid) noexcept {
  return rootid & 0x1;
}

intptr_t RequestsRegistry::genRootId() {
  // Ensure rootid's LSB is always 1.
  // This is to prevent any collision with rootids on folly::RequestsContext() -
  // those are addresses of folly::RequestContext objects.
  return 0x1 | ((nextLocalId_++ << 1) & kLsbMask) |
      (static_cast<uintptr_t>(registryId_) << kLsbBits);
}

void RequestsRegistry::registerStub(DebugStub& req) {
  if (req.stateMachine_.includeInRecentRequests()) {
    requestCounter_.increment();
  }
  uint64_t payloadSize = req.getPayloadSize();
  reqActiveList_.push_back(req);
  if (payloadSize > payloadMemoryLimitPerRequest_) {
    req.releasePayload();
    return;
  }
  reqPayloadList_.push_back(req);
  payloadMemoryUsage_ += payloadSize;
  evictStubPayloads();
}

void RequestsRegistry::moveToFinishedList(RequestsRegistry::DebugStub& stub) {
  if (stub.stateMachine_.includeInRecentRequests()) {
    requestCounter_.decrement();
  }
  if (finishedRequestsLimit_ == 0) {
    return;
  }

  stub.activeRequestsRegistryHook_.unlink();
  stub.incRef();
  stub.prepareAsFinished();
  ++finishedRequestsCount_;
  reqFinishedList_.push_back(stub);

  if (finishedRequestsCount_ > finishedRequestsLimit_) {
    DCHECK(finishedRequestsLimit_ > 0);
    --finishedRequestsCount_;
    auto& front = reqFinishedList_.front();
    reqFinishedList_.pop_front();
    front.decRef();
  }
}

const std::string& RequestsRegistry::DebugStub::getMethodName() const {
  return getCpp2RequestContext() ? getCpp2RequestContext()->getMethodName()
                                 : methodNameIfFinished_;
}

const folly::SocketAddress* RequestsRegistry::DebugStub::getLocalAddress()
    const {
  return getCpp2RequestContext() ? getCpp2RequestContext()->getLocalAddress()
                                 : &localAddressIfFinished_;
}

const folly::SocketAddress* RequestsRegistry::DebugStub::getPeerAddress()
    const {
  return getCpp2RequestContext() ? getCpp2RequestContext()->getPeerAddress()
                                 : &peerAddressIfFinished_;
}

void RequestsRegistry::DebugStub::prepareAsFinished() {
  finished_ = std::chrono::steady_clock::now();
  rctx_.reset();
  methodNameIfFinished_ =
      const_cast<Cpp2RequestContext*>(reqContext_)->releaseMethodName();
  peerAddressIfFinished_ = *reqContext_->getPeerAddress();
  localAddressIfFinished_ = *reqContext_->getLocalAddress();
  reqContext_ = nullptr;
  req_ = nullptr;
}

void RequestsRegistry::DebugStub::incRef() noexcept {
  refCount_++;
}

void RequestsRegistry::DebugStub::decRef() noexcept {
  if (--refCount_ == 0) {
    this->~DebugStub();
    free(this);
  }
}

} // namespace thrift
} // namespace apache
