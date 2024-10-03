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
  delete serializer_.load(std::memory_order_acquire);
}

PayloadSerializer& PayloadSerializer::PayloadSerializerHolder::get() {
  auto* serializer = serializer_.load(std::memory_order_acquire);
  if (FOLLY_UNLIKELY(serializer == nullptr)) {
    auto* newSerializer =
        new PayloadSerializer(LegacyPayloadSerializerStrategy());

    // Try to initialize the serializer
    if (serializer_.compare_exchange_strong(
            serializer,
            newSerializer,
            std::memory_order_release,
            std::memory_order_acquire)) {
      serializer = newSerializer;
    } else {
      // The serializer is already initialized, clean up.
      delete newSerializer;
    }

    FOLLY_SAFE_CHECK(serializer != nullptr, "null serializer");
  }

  return *serializer;
}

void PayloadSerializer::PayloadSerializerHolder::reset() {
  delete serializer_.exchange(nullptr, std::memory_order_acquire);
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
