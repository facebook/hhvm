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

namespace apache::thrift::detail {
enum class ReservedId : int16_t {
  kOffset = -32768, // Lazy deserialization
  kIndex = -32767, // Lazy deserialization
  kExpectedRandomNumber = -32766, // Lazy deserialization
  kActualRandomNumber = -32765, // Lazy deserialization
  kXxh3Checksum = -32764, // Lazy deserialization
  kInjectMetadataFieldsLastId = -2000, // @internal.InjectMetadataFields
  kInjectMetadataFieldsStartId = -1000, // @internal.InjectMetadataFields
};
} // namespace apache::thrift::detail
