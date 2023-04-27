/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include <fmt/core.h>
#include <folly/Synchronized.h>
#include <folly/futures/Future.h>
#include <chrono>
#include <deque>
#include <memory>
#include <unordered_map>
#include "watchman/WatchmanConfig.h"

namespace watchman {

/**
 * LRUCache implements a cache map with an LRU eviction policy.
 *
 * Items are retained in the cache until evicted.
 * Eviction occurs when attempting to insert a new item into
 * a full cache.  Eviction will pick the least-recently-used
 * item to erase.
 *
 * set(), get(), erase() methods are provided for performing
 * immediate operations on the cache contents.  A clear()
 * method allows purging the entire contents at once.
 *
 * An alternative get() API is provided that allows the cache
 * to be satisfied by some function returning a Future<ValueType>.
 * This API has some thundering herd protection built-in;
 * multiple requests for the same key will aggregate on one
 * pending node that will fulfill the request later.
 * Since this mode may experience failure the cache allows
 * setting an errorTTL as a negative cache TTL.  The failure
 * of the lookup will be remembered for the specified duration
 * before allowing a subsequent async get to be tried again.
 *
 * Thread safety: the cache exposes stored values to its client
 * via a Node type.  The Node is a const shared_ptr, making it
 * immutable to the client.  The invariant is that if a client
 * has a reference to a Node, the value() of the node can
 * not be mutated by some other client.  For pending lookups,
 * clients will never be handed a Node; only a Future<Node>.
 *
 * There is a single lock protecting all mutation of the cache
 * and its nodes.  Because the cache is LRU it needs to touch
 * a node as part of a lookup to ensure that it will not
 * be evicted prematurely.
 */

template <typename KeyType, typename ValueType>
class LRUCache;

// Some of these class names are a bit too generic to stash
// in the typical "detail" namespace, so we have a more specific
// container here.
namespace lrucache {
template <typename Node>
class TailQHead;

/** The node tracks the value in the cache.
 * We use a shared_ptr to manage its lifetime safely.
 * Clients of the cache will only ever be handed a
 * shared_ptr<const Node>.  Once published to a client,
 * the underlying value is immutable.
 */
template <typename KeyType, typename ValueType>
class Node {
  friend class TailQHead<Node<KeyType, ValueType>>;
  friend class LRUCache<KeyType, ValueType>;

 public:
  using PromiseList = std::deque<folly::Promise<std::shared_ptr<const Node>>>;

  // Construct a node via LRUCache::set()
  Node(const KeyType& key, ValueType&& value)
      : key_(key), value_(std::move(value)) {}

  // Construct a node using a getter function.
  // The value is empty and the promise list is initialized.
  explicit Node(const KeyType& key)
      : key_(key), promises_(std::make_unique<PromiseList>()) {}

  // Returns the underlying value.
  const ValueType& value() const {
    return value_.value();
  }

  // Returns the underlying value container.
  // This is useful if you want to test for an error in a Future<> get.
  const folly::Try<ValueType>& result() const {
    return value_;
  }

 private:
  // Obtain a future that will be ready when the pending
  // fetches are complete.
  folly::Future<std::shared_ptr<const Node>> subscribe() {
    promises_->emplace_back();
    return promises_->back().getFuture();
  }

  // Test whether an error result can be evicted
  bool expired(std::chrono::steady_clock::time_point now) const {
    return value_.hasException() && now >= deadline_;
  }

  // Address of next element
  Node* next_{nullptr};
  // Address of previous next element
  Node** addressOfPreviousNext_{nullptr};

  // The key
  KeyType key_;
  // The stored value
  folly::Try<ValueType> value_;

  // The collection of clients waiting for this node to be available.
  // This is a pointer so that we can minimize the space usage once
  // it has been fulfilled.
  std::unique_ptr<PromiseList> promises_;

  // Time after which this node is to be considered invalid
  std::chrono::steady_clock::time_point deadline_;
};

// A doubly-linked intrusive list through the cache nodes.
// O(1) append and O(1) removal.
// This is a non-owning list that is used to maintain the
// ordered sets used for eviction.
template <typename Node>
class TailQHead {
 public:
  // First element
  Node* first_;
  // Address of the "next" field in the last element
  Node** addressOfLastNext_;

