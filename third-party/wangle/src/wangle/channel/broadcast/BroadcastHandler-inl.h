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

namespace wangle {

template <typename T, typename R>
void BroadcastHandler<T, R>::read(Context*, T data) {
  onData(data);
  forEachSubscriber([&](Subscriber<T, R>* s) { s->onNext(data); });
}

template <typename T, typename R>
void BroadcastHandler<T, R>::readEOF(Context*) {
  forEachSubscriber([&](Subscriber<T, R>* s) { s->onCompleted(); });
  subscribers_.clear();
  closeIfIdle();
}

template <typename T, typename R>
void BroadcastHandler<T, R>::readException(
    Context*,
    folly::exception_wrapper ex) {
  LOG(ERROR) << "Error while reading from upstream for broadcast: "
             << exceptionStr(ex);

  forEachSubscriber([&](Subscriber<T, R>* s) { s->onError(ex); });
  subscribers_.clear();
  closeIfIdle();
}

template <typename T, typename R>
uint64_t BroadcastHandler<T, R>::subscribe(Subscriber<T, R>* subscriber) {
  auto subscriptionId = nextSubscriptionId_++;
  subscribers_[subscriptionId] = subscriber;
  onSubscribe(subscriber);
  return subscriptionId;
}

template <typename T, typename R>
void BroadcastHandler<T, R>::unsubscribe(uint64_t subscriptionId) {
  auto iter = subscribers_.find(subscriptionId);
  if (iter == subscribers_.end()) {
    return;
  }

  onUnsubscribe(iter->second);
  subscribers_.erase(iter);
  closeIfIdle();
}

template <typename T, typename R>
void BroadcastHandler<T, R>::closeIfIdle() {
  if (subscribers_.empty()) {
    // No more subscribers. Clean up.
    // This will delete the broadcast from the pool.
    this->close(this->getContext());
  }
}

template <typename T, typename R>
uint64_t BroadcastHandler<T, R>::getArbitraryIdentifier() {
  static std::atomic<uint64_t> identifierCounter{42};
  return identifier_ ? identifier_ : (identifier_ = ++identifierCounter);
}

} // namespace wangle
