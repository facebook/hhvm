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

#include <cassert>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <limits>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <string_view>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include <thrift/compiler/whisker/detail/overload.h>
#include <thrift/compiler/whisker/detail/type_traits.h>
#include <thrift/compiler/whisker/diagnostic.h>
#include <thrift/compiler/whisker/managed_ptr.h>
#include <thrift/compiler/whisker/source_location.h>
#include <thrift/compiler/whisker/tree_printer.h>

#include <fmt/core.h>

#include <boost/core/demangle.hpp>

namespace whisker {

// Whisker supports a small set of types.
// They are all represented by the whisker::object sum type, which is one of:
//   * i64     — 64-bit two’s complement signed integer
//   * f64     — IEEE 754 binary64 floating point number
//   * string  — UTF-8 encoded range of Unicode characters
//   * boolean — 1-bit (true or false)
//   * null    — The absence of a value
//   * array   — An ordered list of `whisker::object`s (recursive)
//   * map     — An unordered list of key-value pairs, where the key is a valid
//               identifier, and the value is a `whisker::object` (recursive)
//   * native_function —
//               User-defined (C++) function that operates on `whisker::object`.
//   * native_handle —
//               A pointer to an opaque (C++) object.

using i64 = std::int64_t;
using f64 = double;
using string = std::string;
using boolean = bool;
using null = std::monostate;

class object;
/**
 * Options for whisker::to_string() and whisker::print_to().
 */
struct object_print_options {
  /**
   * The maximum recursive depth of the object to print before truncating the
   * output. This is useful for:
   *   - Preventing infinite recursion when printing a cyclic object
   *   - Preventing excessive output when printing a deeply nested object
   *
   * The depth value refers to the depth of the whisker::object hierarchy, which
   * may not necessarily be the same as the depth of the printed tree.
   *
   * When the depth is reached, the output may be truncated based on the type of
   * object being printed:
   *   - For whisker::map, the property names are printed but values are
   *     truncated.
   *   - For whisker::array, the array indices are printed by elements are
   *     truncated.
   *
   * For all other whisker::object types, no truncation occurs because they do
   * not incur additional depth.
   *
   * Truncated output appears as "...". This indicates that there are values
   * beyond the specified depth that remain unexpanded.
   */
  unsigned max_depth = std::numeric_limits<unsigned>::max();
};

/**
 * An exception that can be thrown to indicate a fatal error in property
 * lookup or function evaluation.
 *
 * This exception is intended to be thrown for any failed expression
 * evaluation. For example:
 *   - failed property lookups (map::lookup_property)
 *   - failed array lookups (array::at)
 *   - failed function calls (native_function::invoke)
 */
struct eval_error : std::runtime_error {
  using std::runtime_error::runtime_error;
};

/**
 * A class that allows map-like named property access.
 */
class map {
 public:
  using ptr = managed_ptr<map>;
  /**
   * There are two main reasons why we are choosing std::map (rather than
   * std::unordered_map):
   *   1. Before C++20, std::unordered_map does not support heterogenous
   *      lookups.
   *   2. *Technically* neither std::map or std::unordered_map are supposed to
   *      be instantiated with an incomplete type. However, the standard library
   *      implementations we use do support it. We know this because mstch has
   *      been doing this for years in production at this point.
   *      std::unordered_map actually breaks for incomplete types in practice.
   */
  using raw = std::map<std::string, object, std::less<>>;
  /**
   * Creates a map from a raw C++ map.
   */
  static ptr of(raw);

  virtual ~map() = default;
  /**
   * Searches for a property on an object whose name matches the provided
   * identifier, returning a non-null pointer if present.
   *
   * Preconditions:
   *   - The provided string is a valid Whisker identifier
   *
   * Throws:
   *   - `eval_error` if the property lookup fails in an irrecoverable way.
   *     Failing to find a property matching the identifier is not a fatal
   *     error. In that case, this function returns nullptr.
   *     This function should only throw if the identifier is recognized but
   *     evaluating its value failed.
   */
  virtual std::optional<object> lookup_property(
      std::string_view identifier) const = 0;

  /**
   * Returns an ordered set of finitely enumerable property names of this map
   * object.
   *
   * If property names are not enumerable (i.e. dynamically generated), then
   * this returns the empty optional.
   *
   * For each name returned in this set, lookup_property must not return
   * nullptr.
   */
  virtual std::optional<std::set<std::string>> keys() const {
    return std::nullopt;
  }

  /**
   * Creates a tree-like string representation of this object, primarily for
   * debugging and diagnostic purposes.
   *
   * The provided tree_printer::scope object abstracts away the details of tree
   * printing as well as encodes the location in an existing print tree where
   * this object should be inline. See its documentation for more details.
   */
  virtual void print_to(
      tree_printer::scope&, const object_print_options&) const;

  /**
   * Produces a textual representation of the data type represented by this
   * object, primarily for debugging and diagnostic purposes.
   */
  virtual std::string describe_type() const;

  /**
   * Determines if two maps have the same properties.
   *
   * This functions returns true if:
   *   - `lhs` and `rhs` are the same object. Or,
   *   - both maps have enumerable keys, and all key-value pairs are equal
   *     between them.
   */
  friend bool operator==(const map& lhs, const map& rhs);

