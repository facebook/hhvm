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

#include <string_view>

#include <thrift/compiler/ast/t_type.h>

namespace apache {
namespace thrift {
namespace compiler {

/**
 * Currently just a string match for cpp.Type override matches folly::IOBuf or
 * unique_ptr<folly::IOBuf>. In future, this should be based on @python.IOBuf{}
 * annotation:
 *  1. avoids adding @cpp.Type annotation to thrift IDL that
 *     may not have any cpp2 usage.
 *  2. avoid any inconsistency due to top-level "::" usage.
 */
bool is_type_iobuf(std::string_view name);

bool is_type_iobuf(const t_type* type);

} // namespace compiler
} // namespace thrift
} // namespace apache
