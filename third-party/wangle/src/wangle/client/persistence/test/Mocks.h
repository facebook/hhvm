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

#include <folly/Optional.h>
#include <folly/portability/GMock.h>
#include <wangle/client/persistence/PersistentCache.h>

namespace wangle {

template <typename K, typename V>
class MockPersistentCache : public PersistentCache<K, V> {
 public:
  MOCK_METHOD1_T(get, folly::Optional<V>(const K&));
  MOCK_METHOD2_T(put, void(const K&, const V&));
  MOCK_METHOD1_T(remove, bool(const K&));
  MOCK_METHOD1_T(clear, void(bool));
  MOCK_METHOD0_T(size, size_t());
};

} // namespace wangle
