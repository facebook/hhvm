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
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <variant>
#include <vector>

#include <thrift/compiler/whisker/detail/overload.h>

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
//   * native_object —
//               User-defined (C++) type with lazily evaluated properties.
//
// Currently, the Whisker templating language only supports i64, null, string,
// and boolean as literals. However, the evaluation engine can recognize all of
// the types above as they are required for compatibility with Mustache /
// Handlebars.

using i64 = std::int64_t;
using f64 = double;
using string = std::string;
using boolean = bool;
using null = std::monostate;

class object;
/**
 * There are two main reasons why we are choosing std::map (rather than
 * std::unordered_map):
 *   1. Before C++20, std::unordered_map does not support heterogenous lookups.
 *   2. *Technically* neither std::map or std::unordered_map are supposed to be
 *      instantiate with an incomplete type. However, the standard library
 *      implementations we use do support it. We know this because mstch has
 *      been doing this for years in production at this point.
 *      std::unordered_map actually breaks for incomplete types in practice.
 */
using map = std::map<std::string, object, std::less<>>;
using array = std::vector<object>;

/**
 * A native_object is the most powerful type in Whisker. Its properties and
 * behavior are fully defined by user C++ code.
 *
 * A native_object implementation may perform arbitrary computation in C++ to
 * resolve a property name as long as the produced result can be marshaled back
 * to a whisker::object type (including possibly another native_object!).
 *
 * Whisker also does not require that all property names exposed by a
 * native_object are statically (or finitely) enumerable. This allows a few
 * freedoms for implementers:
 *   - Properties can be lazily computed at lookup time.
 *   - Property lookup can have side-effects (not recommended, but possible).
 */
class native_object {
 public:
  using ptr = std::shared_ptr<native_object>;
  virtual ~native_object() = default;

  /**
   * Searches for a property on the object referred to by the provided
   * identifier, returning a non-empty optional if present, or std::nullopt
   * otherwise.
   *
   * The returned object is by value because it may outlive this native_object
   * instance. For most whisker::object types, this is fine because they are
   * self-contained.
   * If this property lookup returns a native_object, `foo`, and that object
   * depends on `this`, then `foo` should keep a shared_ptr` to `this` to ensure
   * that `this` outlives `foo`. The Whisker runtime does not provide any
   * lifetime guarantees for `this`.
   *
   * Preconditions:
   *   - The provided string is a valid Whisker identifier
   */
  virtual std::optional<object> lookup_property(
      std::string_view identifier) const = 0;
};

namespace detail {
// This only exists to form a kind-of recursive std::variant with the help of
// forward declared types.
template <typename Self>
using object_base = std::variant<
    null,
    i64,
    f64,
    string,
    boolean,
    native_object::ptr,
    std::vector<Self>,
    std::map<std::string, Self, std::less<>>>;

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
class object final : private detail::object_base<object> {
 private:
  using base = detail::object_base<object>;
  base& as_variant() & { return *this; }
  const base& as_variant() const& { return *this; }

  template <typename T>
  bool holds_alternative() const noexcept {
    assert(!valueless_by_exception());
    return std::holds_alternative<T>(*this);
  }

  template <typename T>
  decltype(auto) as() const {
    assert(!valueless_by_exception());
    return std::get<T>(*this);
  }

  // This function moves out the base variant object which would normally leave
  // the object in a partially moved-from state. However, moving a variant
  // performs a move on the contained alternative, meaniong that assigning to
  // the variant after a move is safe. We have this steal_variant() function
  // hide this detail so we don't trip clang-tidy use-after-move linter.
  base&& steal_variant() & { return std::move(*this); }

 public:
  /* implicit */ object(null = {}) : base(std::in_place_type<null>) {}
  explicit object(boolean value) : base(bool(value)) {}
  explicit object(i64 value) : base(value) {}
  explicit object(f64 value) : base(value) {}
  explicit object(string&& value) : base(std::move(value)) {}
  explicit object(native_object::ptr&& value) : base(std::move(value)) {
    assert(as_native_object() != nullptr);
  }
  explicit object(map&& value) : base(std::move(value)) {}
  explicit object(array&& value) : base(std::move(value)) {}

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
    return detail::variant_match(
        as_variant(), std::forward<Visitors>(visitors)...);
  }

  const i64& as_i64() const { return as<i64>(); }
  bool is_i64() const noexcept { return holds_alternative<i64>(); }