 protected:
  /**
   * A default implementation of whisker::print_to for subclasses to use.
   *
   * For the default implementation to work, property names must be explicitly
   * enumerated.
   *
   * The provided name allows the caller to expose type information.
   */
  void default_print_to(
      std::string_view name,
      const std::set<std::string>& property_names,
      tree_printer::scope&,
      const object_print_options&) const;
};

/**
 * A class that allows array-like random access over a sequence of objects.
 */
class array {
 public:
  using ptr = managed_ptr<array>;
  using raw = std::vector<object>;
  /**
   * Creates a map from a raw C++ map.
   */
  static ptr of(raw);

  virtual ~array() = default;
  /**
   * Returns the number of elements in the sequence.
   */
  virtual std::size_t size() const = 0;
  /**
   * Returns the object at the specified index within the sequence.
   *
   * Preconditions:
   *   - index < size()
   */
  virtual object at(std::size_t index) const = 0;

  /**
   * Creates a tree-like string representation of this object, primarily for
   * debugging and diagnostic purposes.
   *
   * The provided tree_printer::scope object abstracts away the details of tree
   * printing as well as encodes the location in an existing print tree where
   * this object should be inline. See its documentation for more details.
   */
  virtual void print_to(
      tree_printer::scope&, const object_print_options&) const;

  /**
   * Produces a textual representation of the data type represented by this
   * object, primarily for debugging and diagnostic purposes.
   */
  virtual std::string describe_type() const;

  /**
   * Determines if two arrays have the same elements.
   *
   * This functions returns true if:
   *   - `lhs` and `rhs` are the same object. Or,
   *   - both arrays have the same size and their corresponding elements are
   *     equal.
   */
  friend bool operator==(const array& lhs, const array& rhs);

 protected:
  /**
   * A default implementation of whisker::print_to for subclasses to use.
   *
   * The provided name allows the caller to expose type information.
   */
  void default_print_to(
      std::string_view name,
      tree_printer::scope&,
      const object_print_options&) const;
};

/**
 * A native_function represents a user-defined function in Whisker. Its behavior
 * is defined by highly customizable C++ code but its inputs and output are
 * whisker::object.
 *
 * A native_function implementation must implement the `invoke` member function.
 * This function receives a `context` object, which contains information about:
 *   - arguments (positional or named)
 *   - source location
 *   - diagnostics printing
 */
class native_function {
 public:
  using ptr = std::shared_ptr<native_function>;
  virtual ~native_function() = default;

  /**
   * A class that provides information about the executing function to
   * invoke(...).
   *
   * This includes:
   *   - Access to positional and named arguments to the function call
   *   - The ability to output diagnostics
   *   - The source location of the function call
   */
  class context;
  /**
   * The implementation-defined behavior for this function.
   *
   * Whisker's renderer calls this function when evaluating expressions.
   *
   * native_function is a low-level API. Most native_function implements should
   * use a higher-level dsl. See `dsl.h`.
   *
   * Postconditions:
   *  - The returned object is non-null.
   */
  virtual object invoke(context) = 0;

  /**
   * An ordered list of positional argument values. The position is equal to
   * the index in this vector.
   */
  using positional_arguments_t = std::vector<object>;
  /**
   * A map of argument names to their values. The names are guaranteed to be a
   * valid Whisker identifier.
   */
  using named_arguments_t = std::unordered_map<std::string_view, object>;

  /**
   * Produces a textual representation of the function represented by this
   * object, primarily for debugging and diagnostic purposes.
   */
  virtual std::string describe_type() const;
  /**
   * Creates a string representation to described this function, primarily for
   * debugging and diagnostic purposes.
   */
  virtual void print_to(
      tree_printer::scope&, const object_print_options&) const;
};

/**
 * A native_handle represents an opaque reference to any native (C++) data type.
 * It always contains a non-null reference.
 *
 * There are two ways to use these types in Whisker templates:
 *   - Pass them as arguments to native_function which can extract the
 *     underlying C++ type and corresponding data. This resembles the "opaque
 *     pointer" pattern, commonly used in languages like C:
 *       https://en.wikipedia.org/wiki/Opaque_pointer#C
 *   - Access properties that are defined by the attached prototype (see
 *     below).
 */
template <typename T = void>
class native_handle;

/**
 * A "prototype" for a native_handle that can be used for prototype-based
 * programming:
 *   https://en.wikipedia.org/wiki/Prototype-based_programming
 *
 * When property access is performed on a native_handle, the lookup is
 * dispatched to the prototype. That is, for the lookup `foo.bar`, if `foo` is a
 * `native_handle`, then the lookup is resolved by calling
 * `find_descriptor("bar")` on the prototype of `foo`.
 *
 * A prototype may have a parent prototype upon which lookups should fall back
 * to if there is no descriptor for a particular name in this instance. This
 * forms a "chain of prototypes", which can be used to emulate an inheritance
 * hierarchy.
 *
 * Strictly speaking, a prototype is should be a whisker::object because the
 * definition of prototype-based programming requires "reusing existing
 * objects".
 * The implementation here *could* be changed to use whisker::object, but we
 * intentionally choose to use a separate type that is more constrained.
 * The main advantages of this decision are:
 *   - We achieve type-safety in C++ (native_handle<T> only allows
 *     `prototype<T>`).
 *   - We avoid unhelpful states, such as a `string` or `i64` instance being
 *     used as a prototype. This also reduces implementation complexity.
 */
template <typename T = void>
class prototype;
/**
 * Same as prototype<T>::ptr but avoids dependent template type. This
 * allows template argument deduction.
 */
template <typename Self = void>
using prototype_ptr = std::shared_ptr<const prototype<Self>>;

namespace detail {

template <typename T>
class native_handle_base {
 public:
  explicit native_handle_base(
      managed_ptr<T> ref, prototype_ptr<> proto) noexcept
      : ref_(std::move(ref)), proto_(std::move(proto)) {
    assert(ref_ != nullptr);
  }

