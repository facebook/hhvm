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

#include <thrift/lib/cpp2/dynamic/DynamicValue.h>

namespace folly {
struct dynamic;
}

namespace apache::thrift::dynamic {

// Reconstruct a structured annotation value (exposed by SyntaxGraph as a
// folly::dynamic keyed by field name) into a schema-typed DynamicValue of the
// given type. Lives in its own target so SyntaxGraph-backed descriptor builders
// can share it without pulling folly::dynamic into ServiceDescriptor or forming
// a schema/dynamic dependency cycle.
DynamicValue toDynamicValue(
    const folly::dynamic& value, const type_system::TypeRef& type);

} // namespace apache::thrift::dynamic
