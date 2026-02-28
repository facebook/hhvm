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

#include <thrift/conformance/cpp2/AnySerializer.h>

namespace apache::thrift::conformance {

std::any AnySerializer::decode(
    const std::type_info& typeInfo, folly::io::Cursor& cursor) const {
  std::any result;
  decode(typeInfo, cursor, result);
  return result;
}

void AnySerializer::checkType(
    const std::type_info& actual, const std::type_info& expected) {
  if (actual != expected) {
    folly::throw_exception(std::bad_any_cast());
  }
}

} // namespace apache::thrift::conformance
