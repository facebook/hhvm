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

#include <thrift/lib/cpp2/frozen/FrozenUtil.h>

#include <cstdlib>

#include <folly/Conv.h>

// clang-format off
FOLLY_GFLAGS_DEFINE_bool(thrift_frozen_util_disable_mlock, false,
    "Don't mlock() files mmaped by mapFrozen() call.");
FOLLY_GFLAGS_DEFINE_bool(thrift_frozen_util_mlock_on_fault, false,
    "Use mlock2(MLOCK_ONFAULT) instead of mlock().");
// clang-format on

namespace apache::thrift::frozen {

FrozenFileForwardIncompatible::FrozenFileForwardIncompatible(int fileVersion)
    : std::runtime_error(
          folly::to<std::string>(
              "Frozen File version ",
              fileVersion,
              " cannot be read, only versions up to ",
              schema::frozen_constants::kCurrentFrozenFileVersion(),
              " are supported.")),
      fileVersion_(fileVersion) {}

MallocFreezer::Segment::Segment(size_t _size)
    : size(_size),
      // NB: All allocations rounded up to next multiple of 8 due to packed
      // integer read amplification
      buffer(reinterpret_cast<byte*>(calloc(alignBy(size, 8), 1))) {
  if (!buffer) {
    throw std::runtime_error("Couldn't allocate memory");
  }
}

MallocFreezer::Segment::~Segment() {
  if (buffer) {
    free(buffer);
  }
}

size_t MallocFreezer::offsetOf(const byte* ptr) const {
  if (offsets_.empty() || !ptr) {
    return 0;
  }
  auto offsetIt = offsets_.upper_bound(ptr);
  if (offsetIt == offsets_.begin()) {
    throw std::runtime_error("offset");
  }
  --offsetIt;
  return ptr - offsetIt->first;
}

size_t MallocFreezer::distanceToEnd(const byte* ptr) const {
  if (offsets_.empty()) {
    return 0;
  }
  auto offsetIt = offsets_.upper_bound(ptr);
  if (offsetIt == offsets_.begin()) {
    throw std::runtime_error("dist");
  }
  --offsetIt;
  CHECK_GE(ptr, offsetIt->first);
  return (size_ - offsetIt->second) - (ptr - offsetIt->first);
}

folly::MutableByteRange MallocFreezer::appendBuffer(size_t size) {
  Segment segment(size);
  offsets_.emplace(segment.buffer, size_);

  folly::MutableByteRange range(segment.buffer, size);
  size_ += segment.size;
  segments_.push_back(std::move(segment));
  return range;
}

void MallocFreezer::doAppendBytes(
    byte* origin,
    size_t n,
    folly::MutableByteRange& range,
    size_t& distance,
    size_t alignment) {
  if (!n) {
    distance = 0;
    range.reset(nullptr, 0);
    return;
  }
  auto aligned = alignBy(size_, alignment);
  auto padding = aligned - size_;
  distance = distanceToEnd(origin) + padding;
  range = appendBuffer(padding + n);
  range.advance(padding);
}
} // namespace apache::thrift::frozen
