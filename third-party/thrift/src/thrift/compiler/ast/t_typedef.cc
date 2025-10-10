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

#include <thrift/compiler/ast/t_typedef.h>

#include <thrift/compiler/ast/t_program.h>

namespace apache::thrift::compiler {

const std::string* t_typedef::get_first_unstructured_annotation_or_null(
    const t_type* type, const std::vector<std::string_view>& names) {
  const std::string* result = nullptr;
  find_type_if(type, [&result, &names](const t_type* type) {
    return (result = type->find_unstructured_annotation_or_null(names)) !=
        nullptr;
  });
  return result;
}

const t_const* t_typedef::get_first_structured_annotation_or_null(
    const t_type* type, const char* uri) {
  const t_const* result = nullptr;
  find_type_if(type, [&result, uri](const t_type* type) {
    return (result = type->find_structured_annotation_or_null(uri)) != nullptr;
  });
  return result;
}

t_typedef::kind t_typedef::typedef_kind() const {
  if (this->is<t_placeholder_typedef>()) {
    assert(!unnamed_);
    return kind::placeholder;
  } else if (unnamed_) {
    return kind::unnamed;
  } else {
    return kind::defined;
  }
}

std::unique_ptr<t_typedef> t_typedef::make_unnamed(
    const t_program* program, std::string name, t_type_ref type) {
  auto ret = std::make_unique<t_typedef>(program, std::move(name), type);
  ret->unnamed_ = true;
  return ret;
}

bool t_placeholder_typedef::resolve() {
  if (type_.empty()) {
    type_ = t_type_ref::from_ptr(
        program()->find<t_type>(scope::identifier{name(), src_range()}));
    if (!type_.empty()) {
      // Update the type to mirror the underlying one.
      // TODO(afuller): Update codegen to always skip over placeholders via
      // type_ref instead.
      set_name(type_->name());
      set_program(type_->program());
    }
  }
  return !type_.empty();
}

} // namespace apache::thrift::compiler
