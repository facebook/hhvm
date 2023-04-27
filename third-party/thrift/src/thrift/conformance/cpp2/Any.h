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

#include <thrift/conformance/cpp2/Protocol.h>
#include <thrift/conformance/if/gen-cpp2/any_types.h>

namespace apache::thrift::conformance {

// Returns the protocol used by the given any.
Protocol getProtocol(const Any& any) noexcept;

// Returns true iff the any is encoded using the given protocol.
bool hasProtocol(const Any& any, const Protocol& protocol) noexcept;

// Sets the given protocol on the given any.
void setProtocol(const Protocol& protocol, Any& any) noexcept;

// Raises std::invalid_argument if invalid.
void validateAny(const Any& any);

} // namespace apache::thrift::conformance