  const managed_ptr<T>& ptr() const& { return ref_; }
  managed_ptr<T>&& ptr() && { return std::move(ref_); }

  const prototype_ptr<>& proto() const& { return proto_; }
  prototype_ptr<>&& proto() && { return std::move(proto_); }

 private:
  managed_ptr<T> ref_;
  prototype_ptr<> proto_;
};

std::string describe_native_handle_for_type(const std::type_info&);

} // namespace detail

/**
 * A native_handle<void> is a specialized handle object which additionally
 * stores the contained type information (as std::type_info).
 *
 * This type forms the basis of type-safe type-erasure for native_function
 * arguments that operate on native_handle objects.
 */
template <>
class native_handle<void> final : private detail::native_handle_base<void> {
 private:
  using base = detail::native_handle_base<void>;

 public:
  using element_type = void;

  template <typename T>
  explicit native_handle(managed_ptr<T> ref) noexcept
      : native_handle(std::move(ref), nullptr /* prototype */) {}

  template <typename T>
  explicit native_handle(managed_ptr<T> ref, prototype_ptr<> proto) noexcept
      : base(
            managed_ptr<void>(ref, static_cast<const void*>(ref.get())),
            std::move(proto)),
        type_(typeid(T)) {}

  using base::proto;
  using base::ptr;

  /**
   * Returns either:
   *   - the exact type of the object pointed-to by this handle, or
   *   - if the type is polymorphic, then maybe one of its base classes.
   */
  const std::type_info& type() const noexcept { return type_; }

  /**
   * Tries to convert this handle to a handle of type T.
   *
   * This succeeds iff the contained type_info (as returned by type()) is
   * exactly T. Notably, the conversion will not handle any kind of
   * inheritance-based polymorphism.
   *
   * If the type does not match, returns std::nullopt.
   */
  template <typename T>
  std::optional<native_handle<T>> try_as() const noexcept {
    if (type() == typeid(T)) {
      return native_handle<T>(
          std::static_pointer_cast<const T>(ptr()), proto());
    }
    return std::nullopt;
  }

  /**
   * Produces a textual representation of the underlying (C++) type represented
   * by this handle, primarily for debugging and diagnostic purposes.
   */
  std::string describe_type() const;

  /**
   * Produces a textual representation of native_handle<void>, for debugging and
   * diagnostic purposes.
   */
  static std::string describe_class_type();

 private:
  std::reference_wrapper<const std::type_info> type_;
};

template <typename T>
class native_handle final : private detail::native_handle_base<T> {
 private:
  using base = detail::native_handle_base<T>;

 public:
  static_assert(!std::is_const_v<T>);
  static_assert(!std::is_reference_v<T>);
  static_assert(!std::is_void_v<T>);
  using element_type = T;

  using base::base;
  using base::ptr;
  const T& operator*() const noexcept { return *ptr(); }
  const T* operator->() const noexcept { return ptr().get(); }
  using base::proto;

  /* implicit */ operator native_handle<void>() const noexcept {
    return native_handle<void>(ptr(), proto());
  }

  /**
   * Produces a textual representation of native_handle<T>, for debugging and
   * diagnostic purposes.
   */
  static std::string describe_class_type() {
    return detail::describe_native_handle_for_type(typeid(T));
  }
};

template <typename T>
native_handle(managed_ptr<T>) -> native_handle<T>;
template <typename T>
native_handle(managed_ptr<T>, prototype_ptr<>) -> native_handle<T>;

/**
 * Two native_handle objects are equal iff they point to the same data.
 *
 * Example:
 *
 *     native_handle handle1{manage_owned<int>(42)};
 *     native_handle handle2{manage_owned<int>(42)};
 *     assert(handle1 != handle2);
 *
 *     auto integer = manage_owned<int>(42)
 *     native_handle handle1{integer};
 *     native_handle handle2{integer};
 *     assert(handle1 == handle2);
 */
template <typename T, typename U>
inline bool operator==(
    const native_handle<T>& lhs, const native_handle<U>& rhs) noexcept {
  return lhs.ptr() == rhs.ptr();
}
// Before C++20, operator!= is not synthesized from operator==.
template <typename T, typename U>
inline bool operator!=(
    const native_handle<T>& lhs, const native_handle<U>& rhs) noexcept {
  return !(lhs == rhs);
}

namespace detail {
// This only exists to form a kind-of recursive std::variant with the help of
// forward declared types.
using object_base = std::variant<
    null,
    i64,
    f64,
    string,
    boolean,
    native_function::ptr,
    native_handle<>,
    array::ptr,
    map::ptr,
    managed_ptr<object> /* proxied */>;
} // namespace detail

/**
 * A whisker::object represents any possible type of data that is recognized by
 * the Whisker templating language and its evaluation engine.
 *
 * Ergonomically, a whisker::object looks and feels very much like a
 * std::variant when interacting with an instance in C++. There are convenience
 * accessors and methods that make it easier to interact with its possible
 * alternatives.
 *
 * The contained data in a whisker::object is immutable through the instance.
 * That is, once a value is emplaced within whisker::object, the only way to
 * change the value is to assign a new value to the whisker::object instance
 * (for example, std::move-out or operator=). Therefore, whisker::object is not
 * suitable for general data manipulation — it should be the terminal store of
 * value after computation.
 *
 * A default constructed whisker::object has the alternative whisker::null.
 *
 * One important difference from std::variant is the behavior on move-construct
 * or move-assign — the moved from whisker::object is left in a valid state
 * whose value is whisker::null.
 */
class object final : private detail::object_base {
 public:
  using ptr = managed_ptr<object>;