  const f64& as_f64() const { return as<f64>(); }
  bool is_f64() const noexcept { return holds_alternative<f64>(); }

  const string& as_string() const { return as<string>(); }
  bool is_string() const noexcept { return holds_alternative<string>(); }

  const boolean& as_boolean() const { return as<boolean>(); }
  bool is_boolean() const noexcept { return holds_alternative<boolean>(); }

  bool is_null() const noexcept { return holds_alternative<null>(); }

  const native_object::ptr& as_native_object() const {
    return as<native_object::ptr>();
  }
  bool is_native_object() const noexcept {
    return holds_alternative<native_object::ptr>();
  }

  const array& as_array() const { return as<array>(); }
  bool is_array() const noexcept { return holds_alternative<array>(); }

  const map& as_map() const { return as<map>(); }
  bool is_map() const noexcept { return holds_alternative<map>(); }

  friend bool operator==(const object& lhs, null) noexcept {
    return lhs.is_null();
  }
  friend bool operator==(null, const object& rhs) noexcept {
    return rhs.is_null();
  }

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

  friend bool operator==(
      const object& lhs, const native_object::ptr& rhs) noexcept {
    return lhs.is_native_object() && lhs.as_native_object() == rhs;
  }
  friend bool operator==(
      const native_object::ptr& lhs, const object& rhs) noexcept {
    return rhs == lhs;
  }
  // whisker::object is not allowed to have a nullptr native_object
  friend bool operator==(const object&, std::nullptr_t) = delete;
  friend bool operator==(std::nullptr_t, const object&) = delete;

  friend bool operator==(const object& lhs, const array& rhs) noexcept {
    return lhs.is_array() && lhs.as_array() == rhs;
  }
  friend bool operator==(const array& lhs, const object& rhs) noexcept {
    return rhs == lhs;
  }

  friend bool operator==(const object& lhs, const map& rhs) noexcept {
    return lhs.is_map() && lhs.as_map() == rhs;
  }
  friend bool operator==(const map& lhs, const object& rhs) noexcept {
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

  friend bool operator!=(
      const object& lhs, const native_object::ptr& rhs) noexcept {
    return !(lhs == rhs);
  }
  friend bool operator!=(
      const native_object::ptr& lhs, const object& rhs) noexcept {
    return !(lhs == rhs);
  }
  // whisker::object is not allowed to have a nullptr native_object
  friend bool operator!=(const object&, std::nullptr_t) = delete;
  friend bool operator!=(std::nullptr_t, const object&) = delete;

  friend bool operator!=(const object& lhs, const array& rhs) noexcept {
    return !(lhs == rhs);
  }
  friend bool operator!=(const array& lhs, const object& rhs) noexcept {
    return !(lhs == rhs);
  }

  friend bool operator!=(const object& lhs, const map& rhs) noexcept {
    return !(lhs == rhs);
  }
  friend bool operator!=(const map& lhs, const object& rhs) noexcept {
    return !(lhs == rhs);
  }

  friend bool operator!=(const object& lhs, const object& rhs) noexcept {
    return !(lhs == rhs);
  }
};

std::string to_string(const object&);

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
    std::is_integral_v<T> && std::is_signed_v<T> && sizeof(T) <= sizeof(i64);

static_assert(is_supported_integral_type<i64>);
static_assert(is_supported_integral_type<char>);
static_assert(is_supported_integral_type<short>);
static_assert(is_supported_integral_type<int>);
static_assert(is_supported_integral_type<long>);

/**
 * The set of floating point types that are supported by whisker::object (and
 * are therefore also implicitly convertible from).
 */
template <typename T>
static constexpr bool is_supported_floating_point_type =
    std::is_same_v<T, float> || std::is_same_v<T, double>;

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
inline object map(map&& value) {
  return object(std::move(value));
}

/**
 * Creates whisker::object with array type. This function takes ownership of the
 * provided array.
 *
 * Postconditions:
 *   object::is_array() == true
 *   object::as_array() == value
 */
inline object array(array&& value) {
  return object(std::move(value));
}

/**
 * Creates whisker::object with a backing native_object.
 *
 * Postconditions:
 *   object::is_native_object() == true
 *   object::as_native_object() == value
 */
inline object native_object(native_object::ptr value) {
  return object(std::move(value));
}

} // namespace make

} // namespace whisker
