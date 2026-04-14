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

#include <thrift/compiler/ast/t_type.h>

#include <stdexcept>

#include <thrift/compiler/ast/scope_identifier.h>
#include <thrift/compiler/ast/t_enum.h>
#include <thrift/compiler/ast/t_primitive_type.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_typedef.h>

namespace apache::thrift::compiler {

const t_type* t_type::get_true_type() const {
  return t_typedef::find_type_if(
      this, [](const t_type* type) { return !type->is<t_typedef>(); });
}

bool t_type_ref::resolved() const noexcept {
  if (type_ == nullptr) {
    return false;
  }
  if (unresolved_typedef_ != nullptr) {
    return unresolved_typedef_->type().resolved();
  }
  return unresolved_name_.empty();
}

bool t_type_ref::resolve() {
  if (resolved() || empty()) {
    // Already resolved
    return true;
  }

  // Try to resolve via the placeholder typedef
  if (unresolved_typedef_ != nullptr) {
    if (!unresolved_typedef_->resolve()) {
      return false;
    }

    // Try to excise the placeholder typedef so dynamic_cast works.
    if (unresolved_typedef_->unstructured_annotations().empty()) {
      type_ = unresolved_typedef_->type().get_type();
    }
    unresolved_typedef_ = nullptr;
    unresolved_program_ = nullptr;
    unresolved_name_.clear();
    return true;
  }

  if (unresolved_program_ == nullptr) {
    return false;
  }

  const t_type* resolved_type = unresolved_program_->find<t_type>(
      scope::identifier{unresolved_name_, range_});
  if (resolved_type == nullptr) {
    return false;
  }

  type_ = resolved_type;
  unresolved_typedef_ = nullptr;
  unresolved_program_ = nullptr;
  unresolved_name_.clear();
  return true;
}

const t_type& t_type_ref::deref() {
  if (!resolve()) {
    throw std::runtime_error("Could not resolve type: " + unresolved_name_);
  }
  return deref_or_throw();
}

const t_type& t_type_ref::deref_or_throw() const {
  if (type_ == nullptr) {
    if (!unresolved_name_.empty()) {
      throw std::runtime_error("Could not resolve type: " + unresolved_name_);
    }
    throw std::runtime_error("t_type_ref has no type.");
  }
  if (auto ph = dynamic_cast<const t_placeholder_typedef*>(type_)) {
    return ph->type().deref();
  }
  return *type_;
}

t_type_ref t_type_ref::for_placeholder(t_placeholder_typedef& unresolved_type) {
  t_type_ref ref{unresolved_type, unresolved_type, unresolved_type.src_range()};
  // NOTE: not part of the constructor initializers because
  // `t_placeholder_typedef` is incomplete in `t_type.h`, since it's defined in
  // `t_typedef.h` which depends on `t_type.h`
  ref.unresolved_program_ = unresolved_type.program();
  ref.unresolved_name_ = unresolved_type.name();
  return ref;
}

const t_type_ref& t_type_ref::none() {
  static const t_type_ref empty;
  return empty;
}

bool is_scalar(const t_type& type) {
  if (type.is<t_enum>()) {
    return true;
  }

  const auto* primitive = type.try_as<t_primitive_type>();
  return primitive != nullptr &&
      primitive->primitive_type() != t_primitive_type::type::t_binary &&
      primitive->primitive_type() != t_primitive_type::type::t_string &&
      primitive->primitive_type() != t_primitive_type::type::t_void;
}

} // namespace apache::thrift::compiler
