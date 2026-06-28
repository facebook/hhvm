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

#include <thrift/lib/cpp2/transport/rocket/framing/Parser.h>

#include <atomic>
#include <mutex>

// rocket_frame_parser flag specify which frame parser rocket transport decide
// to use. It can take three values
// - "strategy": rocket transport will use FrameLengthParserStrategy to parse
// rocket frames. This parsing strategy reads the rocket frame length (first 3
// bytes) and immediately hands off frames to downstream as soon as a full frame
// is available.
// - "allocating": frame parser that can take a custom allocator, we also
// guarantee buffer returned are not chained (meaning continuous). This can
// provide performance benefits when using cursor-based serialization.
THRIFT_FLAG_DEFINE_string(rocket_frame_parser, "strategy");

namespace apache::thrift::rocket::detail {

#if FOLLY_HAS_MEMORY_RESOURCE
namespace {
// Process-wide resource backing the default parser allocator. Installed via
// setDefaultParserMemoryResource() before any connection is created, and
// captured by getDefaultAllocator() on first use.
std::atomic<std::pmr::memory_resource*>& defaultParserMemoryResourceSlot() {
  static std::atomic<std::pmr::memory_resource*> slot{nullptr};
  return slot;
}

// Set true the first time getDefaultAllocator() captures the resource. A
// store() after this point is a no-op against the live allocator, so
// setDefaultParserMemoryResource() surfaces it as a programming error.
std::atomic<bool>& defaultParserMemoryResourceCaptured() {
  static std::atomic<bool> captured{false};
  return captured;
}

// Serializes the setter's check-then-store against the getter's
// load-then-capture so the two cannot interleave (a TOCTOU that would let a
// setter "succeed" while the live allocator already captured the old resource).
std::mutex& defaultParserMemoryResourceMutex() {
  static std::mutex m;
  return m;
}
} // namespace

void setDefaultParserMemoryResource(std::pmr::memory_resource* resource) {
  std::lock_guard<std::mutex> g(defaultParserMemoryResourceMutex());
  LOG_IF(
      FATAL,
      defaultParserMemoryResourceCaptured().load(std::memory_order_acquire))
      << "setDefaultParserMemoryResource() called after the default parser "
         "allocator was already materialized; the resource is captured on "
         "first use and a later install has no effect. Install it before the "
         "first RocketClient / RocketServerConnection is constructed.";
  defaultParserMemoryResourceSlot().store(resource, std::memory_order_release);
}

ParserAllocatorType& getDefaultAllocator() {
  // The resource is captured on first use. The mutex makes the capture atomic
  // with respect to setDefaultParserMemoryResource(): either the setter's store
  // happens-before the capture (resource is honored), or the capture
  // happens-before the setter's check (setter correctly FATALs as too-late).
  static folly::Indestructible<ParserAllocatorType> defaultAllocator([] {
    std::lock_guard<std::mutex> g(defaultParserMemoryResourceMutex());
    auto* r = defaultParserMemoryResourceSlot().load(std::memory_order_acquire);
    defaultParserMemoryResourceCaptured().store(
        true, std::memory_order_release);
    return r ? r : std::pmr::get_default_resource();
  }());
  return *defaultAllocator;
}
#else
ParserAllocatorType& getDefaultAllocator() {
  static folly::Indestructible<ParserAllocatorType> defaultAllocator;
  return *defaultAllocator;
}
#endif

ParserMode stringToMode(const std::string& modeStr) noexcept {
  if (modeStr == "strategy") {
    return ParserMode::STRATEGY;
  } else if (modeStr == "allocating") {
    return ParserMode::ALLOCATING;
  } else if (modeStr == "aligned") {
    return ParserMode::ALIGNED;
  }

  LOG(WARNING) << "Invalid parser mode: '" << modeStr
               << ", default to ParserMode::STRATEGY";
  return ParserMode::STRATEGY;
}
} // namespace apache::thrift::rocket::detail

#if FOLLY_HAS_MEMORY_RESOURCE
namespace apache::thrift::rocket {

void setDefaultParserMemoryResource(std::pmr::memory_resource* resource) {
  detail::setDefaultParserMemoryResource(resource);
}

} // namespace apache::thrift::rocket
#endif
