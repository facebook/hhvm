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

#include <vector>
#include <thrift/lib/cpp2/server/DecoratorData.h>
#include <thrift/lib/cpp2/server/DecoratorDataKey.h>

namespace apache::thrift::server {

class DecoratorDataHandleFactory {
 public:
  explicit DecoratorDataHandleFactory(
      std::vector<const UntypedDecoratorKey*>& keysToBeAllocated);

  template <typename T>
  DecoratorDataHandle<T> makeHandleForKey(const DecoratorDataKey<T>& key) {
    if (auto it = std::find_if(
            keysToBeAllocated_.begin(),
            keysToBeAllocated_.end(),
            [&](const UntypedDecoratorKey* keyToBeAllocated) {
              return keyToBeAllocated == std::addressof(key);
            });
        it != keysToBeAllocated_.end()) {
      return DecoratorDataHandle<T>::fromIndex(
          static_cast<size_t>(std::distance(keysToBeAllocated_.begin(), it)));
    }
    keysToBeAllocated_.emplace_back(std::addressof(key));
    return DecoratorDataHandle<T>::fromIndex(keysToBeAllocated_.size() - 1);
  }

 private:
  std::vector<const UntypedDecoratorKey*>& keysToBeAllocated_;
};

/**
 * This encapsulates the "finalized" state of known decorator data after server
 * setup.
 */
class DecoratorDataPerRequestBlueprint {
 public:
  /**
   * This class encapsulates the decorator data state upon server setup. It can
   * be used to create the DecoratorDataHandleFactory interfaces that are passed
   * to decorators, interceptors and handers. Each ThriftServer is only expected
   * to create one instance of this.
   *
   * After the server has gathered all of the DecoratorData that will be used,
   * this is converted into DecoratorDataRuntime via finalize()
   */
  class Setup {
   public:
    explicit Setup();

    DecoratorDataHandleFactory getHandleFactory();

    DecoratorDataPerRequestBlueprint finalize() &&;

   private:
    std::vector<const UntypedDecoratorKey*> keysToBeAllocated_;
  };

  struct Locator {
    util::AllocationColocator<>::ArrayLocator<DecoratorDataEntry> entries;
  };

  explicit DecoratorDataPerRequestBlueprint(std::size_t numEntries);

  template <typename T>
  Locator planStorage(util::AllocationColocator<T>& alloc) const {
    return {alloc.template array<DecoratorDataEntry>(numEntries_)};
  }

  template <typename MakeFn>
  DecoratorDataStorage initStorageForRequest(
      MakeFn&& make, Locator locator) const {
    return {numEntries_, make(std::move(locator.entries), [] {
              return DecoratorDataEntry();
            })};
  }

 private:
  std::size_t numEntries_;
};

} // namespace apache::thrift::server
