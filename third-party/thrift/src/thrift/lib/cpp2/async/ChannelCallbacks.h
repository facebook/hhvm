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

#ifndef THRIFT_ASYNC_CHANNELCALLBACKS_H_
#define THRIFT_ASYNC_CHANNELCALLBACKS_H_ 1

#include <deque>
#include <memory>
#include <unordered_map>

#include <folly/io/async/DelayedDestruction.h>
#include <folly/io/async/HHWheelTimer.h>
#include <folly/io/async/Request.h>
#include <thrift/lib/cpp2/async/MessageChannel.h>
#include <thrift/lib/cpp2/async/RequestChannel.h>

namespace apache {
namespace thrift {

class ChannelCallbacks {
 protected:
  /**
   * Callback to manage the lifetime of a two-way call.
   * Deletes itself when it receives both a send and recv callback.
   * Exceptions:
   * 1) If we get a messageSendError, we will never get a recv callback,
   *    so it is safe to delete.
   * 2) timeoutExpired uninstalls the recv callback, so it is safe to delete
   *    if it was already sent.
   *
   * Deletion automatically uninstalls the timeout.
   */
  template <class Channel>
  class TwowayCallback final : public MessageChannel::SendCallback,
                               public folly::HHWheelTimer::Callback,
                               public folly::DelayedDestruction {
   public:
#define X_CHECK_STATE_EQ(state, expected) \
  CHECK_EQ(static_cast<int>(state), static_cast<int>(expected))
#define X_CHECK_STATE_NE(state, expected) \
  CHECK_NE(static_cast<int>(state), static_cast<int>(expected))
    // Keep separate state for send and receive.
    // Starts as queued for receive (as that's how it's created in
    // HeaderClientChannel::sendRequest).
    // We then try to send and either get messageSendError() or sendQueued().
    // If we get sendQueued(), we know to wait for either messageSendError()
    // or messageSent() before deleting.
    TwowayCallback(
        Channel* channel,
        uint32_t sendSeqId,
        RequestClientCallback::Ptr cb,
        folly::HHWheelTimer* timer,
        std::chrono::milliseconds timeout)
        : channel_(channel),
          sendSeqId_(sendSeqId),
          cb_(std::move(cb)),
          sendState_(QState::INIT),
          recvState_(QState::QUEUED) {
      CHECK(cb_);
      if (timeout > std::chrono::milliseconds(0)) {
        timer->scheduleTimeout(this, timeout);
      }
    }
    void sendQueued() override {
      X_CHECK_STATE_EQ(sendState_, QState::INIT);
      sendState_ = QState::QUEUED;
    }
    void messageSent() override {
      DestructorGuard dg(this);
      X_CHECK_STATE_EQ(sendState_, QState::QUEUED);
      sendState_ = QState::DONE;
      maybeDeleteThis();
    }
    void messageSendError(folly::exception_wrapper&& ex) override {
      DestructorGuard dg(this);
      X_CHECK_STATE_NE(sendState_, QState::DONE);
      sendState_ = QState::DONE;
      if (recvState_ == QState::QUEUED) {
        recvState_ = QState::DONE;
        channel_->eraseCallback(sendSeqId_, this);
        cancelTimeout();
      }
      if (cb_) {
        cb_.release()->onResponseError(std::move(ex));
      }
      destroy();
    }
    void replyReceived(
        std::unique_ptr<folly::IOBuf> buf,
        std::unique_ptr<apache::thrift::transport::THeader> header) {
      DestructorGuard dg(this);
      X_CHECK_STATE_NE(sendState_, QState::INIT);
      X_CHECK_STATE_EQ(recvState_, QState::QUEUED);
      recvState_ = QState::DONE;
      cancelTimeout();

      CHECK(cb_);
      cb_.release()->onResponse(
          ClientReceiveState(-1, std::move(buf), std::move(header), nullptr));

      maybeDeleteThis();
    }
    void requestError(folly::exception_wrapper ex) {
      DestructorGuard dg(this);
      X_CHECK_STATE_EQ(recvState_, QState::QUEUED);
      recvState_ = QState::DONE;
      cancelTimeout();
      if (cb_) {
        cb_.release()->onResponseError(std::move(ex));
      }

      maybeDeleteThis();
    }
    void timeoutExpired() noexcept override {
      DestructorGuard dg(this);
      X_CHECK_STATE_EQ(recvState_, QState::QUEUED);
      channel_->eraseCallback(sendSeqId_, this);
      recvState_ = QState::DONE;

      if (cb_) {
        using apache::thrift::transport::TTransportException;

        TTransportException ex(TTransportException::TIMED_OUT, "Timed Out");
        ex.setOptions(TTransportException::CHANNEL_IS_VALID); // framing okay
        cb_.release()->onResponseError(std::move(ex));
      }
      maybeDeleteThis();
    }
    void expire() {
      DestructorGuard dg(this);
      X_CHECK_STATE_EQ(recvState_, QState::QUEUED);
      channel_->eraseCallback(sendSeqId_, this);
      recvState_ = QState::DONE;
      cb_.reset();

      maybeDeleteThis();
    }

   private:
    enum class QState { INIT, QUEUED, DONE };
    void maybeDeleteThis() {
      if (sendState_ == QState::DONE && recvState_ == QState::DONE) {
        destroy();
      }
    }
    ~TwowayCallback() override {
      X_CHECK_STATE_EQ(sendState_, QState::DONE);
      X_CHECK_STATE_EQ(recvState_, QState::DONE);
      CHECK(!cb_);
    }
    Channel* channel_;
    uint32_t sendSeqId_;
    RequestClientCallback::Ptr cb_;
    QState sendState_;
    QState recvState_;
#undef X_CHECK_STATE_NE
#undef X_CHECK_STATE_EQ
  };

  class OnewayCallback final : public MessageChannel::SendCallback,
                               public folly::DelayedDestruction {
   public:
    explicit OnewayCallback(RequestClientCallback::Ptr cb)
        : cb_(std::move(cb)) {}
    void sendQueued() override {}
    void messageSent() override {
      DestructorGuard dg(this);
      CHECK(cb_);
      cb_.release()->onResponse({});
      destroy();
    }
    void messageSendError(folly::exception_wrapper&& ex) override {
      DestructorGuard dg(this);
      CHECK(cb_);
      cb_.release()->onResponseError(std::move(ex));
      destroy();
    }

   private:
    RequestClientCallback::Ptr cb_;
  };
};
} // namespace thrift
} // namespace apache

#endif // THRIFT_ASYNC_CHANNELCALLBACKS_H_
