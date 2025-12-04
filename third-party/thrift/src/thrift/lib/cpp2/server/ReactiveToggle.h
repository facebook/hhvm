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

#include <atomic>
#include <memory>

#include <folly/Function.h>
#include <folly/IntrusiveList.h>
#include <folly/Synchronized.h>
#include <folly/synchronization/RelaxedAtomic.h>

namespace apache::thrift::server {

// A simple context class that is
// shared for communication between the
// toggle and the source
struct ReactiveToggleContext {
  struct CallbackNode {
    folly::Function<void(bool)> func;
    bool removed{false};

    // list hook for intrusive list
    folly::IntrusiveListHook listHook_;
    using List = folly::IntrusiveList<CallbackNode, &CallbackNode::listHook_>;

    CallbackNode() = delete;
    explicit CallbackNode(folly::Function<void(bool)> func)
        : func(std::move(func)) {}

    ~CallbackNode() { DCHECK(removed); }
    CallbackNode(const CallbackNode&) = delete;
    CallbackNode& operator=(const CallbackNode&) = delete;
    CallbackNode(CallbackNode&&) = delete;
    CallbackNode& operator=(CallbackNode&&) = delete;
  };

  // This is used as the indicator of whether
  // a reactive toggle source is currently on
  folly::relaxed_atomic<bool> on{true};

  // An intrusive linked list storing all the callbacks
  // Not using default SharedMutex because we'll only
  // be having one reader
  // e.g. only one thread will invoke the callbacks
  folly::Synchronized<CallbackNode::List, std::mutex> callbacks;
};

class ReactiveToggle;

// Usage:
//
// // source is turned to on by default
// ReactiveToggleSource source;
//
// // grab a toggle from the source
// ReactiveToggle toggle(source);
//
// // setting the callback into toggle
// toggle.addCallback(std::move(f));
//
// // some external interface consumes it
// // addCallback() can also be called by the
// // external interface itself
// someObject.setToggle(std::move(toggle));
//
// // now we can start the acting notifying
// source.set(false);
//
// // multiple toggles can be attached
// auto toggle1 = ReactiveToggle(source);
// auto toggle2 = ReactiveToggle(source);
// toggle1.addCallback(std::move(func));
// toggle2.addCallback(std::move(func));
class ReactiveToggleSource {
 public:
  friend class ReactiveToggle;

  ReactiveToggleSource();

  // move-only
  ReactiveToggleSource(const ReactiveToggleSource&) = delete;
  ReactiveToggleSource& operator=(const ReactiveToggleSource&) = delete;

  ReactiveToggleSource(ReactiveToggleSource&&) = default;
  ReactiveToggleSource& operator=(ReactiveToggleSource&&) = delete;

  ~ReactiveToggleSource() = default;

  // not thread-safe
  // When called, callbacks in hooked ReactiveToggle
  // will be run for one round
  void set(bool on);

 private:
  // initialized when a source is created
  std::shared_ptr<ReactiveToggleContext> context_;
};

// The reason why we want this observer-like primitive instead
// of reusing things like folly::observer is that we want better
// performance + inline execution of the callbacks.
class ReactiveToggle {
 public:
  class CallbackHandle {
   public:
    friend class ReactiveToggle;

    // move-only
    CallbackHandle(const CallbackHandle&) = delete;
    CallbackHandle& operator=(const CallbackHandle&) = delete;
    CallbackHandle(CallbackHandle&&) = default;
    CallbackHandle& operator=(CallbackHandle&&) = delete;
    ~CallbackHandle();

    void cancelAndJoin();

   private:
    CallbackHandle(
        std::shared_ptr<ReactiveToggleContext>, folly::Function<void(bool)>);

    // CallbackHandle will be responsible for managing the lifetime of
    // callback node. It's decoupled from ReactiveToggleContext.
    std::unique_ptr<ReactiveToggleContext::CallbackNode> node_;
    std::shared_ptr<ReactiveToggleContext> context_;
  };

  explicit ReactiveToggle(const ReactiveToggleSource& source);

  bool get() const;

  // addCallback and ReactiveToggleSource::set should not
  // be called concurrently.
  CallbackHandle addCallback(folly::Function<void(bool)>);

 private:
  std::shared_ptr<ReactiveToggleContext> context_;
};

} // namespace apache::thrift::server
