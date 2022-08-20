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

#include <thrift/compiler/ast/t_enum.h>

#include <stdexcept>

namespace apache {
namespace thrift {
namespace compiler {

void t_enum::set_values(t_enum_value_list values) {
  values_.clear();
  values_raw_.clear();
  constants_.clear();
  for (auto& value : values) {
    append(std::move(value));
  }
}

void t_enum::append_value(std::unique_ptr<t_enum_value> enum_value) {
  if (!enum_value->has_value()) {
    auto last_value = values_.empty() ? -1 : values_.back()->get_value();
    if (last_value == INT32_MAX) {
      throw std::runtime_error(
          "enum value overflow: " + enum_value->get_name());
    }
    enum_value->set_implicit_value(last_value + 1);
  }
  auto const_val = std::make_unique<t_const_value>(enum_value->get_value());
  const_val->set_is_enum();
  const_val->set_enum(this);
  const_val->set_enum_value(enum_value.get());
  auto tconst = std::make_unique<t_const>(
      program_,
      &t_base_type::t_i32(),
      enum_value->get_name(),
      std::move(const_val));
  append(std::move(enum_value), std::move(tconst));
}

const t_enum_value* t_enum::find_value(int32_t value) const {
  for (const auto& ev : values_) {
    if (ev->get_value() == value) {
      return ev.get();
    }
  }
  return nullptr;
}

} // namespace compiler
} // namespace thrift
} // namespace apache
