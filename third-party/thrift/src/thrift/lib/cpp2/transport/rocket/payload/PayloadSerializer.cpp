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

#include <thrift/lib/cpp2/transport/rocket/payload/PayloadSerializer.h>

namespace apache::thrift::rocket {

PayloadSerializer::PayloadSerializerHolder::~PayloadSerializerHolder() {
  PayloadSerializer* serializer = serializer_;
  if (serializer) {
    delete serializer;
  }
}

PayloadSerializer& PayloadSerializer::PayloadSerializerHolder::get() {
  auto* serializer = serializer_.load(std::memory_order_relaxed);

  // Fast path when the serializer is already initialized
  if (FOLLY_LIKELY(serializer != nullptr)) {
    return *serializer;
  } else {
    // Slow path when the serializer is not initialized yet that
    // uses a compare-and-swap to initialize it. Avoids the need for a lock.
    auto* newSerializer =
        new PayloadSerializer(LegacyPayloadSerializerStrategy());
    for (;;) {
      // Load the current serializer
      auto* expected = serializer_.load(std::memory_order_relaxed);

      // Check if the serializer is already initialized
      if (expected == nullptr) {
        // Try to initialize the serializer
        if (serializer_.compare_exchange_strong(
                expected, newSerializer, std::memory_order_release)) {
          return *newSerializer;
        }
      } else {
        // The serializer is already initialized, return it and clean up.
        delete newSerializer;
        return *expected;
      }
    }
  }
}

void PayloadSerializer::PayloadSerializerHolder::reset() {
  auto* serializer = serializer_.exchange(nullptr);
  if (serializer) {
    delete serializer;
  }
}

PayloadSerializer::PayloadSerializerHolder&
PayloadSerializer::getPayloadSerializerHolder() {
  static folly::Indestructible<PayloadSerializer::PayloadSerializerHolder>
      holder;

  return *holder;
}

PayloadSerializer& PayloadSerializer::getInstance() {
  return getPayloadSerializerHolder().get();
}

void PayloadSerializer::reset() {
  getPayloadSerializerHolder().reset();
}

} // namespace apache::thrift::rocket