 public:
  TailQHead() : first_(nullptr), addressOfLastNext_(&first_) {}

  void insertTail(Node* node) {
    node->next_ = nullptr;
    node->addressOfPreviousNext_ = addressOfLastNext_;
    *addressOfLastNext_ = node;
    addressOfLastNext_ = &node->next_;
  }

  void remove(Node* node) {
    if (!node->addressOfPreviousNext_) {
      // Not linked, NOP.
      return;
    }

    if (node->next_) {
      node->next_->addressOfPreviousNext_ = node->addressOfPreviousNext_;
    } else {
      addressOfLastNext_ = node->addressOfPreviousNext_;
    }
    *node->addressOfPreviousNext_ = node->next_;
  }

  // Bubble the node to the tail end of the list.
  void touch(Node* node) {
    remove(node);
    insertTail(node);
  }

  // Clear the list contents
  void clear() {
    first_ = nullptr;
    addressOfLastNext_ = &first_;
  }

  // Returns a pointer to the first element.
  // May be nullptr if the tailq is empty.
  Node* head() {
    return first_;
  }
};

struct Stats {
  // Number of times that a get resulted in a usable result
  size_t cacheHit{0};
  // Number of times that a get resulted in sharing a pending get
  size_t cacheShare{0};
  // Number of times that a get resulted in no usable result
  size_t cacheMiss{0};
  // Number of times that an item was evicted for any reason
  size_t cacheEvict{0};
  // Number of times that an item was inserted or replaced
  size_t cacheStore{0};
  // Number of times that a get was attempted
  size_t cacheLoad{0};
  // Number of times that an item was erased via erase()
  size_t cacheErase{0};
  // Number of times that the cache has been clear()'d
  size_t clearCount{0};

  void clear() {
    cacheHit = 0;
    cacheShare = 0;
    cacheMiss = 0;
    cacheEvict = 0;
    cacheStore = 0;
    cacheLoad = 0;
    cacheErase = 0;
    ++clearCount;
  }
};

// Factoring out the internal state struct here, as MSVC
// has a hard time compiling it otherwise.
template <typename KeyType, typename ValueType>
struct InternalState {
  using NodeType = Node<KeyType, ValueType>;
  // This owns the nodes in the map
  std::unordered_map<KeyType, std::shared_ptr<NodeType>> map;

  // Maintain some stats for cache introspection
  Stats stats;

  // To manage eviction we categorize a node into one of
  // three sets and link it into the appropriate tailq
  // below.  A node belongs in only one set at a time.

  // Nodes in the process of being fetched.  We cannot
  // evict anything in this set.  Note that we don't
  // strictly need to materialize this set (it can be
  // inferred from node properties) but it is a nice
  // invariant that any node always has a non-nullptr
  // result from whichQ().
  TailQHead<NodeType> lookupOrder;

  // Nodes with a successful result.  Eviction policy
  // is pure LRU.  Nodes in this set are touched on
  // access, causing the head to be the least recently
  // used node in the set, and the tail to be the
  // most recently used.
  TailQHead<NodeType> evictionOrder;

  // Nodes with an error result; these have a TTL
  // that we must respect as a way to manage load.
  // Nodes in this set are never touched; this means
  // that the head of this list is always the node
  // with the closest deadline.
  TailQHead<NodeType> erroredOrder;
};

} // namespace lrucache

struct CacheStats : public lrucache::Stats {
  CacheStats(const lrucache::Stats s, size_t size) : Stats(s), size(size) {}
  size_t size;
};

// The cache.  More information on this can be found at the
// top of this header file!
template <typename KeyType, typename ValueType>
class LRUCache {
 public:
  using NodeType = lrucache::Node<KeyType, ValueType>;

 private:
  using State = lrucache::InternalState<KeyType, ValueType>;
  using LockedState = typename folly::Synchronized<State>::LockedPtr;

