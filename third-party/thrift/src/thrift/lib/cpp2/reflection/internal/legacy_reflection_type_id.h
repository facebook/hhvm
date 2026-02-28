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

#include <folly/Range.h>
#include <thrift/lib/thrift/gen-cpp2/reflection_types.h>

namespace apache::thrift::legacy_reflection_detail {

using id_t = std::uint64_t;

extern id_t get_type_id(reflection::Type type, folly::StringPiece name);

} // namespace apache::thrift::legacy_reflection_detail
