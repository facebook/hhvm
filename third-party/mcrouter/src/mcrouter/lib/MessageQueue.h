/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <chrono>
#include <memory>

#include <sys/eventfd.h>

#include <folly/FileUtil.h>
#include <folly/MPMCQueue.h>
#include <folly/Random.h>
#include <folly/io/async/EventHandler.h>
#include <folly/io/async/VirtualEventBase.h>

namespace facebook {
namespace memcache {

/**
 * Relaxed notification - slight increase of average (not p99) latency
 * for improved CPU time (fewer cross-thread notifications)
 */
class Notifier {
 public:
  using NowUsecFunc = int64_t (*)();

  /**
   * @param noNotifyRate  Request rate at which we stop all per-request
   *   notifications.  At any rate from 0 to noNotifyRate, we linearly
   *   reduce the fraction of requests that get notified (starting from
   *   100% of requests initially).
   *   If 0, this logic is disabled - we notify on every request.
   *
   * @param waitThreshold  Force notification after this number of us
   *   passed since the queue was last drained.
   *   If 0, this logic is disabled.
   *
   * @param nowFunc  Function that returns current time in us.
   *
   * @param postDrainCallback  Callback to be called after drainig a queue.
   *   As an argument it will be passed false if we're still draining and true
   *   if we're out of drain loop. It should return true if it can guarantee
   *   that the current event_base_loop won't block, false otherwise. The return
   *   value is used as a hint for avoiding unnecessary notifications.
   */
  Notifier(
      size_t noNotifyRate,
      int64_t waitThresholdUs,
      NowUsecFunc nowFunc,
      std::function<bool(bool)> postDrainCallback = nullptr) noexcept;

  void bumpMessages() noexcept {
    ++curMessages_;
  }

  size_t currentNotifyPeriod() const noexcept {
    return period_;
  }

  bool shouldNotify() noexcept {
    return state_.exchange(State::NOTIFIED, std::memory_order_acq_rel) ==
        State::EMPTY;
  }

  bool shouldNotifyRelaxed() noexcept;

  // In contrast to shouldNotify()/shouldNotifyRelaxed(), it is only safe to
  // call drainWhileNonEmpty() from a single thread.
  template <class F>
  void drainWhileNonEmpty(F&& drainFunc) {
    State expected;
    bool nonBlockingLoop;
    // Drain queue and update state to EMPTY. Note, as an optimization, if we
    // know that the loop is non-blocking, we don't mark it as empty to avoid
    // unnecessary client notifications.
    do {
      expected = State::READING;
      state_.store(State::READING, std::memory_order_release);
      drainFunc();
      nonBlockingLoop = postDrainCallback_ ? postDrainCallback_(false) : false;
    } while (state_.load(std::memory_order_acquire) != State::READING ||
             (!nonBlockingLoop &&
              !state_.compare_exchange_strong(
                  expected,
                  State::EMPTY,
                  std::memory_order_acq_rel,
                  std::memory_order_acquire)));
    if (postDrainCallback_) {
      postDrainCallback_(true);
    }
    if (waitThreshold_ > 0) {
      waitStart_ = nowFunc_();
    }
  }

  void maybeUpdatePeriod() noexcept;

  size_t noNotifyRate() const {
    return noNotifyRate_;
  }

 private:
  const size_t noNotifyRate_;
  const int64_t waitThreshold_;
  const NowUsecFunc nowFunc_;
  std::function<bool(bool)> postDrainCallback_;
  int64_t lastTimeUsec_;
  size_t curMessages_{0};

  static constexpr int64_t kUpdatePeriodUsec = 1000000;

  alignas(folly::hardware_destructive_interference_size)
      std::atomic<size_t> period_{0};
  alignas(folly::hardware_destructive_interference_size)
      std::atomic<size_t> counter_{0};
  alignas(folly::hardware_destructive_interference_size)
      std::atomic<int64_t> waitStart_;

  enum class State {
    EMPTY,
    NOTIFIED,
    READING,
  };

  alignas(
      folly::hardware_destructive_interference_size) std::atomic<State> state_;
};

template <class T>
class MessageQueue {
 public:
  /**
   * Must be called from the event base thread.
   *
   * @param capactiy  All queue storage is allocated upfront.
   *   If queue is full, further writes will block.
   * @param onMessage Called on every message from the event base thread.
   * @param noNotifyRate  Request rate at which we stop all per-request
   *   notifications.  At any rate from 0 to noNotifyRate, we linearly
   *   reduce the fraction of requests that get notified (starting from
   *   100% of requests initially).
   *   If 0, this logic is disabled - we notify on every request.
   * @param waitThreshold  Force notification after this number of us
   *   passed since the queue was last drained.
   *   If 0, this logic is disabled.
   * @param nowFunc  Function that returns current time in us.
   * @param notifyCallback  Called every time after a notification
   *   event is posted.
   * @param postDrainCallback  Callback that will be called during the queue
   *   drain phase. See Notifier for more details.
   */
  MessageQueue(
      size_t capacity,
      std::function<void(T&&)> onMessage,
      size_t noNotifyRate,
      int64_t waitThreshold,
      Notifier::NowUsecFunc nowFunc,
      std::function<void()> notifyCallback,
      std::function<bool(bool)> postDrainCallback = nullptr)
      : queue_(capacity),
        onMessage_(std::move(onMessage)),
        notifier_(
            noNotifyRate,
            waitThreshold,
            nowFunc,
            std::move(postDrainCallback)),
        handler_(*this),
        notifyCallback_(std::move(notifyCallback)) {
    efd_ = eventfd(0, EFD_CLOEXEC | EFD_NONBLOCK | EFD_SEMAPHORE);
    PCHECK(efd_ >= 0);
  }

