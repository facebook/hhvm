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

#include <thrift/lib/cpp2/async/Interaction.h>

namespace apache {
namespace thrift {

bool Tile::__fbthrift_maybeEnqueue(
    std::unique_ptr<concurrency::Runnable>&&,
    const concurrency::ThreadManager::ExecutionScope&) {
  return false;
}

void Tile::decRef(folly::EventBase& eb, InteractionReleaseEvent event) {
  eb.dcheckIsInEventBaseThread();
  DCHECK_GT(refCount_, 0u);

  if (event != InteractionReleaseEvent::STREAM_END) {
    if (auto serial = dynamic_cast<SerialInteractionTile*>(this)) {
      auto& queue = serial->taskQueue_;
      if (!queue.empty()) {
        DCHECK_GT(
            refCount_ + (event == InteractionReleaseEvent::STREAM_TRANSFER),
            queue.size());
        auto& item = queue.front();
        folly::RequestContextScopeGuard rctx(item.context);
        if (executor_) {
          auto& serverTask = dynamic_cast<InteractionTask&>(*item.task);
          serverTask.acceptIntoResourcePool(folly::Executor::HI_PRI);
        } else {
          tm_->getKeepAlive(
                 std::move(item.scope),
                 concurrency::ThreadManager::Source::INTERNAL)
              ->add([task = std::move(item.task)]() { task->run(); });
        }
        queue.pop();
      } else {
        serial->hasActiveRequest_ = false;
      }
    }
  }

  if (event != InteractionReleaseEvent::STREAM_TRANSFER && --refCount_ == 0) {
    if (executor_) {
      executor_->add([ptr = std::unique_ptr<Tile>(this)]() {});
    } else if (tm_) {
      std::move(tm_).add([ptr = std::unique_ptr<Tile>(this)](auto&&) {});
    } else {
      delete this;
    }
  }
}

#if FOLLY_HAS_COROUTINES
// Called as soon as termination signal is received
// Destructor may or may not run as soon as this completes
// Not called if connection closes before termination received
folly::coro::Task<void> Tile::co_onTermination() {
  co_return;
}
#endif

void Tile::__fbthrift_onTermination(
    FOLLY_MAYBE_UNUSED TilePtr ptr, FOLLY_MAYBE_UNUSED folly::EventBase& eb) {
#if FOLLY_HAS_COROUTINES
  eb.dcheckIsInEventBaseThread();
  auto* tile = ptr.get();
  if (tile->executor_) {
    tile->co_onTermination()
        .scheduleOn(tile->executor_)
        .start([ptr = std::move(ptr)](auto&&) {});
  } else if (tile->tm_) {
    tile->co_onTermination().scheduleOn(tile->tm_).start(
        [ptr = std::move(ptr)](auto&&) {});
  } else {
    tile->co_onTermination().scheduleOn(&eb).startInlineUnsafe(
        [ptr = std::move(ptr)](auto&&) {});
  }
#endif
}

bool TilePromise::__fbthrift_maybeEnqueue(
    std::unique_ptr<concurrency::Runnable>&& task,
    const concurrency::ThreadManager::ExecutionScope& scope) {
  // If the first request is a factory method we mustn't enqueue it
  if (std::exchange(factoryPending_, false)) {
    return false;
  }

  // If the factory failed, accept the task and immediately fail it
  if (factoryEx_) {
    dynamic_cast<InteractionTask&>(*task).failWith(
        factoryEx_->ew, factoryEx_->exCode);
    return true;
  }

  continuations_.emplace(std::move(task), scope);
  return true;
}

void TilePromise::fulfill(
    Tile& tile, concurrency::ThreadManager* tm, folly::EventBase& eb) {
  if (tile.__fbthrift_runsInEventBase()) {
    tm = nullptr;
  } else {
    DCHECK(tm) << "thread=eb factory function can only create "
               << "process_in_event_base interaction";
    tile.tm_ = tm;
  }
  if (terminated_) {
    Tile::__fbthrift_onTermination({&tile, &eb}, eb);
  }

  // Inline destruction of this is possible at the setTile()
  auto continuations = std::move(continuations_);
  while (!continuations.empty()) {
    auto& item = continuations.front();
    folly::RequestContextScopeGuard rctx(item.context);
    dynamic_cast<InteractionTask&>(*item.task).setTile({&tile, &eb});
    if (!tile.__fbthrift_maybeEnqueue(std::move(item.task), item.scope)) {
      if (tm) {
        tm->getKeepAlive(
              std::move(item.scope),
              concurrency::ThreadManager::Source::EXISTING_INTERACTION)
            ->add([task = std::move(item.task)]() mutable { task->run(); });
      } else {
        item.task->run();
      }
    }
    continuations.pop();
  }
}

void TilePromise::failWith(
    folly::exception_wrapper ew, const std::string& exCode) {
  auto continuations = std::move(continuations_);
  factoryEx_ =
      folly::copy_to_unique_ptr(TilePromise::FactoryException{ew, exCode});
  while (!continuations.empty()) {
    auto& item = continuations.front();
    folly::RequestContextScopeGuard rctx(item.context);
    dynamic_cast<InteractionTask&>(*item.task).failWith(ew, exCode);
    continuations.pop();
  }
}

#if FOLLY_HAS_COROUTINES
folly::coro::Task<void> TilePromise::co_onTermination() {
  DCHECK(!tm_ && !executor_);
  terminated_ = true;
  co_return;
}
#endif

bool SerialInteractionTile::__fbthrift_maybeEnqueue(
    std::unique_ptr<concurrency::Runnable>&& task,
    const concurrency::ThreadManager::ExecutionScope& scope) {
  if (hasActiveRequest_) {
    taskQueue_.emplace(std::move(task), scope);
    return true;
  }

  hasActiveRequest_ = true;
  return false;
}

void TilePtr::release(InteractionReleaseEvent event) {
  if (tile_) {
    if (eb_->inRunningEventBaseThread()) {
      tile_->decRef(*eb_, event);
      tile_ = nullptr;
    } else {
      std::move(eb_).add([tile = std::exchange(tile_, nullptr),
                          event](auto&& eb) { tile->decRef(*eb, event); });
    }
  }
}

TileStreamGuard::TileStreamGuard(TilePtr&& ptr) : tile_(std::move(ptr)) {
  if (auto tile = tile_.get()) {
    tile_->decRef(*tile_.eb_, InteractionReleaseEvent::STREAM_TRANSFER);
  }
}

void TilePromise::fulfill(
    Tile& tile, folly::Executor::KeepAlive<> executor, folly::EventBase& eb) {
  if (!tile.__fbthrift_runsInEventBase()) {
    DCHECK(executor) << "thread=eb factory function can only create "
                     << "process_in_event_base interaction";
    tile.executor_ = executor;
  }
  if (terminated_) {
    Tile::__fbthrift_onTermination({&tile, &eb}, eb);
  }

  // Inline destruction of this is possible at the setTile()
  auto continuations = std::move(continuations_);
  while (!continuations.empty()) {
    auto& item = continuations.front();
    folly::RequestContextScopeGuard rctx(item.context);
    auto& serverTask = dynamic_cast<InteractionTask&>(*item.task);
    serverTask.setTile({&tile, &eb});
    if (!tile.__fbthrift_maybeEnqueue(std::move(item.task), item.scope)) {
      serverTask.acceptIntoResourcePool(folly::Executor::MID_PRI);
    }
    continuations.pop();
  }
}

} // namespace thrift
} // namespace apache
