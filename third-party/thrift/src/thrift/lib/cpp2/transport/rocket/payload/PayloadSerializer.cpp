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

#include <folly/Singleton.h>

namespace apache::thrift::rocket {

namespace {
auto initializationLock = folly::Singleton<std::mutex>().shouldEagerInit();
static folly::Indestructible<std::optional<PayloadSerializer>> serializer;
} // namespace

bool isSerializerInitialized() {
  return serializer->has_value();
}

void PayloadSerializer::tryInitialize(PayloadSerializer&& src) {
  std::lock_guard<std::mutex> lock(*initializationLock.try_get());
  if (!isSerializerInitialized()) {
    serializer->emplace(std::move(src));
  }
}

void PayloadSerializer::tryInitializeDefault() {
  tryInitializeEmplace(PayloadSerializer(LegacyPayloadSerializerStrategy()));
}

PayloadSerializer& PayloadSerializer::getInstance() {
  if (!FOLLY_UNLIKELY(isSerializerInitialized())) {
    tryInitializeDefault();
  }
  std::optional<PayloadSerializer>& opt = *serializer;
  return *opt;
}

void PayloadSerializer::reset() {
  std::lock_guard<std::mutex> lock(*initializationLock.try_get());
  serializer->reset();
}

} // namespace apache::thrift::rocket
