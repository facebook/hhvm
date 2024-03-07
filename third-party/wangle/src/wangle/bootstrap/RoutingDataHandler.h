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

#include <wangle/channel/AsyncSocketHandler.h>

namespace wangle {

template <typename R>
class RoutingDataHandler : public wangle::BytesToBytesHandler {
 public:
  struct RoutingData {
    RoutingData() : bufQueue(folly::IOBufQueue::cacheChainLength()) {}

    R routingData;
    folly::IOBufQueue bufQueue;
  };

  class Callback {
   public:
    virtual ~Callback() = default;
    virtual void onRoutingData(uint64_t connId, RoutingData& routingData) = 0;
    virtual void onError(uint64_t connId, folly::exception_wrapper ex) = 0;
  };

  RoutingDataHandler(uint64_t connId, Callback* cob);
  ~RoutingDataHandler() override = default;

  // BytesToBytesHandler implementation
  void read(Context* ctx, folly::IOBufQueue& q) override;
  void readEOF(Context* ctx) override;
  void readException(Context* ctx, folly::exception_wrapper ex) override;

  /**
   * Parse the routing data from bufQueue into routingData. This
   * will be used to compute the hash for choosing the worker thread.
   *
   * Bytes that need to be passed into the child pipeline (such
   * as additional bytes left in bufQueue not used for parsing)
   * should be moved into RoutingData::bufQueue.
   *
   * @return bool - True on success, false if bufQueue doesn't have
   *                sufficient bytes for parsing. It will call
   *                parseRoutingDataCallback upon returning true.
   */
  virtual bool parseRoutingData(
      folly::IOBufQueue& bufQueue,
      RoutingData& routingData) = 0;

 protected:
  /**
   * Use only if you want to set routingData asynchronously. This
   * function should be called inside parseRoutingData() once
   * routingData resolves to resume callback for non-blocking
   * I/O. If you choose to do so instead of just setting routingData
   * synchronously in parseRoutingData, remember to return false in
   * parseRoutingData otherwise the callback will be executed twice.
   */
  void parseRoutingDataCallback(RoutingData& routingData);

 private:
  uint64_t connId_;
  Callback* cob_{nullptr};
};

template <typename R>
class RoutingDataHandlerFactory {
 public:
  virtual ~RoutingDataHandlerFactory() = default;

  virtual std::shared_ptr<RoutingDataHandler<R>> newHandler(
      uint64_t connId,
      typename RoutingDataHandler<R>::Callback* cob) = 0;
};

} // namespace wangle

#include <wangle/bootstrap/RoutingDataHandler-inl.h>
