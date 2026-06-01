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

/**
 * SrptHeap - 4-ary min-heap implementing Shortest Remaining Processing Time
 * scheduling for frame fragmentation.
 *
 * SRPT is provably optimal for minimizing mean flow completion time. Small
 * streams (single-fragment request-response) are flushed immediately rather
 * than being blocked behind bulk transfers in a round-robin.
 *
 * The heap stores entries of type Value, ordered by a size_t key extracted via
 * KeyFn. An auxiliary frame::read::DirectStreamMap provides O(1) stream ID →
 * heap index lookup for insertions and updates.
 *
 * 4-ary heap: children of node i are 4i+1..4i+4. Shallower than binary
 * (log₄N vs log₂N), 4 children fit in one cache line for the compare scan.
 *
 * Template parameters:
 *   Value - per-stream state type (must be move-constructible)
 *   KeyFn - functor: size_t operator()(const Value&) returns the scheduling
 *           priority (remaining bytes). Lower = higher priority.
 */

#include <thrift/lib/cpp2/fast_thrift/frame/read/DirectStreamMap.h>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <utility>
#include <vector>

namespace apache::thrift::fast_thrift::frame::write {

template <typename Value, typename KeyFn>
class SrptHeap {
  static constexpr size_t kArity = 4;

 public:
  SrptHeap() = default;

  /// Insert a new stream. Returns a reference to the stored entry.
  Value& insert(uint32_t streamId, Value value) {
    size_t pos = heap_.size();
    heap_.push_back({streamId, std::move(value)});
    index_.emplace(streamId, pos);
    siftUp(pos);
    // After siftUp, the entry may have moved — look up its current position.
    auto it = index_.find(streamId);
    return heap_[it->second].value;
  }

  /// Look up a stream by ID. Returns nullptr if not found.
  Value* find(uint32_t streamId) noexcept {
    auto it = index_.find(streamId);
    if (!it) {
      return nullptr;
    }
    return &heap_[it->second].value;
  }

  const Value* find(uint32_t streamId) const noexcept {
    return const_cast<SrptHeap*>(this)->find(streamId);
  }

  /// Access the minimum-priority entry (stream with least remaining work).
  /// Undefined behavior if empty.
  Value& peekMin() noexcept { return heap_[0].value; }
  const Value& peekMin() const noexcept { return heap_[0].value; }

  /// Stream ID of the minimum entry.
  uint32_t peekMinStreamId() const noexcept { return heap_[0].streamId; }

  /// Remove the minimum entry and return it.
  Value extractMin() {
    auto result = std::move(heap_[0].value);
    removeAt(0);
    return result;
  }

  /// Remove a stream by ID. Returns the removed value, or std::nullopt.
  bool erase(uint32_t streamId) {
    auto it = index_.find(streamId);
    if (!it) {
      return false;
    }
    size_t pos = it->second;
    removeAt(pos);
    return true;
  }

  /// Notify the heap that the priority of `streamId` has changed.
  /// Must be called after modifying the value's remaining bytes.
  void update(uint32_t streamId) {
    auto it = index_.find(streamId);
    if (!it) {
      return;
    }
    size_t pos = it->second;
    // Priority could have increased or decreased — try both directions.
    if (!siftUp(pos)) {
      siftDown(pos);
    }
  }

  bool contains(uint32_t streamId) const noexcept {
    return index_.contains(streamId);
  }

  bool empty() const noexcept { return heap_.empty(); }

  size_t size() const noexcept { return heap_.size(); }

  /// Iterate all entries. Callback: void(uint32_t streamId, Value& val).
  /// Order is heap order, not sorted order.
  template <typename F>
  void forEach(F&& fn) noexcept {
    for (auto& e : heap_) {
      fn(e.streamId, e.value);
    }
  }

  void clear() noexcept {
    heap_.clear();
    index_.clear();
  }

 private:
  struct Entry {
    uint32_t streamId{0};
    Value value{};
  };

  size_t priority(size_t pos) const noexcept {
    return keyFn_(heap_[pos].value);
  }

  /// Sift up toward root. Returns true if the element moved.
  bool siftUp(size_t i) {
    bool moved = false;
    while (i > 0) {
      size_t parent = (i - 1) / kArity;
      if (priority(i) < priority(parent)) {
        swapAt(i, parent);
        i = parent;
        moved = true;
      } else {
        break;
      }
    }
    return moved;
  }

  /// Sift down away from root.
  void siftDown(size_t i) {
    for (;;) {
      size_t firstChild = kArity * i + 1;
      if (firstChild >= heap_.size()) {
        break;
      }

      // Find the child with minimum priority among up to kArity children.
      size_t lastChild = std::min(firstChild + kArity, heap_.size());
      size_t minChild = firstChild;
      for (size_t c = firstChild + 1; c < lastChild; ++c) {
        if (priority(c) < priority(minChild)) {
          minChild = c;
        }
      }

      if (priority(minChild) < priority(i)) {
        swapAt(i, minChild);
        i = minChild;
      } else {
        break;
      }
    }
  }

  void removeAt(size_t pos) {
    uint32_t removedId = heap_[pos].streamId;
    index_.erase(removedId);

    if (pos == heap_.size() - 1) {
      // Removing last element — no fixup needed.
      heap_.pop_back();
      return;
    }

    // Swap with last element and pop.
    heap_[pos] = std::move(heap_.back());
    heap_.pop_back();
    index_.find(heap_[pos].streamId)->second = pos;

    // Restore heap property from the swapped position.
    if (!siftUp(pos)) {
      siftDown(pos);
    }
  }

  void swapAt(size_t a, size_t b) {
    index_.find(heap_[a].streamId)->second = b;
    index_.find(heap_[b].streamId)->second = a;
    std::swap(heap_[a], heap_[b]);
  }

  [[no_unique_address]] KeyFn keyFn_{};
  std::vector<Entry> heap_;
  frame::read::DirectStreamMap<size_t> index_;
};

} // namespace apache::thrift::fast_thrift::frame::write