  /**
   * Must be called from the event base thread.
   */
  void attachEventBase(folly::VirtualEventBase& evb) {
    handler_.initHandler(
        &evb.getEventBase(), folly::NetworkSocket::fromFd(efd_));
    handler_.registerHandler(
        folly::EventHandler::READ | folly::EventHandler::PERSIST);

    if (notifier_.noNotifyRate() > 0) {
      waitTimeout_ = folly::AsyncTimeout::schedule(
          std::chrono::milliseconds(kWakeupEveryMs),
          evb.getEventBase(),
          [this]() noexcept {
            drain();
            notifier_.maybeUpdatePeriod();
            waitTimeout_->scheduleTimeout(kWakeupEveryMs);
          });
    }

    class MessageQueueDrainCallback : public folly::EventBase::LoopCallback {
     public:
      MessageQueueDrainCallback(folly::EventBase& evb__, MessageQueue& queue)
          : evb_(evb__), queue_(queue) {
        evb_.runBeforeLoop(this);
      }

      void runLoopCallback() noexcept override {
        queue_.drain();
        evb_.runBeforeLoop(this);
      }

     private:
      folly::EventBase& evb_;
      MessageQueue& queue_;
    };

    queueDrainCallback_ =
        std::make_unique<MessageQueueDrainCallback>(evb.getEventBase(), *this);

    evb.runOnDestruction([queueDrainCallback = queueDrainCallback_]() {
      queueDrainCallback->cancelLoopCallback();
    });
  }

  size_t currentNotifyPeriod() const noexcept {
    return notifier_.currentNotifyPeriod();
  }

  /**
   * Must be called from the event base thread.
   * Manually drains the queue, calling the callback on any remaining messages.
   * Note: the user must guarantee that the queue is empty on destruction.
   */
  void drain() {
    notifier_.drainWhileNonEmpty([this]() { drainImpl(); });
  }

  ~MessageQueue() {
    if (queueDrainCallback_) {
      queueDrainCallback_->cancelLoopCallback();
    }
    handler_.unregisterHandler();
    if (efd_ >= 0) {
      PCHECK(folly::closeNoInt(efd_) == 0);
    }
  }

  /**
   * Put a new element into the queue. Can be called from any thread.
   * Allows inplace construction of the message.
   * Will block if queue is full until the reader catches up.
   */
  template <class... Args>
  void blockingWrite(Args&&... args) noexcept {
    blockingWriteNoNotify(std::forward<Args>(args)...);
    if (notifier_.shouldNotify()) {
      doNotify();
    }
  }

  /**
   * Similar to blockingWrite(), except that it used the relaxed notification
   * semantics. See Notifier class in this file for more details.
   */
  template <class... Args>
  void blockingWriteRelaxed(Args&&... args) noexcept {
    blockingWriteNoNotify(std::forward<Args>(args)...);
    if (notifier_.shouldNotifyRelaxed()) {
      doNotify();
    }
  }

  /**
   * Similar to blockingWrite(), except that it does not guarantee to notify the
   * consumer thread. The caller is responsible for eventually calling
   * notifyRelaxed().
   */
  template <class... Args>
  void blockingWriteNoNotify(Args&&... args) noexcept {
    if (!queue_.writeIfNotFull(std::forward<Args>(args)...)) {
      // If we block here and the consumer is asleep, the caller has no chance
      // to notify it, causing a deadlock. Force a notification in this case
      // before blocking.
      VLOG(2) << "MessageQueue full, forcing notification";
      if (notifier_.shouldNotify()) {
        doNotify();
      }
      queue_.blockingWrite(std::forward<Args>(args)...);
    }
  }

  /**
   * Notify the EventBase thread that there's work pending in the queue.
   * Uses relaxed notification semantics. See Notifier class in this file for
   * more details.
   */
  void notifyRelaxed() noexcept {
    if (notifier_.shouldNotifyRelaxed()) {
      doNotify();
    }
  }

  bool isFull() const noexcept {
    return queue_.isFull();
  }

 private:
  static constexpr int64_t kWakeupEveryMs = 2;
  folly::MPMCQueue<T> queue_;
  std::function<void(T&&)> onMessage_;
  Notifier notifier_;

  class EventHandler : public folly::EventHandler {
   public:
    explicit EventHandler(MessageQueue& q) : parent_(q) {}
    void handlerReady(uint16_t /* events */) noexcept final {
      parent_.onEvent();
    }

   private:
    MessageQueue& parent_;
  };

  EventHandler handler_;
  std::unique_ptr<folly::AsyncTimeout> waitTimeout_;
  std::function<void()> notifyCallback_;
  int efd_{-1};

  void onEvent() {
    uint64_t value;
    auto res = ::read(efd_, &value, sizeof(value));
    CHECK(res == sizeof(value));
    // Note, we use this event fd purely for waking up a thread in epoll_wait.
    // It's usually executed immediately after runBeforeLoop callback, thus
    // no need to drain again.
  }

  void doNotify() {
    assert(efd_ >= 0);
    uint64_t n = 1;
    PCHECK(::write(efd_, &n, sizeof(n)) == sizeof(n));
    if (notifyCallback_) {
      notifyCallback_();
    }
  }

  void drainImpl() {
    T message;
    while (queue_.read(message)) {
      onMessage_(std::move(message));
      notifier_.bumpMessages();
    }
  }

  std::shared_ptr<folly::EventBase::LoopCallback> queueDrainCallback_;
};

// Static member definition
template <class T>
constexpr int64_t MessageQueue<T>::kWakeupEveryMs;

} // namespace memcache
} // namespace facebook
