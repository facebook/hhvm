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
#include <cstdint>
#include <type_traits>

namespace apache {
namespace thrift {
namespace protocol {
enum class PathSegmentId : std::int64_t {};
}
namespace type {
enum class FieldId : std::int16_t {};
enum class ValueId : std::int64_t {};
enum class ProtocolId : std::int64_t {};
enum class TypeId : std::int64_t {};
enum class DefinitionId : std::int64_t {};
enum class PackageId : std::int64_t {};
enum class ProgramId : std::int64_t {};
enum class SourceId : std::int64_t {};
namespace detail {
template <class StrongInteger>
class StrongIntegerAdapter {
  using T = std::underlying_type_t<StrongInteger>;

 public:
  static StrongInteger fromThrift(T t) { return static_cast<StrongInteger>(t); }
  static T toThrift(StrongInteger t) { return static_cast<T>(t); }
};

} // namespace detail
} // namespace type
} // namespace thrift
} // namespace apache
