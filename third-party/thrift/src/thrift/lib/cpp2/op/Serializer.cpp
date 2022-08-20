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

#include <thrift/lib/cpp2/op/Serializer.h>

#include <folly/lang/Exception.h>

namespace apache {
namespace thrift {
namespace op {
using type::AnyValue;
using type::Type;

AnyValue Serializer::decode(const Type& type, folly::io::Cursor& cursor) const {
  AnyValue result;
  decode(type, cursor, result);
  return result;
}

void Serializer::checkType(const Type& actual, const Type& expected) {
  if (actual != expected) {
    // TODO(afuller): Provide a helpful error message.
    folly::throw_exception<std::bad_any_cast>();
  }
}

} // namespace op
} // namespace thrift
} // namespace apache