 private:
  using base = detail::object_base;
  base& as_variant() & { return *this; }
  const base& as_variant() const& { return *this; }

  // If this object contains the ptr variant, it behaves exactly like the target
  // object of that ptr.
  bool is_proxy() const { return std::holds_alternative<ptr>(*this); }
  const ptr& as_proxy() const { return std::get<ptr>(*this); }

  // This function moves out the base variant object which would normally leave
  // the object in a partially moved-from state. However, moving a variant
  // performs a move on the contained alternative, meaning that assigning to
  // the variant after a move is safe. We have this steal_variant() function
  // hide this detail so we don't trip clang-tidy use-after-move linter.
  base&& steal_variant() & { return std::move(*this); }

 public:
  template <typename T>
  bool is() const noexcept {
    static_assert(
        !std::is_same_v<T, ptr>,
        "The proxied ptr should never be exposed via public API");
    assert(!valueless_by_exception());
    if (is_proxy()) {
      return as_proxy()->is<T>();
    }
    return std::holds_alternative<T>(*this);
  }

  template <typename T>
  const T& as() const {
    static_assert(
        !std::is_same_v<T, ptr>,
        "The proxied ptr should never be exposed via public API");
    assert(!valueless_by_exception());
    if (is_proxy()) {
      return as_proxy()->as<T>();
    }
    return std::get<T>(*this);
  }

  /* implicit */ object(null = {}) : base(std::in_place_type<null>) {}

  /**
   * Pointers have the nasty behavior of being implicitly convertible to bool.
   */
  template <
      typename U = boolean,
      std::enable_if_t<std::is_same_v<U, boolean>>* = nullptr>
  explicit object(U value) : base(value) {}

  explicit object(i64 value) : base(value) {}
  explicit object(f64 value) : base(value) {}
  explicit object(string value) : base(std::move(value)) {}
  explicit object(native_function::ptr value) : base(std::move(value)) {
    assert(as_native_function() != nullptr);
  }
  explicit object(native_handle<> value) : base(std::move(value)) {
    assert(as_native_handle().ptr() != nullptr);
  }
  explicit object(map::ptr value) : base(std::move(value)) {
    assert(as_map() != nullptr);
  }
  explicit object(array::ptr value) : base(std::move(value)) {
    assert(as_array() != nullptr);
  }

  /**
   * Creates a "proxy" object that behaves the same as the provided object. This
   * function is useful, for example, for storing an owning reference to
   * existing objects in an array or map without deep-copying.
   *
   * This object will keep the source object alive.
   *
   * Postconditions:
   *   - *this == *object
   */
  /* implicit */ object(ptr source) : base(std::move(source)) {
    assert(as_proxy() != nullptr);
  }

  /* implicit */ object(std::nullptr_t) = delete;

  ~object() noexcept = default;

  object(const object&) = default;
  object& operator=(const object&) = default;

  object(object&& other) noexcept : base(other.steal_variant()) {
    other = null();
  }

  object& operator=(object&& other) noexcept {
    as_variant() = other.steal_variant();
    other = null();
    return *this;
  }
  // Pull in all the variant alternative operator= overloads
  using base::operator=;

  void swap(object& other) noexcept(noexcept(base::swap(other))) {
    base::swap(other);
  }

  template <typename... Visitors>
  decltype(auto) visit(Visitors&&... visitors) const {
    // Remove any layers of proxying since we do not expose it to callers.
    const object* resolved = this;
    while (resolved->is_proxy()) {
      resolved = resolved->as_proxy().get();
    }

    return detail::variant_match(
        resolved->as_variant(),
        [](managed_ptr<object>) -> decltype(detail::overload(
                                    std::forward<Visitors>(visitors)...)(
                                    i64{})) {
          // Unreachable: unwrapped above.
          std::terminate();
        },
        std::forward<Visitors>(visitors)...);
  }

  const i64& as_i64() const { return as<i64>(); }
  bool is_i64() const noexcept { return is<i64>(); }

  const f64& as_f64() const { return as<f64>(); }
  bool is_f64() const noexcept { return is<f64>(); }

  const string& as_string() const { return as<string>(); }
  bool is_string() const noexcept { return is<string>(); }

  const boolean& as_boolean() const { return as<boolean>(); }
  bool is_boolean() const noexcept { return is<boolean>(); }

  bool is_null() const noexcept { return is<null>(); }

  const native_function::ptr& as_native_function() const {
    return as<native_function::ptr>();
  }
  bool is_native_function() const noexcept {
    return is<native_function::ptr>();
  }

