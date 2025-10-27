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

#include <stdint.h>

#include <stdexcept>
#include <string>
#include <utility>

#include <thrift/compiler/ast/t_named.h>

namespace apache::thrift::compiler {

class t_program;
class t_placeholder_typedef;
class t_type;

namespace detail {

template <typename... Visitors>
extern decltype(auto) visit_type(const t_type& ty, Visitors&&... visitors);

}

/**
 * Generic representation of a thrift type.
 *
 * These objects are used by the parser module to build up a tree of object
 * that are all explicitly typed. The generic t_type class exports a variety
 * of useful methods that are used by the code generator to branch based
 * upon different handling for the various types.
 */
class t_type : public t_named {
 public:
  // Returns the full name for the given type. For example:
  // `list<string, string>`
  virtual std::string get_full_name() const { return get_scoped_name(); }

  /**
   * Resolves all typedefs (if any) to get the true type.
   */
  const t_type* get_true_type() const;

  // Conversions and checks to more specific types.
  template <typename T>
  bool is() const {
    static_assert(std::is_same_v<std::decay_t<std::remove_pointer_t<T>>, T>);
    return dynamic_cast<const T*>(this) != nullptr;
  }
  template <typename T>
  T& as() {
    static_assert(std::is_same_v<std::decay_t<std::remove_pointer_t<T>>, T>);
    return dynamic_cast<T&>(*this);
  }
  template <typename T>
  const T& as() const {
    static_assert(std::is_same_v<std::decay_t<std::remove_pointer_t<T>>, T>);
    return dynamic_cast<const T&>(*this);
  }
  template <typename T>
  T* try_as() {
    static_assert(std::is_same_v<std::decay_t<std::remove_pointer_t<T>>, T>);
    return dynamic_cast<T*>(this);
  }
  template <typename T>
  const T* try_as() const {
    static_assert(std::is_same_v<std::decay_t<std::remove_pointer_t<T>>, T>);
    return dynamic_cast<const T*>(this);
  }

  // Visitor API
  // Note: this requires including the `type_visitor.h` header to prevent a
  // circular dependency.
  template <typename... Visitors>
  decltype(auto) visit(Visitors&&... visitors) const {
    return detail::visit_type(*this, std::forward<Visitors>(visitors)...);
  }

 protected:
  /**
   * Default constructor for t_type
   *
   * A t_type object can't be initialized by itself. The constructors
   * are protected and only t_type's children can initialize it.
   */
  t_type() = default;

  /**
   * Constructor for t_type
   *
   * @param program - An entire thrift program
   */
  explicit t_type(const t_program* program) : t_named(program) {}

  /**
   * Constructor for t_type
   *
   * @param name - The symbolic name of the thrift type
   */
  explicit t_type(std::string name) : t_named(nullptr, std::move(name)) {}

  /**
   * Constructor for t_type
   *
   * @param program - An entire thrift program
   * @param name    - The symbolic name of the thrift type
   */
  t_type(const t_program* program, std::string name)
      : t_named(program, std::move(name)) {}

  // TODO(T227540797): Delete everything below this point. It's only here for
  // backwards compatibility.
 public:
  /*
   * All the thrift supported types
   */
  enum class type {
    // Base types.
    t_void = 0,
    t_bool = 2,
    t_byte = 3,
    t_i16 = 4,
    t_i32 = 5,
    t_i64 = 6,
    t_float = 15,
    t_double = 7,
    t_string = 1,
    t_binary = 18,

    // Container types.
    t_list = 9,
    t_set = 10,
    t_map = 11,

    // Declared types
    t_enum = 8,
    t_structured = 12,
    t_service = 13,
  };

  static constexpr size_t kTypeCount = 19;
  static const std::string& type_name(type t);

  // TODO: Rename function.
  virtual type get_type_value() const = 0;

  /**
   * Default returns for every thrift type
   */
  virtual bool is_void() const { return false; }
  virtual bool is_string() const { return false; }
  virtual bool is_bool() const { return false; }
  virtual bool is_byte() const { return false; }
  virtual bool is_i16() const { return false; }
  virtual bool is_i32() const { return false; }
  virtual bool is_i64() const { return false; }
  virtual bool is_float() const { return false; }
  virtual bool is_double() const { return false; }
  virtual bool is_binary() const { return false; }

  bool is_string_or_binary() const { return is_string() || is_binary(); }
  bool is_any_int() const { return is_i16() || is_i32() || is_i64(); }
  bool is_floating_point() const { return is_double() || is_float(); }
};

/**
 * A reference to a thrift type.
 *
 * Type references are different from other references because they can be
 * annotated and unresolved.
 *
 * TODO: Make an unresolved reference directly representable in the AST.
 */
class t_type_ref final {
 public:
  t_type_ref() = default;
  /* implicit */ t_type_ref(const t_type& type, source_range range = {})
      : t_type_ref(&type, range) {}
  /* implicit */ t_type_ref(t_type&&, source_range = {}) = delete;

  // Returns the type being referenced, resolving it if need be.
  //
  // Throws a std::runtime_error if the type has not been set could not be
  // resolved.
  const t_type& deref();

  // Returns the resolved type being referenced.
  //
  // Throws a std::runtime_error if the type has not been set, or an unresolved
  // t_placeholder_typedef is encountered.
  const t_type& deref() const { return deref_or_throw(); }

  // Returns true the type reference has not been initalized.
  bool empty() const noexcept { return type_ == nullptr; }
  explicit operator bool() const { return !empty(); }
  // Returns true if the type has been resolved.
  bool resolved() const noexcept;
  bool resolve();

  t_placeholder_typedef* unresolved_type() { return unresolved_type_; }

  source_range src_range() const { return range_; }

  // Helpers for constructing from pointers.
  static t_type_ref from_ptr(const t_type* type, source_range range = {}) {
    return t_type_ref(type, range);
  }
  static t_type_ref from_req_ptr(const t_type* type, source_range range = {}) {
    if (type == nullptr) {
      throw std::runtime_error("type required");
    }
    return from_ptr(type, range);
  }

  // Smart pointer-like operators.
  const t_type* operator->() { return &deref(); }
  const t_type& operator*() { return deref(); }
  const t_type* operator->() const { return &deref(); }
  const t_type& operator*() const { return deref(); }

  static const t_type_ref& none();

  static t_type_ref for_placeholder(t_placeholder_typedef& unresolved_type);

 private:
  const t_type* type_ = nullptr;
  source_range range_;
  // The placeholder we have write access to, if we need to resolve the type
  // before derefing.
  // Note: It is not thread safe to access this value if 'this' is const.s
  t_placeholder_typedef* unresolved_type_ = nullptr;
  explicit t_type_ref(
      const t_type& type,
      t_placeholder_typedef& unresolved_type,
      source_range range)
      : type_(&type), range_(range), unresolved_type_(&unresolved_type) {}

  // Note: Use from_ptr or from_req_ptr for public access.
  explicit t_type_ref(const t_type* type, source_range range)
      : type_(type), range_(range) {}

  const t_type& deref_or_throw() const;

  // TODO(T227540797): Remove everything below this comment. It is only provided
  // for backwards compatibility.
 public:
  const t_type* get_type() const { return type_; }
};

bool is_scalar(const t_type& type);

} // namespace apache::thrift::compiler