 public:
  // Construct a cache with a defined limit and the specified
  // negative caching TTL duration.  The errorTTL is measured
  // from the start of the lookup, not its completion.
  LRUCache(
      size_t maxItems,
      std::chrono::milliseconds errorTTL,
      std::chrono::milliseconds fetchTimeout = std::chrono::seconds(300))
      : maxItems_(maxItems), errorTTL_(errorTTL), fetchTimeout_(fetchTimeout) {}

  LRUCache(
      Configuration&& cfg,
      const char* configPrefix,
      size_t defaultMaxItems,
      size_t errorTTLSeconds,
      size_t fetchTimeoutSeconds = 60)
      : maxItems_(cfg.getInt(
            fmt::format("{}_cache_size", configPrefix).c_str(),
            defaultMaxItems)),
        errorTTL_(std::chrono::seconds(cfg.getInt(
            fmt::format("{}_cache_error_ttl_seconds", configPrefix).c_str(),
            errorTTLSeconds))),
        fetchTimeout_(std::chrono::seconds(cfg.getInt(
            fmt::format("{}_fetch_timeout_seconds", configPrefix).c_str(),
            fetchTimeoutSeconds))) {}

  // No moving or copying
  LRUCache(const LRUCache&) = delete;
  LRUCache& operator=(const LRUCache&) = delete;
  LRUCache(LRUCache&&) = delete;
  LRUCache& operator=(LRUCache&&) = delete;

  // Lookup key and return the result.
  // If the key was not present then the result will be nullptr.
  // If there is an outstanding fetch in progress, throws an error;
  // you must consistently use the Future<> version of get() for
  // such a key.
  std::shared_ptr<const NodeType> get(
      const KeyType& key,
      std::chrono::steady_clock::time_point now =
          std::chrono::steady_clock::now()) {
    auto state = state_.wlock();
    ++state->stats.cacheLoad;

    auto it = state->map.find(key);
    if (it == state->map.end()) {
      ++state->stats.cacheMiss;
      return nullptr;
    }

    auto node = it->second;
    auto q = whichQ(node.get(), state);

    // Remove expired item
    if (node->expired(now)) {
      state->map.erase(it);
      q->remove(node.get());
      ++state->stats.cacheMiss;
      ++state->stats.cacheEvict;
      return nullptr;
    }

    if (q == &state->lookupOrder) {
      // There's no safe way to allow this mode of fetch to subscribe
      // to the pending lookup.  The caller should use the getter
      // flavor exclusively for this key.
      throw std::runtime_error("mixing Future getter with direct getter");
    }

    if (q == &state->evictionOrder) {
      q->touch(node.get());
    }

    ++state->stats.cacheHit;
    return node;
  }

