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

namespace apache {
namespace thrift {
namespace compiler {

class t_program;
class t_placeholder_typedef;

/**
 * Generic representation of a thrift type.
 *
 * These objects are used by the parser module to build up a tree of object that
 * are all explicitly typed. The generic t_type class exports a variety of
 * useful methods that are used by the code generator to branch based upon
 * different handling for the various types.
 */
class t_type : public t_named {
 public:
  // Returns a string in the format "program_name.type_name"
  std::string get_scoped_name() const;

  // Returns the full name for the given type. For example:
  // `list<string, string>`
  virtual std::string get_full_name() const { return get_scoped_name(); }

  /**
   * Resolves all typedefs (if any) to get the true type.
   */
  const t_type* get_true_type() const;

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

  // TODO(afuller): Delete everything below this point. It's only here for
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
    t_struct = 12,
    t_service = 13,
    t_stream = 17,
    t_program = 14,
  };
  static constexpr size_t kTypeCount = 19;
  // TODO: add description
  static constexpr size_t kTypeBits = 5;
  // TODO: add description
  static constexpr uint64_t kTypeMask = (1ULL << kTypeBits) - 1;
  static const std::string& type_name(type t);

  std::string get_impl_full_name() const { return get_full_name(); }

  // TODO: Rename function.
  virtual type get_type_value() const = 0;

  /**
   * Default returns for every thrift type
   */
  virtual bool is_void() const { return false; }
  virtual bool is_base_type() const { return false; }
  virtual bool is_string() const { return false; }
  virtual bool is_bool() const { return false; }
  virtual bool is_byte() const { return false; }
  virtual bool is_i16() const { return false; }
  virtual bool is_i32() const { return false; }
  virtual bool is_i64() const { return false; }
  virtual bool is_float() const { return false; }
  virtual bool is_double() const { return false; }
  virtual bool is_typedef() const { return false; }
  virtual bool is_enum() const { return false; }
  virtual bool is_struct() const { return false; }
  virtual bool is_union() const { return false; }
  virtual bool is_exception() const { return false; }
  virtual bool is_container() const { return false; }
  virtual bool is_list() const { return false; }
  virtual bool is_set() const { return false; }
  virtual bool is_map() const { return false; }
  virtual bool is_streamresponse() const { return false; }
  virtual bool is_service() const { return false; }
  virtual bool is_binary() const { return false; }
  virtual bool is_paramlist() const { return false; }

  bool is_string_or_binary() const { return is_string() || is_binary(); }
  bool is_any_int() const { return is_i16() || is_i32() || is_i64(); }
  bool is_floating_point() const { return is_double() || is_float(); }
  bool is_scalar() const {
    return is_enum() || is_any_int() || is_byte() || is_bool() ||
        is_floating_point();
  }

  /**
   * Create a unique hash number based on t_type's properties.
   */
  virtual uint64_t get_type_id() const;

  t_type* get_true_type() {
    return const_cast<t_type*>(
        const_cast<const t_type*>(this)->get_true_type());
  }

  const t_program* get_program() const { return program(); }
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
  /* implicit */ t_type_ref(const t_type& type) : t_type_ref(&type) {}
  /* implicit */ t_type_ref(t_type&&) = delete;

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

  // Helpers for constructing from pointers.
  static t_type_ref from_ptr(const t_type* type) { return t_type_ref(type); }
  static t_type_ref from_req_ptr(const t_type* type) {
    if (type == nullptr) {
      throw std::runtime_error("type required");
    }
    return from_ptr(type);
  }

  // Smart pointer-like operators.
  const t_type* operator->() { return &deref(); }
  const t_type& operator*() { return deref(); }
  const t_type* operator->() const { return &deref(); }
  const t_type& operator*() const { return deref(); }

  static const t_type_ref& none();

 private:
  const t_type* type_ = nullptr;
  // The placeholder we have write access to, if we need to resolve the type
  // before derefing.
  // Note: It is not thread safe to access this value if 'this' is const.s
  t_placeholder_typedef* unresolved_type_ = nullptr;
  explicit t_type_ref(
      const t_type& type, t_placeholder_typedef& unresolved_type)
      : type_(&type), unresolved_type_(&unresolved_type) {}

  // Note: Use from_ptr or from_req_ptr for public access.
  explicit t_type_ref(const t_type* type) : type_(type) {}

  const t_type& deref_or_throw() const;

  // TODO(afuller): Remove everything below this comment. It is only provided
  // for backwards compatibility.
 public:
  const t_type* get_type() const { return type_; }
  static t_type_ref for_placeholder(t_placeholder_typedef& unresolved_type);
  t_placeholder_typedef* get_unresolved_type() { return unresolved_type_; }
};

} // namespace compiler
} // namespace thrift
} // namespace apache
