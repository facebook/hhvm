/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <folly/Synchronized.h>
#include "watchman/thirdparty/jansson/jansson.h"
#include "watchman/watchman_string.h"
#include "watchman/watchman_system.h"

#include <deque>
#include <functional>
#include <vector>

namespace watchman {

class Publisher : public std::enable_shared_from_this<Publisher> {
 public:
  struct Item {
    Item(uint64_t s, json_ref p) : serial{s}, payload{std::move(p)} {}

    // copy of nextSerial_ at the time this was created.
    // The item can be released when all subscribers have
    // observed this serial number.
    uint64_t serial;
    json_ref payload;
  };

  // Generic callback that subscribers can register to arrange
  // to be woken up when something is published
  using Notifier = std::function<void()>;

  // Each subscriber is represented by one of these
  class Subscriber : public std::enable_shared_from_this<Subscriber> {
    // The serial of the last Item to be consumed by
    // this subscriber.
    uint64_t serial_;
    // Subscriber keeps the publisher alive so that no Items are lost
    // if the Publisher is released before all of the subscribers.
    std::shared_ptr<Publisher> publisher_;
    // Advising the subscriber that there may be more items available
    Notifier notify_;
    // Information for debugging purposes
    const std::optional<json_ref> info_;

   public:
    ~Subscriber();
    Subscriber(
        std::shared_ptr<Publisher> pub,
        Notifier notify,
        const std::optional<json_ref>& info);
    Subscriber(const Subscriber&) = delete;

    // Returns all as yet unseen published items for this subscriber.
    void getPending(std::vector<std::shared_ptr<const Item>>& pending);

    uint64_t getSerial() const {
      return serial_;
    }

    Notifier& getNotify() {
      return notify_;
    }

    const std::optional<json_ref>& getInfo() const {
      return info_;
    }
  };

  // Register a new subscriber.
  // When the Subscriber object is released, the registration is
  // automatically removed.
  std::shared_ptr<Subscriber> subscribe(
      Notifier notify,
      const std::optional<json_ref>& info = std::nullopt);

  // Returns true if there are any subscribers.
  // This is racy and intended to be used to gate building a payload
  // if there are no current subscribers.
  bool hasSubscribers() const;

  // Enqueue a new item, but only if there are subscribers.
  // Returns true if the item was queued.
  bool enqueue(json_ref&& payload);

  // Return debugging info useful for state inspection.
  json_ref getDebugInfo() const;

 private:
  struct state {
    state() = default;
    state(const state&) = delete;
    // Serial number to use for the next Item
    uint64_t nextSerial{1};
    // The stream of Items
    std::deque<std::shared_ptr<const Item>> items;
    // The subscribers
    std::vector<std::weak_ptr<Subscriber>> subscribers;

    void collectGarbage();
    void enqueue(json_ref&& payload);
  };
  folly::Synchronized<state> state_;

  friend class Subscriber;
};

// Equivalent to calling getPending on up to two Subscriber and
// joining the resultant vectors together.
void getPending(
    std::vector<std::shared_ptr<const Publisher::Item>>& pending,
    const std::shared_ptr<Publisher::Subscriber>& sub1,
    const std::shared_ptr<Publisher::Subscriber>& sub2);
} // namespace watchman
