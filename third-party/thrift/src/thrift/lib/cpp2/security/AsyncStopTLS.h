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

#include <fizz/protocol/AsyncFizzBase.h>
#include <folly/io/async/DelayedDestruction.h>

namespace apache::thrift {
/**
 *
 * INTERNAL THRIFT DETAIL.
 *
 * AsyncStopTLS represents the async frame equivalent of:
 *
 * ```
 *    (transport, err) = await fizzConnection->tlsShutdown();
 * ```
 *
 * After `AsyncStopTLS::start()` is invoked, it is guaranteed that either of
 * the following methods is invoked:
 *
 *  * Callback::stopTLSSuccess()
 *  * Callback::stopTLSError()
 *
 */
class AsyncStopTLS : public folly::DelayedDestruction,
                     private fizz::AsyncFizzBase::EndOfTLSCallback,
                     private fizz::AsyncFizzBase::ReadCallback {
 public:
  using UniquePtr = std::unique_ptr<AsyncStopTLS, Destructor>;
  class Callback {
   public:
    virtual void stopTLSSuccess(std::unique_ptr<folly::IOBuf> postTLSData) = 0;
    virtual void stopTLSError(const folly::exception_wrapper& ew) = 0;
    virtual ~Callback() = default;
  };
  explicit AsyncStopTLS(Callback& awaiter) : awaiter_(&awaiter) {}

  enum class Role { Client, Server };

  /**
   * Begins an asynchronous StopTLS transaction on the supplied transport.
   *
   * The transport must outlive AsyncStopTLS.
   *
   * @param  transport  A fizz::AsyncFizzBase instance that the StopTLS
   *                    negotiation will be performed on. The `transport` must
   *                    outlive `this`.
   * @param  role       Determines whether we are the Server or the Client in
   *                    a StopTLS transaction.
   * @param  timeout    If non-zero, an internal timer will ensure that the
   *                    StopTLS callback will be invoked within the specified
   *                    time.
   */
  void start(
      fizz::AsyncFizzBase* transport,
      Role role,
      std::chrono::milliseconds timeout);

  void attachEventBase(folly::EventBase* evb);
  void detachEventBase();

 protected:
  ~AsyncStopTLS() override {
    // We guarantee that if `start()` was invoked, then the terminal callback
    // must have been invoked by the time we are destroyed.
    //
    // This means we can only be in one of two states -- the initial state
    // or the finished state.
    //
    // The initial state can be represented as a non-null awaiter with a null
    // transport, and the finished state can be represented as a null awaiter
    // with a non-null transport
    DCHECK((awaiter_ && !transport_) || (!awaiter_ && transport_));
  }
  void endOfTLS(
      fizz::AsyncFizzBase*, std::unique_ptr<folly::IOBuf> postTLSData) override;
  bool isBufferMovable() noexcept override { return true; }
  void getReadBuffer(void**, size_t* lenReturn) override;
  void readDataAvailable(size_t) noexcept override;
  void readEOF() noexcept override;
  void readBufferAvailable(std::unique_ptr<folly::IOBuf>) noexcept override;
  void readErr(const folly::AsyncSocketException& ex) noexcept override;

  void stopTLSTimeoutExpired() noexcept;

  Callback* prepareForTerminalCallback() noexcept {
    if (transactionTimeout_) {
      transactionTimeout_.reset();
    }
    transport_->setReadCB(nullptr);
    transport_->setEndOfTLSCallback(nullptr);
    return std::exchange(awaiter_, nullptr);
  }

  Callback* awaiter_{nullptr};
  fizz::AsyncFizzBase* transport_{nullptr};
  std::chrono::milliseconds timeout_{0};
  std::unique_ptr<folly::AsyncTimeout> transactionTimeout_{nullptr};
};

} // namespace apache::thrift