  const native_handle<>& as_native_handle() const {
    return as<native_handle<>>();
  }
  bool is_native_handle() const noexcept { return is<native_handle<>>(); }

  const array::ptr& as_array() const { return as<array::ptr>(); }
  bool is_array() const noexcept { return is<array::ptr>(); }

  const map::ptr& as_map() const { return as<map::ptr>(); }
  bool is_map() const noexcept { return is<map::ptr>(); }

  friend bool operator==(const object& lhs, null) noexcept {
    return lhs.is_null();
  }
  friend bool operator==(null, const object& rhs) noexcept {
    return rhs.is_null();
  }

  /**
   * Produces a textual representation of the data type contained within this
   * object, primarily for debugging and diagnostic purposes.
   */
  std::string describe_type() const;

  // Prevent implicit conversion from const char* to bool
  template <
      typename T = boolean,
      typename = std::enable_if_t<std::is_same_v<T, boolean>>>
  friend bool operator==(const object& lhs, T rhs) noexcept {
    return lhs.is_boolean() && lhs.as_boolean() == rhs;
  }
  template <
      typename T = boolean,
      typename = std::enable_if_t<std::is_same_v<T, boolean>>>
  friend bool operator==(T lhs, const object& rhs) noexcept {
    return rhs == lhs;
  }

  friend bool operator==(const object& lhs, i64 rhs) noexcept {
    return lhs.is_i64() && lhs.as_i64() == rhs;
  }
  friend bool operator==(i64 lhs, const object& rhs) noexcept {
    return rhs == lhs;
  }

  friend bool operator==(const object& lhs, f64 rhs) noexcept {
    return lhs.is_f64() && lhs.as_f64() == rhs;
  }
  friend bool operator==(f64 lhs, const object& rhs) noexcept {
    return rhs == lhs;
  }

  friend bool operator==(const object& lhs, std::string_view rhs) noexcept {
    return lhs.is_string() && lhs.as_string() == rhs;
  }
  friend bool operator==(std::string_view lhs, const object& rhs) noexcept {
    return rhs == lhs;
  }

  friend bool operator==(const object&, std::nullptr_t) = delete;
  friend bool operator==(std::nullptr_t, const object&) = delete;

  friend bool operator==(
      const object& lhs, const native_function::ptr& rhs) noexcept {
    return lhs.is_native_function() && rhs != nullptr &&
        lhs.as_native_function() == rhs;
  }
  friend bool operator==(
      const native_function::ptr& lhs, const object& rhs) noexcept {
    return rhs == lhs;
  }

  friend bool operator==(
      const object& lhs, const native_handle<>& rhs) noexcept {
    return lhs.is_native_handle() && lhs.as_native_handle() == rhs;
  }
  friend bool operator==(
      const native_handle<>& lhs, const object& rhs) noexcept {
    return rhs == lhs;
  }

  friend bool operator==(const object& lhs, const array::ptr& rhs) noexcept {
    if (lhs.is_array() && rhs != nullptr) {
      return *lhs.as_array() == *rhs;
    }
    return false;
  }
  friend bool operator==(const array::ptr& lhs, const object& rhs) noexcept {
    return rhs == lhs;
  }

  friend bool operator==(const object& lhs, const map::ptr& rhs) noexcept {
    if (lhs.is_map() && rhs != nullptr) {
      return *lhs.as_map() == *rhs;
    }
    return false;
  }
  friend bool operator==(const map::ptr& lhs, const object& rhs) noexcept {
    return rhs == lhs;
  }

  friend bool operator==(const object& lhs, const object& rhs) noexcept {
    return lhs.visit([&rhs](auto&& value) { return rhs == value; });
  }

  // Before C++20, operator!= is not synthesized from operator==.

  friend bool operator!=(const object& lhs, null) noexcept {
    return !lhs.is_null();
  }
  friend bool operator!=(null, const object& rhs) noexcept {
    return !rhs.is_null();
  }

  // Prevent implicit conversion from const char* to bool
  template <
      typename T = boolean,
      typename = std::enable_if_t<std::is_same_v<T, boolean>>>
  friend bool operator!=(const object& lhs, T rhs) noexcept {
    return !(lhs == rhs);
  }
  template <
      typename T = boolean,
      typename = std::enable_if_t<std::is_same_v<T, boolean>>>
  friend bool operator!=(T lhs, const object& rhs) noexcept {
    return !(lhs == rhs);
  }

  friend bool operator!=(const object& lhs, i64 rhs) noexcept {
    return !(lhs == rhs);
  }
  friend bool operator!=(i64 lhs, const object& rhs) noexcept {
    return !(lhs == rhs);
  }

  friend bool operator!=(const object& lhs, f64 rhs) noexcept {
    return !(lhs == rhs);
  }
  friend bool operator!=(f64 lhs, const object& rhs) noexcept {
    return !(lhs == rhs);
  }

  friend bool operator!=(const object& lhs, std::string_view rhs) noexcept {
    return !(lhs == rhs);
  }
  friend bool operator!=(std::string_view lhs, const object& rhs) noexcept {
    return !(lhs == rhs);
  }

  friend bool operator!=(const object&, std::nullptr_t) = delete;
  friend bool operator!=(std::nullptr_t, const object&) = delete;

