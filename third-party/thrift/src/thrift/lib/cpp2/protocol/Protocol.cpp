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

#include <thrift/lib/cpp2/protocol/Protocol.h>

#include <folly/portability/GFlags.h>

FOLLY_GFLAGS_DEFINE_int32(
    thrift_protocol_max_depth,
    12500,
    "How many nested struct/list/set/map are allowed");

namespace apache {
namespace thrift {
namespace detail {

ProtocolBase::ProtocolBase()
    : height_(static_cast<size_t>(FLAGS_thrift_protocol_max_depth) + 1) {}
} // namespace detail
} // namespace thrift
} // namespace apache
