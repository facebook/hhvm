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

#include <map>
#include <memory>
#include <memory_resource>
#include <scoped_allocator>
#include <set>
#include <string>
#include <vector>

#include <folly/Memory.h>
#include <folly/sorted_vector_types.h>

using PmrByteAlloc = std::pmr::polymorphic_allocator<std::byte>;

template <class T>
struct AlwaysThrowAllocator : private std::allocator<T> {
  using value_type = T;

  AlwaysThrowAllocator() = default;
  AlwaysThrowAllocator(const AlwaysThrowAllocator&) noexcept = default;
  AlwaysThrowAllocator& operator=(const AlwaysThrowAllocator&) noexcept =
      default;
  template <class U>
  explicit AlwaysThrowAllocator(const AlwaysThrowAllocator<U>&) noexcept {}
  ~AlwaysThrowAllocator() = default;

  T* allocate(size_t) { throw std::bad_alloc(); }

  void deallocate(T*, size_t) {}

  template <class U>
  friend bool operator==(
      const AlwaysThrowAllocator<T>&, const AlwaysThrowAllocator<U>&) noexcept {
    return true;
  }

  template <class U>
  friend bool operator!=(
      const AlwaysThrowAllocator<T>&, const AlwaysThrowAllocator<U>&) noexcept {
    return false;
  }
};

template <typename T = char>
using ScopedAlwaysThrowAlloc =
    std::scoped_allocator_adaptor<AlwaysThrowAllocator<T>>;

template <class T>
using AlwaysThrowVector = std::vector<T, ScopedAlwaysThrowAlloc<T>>;

template <class T>
using AlwaysThrowSet = std::set<T, std::less<T>, ScopedAlwaysThrowAlloc<T>>;

template <class K, class V>
using AlwaysThrowMap =
    std::map<K, V, std::less<K>, ScopedAlwaysThrowAlloc<std::pair<const K, V>>>;

using AlwaysThrowString =
    std::basic_string<char, std::char_traits<char>, ScopedAlwaysThrowAlloc<>>;

template <class T>
struct StatefulAlloc : private std::allocator<T> {
  using value_type = T;

  StatefulAlloc() = default;
  StatefulAlloc(const StatefulAlloc&) = default;
  StatefulAlloc& operator=(const StatefulAlloc&) noexcept = default;
  explicit StatefulAlloc(int state) : state_(state) {}
  template <class U>
  explicit StatefulAlloc(const StatefulAlloc<U>& other) noexcept
      : state_(other.state_) {}

  int state_ = 0;

  T* allocate(size_t size) { return std::allocator<T>::allocate(size); }

  void deallocate(T* p, size_t size) { std::allocator<T>::deallocate(p, size); }

  template <class U>
  friend bool operator==(
      const StatefulAlloc<T>& a, const StatefulAlloc<U>& b) noexcept {
    return a.state_ == b.state_;
  }

  template <class U>
  friend bool operator!=(
      const StatefulAlloc<T>& a, const StatefulAlloc<U>& b) noexcept {
    return a.state_ != b.state_;
  }
};

template <typename T = char>
using ScopedStatefulAlloc = std::scoped_allocator_adaptor<StatefulAlloc<T>>;

template <class T>
using StatefulAllocVector = std::vector<T, ScopedStatefulAlloc<T>>;

template <class T>
using StatefulAllocSet = std::set<T, std::less<T>, ScopedStatefulAlloc<T>>;

template <class K, class V>
using StatefulAllocMap =
    std::map<K, V, std::less<K>, ScopedStatefulAlloc<std::pair<const K, V>>>;

template <class T>
using StatefulAllocSortedVectorSet =
    folly::sorted_vector_set<T, std::less<T>, ScopedStatefulAlloc<T>>;

template <class K, class V>
using StatefulAllocSortedVectorMap = folly::
    sorted_vector_map<K, V, std::less<K>, ScopedStatefulAlloc<std::pair<K, V>>>;

template <class T>
struct CountingAlloc : private std::allocator<T> {
  using value_type = T;

  CountingAlloc() : counter_(std::make_shared<int>(0)) {}

  CountingAlloc(const CountingAlloc&) = default;
  CountingAlloc& operator=(const CountingAlloc&) noexcept = default;
  template <class U>
  explicit CountingAlloc(const CountingAlloc<U>& other) noexcept
      : counter_(other.counter_) {}

  std::shared_ptr<int> counter_;

  int getCount() const { return *counter_; }

  T* allocate(size_t size) {
    (*counter_)++;
    return std::allocator<T>::allocate(size);
  }

  void deallocate(T* p, size_t size) { std::allocator<T>::deallocate(p, size); }

  template <class U>
  friend bool operator==(
      const CountingAlloc<T>& a, const CountingAlloc<U>& b) noexcept {
    return a.counter_.get() == b.counter_.get(); // both point to same counter
  }

  template <class U>
  friend bool operator!=(
      const CountingAlloc<T>& a, const CountingAlloc<U>& b) noexcept {
    return !(a == b);
  }
};

template <typename T = char>
using ScopedCountingAlloc = std::scoped_allocator_adaptor<CountingAlloc<T>>;

template <class T>
using CountingVector = std::vector<T, ScopedCountingAlloc<T>>;