  friend bool operator!=(
      const object& lhs, const native_function::ptr& rhs) noexcept {
    return !(lhs == rhs);
  }
  friend bool operator!=(
      const native_function::ptr& lhs, const object& rhs) noexcept {
    return !(lhs == rhs);
  }

  friend bool operator!=(
      const object& lhs, const native_handle<>& rhs) noexcept {
    return !(lhs == rhs);
  }
  friend bool operator!=(
      const native_handle<>& lhs, const object& rhs) noexcept {
    return !(lhs == rhs);
  }

  friend bool operator!=(const object& lhs, const array::ptr& rhs) noexcept {
    return !(lhs == rhs);
  }
  friend bool operator!=(const array::ptr& lhs, const object& rhs) noexcept {
    return !(lhs == rhs);
  }

  friend bool operator!=(const object& lhs, const map::ptr& rhs) noexcept {
    return !(lhs == rhs);
  }
  friend bool operator!=(const map::ptr& lhs, const object& rhs) noexcept {
    return !(lhs == rhs);
  }

  friend bool operator!=(const object& lhs, const object& rhs) noexcept {
    return !(lhs == rhs);
  }
};

// Defined here because object is an incomplete type where this is declared.
class native_function::context {
 public:
  context(
      source_range loc,
      diagnostics_engine& diags,
      object self,
      positional_arguments_t positional_args,
      named_arguments_t named_args)
      : loc_(loc),
        diags_(diags),
        self_(std::move(self)),
        positional_args_(std::move(positional_args)),
        named_args_(std::move(named_args)) {}

  context(const context&) = delete;
  context& operator=(const context&) = delete;
  context(context&&) = default;
  context& operator=(context&&) = default;
  ~context() noexcept = default;

  source_range location() const noexcept { return loc_; }
  diagnostics_engine& diagnostics() const noexcept { return diags_; }
  const object& self() const noexcept { return self_; }
  const positional_arguments_t& positional_arguments() const noexcept {
    return positional_args_;
  }
  const named_arguments_t& named_arguments() const noexcept {
    return named_args_;
  }

  source_range loc_;
  std::reference_wrapper<diagnostics_engine> diags_;
  object self_;
  positional_arguments_t positional_args_;
  named_arguments_t named_args_;
};
static_assert(std::is_move_constructible_v<native_function::context>);

template <>
class prototype<void> {
 public:
  using ptr = std::shared_ptr<const prototype>;
  virtual ~prototype() noexcept = default;

  /**
   * Property descriptors are immediately invoked during property lookup.
   *
   * For the property lookup `foo.bar`, if `bar` is a property descriptor, then
   * the result of the lookup is computed by invoking the function stored in the
   * `bar`.
   *
   * A property descriptor behaves like a native_function with no arguments.
   */
  struct property {
    native_function::ptr function;
    /* implicit */ property(native_function::ptr f) : function(std::move(f)) {}
  };
  /**
   * Fixed object descriptors are statically bound objects to the prototype.
   *
   * For the property lookup `foo.bar`, if `bar` is a fixed object descriptor,
   * then the result of the lookup is the object stored in the `bar`.
   *
   * Objects such as native_function that are intended to be "member functions"
   * are a good candidate for a fixed object descriptor.
   */
  struct fixed_object {
    object value;
    /* implicit */ fixed_object(object o) : value(std::move(o)) {}
  };
  using descriptor = std::variant<property, fixed_object>;

  /**
   * Tries to look up a descriptor defined in this prototype. If there is no
   * entry for an identifier in this map, this should return nullptr.
   */
  virtual const descriptor* find_descriptor(std::string_view) const = 0;
  /**
   * Returns the names of all descriptors defined in this prototype.
   */
  virtual std::set<std::string> keys() const = 0;
  /**
   * Returns the fallback parent prototype, if one is provided.
   * This is used in case find_descriptor returns nullptr.
   */
  virtual const ptr& parent() const = 0;

  /**
   * Returns the name for this prototype, if one is registered.
   */
  virtual const std::string_view& name() const = 0;

  using descriptors_map = std::map<std::string, descriptor, std::less<>>;
  /**
   * Creates a prototype from the provided map of descriptors and
   * (optionally) a parent.
   */
  static ptr from(
      descriptors_map, ptr parent = nullptr, const std::string_view& name = "");
};

/**
 * A type-tagged prototype that is intended for native_handle<Self>.
 */
template <typename Self>
class prototype : public prototype<> {
 public:
  using ptr = std::shared_ptr<const prototype>;
  /**
   * Creates a prototype from the provided map of descriptors and
   * (optionally) a parent.
   */
  static ptr from(descriptors_map, prototype<>::ptr parent);
};

/**
 * A "basic" untyped implementation of a prototype that is backed by a
 * static map.
 *
 * Prototypes *should* be static so this implementation is sufficient for most
 * use cases.
 */
template <typename T = void>
class basic_prototype : public prototype<T> {
 public:
  const prototype<>::descriptor* find_descriptor(
      std::string_view name) const override {
    if (auto found = descriptors_.find(name); found != descriptors_.end()) {
      return &found->second;
    }

    // Recurse through the prototype chain until the first matching descriptor
    // is found. This is similar to JavaScript:
    //   https://developer.mozilla.org/en-US/docs/Web/JavaScript/Guide/Inheritance_and_the_prototype_chain
    return parent_ == nullptr ? nullptr : parent_->find_descriptor(name);
  }

