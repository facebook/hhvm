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

#include <thrift/lib/cpp2/transport/rocket/compression/CustomCompressorRegistry.h>

#include <folly/concurrency/AtomicSharedPtr.h>

namespace apache::thrift::rocket {
namespace {

/**
 * Implements a global, thread-safe registry of compressors via an RCU pattern.
 * (We expect updates to be extremely infrequent, and reads quite frequent.)
 */
class CustomCompressorRegistryInstance {
 private:
  using Factory = std::shared_ptr<const CustomCompressorFactory>;
  using NameFactoryPair = std::pair<std::string, Factory>;

 public:
  static CustomCompressorRegistryInstance& instance() {
    // Intentionally leak this registry as it may be used during global object
    // destruction.
    static CustomCompressorRegistryInstance* static_instance =
        new CustomCompressorRegistryInstance{};
    DCHECK(static_instance);
    return *static_instance;
  }

  bool registerFactory(Factory factory) {
    auto existing = nameFactoryPair_.load();
    if (existing) {
      return false;
    }

    auto name = factory->getCompressorName();
    auto updated =
        std::make_shared<NameFactoryPair>(std::move(name), std::move(factory));
    return nameFactoryPair_.compare_exchange_strong(existing, updated);
  }

  bool unregister(const std::string& name) {
    auto existing = nameFactoryPair_.load();
    if (!existing) {
      return false;
    }

    if (existing->first != name) {
      return false;
    }

    auto updated = std::shared_ptr<NameFactoryPair>();
    return nameFactoryPair_.compare_exchange_strong(existing, updated);
  }

  Factory get(const std::string& name) const {
    auto pair = nameFactoryPair_.load();
    if (!pair) {
      return nullptr;
    }

    if (pair->first != name) {
      return nullptr;
    }

    return pair->second;
  }

 private:
  CustomCompressorRegistryInstance() = default;

  folly::atomic_shared_ptr<const NameFactoryPair> nameFactoryPair_;
};

} // namespace

bool CustomCompressorRegistry::registerFactory(
    std::shared_ptr<const CustomCompressorFactory> factory) {
  return CustomCompressorRegistryInstance::instance().registerFactory(
      std::move(factory));
}

bool CustomCompressorRegistry::unregister(const std::string& name) {
  return CustomCompressorRegistryInstance::instance().unregister(name);
}

std::shared_ptr<const CustomCompressorFactory> CustomCompressorRegistry::get(
    const std::string& name) {
  return CustomCompressorRegistryInstance::instance().get(name);
}

} // namespace apache::thrift::rocket
