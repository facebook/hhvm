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

#include <folly/Try.h>

#include <thrift/lib/cpp2/async/StreamCallbacks.h>
#include <thrift/lib/cpp2/async/TwoWayBridge.h>
#include <thrift/lib/cpp2/async/TwoWayBridgeUtil.h>

namespace apache::thrift {

struct BufferOptions {
  int32_t chunkSize{100};
  size_t memSize{0};
  // Used only with memory buffer sized based throttling to cap the
  // number of credits to the give maxChunks value even if there is
  // enough buffer. Useful when we expect sudden spikes of large
  // message payloads.
  int32_t maxChunkSize{std::numeric_limits<int32_t>::max()};
};

namespace detail {

class ClientStreamBridge;

// This template explicitly instantiated in ClientStreamBridge.cpp
extern template class TwoWayBridge<
    QueueConsumer,
    folly::Try<StreamPayload>,
    ClientStreamBridge,
    int64_t,
    ClientStreamBridge>;

class ClientStreamBridge : public TwoWayBridge<
                               QueueConsumer,
                               folly::Try<StreamPayload>,
                               ClientStreamBridge,
                               int64_t,
                               ClientStreamBridge>,
                           private StreamClientCallback {
 public:
  ~ClientStreamBridge() override;
  ClientStreamBridge(const ClientStreamBridge&) = delete;
  ClientStreamBridge& operator=(const ClientStreamBridge&) = delete;
  ClientStreamBridge(ClientStreamBridge&&) = delete;
  ClientStreamBridge& operator=(ClientStreamBridge&&) = delete;

  struct ClientDeleter : Deleter {
    void operator()(ClientStreamBridge* ptr);
  };
  using ClientPtr = std::unique_ptr<ClientStreamBridge, ClientDeleter>;

  using FirstResponseCallback = FirstResponseClientCallback<ClientPtr>;

  static StreamClientCallback* create(FirstResponseCallback* callback);

  bool wait(QueueConsumer* consumer);

  ClientQueue getMessages();

  void requestN(int64_t credits);

  void cancel();

  bool isCanceled();

  void consume();

  void canceled();

 private:
  explicit ClientStreamBridge(FirstResponseCallback* callback);

  bool onFirstResponse(
      FirstResponsePayload&& payload,
      folly::EventBase* evb,
      StreamServerCallback* streamServerCallback) override;

  void onFirstResponseError(folly::exception_wrapper ew) override;

  bool onStreamNext(StreamPayload&& payload) override;

  void onStreamError(folly::exception_wrapper ew) override;

  void onStreamComplete() override;

  bool onStreamHeaders(HeadersPayload&& payload) override;

  void resetServerCallback(StreamServerCallback& serverCallback) override;

  void processCredits();

  void serverCleanup();

  union {
    FirstResponseCallback* firstResponseCallback_;
    StreamServerCallback* streamServerCallback_;
  };
  folly::Executor::KeepAlive<> serverExecutor_;
};
} // namespace detail
} // namespace apache::thrift
