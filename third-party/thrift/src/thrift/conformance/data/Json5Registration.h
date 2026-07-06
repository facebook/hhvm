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

#include <thrift/conformance/cpp2/AnyRegistry.h>

namespace apache::thrift::conformance::data {

// Registers Json5 serializers for the round-trip suite's types (protocol::Value
// and the testset struct/union variants). The `any` codegen option emits only
// Binary/Compact/SimpleJson, so Json5 must be registered manually.
void registerRoundTripJson5Serializers(AnyRegistry& registry);

} // namespace apache::thrift::conformance::data
