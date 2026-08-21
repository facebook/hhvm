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

#include <memory>
#include <typeindex>
#include <typeinfo>
#include <utility>

#include <folly/container/F14Map.h>

#include <thrift/lib/cpp2/fast_thrift/rocket/common/TypeErasedPtr.h>

namespace apache::thrift::fast_thrift::thrift {

/**
 * Per-connection state shared between extensions, keyed by state type.
 *
 * One extension instance is constructed per connection, so an extension that
 * needs nothing from its peers keeps its state as a member. This store exists
 * for the other case: several cooperating extensions on the same connection
 * that must read and write one object — an identity resolved by one and
 * authorized against by the next.
 *
 * A slot is keyed by the state type itself, so two extensions reach the same
 * object exactly when they name the same type, and unrelated extensions cannot
 * collide however they are composed. The object is heap-owned, so a reference
 * handed out at construction stays valid for the connection even as later
 * slots are added.
 *
 * Not thread-safe, and needs not be: the store, the pipeline it belongs to, and
 * every extension reading it live on one connection's EventBase.
 */
class ExtensionStateStore {
 public:
  ExtensionStateStore() = default;

  ExtensionStateStore(const ExtensionStateStore&) = delete;
  ExtensionStateStore& operator=(const ExtensionStateStore&) = delete;
  ExtensionStateStore(ExtensionStateStore&&) noexcept = default;
  ExtensionStateStore& operator=(ExtensionStateStore&&) noexcept = default;

  /**
   * The connection's `T`, constructing it from `args` on the first call for
   * that type. Later calls return the same object and ignore `args`.
   *
   * The extension path never passes `args`: an extension's ConnState is
   * default-constructed, because the extension that happens to be spliced in
   * first is not a meaningful choice of constructor arguments. `args` is for
   * direct users of the store, which today means tests.
   */
  template <typename T, typename... Args>
  T& getOrCreate(Args&&... args) {
    const std::type_index key{typeid(T)};
    if (auto it = slots_.find(key); it != slots_.end()) {
      // Sound because the slot is keyed by typeid(T): only a T was ever
      // stored under this key.
      return *static_cast<T*>(it->second.get());
    }
    auto owned = std::make_unique<T>(std::forward<Args>(args)...);
    T* state = owned.get();
    slots_.emplace(key, rocket::from_unique_ptr(std::move(owned)));
    return *state;
  }

  /**
   * Whether a slot for `T` has been created. For tests and diagnostics; the
   * message path uses getOrCreate.
   */
  template <typename T>
  bool contains() const noexcept {
    return slots_.contains(std::type_index{typeid(T)});
  }

  std::size_t size() const noexcept { return slots_.size(); }

 private:
  folly::F14FastMap<std::type_index, rocket::TypeErasedPtr> slots_;
};

} // namespace apache::thrift::fast_thrift::thrift
