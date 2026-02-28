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

#include <optional>
#include <folly/system/MemoryMapping.h>
#include <folly/testing/TestUtil.h>

namespace apache::thrift::test {
template <class Base>
struct MockSize : Base {
  using Base::Base;
  size_t size() const {
    if (mockedSize) {
      return *mockedSize;
    }
    return Base::size();
  }

  std::optional<size_t> mockedSize;
};

// `off_t` is 32-bit on Windows
constexpr int64_t kPlatformMaxSize =
    std::numeric_limits<int32_t>::max() / (folly::kIsWindows ? 2 : 1);

constexpr int64_t kMaxSize = kPlatformMaxSize - 10;
constexpr int64_t kExceededSize = kPlatformMaxSize + 1LL;

// Allocated memory size = string size + thrift struct serialization overhead
constexpr size_t kMemSize = kExceededSize + 100;

template <class Tag = void>
void* getAddr() {
  static folly::test::TemporaryFile tmpFile{
      {},
      {},
      folly::test::TemporaryFile::Scope::UNLINK_IMMEDIATELY,
  };
  static folly::MemoryMapping memory{
      tmpFile.fd(),
      0,
      kMemSize,
      folly::MemoryMapping::Options{}.setWritable(true).setGrow(true),
  };
  return static_cast<void*>(memory.asWritableRange<char>().data());
}

template <class T>
struct TestFileBackedAllocator : std::allocator<T> {
  T* allocate(size_t, const void* = nullptr) {
    return static_cast<T*>(getAddr());
  }
  void deallocate(T*, size_t) {}
  template <class U>
  struct rebind {
    using other = TestFileBackedAllocator<U>;
  };
};

using TestFileBackedString = std::
    basic_string<char, std::char_traits<char>, TestFileBackedAllocator<char>>;
} // namespace apache::thrift::test
