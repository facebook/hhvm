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

#include <thrift/lib/cpp2/op/StdSerializer.h>

// NOTE: link_whole is enabled for this file, all symbols won't be dropped from
// the binary. Please keep it as small as possible.

namespace apache {
namespace thrift {
namespace type {
namespace {
FOLLY_MAYBE_UNUSED const auto registerPrimitiveTypes = [] {
  auto registry = [](auto tag) {
    op::registerStdSerializers<
        decltype(tag),
        StandardProtocol::SimpleJson,
        StandardProtocol::Compact,
        StandardProtocol::Binary>(detail::getGeneratedTypeRegistry());
  };

  registry(bool_t{});
  registry(byte_t{});
  registry(i16_t{});
  registry(i32_t{});
  registry(i64_t{});
  registry(float_t{});
  registry(double_t{});
  registry(string_t{});
  registry(binary_t{});
  return 0;
}();
}
} // namespace type
} // namespace thrift
} // namespace apache
