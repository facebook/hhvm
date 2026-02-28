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

namespace apache::thrift {

/**
 * The type class of a type as declared in a .thrift file.
 *
 * Primarily useful in the case where it is desired to select an appropriate
 * template partial specialization for dealing with some data type declared or
 * used in a .thrift file, without having available a full specialization. Full
 * specializations cannot generally be available, as Thrift supports the use of
 * custom types with the `cpp.type` and `cpp.template` annotations.
 */
namespace type_class {
/**
 * A sentinel type-class indicating that the actual type-class is unknown.
 */
struct unknown {};

/**
 * Represents types with no actual data representation. Most commonly `void`.
 */
struct nothing {};

/**
 * Represents all signed and unsigned integral types, including `bool`.
 */
struct integral {};

/**
 * Represents all floating point types.
 */
struct floating_point {};

/**
 * Represents opaque binary data.
 */
struct binary {};

/**
 * Represents all known string implementations.
 */
struct string {};

/**
 * Represents an enum.
 */
struct enumeration {};

/**
 * Represents an class or structure.
 */
struct structure {};

/**
 * Represents a variant (or union, as the Thrift IDL grammar goes).
 */
struct variant {};

/**
 * Represents all list implementations.
 */
template <typename ValueTypeClass>
struct list {};

/**
 * Represents all set implementations.
 */
template <typename ValueTypeClass>
struct set {};

/**
 * Represents all map implementations.
 */
template <typename KeyTypeClass, typename MappedTypeClass>
struct map {};

} // namespace type_class

namespace detail {
/*
 * A placeholder tag type for indicating indirection.
 */
template <typename TypeClass, typename Indirection>
struct indirection_tag;
} // namespace detail
} // namespace apache::thrift
