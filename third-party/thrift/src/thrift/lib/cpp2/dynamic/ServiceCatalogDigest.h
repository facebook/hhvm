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
#include <cstddef>
#include <cstdint>

#include <thrift/lib/cpp2/dynamic/ServiceDescriptor.h>
#include <thrift/lib/cpp2/dynamic/TypeSystemDigest.h>

namespace apache::thrift::type_system {
class SerializableServiceCatalog;
} // namespace apache::thrift::type_system

namespace apache::thrift::dynamic {

// SHA-256 digest for `SerializableServiceCatalog`
using ServiceCatalogDigest = std::array<std::byte, 32>;
using ServiceDescriptorDigest = ServiceCatalogDigest;

// Current hash algorithm version.
// This allows us to introduce backwards-incompatible changes in the future.
inline constexpr uint8_t kServiceCatalogDigestVersion = 1;

/**
 * Compute a canonical & deterministic SHA-256 digest of a
 * `SerializableServiceCatalog`.
 *
 * Properties:
 * - Equivalent service catalogs always produce the same digest
 * - Order-independent: URI, function, and field-id ordering does not matter
 * - Type universes are represented by `TypeSystemDigest`
 * - Inline `types` and out-of-band `typesDigest` forms hash equivalently
 *
 * The runtime `ServiceDescriptor` overload hashes the same service-catalog
 * projection directly from the descriptor.
 *
 * Floating-point values in annotations are hashed by their IEEE 754 bit
 * representation, matching `TypeSystemHasher`.
 */
struct ServiceCatalogHasher {
  // Selects what the digest covers; see DigestMode. Structural is an opt-in for
  // callers that treat annotation differences as equivalent.
  type_system::DigestMode mode = type_system::DigestMode::Full;

  /**
   * Compute the digest for all interfaces in the service catalog.
   */
  ServiceCatalogDigest operator()(
      const type_system::SerializableServiceCatalog& catalog) const;

  /**
   * Compute the digest for the descriptor serialized under serviceUri.
   */
  ServiceCatalogDigest operator()(
      const ServiceDescriptor& descriptor,
      type_system::UriView serviceUri) const;
};

using ServiceDescriptorHasher = ServiceCatalogHasher;

} // namespace apache::thrift::dynamic
