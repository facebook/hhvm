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

#include <cstddef>
#include <cstdint>
#include <utility>
#include <vector>

namespace apache::thrift::fast_thrift::frame::read {

// ── Index policies (stateless, zero-cost) ──────────────────────────────────

/// For RSocket stream IDs (uint32_t, stride +2).
/// (key >> 1) maps odd-only or even-only IDs to consecutive slots.
struct StreamIdIndex {
  template <typename K>
  static size_t apply(K key, size_t mask) noexcept {
    return (key >> 1) & mask;
  }
};

/// For sequential IDs (uint64_t request IDs, stride +1).
/// Identity mask — consecutive keys land in consecutive slots.
struct SequentialIndex {
  template <typename K>
  static size_t apply(K key, size_t mask) noexcept {
    return static_cast<size_t>(key) & mask;
  }
};

// ── DirectStreamMap ────────────────────────────────────────────────────────

/**
 * DirectStreamMap - Direct-mapped open-addressing table for integer keys.
 *
 * Template parameters:
 *   Value       - mapped type (must be move-constructible and
 *                 default-constructible)
 *   Key         - key type (default: uint32_t for RSocket stream IDs)
 *   IndexPolicy - static policy with `size_t apply(Key, size_t mask)` that
 *                 maps a key to an initial slot index.
 *                 Default: StreamIdIndex  ((key >> 1) & mask)
 *
 * RSocket stream IDs increment by 2 (odd=client, even=server), so >> 1
 * produces sequential indices with zero collisions for same-parity traffic.
 * For sequential +1 keys (e.g. request IDs), use SequentialIndex.
 *
 * Linear probe on the rare collision. Backshift delete (Robin Hood style) —
 * no tombstones, find() always stops at the first empty slot.
 */
template <
    typename Value,
    typename Key = uint32_t,
    typename IndexPolicy = StreamIdIndex>
class DirectStreamMap {
  static constexpr size_t kInitialCapacity = 16;
  static constexpr size_t kLoadNum = 7;
  static constexpr size_t kLoadDen = 8;

  enum class Tag : uint8_t { Empty, Live };

 public:
  struct Slot {
    Key first{0};
    Value second{};
    Tag tag{Tag::Empty};
  };

  using iterator = Slot*;
  using const_iterator = const Slot*;

  DirectStreamMap() : slots_(kInitialCapacity) {}

  explicit DirectStreamMap(size_t cap) : slots_(nextPow2(cap)) {}

  ~DirectStreamMap() = default;

  DirectStreamMap(const DirectStreamMap&) = delete;
  DirectStreamMap& operator=(const DirectStreamMap&) = delete;
  DirectStreamMap(DirectStreamMap&&) = default;
  DirectStreamMap& operator=(DirectStreamMap&&) = default;

  iterator find(Key key) noexcept {
    auto m = mask();
    auto idx = IndexPolicy::apply(key, m);
    for (size_t i = 0; i < slots_.size(); ++i) {
      auto& s = slots_[idx];
      if (s.tag == Tag::Empty) {
        return nullptr;
      }
      if (s.first == key) {
        return &s;
      }
      idx = (idx + 1) & m;
    }
    return nullptr;
  }

  const_iterator find(Key key) const noexcept {
    return const_cast<DirectStreamMap*>(this)->find(key);
  }

  iterator end() noexcept { return nullptr; }
  const_iterator end() const noexcept { return nullptr; }

  template <typename... Args>
  std::pair<iterator, bool> emplace(Key key, Args&&... args) {
    if (size_ * kLoadDen > slots_.size() * kLoadNum) {
      grow();
    }
    return insertInternal(key, std::forward<Args>(args)...);
  }

  void erase(iterator it) noexcept {
    if (!it) {
      return;
    }
    eraseSlot(static_cast<size_t>(it - slots_.data()));
  }

  void erase(Key key) noexcept {
    auto it = find(key);
    if (it) {
      eraseSlot(static_cast<size_t>(it - slots_.data()));
    }
  }

  /// Iterate all live entries. Callback signature: void(Key key, Value& val)
  template <typename F>
  void forEach(F&& fn) noexcept {
    for (auto& s : slots_) {
      if (s.tag == Tag::Live) {
        fn(s.first, s.second);
      }
    }
  }

  bool contains(Key key) const noexcept { return find(key) != nullptr; }

  bool empty() const noexcept { return size_ == 0; }
  size_t size() const noexcept { return size_; }
  size_t capacity() const noexcept { return slots_.size(); }

  void clear() noexcept {
    for (auto& s : slots_) {
      s.tag = Tag::Empty;
      s.first = Key{0};
      s.second = Value{};
    }
    size_ = 0;
  }

 private:
  size_t mask() const noexcept { return slots_.size() - 1; }

  static size_t nextPow2(size_t n) noexcept {
    if (n <= kInitialCapacity) {
      return kInitialCapacity;
    }
    return size_t{1} << (64 - __builtin_clzll(n - 1));
  }

  // Circular displacement check: is 'gap' between 'nat' and 'cur'?
  // If so, the entry at 'cur' was displaced past 'gap' and should
  // be shifted back.
  static bool displaced(size_t nat, size_t gap, size_t cur) noexcept {
    if (nat <= gap) {
      return gap < cur || cur < nat;
    }
    return gap < cur && cur < nat;
  }

  void eraseSlot(size_t idx) noexcept {
    auto m = mask();
    slots_[idx].tag = Tag::Empty;
    slots_[idx].second = Value{};
    --size_;

    // Backshift: walk forward, pull displaced entries back to fill the gap.
    auto next = (idx + 1) & m;
    while (slots_[next].tag == Tag::Live) {
      auto nat = IndexPolicy::apply(slots_[next].first, m);
      if (displaced(nat, idx, next)) {
        slots_[idx] = std::move(slots_[next]);
        slots_[next].tag = Tag::Empty;
        slots_[next].second = Value{};
        idx = next;
      }
      next = (next + 1) & m;
    }
  }

  void grow() {
    auto old = std::move(slots_);
    slots_ = std::vector<Slot>(old.size() * 2);
    size_ = 0;
    for (auto& s : old) {
      if (s.tag == Tag::Live) {
        insertInternal(s.first, std::move(s.second));
      }
    }
  }

  template <typename... Args>
  std::pair<iterator, bool> insertInternal(Key key, Args&&... args) {
    auto m = mask();
    auto idx = IndexPolicy::apply(key, m);
    for (size_t i = 0; i < slots_.size(); ++i) {
      auto& s = slots_[idx];
      if (s.tag == Tag::Empty) {
        s.first = key;
        s.tag = Tag::Live;
        s.second = Value(std::forward<Args>(args)...);
        ++size_;
        return {&s, true};
      }
      if (s.first == key) {
        return {&s, false};
      }
      idx = (idx + 1) & m;
    }
    return {nullptr, false};
  }

  std::vector<Slot> slots_;
  size_t size_{0};
};

} // namespace apache::thrift::fast_thrift::frame::read
