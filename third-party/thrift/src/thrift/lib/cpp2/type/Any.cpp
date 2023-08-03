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

#include <thrift/lib/cpp2/type/Any.h>

#include <stdexcept>

#include <fmt/format.h>
#include <folly/lang/Exception.h>
#include <thrift/lib/cpp2/protocol/DebugProtocol.h>

namespace apache {
namespace thrift {
namespace type {

AnyData::AnyData(SemiAny semiAny) {
  if (semiAny.type() == void_t{}) {
    folly::throw_exception<std::runtime_error>("SemiAny missing type");
  }
  if (semiAny.protocol()->empty()) {
    folly::throw_exception<std::runtime_error>("SemiAny missing protocol");
  }
  data_.type() = std::move(*semiAny.type());
  data_.protocol() = std::move(*semiAny.protocol());
  data_.data() = std::move(*semiAny.data());
}

void AnyData::throwTypeMismatchException(const Type& want, const Type& actual) {
  folly::throw_exception<std::runtime_error>(fmt::format(
      "Type mismatch. Want: {}, Actual: {}.",
      debugStringViaEncode(want),
      debugStringViaEncode(actual)));
}

SemiAny AnyData::moveToSemiAny() && {
  SemiAny semiAny;
  semiAny.data() = std::move(*data_.data());
  semiAny.type() = std::move(*data_.type());
  semiAny.protocol() = std::move(*data_.protocol());
  return semiAny;
}

} // namespace type
} // namespace thrift
} // namespace apache