  // Lookup key using a getter function.
  // If the key is not present in the cache, initiates a (possible async)
  // load of that value using the supplied getter function.
  // The getter function is a functor which behaves like:
  // std::function<Future<ValueType>(const KeyType&)>
  //
  // If the key is present but not yet satisfied, append a Promise
  // to the chain in the node and return a Future that will yield the
  // node once the lookup is complete.
  //
  // If the key is present and satisfied, return a Future that will
  // immediately yield the node.
  //
  // The purpose of this function is to reduce the exposure to
  // "thundering herd" style problems.  In watchman we tend to have
  // limited contention for the same key but may have a great many
  // requests for different keys; it is desirable to collapse 2x or 3x
  // the requests for the same key into a single request.
  //
  // It is not safe to delete the LRUCache while there are outstanding
  // getter calls.  It is the responsibility of the caller to ensure
  // that this does not happen.
  template <typename Func>
  folly::Future<std::shared_ptr<const NodeType>> get(
      const KeyType& key,
      Func&& getter,
      std::chrono::steady_clock::time_point now =
          std::chrono::steady_clock::now()) {
    std::shared_ptr<NodeType> node;
    auto future = folly::Future<std::shared_ptr<const NodeType>>::makeEmpty();

    // Only hold the lock on the state while we set up the map entry.
    {
      auto state = state_.wlock();
      ++state->stats.cacheLoad;

      auto it = state->map.find(key);
      if (it != state->map.end()) {
        node = it->second;

        auto q = whichQ(node.get(), state);

        if (!node->expired(now)) {
          // Only touch successful nodes
          if (q == &state->evictionOrder) {
            q->touch(node.get());
          }

          if (node->promises_) {
            // Not yet satisfied, so chain on a promise
            ++state->stats.cacheShare;
            return node->subscribe();
          }

          // Available now
          ++state->stats.cacheHit;
          return folly::makeFuture<std::shared_ptr<const NodeType>>(node);
        }

        // Remove invalid node.
        // We can't re-use it without introducing locking in
        // the node itself.
        q->remove(node.get());
        state->map.erase(it);
        ++state->stats.cacheEvict;
      }

      // Try to make a new node; this can fail if we are too full
      node = makeNode(state, now, key);

      // Insert into map and the appropriate tailq
      state->map.emplace(std::make_pair(node->key_, node));
      state->lookupOrder.insertTail(node.get());

      ++state->stats.cacheMiss;
      // and ensure that we capture a subscription before we release
      // the lock!
      future = node->subscribe();
    }

    // Arrange for the value to be populated.
    // This is done outside the lock in case the getter is a
    // simple callback that executes immediately in our context;
    // if that were to then try to operate on the cache, it would
    // deadlock.
    getter(key).thenTry([node, this, now](folly::Try<ValueType>&& result) {
      // We're going to steal the promises so that we can fulfil
      // them outside of the lock
      std::unique_ptr<typename NodeType::PromiseList> promises;
      {
        auto state = state_.wlock();
        node->value_ = std::move(result);

        if (whichQ(node.get(), state) != &state->lookupOrder) {
          // Should never happen...
          abort();
        }

        ++state->stats.cacheStore;

        // We're no longer looking this up; we'll put it in the
        // correct bucket just before we release the lock below.
        state->lookupOrder.remove(node.get());

        // We only need a TTL for errors
        if (node->value_.hasException()) {
          // Note that we don't account for the time it takes to
          // arrive at the error condition in this TTL.  This
          // is semi-deliberate; the for the sake of testing we
          // are passing `now` through from outside.
          node->deadline_ = now + errorTTL_;
        }

        // Steal the promises; we will fulfill them below
        // after we've released the lock.
        std::swap(promises, node->promises_);

        if (whichQ(node.get(), state) == &state->lookupOrder) {
          // Should never happen...
          abort();
        }

        // Now that the promises have been stolen, insert into
        // the appropriate queue.
        whichQ(node.get(), state)->insertTail(node.get());

        // If we were saturated at the start of the query, we may
        // not have been able to make room and may have taken on
        // more requests than the cache limits allow.  Now that we're
        // done we should be able to free up some of those entries,
        // so take a stab at that now.
        while (state->map.size() > maxItems_) {
          if (!evictOne(state, now, true)) {
            // We were not able to evict anything, so stop
            // trying.  We'll be over our cache size limit,
            // but there's not much more we can do.
            break;
          }
        }
      }

      // Wake up all waiters
      for (auto& p : *promises) {
        p.setValue(node);
      }
    });

    return std::move(future).within(fetchTimeout_);
  }

  // Explicitly set the value for a key.
  // This will create or update a node as appropriate.
  // This may fail if there is no space and no items can be evicted.
  std::shared_ptr<const NodeType> set(
      const KeyType& key,
      ValueType&& value,
      std::chrono::steady_clock::time_point now =
          std::chrono::steady_clock::now()) {
    auto state = state_.wlock();
    auto it = state->map.find(key);

    if (it != state->map.end()) {
      // Remove this item.  We can't update the value in the
      // item without introducing per-node locks, so we simply
      // allow any external references to see their immutable
      // value while we insert a new value in the map.

      // Note that we don't check node->expired() here like
      // we do in the get() path.  The assumption is that the
      // caller is deliberately replacing an errored node
      // with some valid value.
      auto oldNode = it->second;
      whichQ(oldNode.get(), state)->remove(oldNode.get());
      state->map.erase(it);
      ++state->stats.cacheEvict;
    }

    auto node = makeNode(state, now, key, std::move(value));
    state->map.emplace(std::make_pair(node->key_, node));
    whichQ(node.get(), state)->insertTail(node.get());
    ++state->stats.cacheStore;

    return node;
  }

