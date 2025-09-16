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

#include <thrift/lib/cpp2/server/RequestDebugLog.h>

#include <folly/AtomicLinkedList.h>
#include <folly/logging/xlog.h>

FOLLY_GFLAGS_DEFINE_uint32(
    thrift_server_request_debug_log_entries_max,
    10000,
    "A limit specifying how many log entries can be in memory per request.");

namespace {
const folly::RequestToken& getToken() {
  static const folly::RequestToken token{"_THRIFT_REQUEST_DEBUG_LOG"};
  return token;
}

class RequestDebugLog : public folly::RequestData {
 public:
  bool hasCallback() override { return false; }

  template <typename T>
  void appendEntry(T&& msg) {
    if (counter_.fetch_add(1, std::memory_order_relaxed) >=
        FLAGS_thrift_server_request_debug_log_entries_max) {
      counter_.fetch_sub(1, std::memory_order_relaxed);
      XLOG_EVERY_MS(WARNING, 10000)
          << "Request debug log reaches limit: "
          << FLAGS_thrift_server_request_debug_log_entries_max
          << ". New log entries will be dropped.";
      return;
    }
    insertedMessages_.insertHead(std::forward<T>(msg));
  }

  /**
   * Collect appended log entries at a given point. Ideally this should not be
   * scheduled on any other place except each worker's eventbase, so that for
   * each request log there'll be serialized reads to it. In the future it's
   * possible to enforce this with RequestEventBase and isInEventBaseThread().
   * For now we put a  mutex here to guard against accidental concurrent reads.
   */
  std::vector<std::string> getEntries() {
    std::lock_guard<std::mutex> g(readMutex_);
    insertedMessages_.sweep(
        [&](std::string&& msg) { entries_.push_back(std::move(msg)); });
    if (!truncated_ &&
        entries_.size() >= FLAGS_thrift_server_request_debug_log_entries_max) {
      truncated_ = true;
      entries_.emplace_back("THE REST OF LOG WILL BE TRUNCATED.");
    }
    return entries_;
  }

 private:
  folly::AtomicLinkedList<std::string> insertedMessages_;
  std::vector<std::string> entries_;
  std::atomic<uint32_t> counter_{0};
  std::mutex readMutex_;
  bool truncated_{false};
};

template <typename T>
void appendRequestDebugLogImpl(T&& msg) {
  if (auto rctx = folly::RequestContext::try_get()) {
    auto ctxData = rctx->getContextData(getToken());
    if (ctxData == nullptr) {
      rctx->setContextDataIfAbsent(
          getToken(), std::make_unique<RequestDebugLog>());
      ctxData = rctx->getContextData(getToken());
    }

    if (auto log = dynamic_cast<RequestDebugLog*>(ctxData)) {
      log->appendEntry(std::forward<T>(msg));
    }
  }
}
} // namespace

namespace apache::thrift {
void appendRequestDebugLog(std::string&& msg) {
  appendRequestDebugLogImpl(std::move(msg));
}

void appendRequestDebugLog(const std::string& msg) {
  appendRequestDebugLogImpl(msg);
}

std::vector<std::string> collectRequestDebugLog(
    const std::shared_ptr<folly::RequestContext>& rctx) {
  DCHECK(rctx.get() != nullptr);
  auto log = dynamic_cast<RequestDebugLog*>(rctx->getContextData(getToken()));
  if (log == nullptr) {
    return {};
  }
  return log->getEntries();
}

std::vector<std::string> collectRequestDebugLog(
    const RequestsRegistry::DebugStub& stub) {
  auto rctx = stub.getRequestContext();
  if (rctx == nullptr) {
    return {};
  }
  return collectRequestDebugLog(rctx);
}

} // namespace apache::thrift