  std::set<std::string> keys() const override {
    std::set<std::string> result;
    for (const auto& [key, _] : descriptors_) {
      result.insert(key);
    }
    return result;
  }

  const prototype<>::ptr& parent() const override { return parent_; }

  const std::string_view& name() const override { return name_; }

  basic_prototype(
      prototype<>::descriptors_map descriptors,
      prototype<>::ptr parent,
      const std::string_view& name)
      : descriptors_(std::move(descriptors)),
        parent_(std::move(parent)),
        name_(name) {}

 private:
  prototype<>::descriptors_map descriptors_;
  prototype<>::ptr parent_;
  std::string_view name_;
};

template <typename T>
/* static */ typename prototype<T>::ptr prototype<T>::from(
    descriptors_map descriptors, prototype<>::ptr parent) {
  return std::make_shared<basic_prototype<T>>(
      std::move(descriptors), std::move(parent));
}

namespace detail {
template <typename T, typename... Alternatives>
static constexpr bool is_any_of = (std::is_same_v<T, Alternatives> || ...);
}

/**
 * A type trait which checks that the provided type T can be used to construct
 * whisker::object.
 */
template <typename T>
constexpr inline bool is_any_object_type =
    detail::is_specialization_v<T, whisker::native_handle> ||
    detail::is_any_of<
        T,
        i64,
        f64,
        string,
        boolean,
        null,
        array::ptr,
        map::ptr,
        native_function::ptr,
        native_handle<>>;

/**
 * An alternative to to_string() that allows printing a whisker::object within
 * an ongoing tree printing session. The output is appended to the provided
 * tree_printer::scope's output stream.
 *
 * This is primarily needed for map::print_to() and array::print_to()
 * implementations.
 */
void print_to(const object&, tree_printer::scope&, const object_print_options&);
std::string to_string(const object&, const object_print_options& = {});

std::ostream& operator<<(std::ostream&, const object&);

// The make::* functions are syntactic sugar for constructing whisker::object
namespace make {

namespace detail {

/**
 * The set of integral types that are supported by whisker::object (and are
 * therefore also implicitly convertible from).
 */
template <typename T>
static constexpr bool is_supported_integral_type =
    whisker::detail::is_any_of<T, signed char, short, int, long, long long> &&
    sizeof(T) <= sizeof(whisker::i64);

/**
 * The set of floating point types that are supported by whisker::object (and
 * are therefore also implicitly convertible from).
 */
template <typename T>
static constexpr bool is_supported_floating_point_type =
    whisker::detail::is_any_of<T, float, double>;

} // namespace detail

/**
 * An empty whisker::object. null.is_null() == true.
 */
inline const object null;

/**
 * Creates whisker::object with boolean type.
 *
 * Postconditions:
 *   object::is_boolean() == true
 *   object::as_boolean() == value
 */
inline object boolean(boolean value) {
  return object(value);
}

/**
 * A whisker::object that represents the boolean `true`.
 * This constant is useful for functions that return references.
 */
inline const object true_{whisker::boolean(true)};
/**
 * A whisker::object that represents the boolean `true`.
 * This constant is useful for functions that return references.
 */
inline const object false_{whisker::boolean(false)};

/**
 * Creates whisker::object with integral type.
 *
 * Postconditions:
 *   object::is_i64() == true
 *   object::as_i64() == value
 */
template <
    typename T = i64,
    typename = std::enable_if_t<detail::is_supported_integral_type<T>>>
object i64(T value) {
  // Smaller signed integrals types are implicitly converted to i64.
  return object(whisker::i64(value));
}

/**
 * Creates whisker::object with floating point type.
 *
 * Postconditions:
 *   object::is_f64() == true
 *   object::as_f64() == value
 */
template <
    typename T = f64,
    typename = std::enable_if_t<detail::is_supported_floating_point_type<T>>>
object f64(T value) {
  return object(whisker::f64(value));
}

/**
 * Creates whisker::object with string type. This function takes ownership of
 * the provided string.
 *
 * Postconditions:
 *   object::is_string() == true
 *   object::as_string() == value
 */
inline object string(whisker::string&& value) {
  return object(std::move(value));
}
/**
 * Creates whisker::object with string type from a (possibly not NUL-terminated)
 * string.
 *
 * Postconditions:
 *   object::is_string() == true
 *   object::as_string() == value
 */
inline object string(std::string_view value) {
  return object(whisker::string(value));
}
/**
 * Creates whisker::object with string type from a NUL-terminated C string.
 *
 * Postconditions:
 *   object::is_string() == true
 *   object::as_string() == value
 */
inline object string(const char* value) {
  return object(whisker::string(value));
}

/**
 * Creates whisker::object with map type. This function takes ownership of the
 * provided map.
 *
 * Postconditions:
 *   object::is_map() == true
 *   object::as_map() == value
 */
inline object map(map::ptr value) {
  return object(std::move(value));
}
inline object map(map::raw value = {}) {
  return object(map::of(std::move(value)));
}

/**
 * Creates a map::ptr of a concrete type with the given arguments.
 *
 * Postconditions:
 *   object::is_map() == true
 *   object::as_map() == value
 */
template <typename T, typename... Args>
object make_map(Args&&... args) {
  return map(std::make_shared<T>(std::forward<Args>(args)...));
}

/**
 * Creates whisker::object with array type. This function takes ownership of the
 * provided array.
 *
 * Postconditions:
 *   object::is_array() == true
 *   object::as_array() == value
 */
inline object array(array::ptr value) {
  return object(std::move(value));
}
inline object array(array::raw value = {}) {
  return object(array::of(std::move(value)));
}

/**
 * Creates a map::ptr of a concrete type with the given arguments.
 *
 * Postconditions:
 *   object::is_array() == true
 *   object::as_array() == value
 */
template <typename T, typename... Args>
object make_array(Args&&... args) {
  return array(std::make_shared<T>(std::forward<Args>(args)...));
}

/**
 * Creates whisker::object with a backing native_function.
 *
 * Postconditions:
 *   object::is_native_function() == true
 *   object::as_native_function() == value
 */
inline object native_function(native_function::ptr value) {
  return object(std::move(value));
}

/**
 * Creates a native_function::ptr of a concrete type with the given arguments.
 *
 * Postconditions:
 *   object::is_native_function() == true
 *   object::as_native_function() == value
 */
template <typename T, typename... Args>
object make_native_function(Args&&... args) {
  return native_function(std::make_shared<T>(std::forward<Args>(args)...));
}

/**
 * Creates a native_handle with a reference to the given arguments.
 *
 * Postconditions:
 *   object::is_native_handle() == true
 *   object::as_native_handle().ptr() == value
 */
template <typename T>
object native_handle(managed_ptr<T> value, prototype<>::ptr prototype) {
  return object(
      whisker::native_handle<>(std::move(value), std::move(prototype)));
}
template <typename T>
object native_handle(whisker::native_handle<T> handle) {
  return object(std::move(handle));
}

} // namespace make

/**
 * The prototype database stores and caches prototype indexed by typeid.
 *
 * This allows reusing prototypes when dealing with recursive native types such
 * as AST classes.
 */
class prototype_database {
 public:
  template <typename T>
  void define(prototype_ptr<T> prototype) {
    auto [_, inserted] =
        prototypes_.emplace(std::type_index(typeid(T)), std::move(prototype));
    if (!inserted) {
      throw std::runtime_error(fmt::format(
          "Prototype for type '{}' already exists.",
          boost::core::demangle(typeid(T).name())));
    }
  }

