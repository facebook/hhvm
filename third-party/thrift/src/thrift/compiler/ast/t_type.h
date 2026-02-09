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

  /**
   * Returns whether this type is sealed according to the Thrift Object Model.
   *
   * A sealed type is one where any schema change implies that the
   * project-embed round-trip should fail. Only sealed types may be used as
   * set elements or map keys.
   *
   * In Thrift IDL, sealed types include:
   * - All primitive types
   * - Enum types
   * - Typedefs, if their target type is sealed
   * - List types, if the element type is sealed
   * - Set types (since the element type must be sealed)
   * - Map types, if the value type is sealed (key type must be sealed)
   * - Structured types, if explicitly marked as sealed (not yet supported in
   *   Thrift IDL)
   *
   * NOTE that the set of IDL types differs slightly from Object Model (TOM)
   * types:
   *   - TOM primitive types include `any`
   *   - Typedefs do not exist in the TOM, they are purely an IDL concept.
   *   - Opaque Alias Types, defined in the TOM, are not implemented yet in IDL.
   *   - The current implementation of the Thrift IDL compiler treats
   *     "interfaces" (i.e., services and interactions) as "types", mostly for
   *     legacy reasons (because functions can return interactions). These are
   *     obviously not "types" in the TOM sense.
   *
   * @return `true` if this type is sealed, `false` otherwise
   *
   * @throws std::exception if a determination cannot be made (e.g., due to
   *         unresolvable type references) or non-sensical (for services and
   *         interactions, which inherit from t_type for legacy reasons).
   */
  virtual bool is_sealed() const = 0;

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
 * A reference to a Thrift type in IDL code.
 *
 * For example, in the following struct definition:
 *
 * ```
 * struct Foo {
 *   1: i32 a;
 * }
 * ```
 *
 * The field type "i32" in `1: i32 a;` is a *reference* to a type (in this case,
 * the primitive built-in type `i32`).
 *
 * Type references are different from other references because they can be
 * annotated and unresolved.
 *
 * Instances of `t_type_ref` provide pass-through dereference semantics (i.e.,
 * `->` and `*` operators) to the underlying referenced `t_type`.
 *
 * Note that this type (`t_type_ref`) does not represent an AST node (unlike
 * `t_type`, `t_typedef`, etc.).
 */
class t_type_ref final {
 public:
  t_type_ref() = default;

  /* implicit */ t_type_ref(const t_type& type, source_range range = {})
      : t_type_ref(&type, range) {}

  /* implicit */ t_type_ref(t_type&&, source_range = {}) = delete;

  /**
   * Returns the type being referenced, resolving it if need be.
   *
   * Throws a std::runtime_error if the referenced type has not been set (i.e.,
   * this instance is `empty()`) or could not be resolved.
   */
  const t_type& deref();

  /**
   * Returns the resolved type being referenced.
   *
   * Throws a std::runtime_error if the type has not been set, or an unresolved
   * t_placeholder_typedef is encountered.
   */
  const t_type& deref() const { return deref_or_throw(); }

  // Returns true the type reference has not been initalized.
  bool empty() const noexcept { return type_ == nullptr; }

  /**
   * Boolean operator is equivalent to "not empty".
   */
  explicit operator bool() const { return !empty(); }

  /**
   * Returns true if this reference has been initialized (i.e., is not
   * `empty()`) and any previously unresolved "placeholder typedef" has been
   * resolved.
   */
  bool resolved() const noexcept;

  /**
   * Attempts to resolve the underlying unresolved "placeholder typedef", if
   * any.
   */
  bool resolve();

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

 private:
  /**
   * The resolved underlying type being referred to by this t_type_ref, if any.
   *
   * `type == nullptr`  <=> `this->empty()` <=> `!bool(*this)`
   */
  const t_type* type_ = nullptr;

  source_range range_;
  // The placeholder we have write access to, if we need to resolve the type
  // before derefing.
  // Note: It is not thread safe to access this value if 'this' is const.s
  //
  // TODO(T244601847): Make an unresolved reference directly representable in
  // the AST, merging `t_placeholder_typedef` into `t_type_ref`.
  t_placeholder_typedef* unresolved_typedef_ = nullptr;

  explicit t_type_ref(
      const t_type& type,
      t_placeholder_typedef& unresolved_type,
      source_range range)
      : type_(&type), range_(range), unresolved_typedef_(&unresolved_type) {}

  // Note: Use from_ptr or from_req_ptr for public access.
  explicit t_type_ref(const t_type* type, source_range range)
      : type_(type), range_(range) {}

  /**
   * Returns the resolved underlying type being referred to, or throws an
   * exception.
   */
  const t_type& deref_or_throw() const;

 public:
  // TODO(T244601847): Remove get_type() once t_placeholder_typedef, at which
  // point resolved() and deref() are all that's necessary
  const t_type* get_type() const { return type_; }

  static t_type_ref for_placeholder(t_placeholder_typedef& unresolved_type);

  // TODO(T244601847): unresolved_type() and t_placeholder_typedef will
  // eventually be removed, and instead unresolved types will be represented by
  // a t_type_ref with NO underlying type (placeholder or otherwise). Such a
  // t_type_ref will be identifiable by calling the `resolved()` method
  // (returning false).
  /**
   * When we parse an AST, `t_type_ref` represents all type references. E.g. the
   * type of a field `1: Foo myField` is a type-ref to the AST type `Foo`.
   *
   * When a type is not immediately resolvable at parse time (i.e. the parser
   * has not yet encountered the declaration of type `Foo`), we generate a
   * "placeholder typedef"  to Foo (i.e. instead of pointing to the definition
   * `Foo`, the type-ref points to a t_placeholder_typedef representing Foo).
   *
   * After parsing completes, we attempt deferred resolution for placeholder
   * typedefs to resolve types which were declared after their use, and emit
   * errors for types not found even during deferred resolution.
   */
  t_placeholder_typedef* unresolved_type() { return unresolved_typedef_; }

  /**
   * See (non-const) `unresolved_type()`.
   */
  const t_placeholder_typedef* unresolved_type() const {
    return unresolved_typedef_;
  }
};

bool is_scalar(const t_type& type);

} // namespace apache::thrift::compiler
