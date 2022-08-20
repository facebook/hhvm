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

#include <folly/CPortability.h>

// This is a reimplementation of module_sinit for custom type
// github.com/facebook/fbthrift/blob/main/thrift/compiler/generate/templates/cpp2/module_sinit.cpp.mustache
//
// TODO(ytj): Implement this logic in mustache

namespace apache::thrift::conformance::register_protocol_object_and_value {
extern FOLLY_EXPORT bool static_init_protocol_object_and_value;
FOLLY_EXPORT bool static_reg_protocol_object_and_value =
    static_init_protocol_object_and_value;
} // namespace apache::thrift::conformance::register_protocol_object_and_value
