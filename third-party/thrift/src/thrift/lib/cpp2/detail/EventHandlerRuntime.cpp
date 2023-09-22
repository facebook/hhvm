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

#include <thrift/lib/cpp2/detail/EventHandlerRuntime.h>

#include <atomic>
#include <mutex>
#include <optional>
#include <type_traits>

#include <folly/synchronization/Rcu.h>

#include <thrift/lib/cpp2/util/AllocationColocator.h>

using apache::thrift::util::AllocationColocator;

/**
 * Here's how isClientMethodBypassed's implementation works:
 *
 * All bypassed service and method names are stored in one contiguous buffer
 * using AllocationColocator.
 *
 * A string is represented in memory like:
 *   <8-byte size = N> <N bytes> <1 byte for null terminator>
 * Thus a string of length N uses (8 + N + 1) bytes in memory.
 *
 * We can repeat the pattern above to create a contiguous list of strings.
 *
 * To traverse the list, we just need two things (see InlineNameList):
 *   - a cursor pointing to the beginning of the list in the buffer
 *   - a count of the number of strings in the list
 *
 * We create two such lists: serviceNames and methodNames, in that order, and
 * all contiguous (see MethodNameStorageImpl).
 *
 * Finally, we use folly::rcu and std::atomic to manage these lists.
 *
 * Why go to all this trouble?
 * This code will be run for every RPC on the client. Colocation helps with
 * locality and avoiding page faults. folly::rcu makes the read path extremely
 * low overhead.
 */
namespace {

struct InlineNameList {
  std::size_t count = 0;
  AllocationColocator<>::ConstUnsafeCursor cursor{nullptr};

  bool find(std::string_view value) const {
    auto c = cursor;
    for (std::size_t i = 0; i < count; ++i) {
      auto size = *c.object<std::size_t>();
      auto entry = c.string(size);
      if (entry == value) {
        return true;
      }
    }
    return false;
  }
};

struct MethodNameStorageImpl {
  InlineNameList serviceNames;
  InlineNameList methodNames;
};

std::atomic<MethodNameStorageImpl*> globalClientMethodsToBypass{nullptr};
static_assert(decltype(globalClientMethodsToBypass)::is_always_lock_free);

void removeEmptyNames(std::vector<std::string>& names) {
  names.erase(
      std::remove_if(
          names.begin(),
          names.end(),
          [](const std::string& name) { return name.empty(); }),
      names.end());
}

} // namespace

namespace std {
template <>
struct default_delete<MethodNameStorageImpl>
    : AllocationColocator<MethodNameStorageImpl>::Deleter {};
} // namespace std

namespace apache::thrift::detail {

/* static */ void EventHandlerRuntime::setClientMethodsToBypass(
    MethodNameSet bypassSets) {
  static const auto setMethodNameStorageImpl =
      [](MethodNameStorageImpl* bypassSets) {
        MethodNameStorageImpl* old = globalClientMethodsToBypass.exchange(
            bypassSets, std::memory_order_release);
        if (old != nullptr) {
          folly::rcu_retire(old);
        }
      };

  removeEmptyNames(bypassSets.serviceNames);
  removeEmptyNames(bypassSets.methodNames);

  if (bypassSets.serviceNames.empty() && bypassSets.methodNames.empty()) {
    setMethodNameStorageImpl(nullptr);
    return;
  }

  AllocationColocator<MethodNameStorageImpl> alloc;

  struct NameLocators {
    AllocationColocator<>::ObjectLocator<std::size_t> size;
    AllocationColocator<>::StringLocator string;
  };

  const auto locatorsFor =
      [&alloc](
          const std::vector<std::string>& names) -> std::vector<NameLocators> {
    std::vector<NameLocators> locators;
    locators.reserve(names.size());
    for (auto& name : names) {
      auto size = alloc.object<std::size_t>();
      auto str = alloc.string(name.length());
      locators.emplace_back(NameLocators{std::move(size), std::move(str)});
    }
    return locators;
  };

  static const auto copyNames =
      [](std::vector<NameLocators> locators,
         const std::vector<std::string>& names,
         auto make) -> AllocationColocator<>::ConstUnsafeCursor {
    DCHECK_EQ(names.size(), locators.size());
    const std::size_t* head = nullptr;
    for (std::size_t i = 0; i < names.size(); ++i) {
      auto size = make(std::move(locators[i].size), names[i].length());
      if (head == nullptr) {
        head = size;
      }
      make(std::move(locators[i].string), names[i]);
    }
    return AllocationColocator<>::ConstUnsafeCursor(
        reinterpret_cast<const std::byte*>(head));
  };

  auto storage = alloc.allocate(
      [&,
       serviceNameLocators = locatorsFor(bypassSets.serviceNames),
       methodNameLocators = locatorsFor(bypassSets.methodNames)](
          auto make) mutable -> MethodNameStorageImpl {
        MethodNameStorageImpl storage;
        storage.serviceNames = InlineNameList{
            bypassSets.serviceNames.size(),
            copyNames(
                std::move(serviceNameLocators), bypassSets.serviceNames, make)};
        storage.methodNames = InlineNameList{
            bypassSets.methodNames.size(),
            copyNames(
                std::move(methodNameLocators), bypassSets.methodNames, make)};
        return storage;
      });

  // We are okay to throw away the custom deleter because there is a
  // specialization of std::default_delete that will invoke it.
  setMethodNameStorageImpl(storage.release());
}

/* static */ bool EventHandlerRuntime::isClientMethodBypassed(
    std::string_view serviceName, std::string_view methodName) {
  std::scoped_lock<folly::rcu_domain> guard(folly::rcu_default_domain());

  const MethodNameStorageImpl* bypassSets =
      globalClientMethodsToBypass.load(std::memory_order_acquire);
  if (bypassSets == nullptr) {
    return false;
  }

  if (bypassSets->serviceNames.find(serviceName)) {
    return true;
  }
  if (bypassSets->methodNames.find(methodName)) {
    return true;
  }
  return false;
}

} // namespace apache::thrift::detail
