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

#include <array>
#include <cstdint>

#include <thrift/lib/cpp2/dynamic/TypeSystem.h>
#include <thrift/lib/thrift/gen-cpp2/type_system_types.h>

namespace apache::thrift::type_system {

// SHA-256 digest for `TypeSystem`
using TypeSystemDigest = std::array<std::byte, 32>;

// Current hash algorithm version.
// This allows us to introduce backwards-incompatible changes in the future.
inline constexpr uint8_t kTypeSystemDigestVersion = 2;

/**
 * Compute a canonical & deterministic SHA-256 digest of a `TypeSystem`.
 *
 * Properties:
 * - Equivalent TypeSystems always produces the same digest
 * - Order-independent: URI ordering does not matter
 * - Excludes sourceInfo: File paths are not semantically significant
 *
 * Floating-point values (in custom defaults and annotations) are hashed by
 * their IEEE 754 bit representation. This relies on SerializableRecord
 * rejecting NaN and negative zero at construction time to guarantee
 * determinism.
 */
struct TypeSystemHasher {
  /**
   * Compute the digest for all types in the type system.
   */
  TypeSystemDigest operator()(const SerializableTypeSystem& typeSystem) const;

  /**
   * Compute the digest for all types in the type system.
   */
  TypeSystemDigest operator()(const TypeSystem& typeSystem) const;

  /**
   * Compute a digest for a user-defined type reference.
   *
   * Only accepts structured types: struct, union, enum, or opaque alias.
   * For these types, this produces the same digest as hashing the equivalent
   * SerializableTypeDefinition.
   *
   * @throws std::invalid_argument if typeRef is a primitive or container type.
   */
  TypeSystemDigest operator()(const TypeRef& typeRef) const;

  /**
   * Compute a digest for a serializable type definition.
   *
   * This produces the same digest as hashing the equivalent runtime TypeRef
   * for user-defined types.
   */
  TypeSystemDigest operator()(const SerializableTypeDefinition& def) const;
  TypeSystemDigest operator()(const SerializableStructDefinition& def) const;
  TypeSystemDigest operator()(const SerializableUnionDefinition& def) const;
  TypeSystemDigest operator()(const SerializableEnumDefinition& def) const;
  TypeSystemDigest operator()(
      const SerializableOpaqueAliasDefinition& def) const;

  /**
   * Compute a digest for a type identity.
   *
   * The runtime TypeId and serializable TypeIdUnion produce the same digest
   * for equivalent type identities.
   */
  TypeSystemDigest operator()(const TypeId& typeId) const;
  TypeSystemDigest operator()(const TypeIdUnion& typeId) const;
};

} // namespace apache::thrift::type_system
