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

#include <thrift/compiler/lib/cpp2/util.h>

namespace apache {
namespace thrift {
namespace compiler {

bool is_type_iobuf(std::string_view name) {
  return name == "folly::IOBuf" || name == "std::unique_ptr<folly::IOBuf>";
}

bool is_type_iobuf(const t_type* type) {
  return is_type_iobuf(cpp2::get_type(type));
}

} // namespace compiler
} // namespace thrift
} // namespace apache