  // Erase the entry associated with key.
  // Returns the node if it was present, else nullptr.
  std::shared_ptr<const NodeType> erase(const KeyType& key) {
    auto state = state_.wlock();
    auto it = state->map.find(key);
    if (it == state->map.end()) {
      return nullptr;
    }

    auto node = it->second;
    // Note that we don't check node->expired() here like
    // we do in the get() path.  The assumption is that the
    // caller is deliberately invalidating an errored node.
    whichQ(node.get(), state)->remove(node.get());
    state->map.erase(it);
    ++state->stats.cacheErase;

    return node;
  }

  // Returns the number of cached items
  size_t size() const {
    auto state = state_.rlock();
    return state->map.size();
  }

  // Returns cache statistics
  CacheStats stats() const {
    auto state = state_.rlock();
    return CacheStats(state->stats, state->map.size());
  }

  // Purge all of the entries from the cache
  void clear() {
    auto state = state_.wlock();
    state->evictionOrder.clear();
    state->erroredOrder.clear();
    state->lookupOrder.clear();
    state->map.clear();
    state->stats.clear();
  }

 private:
  // Small helper for creating a new Node.  This checks for capacity
  // and attempts to evict an item to make room if needed.
  // The eviction may fail in some cases, which results in this method
  // also throwing.
  template <typename... Args>
  std::shared_ptr<NodeType> makeNode(
      LockedState& state,
      std::chrono::steady_clock::time_point now,
      Args&&... args) {
    // If we are too full, try to evict an item; this may throw if there are no
    // evictable items!
    if (state->map.size() + 1 > maxItems_) {
      evictOne(state, now, true);
    }

    return std::make_shared<NodeType>(std::forward<Args>(args)...);
  }

  // Returns the queue into which the node should be placed (for new nodes),
  // or should currently be linked into (for existing nodes).
  lrucache::TailQHead<NodeType>* whichQ(NodeType* node, LockedState& state) {
    if (node->promises_) {
      return &state->lookupOrder;
    }

    if (!node->value_.hasValue()) {
      return &state->erroredOrder;
    }

    return &state->evictionOrder;
  }

  // Attempt to evict a single item to make space for a new Node.
  // if `forceRemoval` is true, then we're being called to flush
  // out any excess items that we were forced to absorb earlier,
  // so we can consider flushing out the error entries.
  bool evictOne(
      LockedState& state,
      std::chrono::steady_clock::time_point now,
      bool forceRemoval) {
    // Since errors have a TTL (as opposed to infinite), try to
    // evict one of those first.  That keeps the cache focused
    // on usable items rather than prematurely evicting something
    // that might be useful again in the future.
    auto errorNode = state->erroredOrder.head();
    if (errorNode && errorNode->expired(now)) {
      state->erroredOrder.remove(errorNode);
      // Erase from the map last, as this will invalidate node
      state->map.erase(errorNode->key_);
      ++state->stats.cacheEvict;
      return true;
    }

    // Second choice is to evict a successful item
    auto node = state->evictionOrder.head();
    if (node) {
      state->evictionOrder.remove(node);
      // Erase from the map last, as this will invalidate node
      state->map.erase(node->key_);
      ++state->stats.cacheEvict;
      return true;
    }

    // We couldn't find any other options and it is important
    // to remove something, so let's eliminate an error item
    // that we found earlier.
    if (forceRemoval && errorNode) {
      state->erroredOrder.remove(errorNode);
      // Erase from the map last, as this will invalidate node
      state->map.erase(errorNode->key_);
      ++state->stats.cacheEvict;
      return true;
    }

    // There are no evictable items to purge, so we are too full.
    return false;
  }

  // The maximum allowed capacity
  const size_t maxItems_;
  // How long to cache items that have an error Result
  const std::chrono::milliseconds errorTTL_;
  const std::chrono::milliseconds fetchTimeout_;
  folly::Synchronized<State> state_;
};
} // namespace watchman
