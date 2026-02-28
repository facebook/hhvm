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
  if (auto s = serializer_.load(std::memory_order_acquire)) {
    s->retire();
  }
}

PayloadSerializer::Ptr PayloadSerializer::PayloadSerializerHolder::get() {
  PayloadSerializer::Ptr ptr(serializer_);
  if (FOLLY_UNLIKELY(ptr == nullptr)) {
    auto* newSerializer =
        new PayloadSerializer(DefaultPayloadSerializerStrategy());

    // Try to initialize the serializer
    if (!serializer_.compare_exchange_strong(
            ptr.serializer_,
            newSerializer,
            std::memory_order_release,
            std::memory_order_relaxed)) {
      // The serializer is already initialized, clean up.
      delete newSerializer;
    }
    // Call get() again to get the initialized serializer.
    return get();
  } else {
    return ptr;
  }
}

void PayloadSerializer::PayloadSerializerHolder::reset() {
  if (auto s = serializer_.exchange(nullptr, std::memory_order_acquire)) {
    s->retire();
  }
}

PayloadSerializer::PayloadSerializerHolder&
PayloadSerializer::getPayloadSerializerHolder() {
  static folly::Indestructible<PayloadSerializer::PayloadSerializerHolder>
      holder;

  return *holder;
}

PayloadSerializer::Ptr PayloadSerializer::getInstance() {
  return getPayloadSerializerHolder().get();
}

void PayloadSerializer::reset() {
  getPayloadSerializerHolder().reset();
}

} // namespace apache::thrift::rocket