template <class T>
using CountingSet = std::set<T, std::less<T>, ScopedCountingAlloc<T>>;

template <class K, class V>
using CountingMap =
    std::map<K, V, std::less<K>, ScopedCountingAlloc<std::pair<const K, V>>>;

using CountingString =
    std::basic_string<char, std::char_traits<char>, ScopedCountingAlloc<>>;

struct CountingPmrResource : public std::pmr::memory_resource {
  CountingPmrResource() : counter_(std::make_shared<int>(0)) {}
  CountingPmrResource(const CountingPmrResource&) = default;
  CountingPmrResource& operator=(const CountingPmrResource&) = default;

  int getCount() const { return *counter_; }

 private:
  std::shared_ptr<int> counter_;

  void* do_allocate(size_t bytes, size_t alignment) override {
    (*counter_)++;
    return std::pmr::get_default_resource()->allocate(bytes, alignment);
  }
  void do_deallocate(void* p, size_t bytes, size_t alignment) override {
    std::pmr::get_default_resource()->deallocate(p, bytes, alignment);
  }
  bool do_is_equal(const memory_resource&) const noexcept override {
    return true;
  }
};

using PropagateAllocBase = StatefulAlloc<char>;
struct PropagateAllAlloc : public PropagateAllocBase {
  using PropagateAllocBase::PropagateAllocBase;
  using propagate_on_container_copy_assignment = std::true_type;
  using propagate_on_container_move_assignment = std::true_type;
  using propagate_on_container_swap = std::true_type;
};

struct PropagateNoneAlloc : public PropagateAllocBase {
  using PropagateAllocBase::PropagateAllocBase;
  using propagate_on_container_copy_assignment = std::false_type;
  using propagate_on_container_move_assignment = std::false_type;
  using propagate_on_container_swap = std::false_type;
  PropagateNoneAlloc(const PropagateNoneAlloc&) = default;
  PropagateNoneAlloc(PropagateNoneAlloc&&) = default;
  PropagateNoneAlloc& operator=(const PropagateNoneAlloc&) = delete;
  PropagateNoneAlloc& operator=(PropagateNoneAlloc&&) = delete;
};

struct PropagateOnlyCopyAlloc : public PropagateAllocBase {
  using PropagateAllocBase::PropagateAllocBase;
  using propagate_on_container_copy_assignment = std::true_type;
  using propagate_on_container_move_assignment = std::false_type;
  using propagate_on_container_swap = std::false_type;
  PropagateOnlyCopyAlloc(const PropagateOnlyCopyAlloc&) = default;
  PropagateOnlyCopyAlloc(PropagateOnlyCopyAlloc&&) = default;
  PropagateOnlyCopyAlloc& operator=(const PropagateOnlyCopyAlloc&) = default;
  PropagateOnlyCopyAlloc& operator=(PropagateOnlyCopyAlloc&&) = delete;
};

struct PropagateOnlyMoveAlloc : public PropagateAllocBase {
  using PropagateAllocBase::PropagateAllocBase;
  using propagate_on_container_copy_assignment = std::false_type;
  using propagate_on_container_move_assignment = std::true_type;
  using propagate_on_container_swap = std::false_type;
  PropagateOnlyMoveAlloc(const PropagateOnlyMoveAlloc&) = default;
  PropagateOnlyMoveAlloc(PropagateOnlyMoveAlloc&&) = default;
  PropagateOnlyMoveAlloc& operator=(const PropagateOnlyMoveAlloc&) = delete;
  PropagateOnlyMoveAlloc& operator=(PropagateOnlyMoveAlloc&&) = default;
};

struct PropagateOnlySwapAlloc : public PropagateAllocBase {
  using PropagateAllocBase::PropagateAllocBase;
  using propagate_on_container_copy_assignment = std::false_type;
  using propagate_on_container_move_assignment = std::false_type;
  using propagate_on_container_swap = std::true_type;
};

struct PropagateCopyMoveAlloc : public PropagateAllocBase {
  using PropagateAllocBase::PropagateAllocBase;
  using propagate_on_container_copy_assignment = std::true_type;
  using propagate_on_container_move_assignment = std::true_type;
  using propagate_on_container_swap = std::false_type;
};

struct PropagateCopySwapAlloc : public PropagateAllocBase {
  using PropagateAllocBase::PropagateAllocBase;
  using propagate_on_container_copy_assignment = std::true_type;
  using propagate_on_container_move_assignment = std::false_type;
  using propagate_on_container_swap = std::true_type;
};

struct PropagateMoveSwapAlloc : public PropagateAllocBase {
  using PropagateAllocBase::PropagateAllocBase;
  using propagate_on_container_copy_assignment = std::false_type;
  using propagate_on_container_move_assignment = std::true_type;
  using propagate_on_container_swap = std::true_type;
  PropagateMoveSwapAlloc(const PropagateMoveSwapAlloc&) = default;
  PropagateMoveSwapAlloc(PropagateMoveSwapAlloc&&) = default;
  PropagateMoveSwapAlloc& operator=(const PropagateMoveSwapAlloc&) = delete;
  PropagateMoveSwapAlloc& operator=(PropagateMoveSwapAlloc&&) = default;
};
