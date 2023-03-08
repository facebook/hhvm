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
  static constexpr std::size_t kDefaultSyncIndex = 0;
  static constexpr std::size_t kDefaultAsyncIndex = 1;
  static constexpr std::size_t kMaxReservedIndex = kDefaultAsyncIndex;

  std::size_t index() const { return index_; }
  std::string_view name() const { return name_; }

  // Returns pointer to a singleton default Sync handle. Users should cache the
  // result of this.
  static const ResourcePoolHandle& defaultSync();

  // Returns pointer to a singleton default aysnc handle. Users should cache the
  // result of this.
  static const ResourcePoolHandle& defaultAsync();

  // Create a new handle (cannot be used to create kDefaultSync or
  // kDefaultAsync handles).
  static ResourcePoolHandle makeHandle(
      std::string_view name, std::size_t index) {
    CHECK(index != kDefaultSyncIndex && index != kDefaultAsyncIndex);
    return ResourcePoolHandle{name, index};
  }

 private:
  ResourcePoolHandle(std::string_view name, std::size_t index)
      : name_(name), index_(index) {}

  std::string name_;
  std::size_t index_;
};

} // namespace apache::thrift