  /**
   * Gets the cached prototype for the given type, or throws an exception if
   * the type is unknown.
   *
   * If allow_lazy is true, then a failed lookup falls back to a "lazy"
   * prototype which is resolved when used. This is helpful when there is a
   * cycle of type references.
   */
  template <typename T>
  prototype_ptr<T> of(bool allow_lazy = true) const {
    auto found = prototypes_.find(std::type_index(typeid(T)));
    if (found == prototypes_.end()) {
      if (allow_lazy) {
        return this->lazy<T>();
      }
      throw std::runtime_error(fmt::format(
          "Prototype for type '{}' does not exist.",
          boost::core::demangle(typeid(T).name())));
    }
    auto casted = std::dynamic_pointer_cast<const prototype<T>>(found->second);
    if (casted == nullptr) {
      throw std::runtime_error(fmt::format(
          "Prototype for type '{}' is of an unexpected type.",
          boost::core::demangle(typeid(T).name())));
    }
    return casted;
  }

  /**
   * Creates a native_handle for the given reference with a prototype stored
   * in this database.
   *
   * std::remove_reference_t<T> forces the caller to explicitly specify the
   * template argument. This is to prevent accidental use of the wrong type.
   */
  template <typename T>
  whisker::native_handle<T> create(
      whisker::managed_ptr<std::remove_reference_t<T>> o) const {
    return whisker::native_handle<T>(std::move(o), of<T>());
  }
  template <typename T>
  whisker::native_handle<T> create(const std::remove_reference_t<T>& o) const {
    return this->create<T>(whisker::manage_as_static(o));
  }
  template <typename T>
  whisker::object create_nullable(const std::remove_reference_t<T>* o) const {
    return o == nullptr ? whisker::make::null
                        : whisker::object(this->create<T>(*o));
  }

  /**
   * A "lazy" prototype is one whose definition can be deferred until first
   * use. This allows prototypes to refer to each other in cycles that have
   * cyclic references.
   *
   * Note that cyclical prototypes chains are still disallowed.
   */
  template <typename T>
  prototype_ptr<T> lazy() const {
    class lazy_prototype final : public prototype<T> {
     public:
      explicit lazy_prototype(const prototype_database& db) : db_(db) {}

      const prototype<>::descriptor* find_descriptor(
          std::string_view name) const final {
        return this->resolve()->find_descriptor(name);
      }
      std::set<std::string> keys() const final {
        return this->resolve()->keys();
      }
      const prototype<>::ptr& parent() const final {
        return this->resolve()->parent();
      }
      const std::string_view& name() const final {
        return this->resolve()->name();
      }

     private:
      prototype_ptr<T> resolve() const {
        return db_.of<T>(false /* allow_lazy */);
      }
      const prototype_database& db_;
    };
    return std::make_shared<const lazy_prototype>(*this);
  }

 private:
  std::unordered_map<std::type_index, whisker::prototype<>::ptr> prototypes_;
};

} // namespace whisker
