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

#include <thrift/conformance/cpp2/AnyRef.h>

namespace apache::thrift::conformance {

const std::type_info& any_ref::type() const noexcept {
  if (details_->type == typeid(std::any)) {
    const auto& value = *static_cast<const std::any*>(value_);
    if (value.has_value()) {
      return value.type();
    }
  }
  return details_->type;
}

bool any_ref::has_value() const noexcept {
  if (!has_reference()) {
    return false;
  }
  if (details_->type == typeid(std::any)) {
    const auto& value = *static_cast<const std::any*>(value_);
    return value.has_value();
  }
  return true;
}

} // namespace apache::thrift::conformance
