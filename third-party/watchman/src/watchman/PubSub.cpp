/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "watchman/PubSub.h"
#include <algorithm>
#include <iterator>

namespace watchman {

Publisher::Subscriber::Subscriber(
    std::shared_ptr<Publisher> pub,
    Notifier notify,
    const std::optional<json_ref>& info)
    : serial_(0),
      publisher_(std::move(pub)),
      notify_(notify),
      info_(std::move(info)) {}

Publisher::Subscriber::~Subscriber() {
  // In the loop below we may own a reference to some other
  // Subscriber instance.  That is fine, but we need to take
  // care: if we end up with the last reference to that subscriber
  // we will end up calling its destructor and effective recurse
  // and attempt to acquire the wlock.   We therefore need to
  // defer releasing any of the shared_ptr's that we lock in
  // the loop below until after we have released the wlock.
  std::vector<std::shared_ptr<Subscriber>> subscribers;

  {
    auto wlock = publisher_->state_.wlock();
    auto it = wlock->subscribers.begin();
    while (it != wlock->subscribers.end()) {
      auto sub = it->lock();
      // Prune vacated weak_ptr's or those that point to us
      if (!sub || sub.get() == this) {
        it = wlock->subscribers.erase(it);
      } else {
        ++it;
        // Defer releasing the sub reference until after we've
        // release the wlock!
        subscribers.emplace_back(std::move(sub));
      }
    }
    // Take this opportunity to reap anything that is no longer
    // referenced now that we've removed some subscriber(s)
    wlock->collectGarbage();
  }

  // It is now safe for subscribers to be torn down and release
  // any references we took ownership of in the loop above.
}

void Publisher::Subscriber::getPending(
    std::vector<std::shared_ptr<const Item>>& pending) {
  {
    auto rlock = publisher_->state_.rlock();
    auto& items = rlock->items;

    if (items.empty()) {
      return;
    }

    // First we walk back to find the end of the range that
    // we have seen previously.
    int firstIndex;
    for (firstIndex = int(items.size()) - 1; firstIndex >= 0; --firstIndex) {
      if (items[firstIndex]->serial <= serial_) {
        break;
      }
    }

    // We found the item before the one we really want, so
    // increment the index; we'll copy the remaining items.
    ++firstIndex;
    bool updated = false;

    while (firstIndex < int(items.size())) {
      pending.push_back(items[firstIndex]);
      ++firstIndex;
      updated = true;
    }

    if (updated) {
      serial_ = pending.back()->serial;
    }

    return;
  }
}

void getPending(
    std::vector<std::shared_ptr<const Publisher::Item>>& items,
    const std::shared_ptr<Publisher::Subscriber>& sub1,
    const std::shared_ptr<Publisher::Subscriber>& sub2) {
  if (sub1) {
    sub1->getPending(items);
  }
  if (sub2) {
    sub2->getPending(items);
  }
}

std::shared_ptr<Publisher::Subscriber> Publisher::subscribe(
    Notifier notify,
    const std::optional<json_ref>& info) {
  auto sub =
      std::make_shared<Publisher::Subscriber>(shared_from_this(), notify, info);
  state_.wlock()->subscribers.emplace_back(sub);
  return sub;
}

bool Publisher::hasSubscribers() const {
  return !state_.rlock()->subscribers.empty();
}

void Publisher::state::collectGarbage() {
  if (items.empty()) {
    return;
  }

  uint64_t minSerial = std::numeric_limits<uint64_t>::max();
  for (auto& it : subscribers) {
    auto sub = it.lock();
    if (sub) {
      minSerial = std::min(minSerial, sub->getSerial());
    }
  }

  while (!items.empty() && items.front()->serial < minSerial) {
    items.pop_front();
  }
}

bool Publisher::enqueue(json_ref&& payload) {
  std::vector<std::shared_ptr<Subscriber>> subscribers;

  {
    auto wlock = state_.wlock();

    // We need to collect live references for the notify portion,
    // but since we're holding the wlock, take this opportunity to
    // detect and prune dead subscribers and clean up some garbage.
    auto it = wlock->subscribers.begin();
    while (it != wlock->subscribers.end()) {
      auto sub = it->lock();
      // Prune vacated weak_ptr's
      if (!sub) {
        it = wlock->subscribers.erase(it);
      } else {
        ++it;
        // Remember that live reference so that we can notify
        // outside of the lock below.
        subscribers.emplace_back(std::move(sub));
      }
    }

    wlock->collectGarbage();

    if (subscribers.empty()) {
      return false;
    }

    wlock->items.emplace_back(
        std::make_shared<Item>(wlock->nextSerial++, std::move(payload)));
  }

  // and notify them outside of the lock
  for (auto& sub : subscribers) {
    auto& n = sub->getNotify();
    if (n) {
      n();
    }
  }
  return true;
}

json_ref Publisher::getDebugInfo() const {
  auto ret = json_object();

  auto rlock = state_.rlock();
  ret.set("next_serial", json_integer(rlock->nextSerial));

  std::vector<json_ref> subscribers_arr;

  for (auto& sub_ref : rlock->subscribers) {
    auto sub = sub_ref.lock();
    if (sub) {
      auto sub_json = json_object({
          {"serial", json_integer(sub->getSerial())},
      });
      if (auto& info = sub->getInfo()) {
        sub_json.set("info", json_ref(*info));
      }
      subscribers_arr.push_back(std::move(sub_json));
    } else {
      // This is a subscriber that is now dead. It will be cleaned up the next
      // time enqueue is called.
    }
  }

  ret.set("subscribers", json_array(std::move(subscribers_arr)));

  std::vector<json_ref> items_arr;

  for (auto& item : rlock->items) {
    auto item_json = json_object(
        {{"serial", json_integer(item->serial)}, {"payload", item->payload}});
    items_arr.emplace_back(item_json);
  }

  ret.set("items", json_array(std::move(items_arr)));

  return ret;
}
} // namespace watchman
