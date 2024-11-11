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

#include <thrift/lib/cpp2/gen/module_constants_cpp.h>

namespace apache::thrift::detail::mc {

::std::string_view readSchema(::std::string_view (*access)()) {
  return access == nullptr ? ::std::string_view() : access();
}

::std::string_view readSchemaInclude(
    ::folly::Range<const ::std::string_view*> (*access)(),
    ::std::size_t index) {
  return access == nullptr ? ::std::string_view() : access()[index];
}

} // namespace apache::thrift::detail::mc
