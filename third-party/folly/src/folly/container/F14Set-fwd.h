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

#include <folly/container/detail/F14Defaults.h>
#include <folly/memory/MemoryResource.h>

namespace folly {
template <
    typename Key,
    typename Hasher = f14::DefaultHasher<Key>,
    typename KeyEqual = f14::DefaultKeyEqual<Key>,
    typename Alloc = f14::DefaultAlloc<Key>>
class F14NodeSet;

template <
    typename Key,
    typename Hasher = f14::DefaultHasher<Key>,
    typename KeyEqual = f14::DefaultKeyEqual<Key>,
    typename Alloc = f14::DefaultAlloc<Key>>
class F14ValueSet;

template <
    typename Key,
    typename Hasher = f14::DefaultHasher<Key>,
    typename KeyEqual = f14::DefaultKeyEqual<Key>,
    typename Alloc = f14::DefaultAlloc<Key>>
class F14VectorSet;

template <
    typename Key,
    typename Hasher = f14::DefaultHasher<Key>,
    typename KeyEqual = f14::DefaultKeyEqual<Key>,
    typename Alloc = f14::DefaultAlloc<Key>>
class F14FastSet;

#if FOLLY_HAS_MEMORY_RESOURCE
namespace pmr {
template <
    typename Key,
    typename Hasher = f14::DefaultHasher<Key>,
    typename KeyEqual = f14::DefaultKeyEqual<Key>>
using F14NodeSet = folly::F14NodeSet<
    Key,
    Hasher,
    KeyEqual,
    folly::detail::std_pmr::polymorphic_allocator<Key>>;

template <
    typename Key,
    typename Hasher = f14::DefaultHasher<Key>,
    typename KeyEqual = f14::DefaultKeyEqual<Key>>
using F14ValueSet = folly::F14ValueSet<
    Key,
    Hasher,
    KeyEqual,
    folly::detail::std_pmr::polymorphic_allocator<Key>>;

template <
    typename Key,
    typename Hasher = f14::DefaultHasher<Key>,
    typename KeyEqual = f14::DefaultKeyEqual<Key>>
using F14VectorSet = folly::F14VectorSet<
    Key,
    Hasher,
    KeyEqual,
    folly::detail::std_pmr::polymorphic_allocator<Key>>;

template <
    typename Key,
    typename Hasher = f14::DefaultHasher<Key>,
    typename KeyEqual = f14::DefaultKeyEqual<Key>>
using F14FastSet = folly::F14FastSet<
    Key,
    Hasher,
    KeyEqual,
    folly::detail::std_pmr::polymorphic_allocator<Key>>;
} // namespace pmr
#endif // FOLLY_HAS_MEMORY_RESOURCE

} // namespace folly
