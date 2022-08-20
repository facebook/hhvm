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

#include <thrift/lib/cpp2/visitation/visit_by_thrift_field_metadata.h>

#include <fmt/format.h>
#include <folly/lang/Exception.h>

namespace apache {
namespace thrift {
namespace detail {
[[noreturn]] void throwInvalidThriftId(size_t id, std::string_view type) {
  folly::throw_exception<InvalidThriftId>(
      fmt::format("{} is invalid thrift id in struct {}", id, type));
}
} // namespace detail
} // namespace thrift
} // namespace apache
