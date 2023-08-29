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

#include <forward_list>

#include <folly/ExceptionWrapper.h>
#include <folly/experimental/coro/Task.h>
#include <folly/io/async/EventBase.h>
#include <thrift/lib/cpp/concurrency/ThreadManager.h>

namespace apache {
namespace thrift {
namespace detail {

enum class InternalPriority;

struct InteractionTask {
  std::unique_ptr<concurrency::Runnable> task;
  concurrency::ThreadManager::ExecutionScope scope;
  std::shared_ptr<folly::RequestContext> context;

  explicit InteractionTask(
      std::unique_ptr<concurrency::Runnable>&& t,
      concurrency::ThreadManager::ExecutionScope sc)
      : task(std::move(t)),
        scope(sc),
        context(folly::RequestContext::saveContext()) {}
};

using InteractionTaskQueue =
    std::queue<InteractionTask, std::list<InteractionTask>>;
} // namespace detail

class InteractionId {
 public:
  InteractionId() : id_(0) {}
  InteractionId(InteractionId&& other) noexcept {
    id_ = other.id_;
    other.release();
  }
  InteractionId& operator=(InteractionId&& other) {
    if (this != &other) {
      CHECK_EQ(id_, 0) << "Interactions must always be terminated";
      id_ = other.id_;
      other.release();
    }
    return *this;
  }
  InteractionId(const InteractionId&) = delete;
  InteractionId& operator=(const InteractionId&) = delete;
  ~InteractionId() {
    CHECK_EQ(id_, 0) << "Interactions must always be terminated";
  }

  operator int64_t() const { return id_; }

 private:
  InteractionId(int64_t id) : id_(id) {}

  void release() { id_ = 0; }

  int64_t id_;

  friend class RequestChannel;
};

enum class InteractionReleaseEvent {
  NORMAL,
  STREAM_TRANSFER,
  STREAM_END,
};

class TilePtr;

class Tile {
 public:
  virtual ~Tile() { DCHECK_EQ(refCount_, 0); }

#if FOLLY_HAS_COROUTINES
  // Called as soon as termination signal is received
  // Destructor may or may not run as soon as this completes
  // Not called if connection closes before termination received
  virtual folly::coro::Task<void> co_onTermination();
#endif

 private:
  // Only moves in arg when it returns true
  virtual bool maybeEnqueue(
      std::unique_ptr<concurrency::Runnable>&& task,
      const concurrency::ThreadManager::ExecutionScope& scope);

  static void onTermination(TilePtr tile, folly::EventBase& eb);

  virtual bool runsInEventBase() { return false; }

  void incRef(folly::EventBase& eb) {
    eb.dcheckIsInEventBaseThread();
    ++refCount_;
  }
  void decRef(folly::EventBase& eb, InteractionReleaseEvent event);

  size_t refCount_{0};
  folly::Executor::KeepAlive<concurrency::ThreadManager> tm_;
  folly::Executor::KeepAlive<> executor_{}; // Used only for ResourcePools
  friend class TilePromise;
  friend class TilePtr;
  friend class TileStreamGuard;
  friend class GeneratedAsyncProcessorBase;
};

class SerialInteractionTile : public Tile {
 private:
  bool maybeEnqueue(
      std::unique_ptr<concurrency::Runnable>&& task,
      const concurrency::ThreadManager::ExecutionScope& scope) override;

  detail::InteractionTaskQueue taskQueue_;
  bool hasActiveRequest_{false};
  friend class Tile;
};

class EventBaseTile : public Tile {
 private:
  bool runsInEventBase() final { return true; }
};

class TilePromise final : public Tile {
 public:
  explicit TilePromise(bool isFactoryFunction)
      : factoryPending_(isFactoryFunction) {}

  void fulfill(
      Tile& tile, concurrency::ThreadManager* tm, folly::EventBase& eb);

  void fulfill(
      Tile& tile, folly::Executor::KeepAlive<> executor, folly::EventBase& eb);

  void failWith(folly::exception_wrapper ew, const std::string& exCode);

#if FOLLY_HAS_COROUTINES
  folly::coro::Task<void> co_onTermination() override;
#endif

 private:
  bool maybeEnqueue(
      std::unique_ptr<concurrency::Runnable>&& task,
      const concurrency::ThreadManager::ExecutionScope& scope) override;

  detail::InteractionTaskQueue continuations_;
  bool terminated_{false};
  bool factoryPending_;

  struct FactoryException {
    folly::exception_wrapper ew;
    std::string exCode;
  };
  std::unique_ptr<FactoryException> factoryEx_;
};

class TilePtr {
 public:
  TilePtr() = default;
  TilePtr(Tile* tile, folly::Executor::KeepAlive<folly::EventBase> eb)
      : tile_(tile), eb_(std::move(eb)) {
    tile_->incRef(*eb_);
  }

  TilePtr(TilePtr&& that) noexcept
      : tile_(std::exchange(that.tile_, nullptr)), eb_(std::move(that.eb_)) {}
  TilePtr& operator=(TilePtr&& that) {
    if (this != &that) {
      release(InteractionReleaseEvent::NORMAL);
    }
    tile_ = std::exchange(that.tile_, nullptr);
    eb_ = std::move(that.eb_);
    return *this;
  }

  ~TilePtr() { release(InteractionReleaseEvent::NORMAL); }

  explicit operator bool() const { return tile_; }

  Tile* get() const { return tile_; }
  Tile& operator*() const { return *tile_; }
  Tile* operator->() const { return tile_; }

 private:
  void release(InteractionReleaseEvent event);

  Tile* tile_{nullptr};
  folly::Executor::KeepAlive<folly::EventBase> eb_;
  friend class TileStreamGuard;
};

class TileStreamGuard {
 public:
  TileStreamGuard() = default;

  TileStreamGuard(TileStreamGuard&& tile) noexcept = default;
  TileStreamGuard& operator=(TileStreamGuard&& that) {
    if (this != &that) {
      tile_.release(InteractionReleaseEvent::STREAM_END);
    }
    tile_ = std::move(that.tile_);
    return *this;
  }

  ~TileStreamGuard() { tile_.release(InteractionReleaseEvent::STREAM_END); }

  // must call in eb thread
  static TileStreamGuard transferFrom(TilePtr&& ptr) {
    return TileStreamGuard(std::move(ptr));
  }

 private:
  explicit TileStreamGuard(TilePtr&& ptr);
  TilePtr tile_;
};

class InteractionTask {
 public:
  virtual ~InteractionTask() = default;
  virtual void setTile(TilePtr&&) = 0;
  virtual void failWith(folly::exception_wrapper ew, std::string exCode) = 0;
  virtual void acceptIntoResourcePool(int8_t) {
    LOG(FATAL) << "Unimplemented acceptIntoResourcePool() method";
  }
};

} // namespace thrift
} // namespace apache
