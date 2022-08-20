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
#include <string>

#include <folly/GLog.h>

namespace apache::thrift {

// This is used to identify a ResourcePool within a ResourcePoolSet. This
// is just data. It is the responsibility of the code that creates and
// then accepts these to do any validation.
//
// To help with that we provide an ownerId value which can be set on creation by
// a class that returns ResourcePoolHandles to a unique value and then when that
// class is passed a ResourcePoolHandle it can check that it is one of its own
// using checkOwner().
class ResourcePoolHandle {
 public:
  // An opqaue type that is used to verify that a ResourcePoolHandle is being
  // provided to an object that originally returned it.
  using OwnerId = intptr_t;

  static constexpr std::size_t kDefaultSync = 0;
  static constexpr std::size_t kDefaultAsync = 1;
  static constexpr std::size_t kMaxReservedHandle = kDefaultAsync;

  static constexpr OwnerId kNoOwner = 0;

  ResourcePoolHandle() = default;

  std::size_t index() const { return index_; }
  std::string_view name() const { return name_; }

  // Check that this handle either has no owner set or that the owner matches
  // the specified ownerId (normally this verifies that this ResourcePoolHandle
  // was created by the same object it is being provided to)
  bool checkOwner(OwnerId ownerId) {
    DCHECK(index_ != kInvalidHandle);
    return (ownerId_ == kNoOwner || (ownerId == ownerId_));
  }

  // Returns pointer to a singleton default Sync handle. Users should cache the
  // result of this.
  static const ResourcePoolHandle& defaultSync();

  // Returns pointer to a singleton default aysnc handle. Users should cache the
  // result of this.
  static const ResourcePoolHandle& defaultAsync();

  // Create a new handle (cannot be used to create kDefaultSync or
  // kDefaultAsync handles).
  static ResourcePoolHandle makeHandle(
      std::string_view name, std::size_t index, OwnerId ownerId = kNoOwner) {
    CHECK(index != kDefaultSync && index != kDefaultAsync);
    return ResourcePoolHandle{name, index, ownerId};
  }

 private:
  static ResourcePoolHandle makeDefaultHandle(std::size_t index);

  static constexpr std::size_t kInvalidHandle =
      std::numeric_limits<std::size_t>::max();

  ResourcePoolHandle(
      std::string_view name, std::size_t index, std::intptr_t ownerId)
      : name_(name), index_(index), ownerId_(ownerId) {}

  std::string name_;
  std::size_t index_{kInvalidHandle};
  OwnerId ownerId_{kNoOwner};
};

} // namespace apache::thrift
