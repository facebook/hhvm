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

#include <memory>

#include <folly/Range.h>
#include <folly/io/async/EventBase.h>

namespace folly {

class EventBase;
class EventBaseBackendBase;
class EventBaseManager;
class ScopedEventBaseThread;

class EventBaseThread {
 public:
  EventBaseThread();
  explicit EventBaseThread(
      bool autostart,
      EventBaseManager* ebm = nullptr,
      folly::StringPiece threadName = folly::StringPiece());
  EventBaseThread(
      bool autostart,
      EventBase::Options eventBaseOptions,
      EventBaseManager* ebm = nullptr,
      folly::StringPiece threadName = folly::StringPiece());
  explicit EventBaseThread(EventBaseManager* ebm);
  ~EventBaseThread();

  EventBaseThread(EventBaseThread const&) = delete;
  EventBaseThread& operator=(EventBaseThread const&) = delete;
  EventBaseThread(EventBaseThread&&) noexcept;
  EventBaseThread& operator=(EventBaseThread&&) noexcept;

  EventBase* getEventBase() const;

  bool running() const;
  void start(folly::StringPiece threadName = folly::StringPiece());
  void stop();

 private:
  EventBaseManager* ebm_;
  EventBase::Options ebOpts_;
  std::unique_ptr<ScopedEventBaseThread> th_;
};
} // namespace folly
