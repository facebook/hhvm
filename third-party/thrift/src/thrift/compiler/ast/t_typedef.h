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

#pragma once

#include <memory>
#include <string>

#include <thrift/compiler/ast/t_const.h>
#include <thrift/compiler/ast/t_type.h>

namespace apache {
namespace thrift {
namespace compiler {

/**
 * Represents a typedef definition.
 *
 * A typedef introduces a named alias of a type.
 */
class t_typedef : public t_type {
 public:
  t_typedef(t_program* program, std::string name, t_type_ref type)
      : t_type(program, std::move(name)), type_(std::move(type)) {}

  const t_type_ref& type() const { return type_; }

  // Returns the first type, in the typedef type hierarchy, matching the
  // given predicate or nullptr.
  template <typename UnaryPredicate>
  static const t_type* find_type_if(const t_type* type, UnaryPredicate&& pred) {
    while (true) {
      if (!type) {
        return nullptr;
      }
      if (pred(type)) {
        return type;
      }
      if (const auto* as_typedef = dynamic_cast<const t_typedef*>(type)) {
        type = as_typedef->get_type();
      } else {
        return nullptr;
      }
    };
  }

  // Finds the first matching annoation in the typedef's type hierarchy.
  // Return null if not found.
  static const std::string* get_first_annotation_or_null(
      const t_type* type, const std::vector<std::string_view>& names);

  // Finds the first matching annoation in the typedef's type hierarchy.
  // Return default_value or "" if not found.
  template <typename D = const std::string*>
  static auto get_first_annotation(
      const t_type* type,
      const std::vector<std::string_view>& names,
      D&& default_value = nullptr) {
    return annotation_or(
        get_first_annotation_or_null(type, names),
        std::forward<D>(default_value));
  }

  // Finds the first matching structured annoation in the typedef's hierarchy.
  // Return null if not found.
  static const t_const* get_first_structured_annotation_or_null(
      const t_type* type, const char* uri);

  std::string get_full_name() const override { return get_scoped_name(); }

 protected:
  t_type_ref type_;

  // TODO(afuller): Remove everything below here, as it is just provided for
  // backwards compatibility.
 public:
  t_typedef(t_program* program, const t_type* type, std::string name, void*)
      : t_typedef(program, std::move(name), t_type_ref::from_req_ptr(type)) {}

  const t_type* get_type() const { return type_.get_type(); }
  void set_type(t_type_ref type) { type_ = type; }

  bool is_typedef() const override { return true; }
  t_type::type get_type_value() const override {
    return get_type()->get_type_value();
  }

  uint64_t get_type_id() const override { return get_type()->get_type_id(); }
  const std::string& get_symbolic() const { return name(); }

  enum class kind {
    defined,
    unnamed,
    placeholder,
  };
  kind typedef_kind() const;
  static std::unique_ptr<t_typedef> make_unnamed(
      t_program* program, std::string name, t_type_ref type);

 private:
  bool unnamed_{false};
};

// A placeholder for a type that can't be resolved at parse time.
//
// TODO(afuller): Merge this class with t_type_ref and resolve all types after
// parsing. This class assumes that, since the type was referenced by name, it
// is safe to create a dummy typedef to use as a proxy for the original type.
// However, this actually breaks dynamic_cast for t_node and t_type::is_* calls,
// resulting in a lot of subtle bugs that may or may not show up, depending on
// the order of IDL declarations.
class t_placeholder_typedef final : public t_typedef {
 public:
  t_placeholder_typedef(t_program* program, std::string name)
      : t_typedef(program, std::move(name), {}) {}

  /**
   * Resolve and find the actual type that the symbolic name refers to.
   * Return true iff the type exists in the scope.
   */
  bool resolve();

  std::string get_full_name() const override {
    return type_ ? type_->get_full_name() : name_;
  }
};

} // namespace compiler
} // namespace thrift
} // namespace apache
