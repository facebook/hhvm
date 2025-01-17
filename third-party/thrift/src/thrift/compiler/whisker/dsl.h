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

#include <thrift/compiler/whisker/detail/type_traits.h>
#include <thrift/compiler/whisker/managed_ptr.h>
#include <thrift/compiler/whisker/object.h>

#include <fmt/core.h>

#include <cstddef>
#include <initializer_list>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

namespace whisker::dsl {

// This macro greatly simplifies the SFINAE tricks required by dsl.
#define WHISKER_DSL_REQUIRES(...) std::enable_if_t<__VA_ARGS__>* = nullptr

/**
 * A class that abstracts over the difference between a whisker::array and a
 * native_object::array_like object.
 *
 * This is useful for the common case where native_function implementations
 * are ambivalent to the underlying array-like type.
 */
class array_like final : public native_object::array_like {
 public:
  std::size_t size() const final;
  object::ptr at(std::size_t index) const final;

  /**
   * Tries to marshal the provided object into an array-like object, if the
   * underlying type matches. Otherwise, returns an empty optional.
   */
  static std::optional<array_like> try_from(const object::ptr&);

  explicit array_like(native_object::array_like::ptr&& arr)
      : which_(std::move(arr)) {}
  explicit array_like(managed_ptr<array>&& arr) : which_(std::move(arr)) {}

 private:
  std::variant<native_object::array_like::ptr, managed_ptr<array>> which_;
};
static_assert(std::is_move_constructible_v<array_like>);
static_assert(std::is_copy_constructible_v<array_like>);

/**
 * A class that abstracts over the difference between a whisker::map and a
 * native_object::map_like object.
 *
 * This is useful for the common case where native_function implementations
 * are ambivalent to the underlying map-like type.
 */
class map_like final : public native_object::map_like {
 public:
  object::ptr lookup_property(std::string_view identifier) const final;
  std::optional<std::vector<std::string>> keys() const final;

  /**
   * Tries to marshal the provided object into an map-like object, if the
   * underlying type matches. Otherwise, returns an empty optional.
   */
  static std::optional<map_like> try_from(const object::ptr&);

  explicit map_like(native_object::map_like::ptr&& m) : which_(std::move(m)) {}
  explicit map_like(managed_ptr<map>&& m) : which_(std::move(m)) {}

 private:
  std::variant<native_object::map_like::ptr, managed_ptr<map>> which_;
};
static_assert(std::is_move_constructible_v<map_like>);
static_assert(std::is_copy_constructible_v<map_like>);

namespace detail {
/**
 * Determines the result of trying to access a typed argument via
 * argument<T>(...) or named_argument<T>(...).
 *
 * Small primitive types (i64, f64, boolean) are returned by value.
 * The larger primitive type (string) is returned as a managed_ptr<string>.
 * native_handle<T> is returned by value.
 *
 * Maps and arrays are wrapped by helper classes, in order to abstract away
 * differences with native_object::array_like and native_object::map_like
 * respectively.
 */
template <typename T>
struct function_argument_result;
} // namespace detail

/**
 * A class providing ergonomic APIs (sugar) for implementing native_functions.
 *
 * Features include:
 *   - arguments validation (type checking)
 *   - diagnostics formatting APIs
 *   - Exposing better APIs like array_like and map_like instead of raw objects.
 *
 * Example:
 *
 *     class i64_eq : public function {
 *       object::ptr invoke(context ctx) override {
 *         ctx.declare_arity(2);
 *         ctx.declare_named_arguments({});
 *         i64 a = ctx.argument<i64>(0);
 *         i64 b = ctx.argument<i64>(1);
 *         return manage_owned<object>(whisker::make::boolean(a == b));
 *       }
 *     };
 *
 *     {{ (i64_eq 42 42) }}
 *     {{! Produces true }}
 *
 * Example (variadic, named arguments):
 *
 *     class str_concat : public function {
 *       object::ptr invoke(context ctx) override {
 *         ctx.declare_named_arguments({"sep"});
 *         const std::string sep = [&] {
 *           auto arg = ctx.named_argument<string>("sep", context::optional);
 *           return arg == nullptr ? "" : *arg;
 *         }();
 *         string result;
 *         for (std::size_t i = 0; i < ctx.arity(); ++i) {
 *           if (i != 0) {
 *             result += sep;
 *           }
 *           result += *ctx.argument<string>(i);
 *         }
 *         return manage_owned<object>(
 *             whisker::make::string(std::move(result)));
 *       }
 *     };
 *
 *     {{ (str_concat "apache" "thrift" "test" sep="::") }}
 *     {{! Produces "apache::thrift::test" }}
 *
 * The standard library contains more examples.
 */
class function : public native_function {
 public:
  class context;
  /**
   * The implementation-defined behavior for this function.
   *
   * Postconditions:
   *  - The returned object is non-null.
   */
  virtual object::ptr invoke(context) = 0;

  using raw_context = native_function::context;
  class context {
   public:
    /**
     * The raw native_function::context object that this type is wrapping.
     */
    const raw_context& raw() const noexcept { return raw_; }
    /**
     * Returns the number of positional arguments to this function call.
     * Note that named arguments do not affect the arity.
     */
    std::size_t arity() const noexcept {
      return raw().positional_arguments().size();
    }

    template <typename T>
    using argument_result_t =
        typename detail::function_argument_result<T>::type;
    /**
     * Returns a reference to a positional argument, checked against the desired
     * type `T`.
     *
     * If the argument is not of the correct type, then this throws an error.
     *
     * Preconditions:
     *  - index < arity()
     *
     * Postconditions:
     *   - The returned object is non-null.
     */
    template <typename T>
    argument_result_t<T> argument(std::size_t index) const {
      return extract_argument<T>(
          raw().positional_arguments().at(index),
          [index] { return fmt::format("argument at index {}", index); });
    }

    /**
     * Signals the intent of a named argument. Named arguments are often used as
     * options, but not always.
     */
    enum class named_argument_presence : bool {
      required,
      optional,
    };
    // For convenience, we make these names available in class scope.
    static constexpr named_argument_presence required =
        named_argument_presence::required;
    static constexpr named_argument_presence optional =
        named_argument_presence::optional;

    /**
     * Returns a pointer to a named argument, if present.
     *
     * If the argument is not present and presence is required, then this throws
     * an error. Otherwise, returns nullptr.
     */
    object::ptr named_argument(
        std::string_view name,
        named_argument_presence = named_argument_presence::required) const;

    template <typename T>
    using argument_result_optional_t =
        typename detail::function_argument_result<T>::optional_type;
    /**
     * Returns a reference to a named argument, checked against the desired type
     * `T`, if present.
     *
     * If the argument is not present and presence is required, then this throws
     * an error. Otherwise, returns nullptr (or empty optional).
     *
     * If the argument is present but is not of the correct type, then this
     * throws an error.
     */
    template <typename T>
    argument_result_optional_t<T> named_argument(
        std::string_view name,
        named_argument_presence presence =
            named_argument_presence::required) const {
      const object::ptr& arg = this->named_argument(name, presence);
      if (arg == nullptr) {
        assert(presence == named_argument_presence::optional);
        // either nullptr or empty optional
        return {};
      }
      return extract_argument<T>(
          arg, [name] { return fmt::format("named argument '{}'", name); });
    }

    /**
     * Logs a fatal error in function evaluation.
     *
     * Calling this function will prevent further evaluation of this function
     * and cause text rendering to fail.
     */
    template <typename... T>
    [[noreturn]] void error(fmt::format_string<T...> msg, T&&... args) const {
      throw native_function::fatal_error{
          fmt::format(msg, std::forward<T>(args)...)};
    }

    /**
     * Logs a non-fatal warning in function evaluation.
     */
    template <typename... T>
    void warning(fmt::format_string<T...> msg, T&&... args) const {
      this->do_warning(fmt::format(msg, std::forward<T>(args)...));
    }

    /**
     * Throws an error if arity() != expected. Otherwise, this a no-op.
     */
    void declare_arity(std::size_t expected) const;
    /**
     * Throws an error if the set of named arguments contains any names not
     * provided here. This can catch typos in function calls. Otherwise, this is
     * a no-op.
     *
     * Note that the named arguments at runtime must be a *subset* of the set
     * provided here, NOT an exact match. This is because named arguments may be
     * optional.
     */
    void declare_named_arguments(
        std::initializer_list<std::string_view> expected) const;

   private:
    void do_warning(std::string msg) const;

    template <typename T>
    static std::string_view describe_primitive_type() {
      if constexpr (std::is_same_v<T, boolean>) {
        return "boolean";
      } else if constexpr (std::is_same_v<T, i64>) {
        return "i64";
      } else if constexpr (std::is_same_v<T, f64>) {
        return "f64";
      } else if constexpr (std::is_same_v<T, string>) {
        return "string";
      } else if constexpr (std::is_same_v<T, null>) {
        return "null";
      } else {
        static_assert(sizeof(T) == 0, "Invalid primitive type");
      }
    }

    template <typename T, typename DescribeArgumentFunc>
    argument_result_t<T> extract_argument(
        const object::ptr& arg,
        DescribeArgumentFunc&& describe_argument) const {
      if constexpr (std::is_same_v<T, array>) {
        if (auto arr = array_like::try_from(arg)) {
          return std::move(*arr);
        }
        error(
            "Expected type of {} to be `array` or `array-like native_object`, but found `{}`.",
            describe_argument(),
            arg->describe_type());
      } else if constexpr (std::is_same_v<T, map>) {
        if (auto m = map_like::try_from(arg)) {
          return std::move(*m);
        }
        error(
            "Expected type of {} to be `map` or `map-like native_object`, but found `{}`.",
            describe_argument(),
            arg->describe_type());
      } else if constexpr (whisker::detail::
                               is_specialization_v<T, native_handle>) {
        using element_type = typename T::element_type;
        if (!arg->is_native_handle()) {
          error(
              "Expected type of {} to be `{}`, but found `{}`.",
              describe_argument(),
              T::describe_class_type(),
              arg->describe_type());
        }
        const native_handle<>& handle = arg->as_native_handle();
        if constexpr (std::is_same_v<element_type, void>) {
          return handle;
        } else {
          if (!handle.is<element_type>()) {
            error(
                "Expected type of {} to be `{}`, but found `{}`.",
                describe_argument(),
                T::describe_class_type(),
                arg->describe_type());
          }
          return handle.as<element_type>();
        }
      } else {
        // Primitive types
        static_assert(
            std::is_same_v<T, boolean> || std::is_same_v<T, i64> ||
            std::is_same_v<T, f64> || std::is_same_v<T, string> ||
            std::is_same_v<T, null>);
        if (!arg->is<T>()) {
          error(
              "Expected type of {} to be `{}`, but found `{}`.",
              describe_argument(),
              describe_primitive_type<T>(),
              arg->describe_type());
        }
        if constexpr (std::is_same_v<T, string>) {
          return manage_derived_ref<T>(arg, arg->as<T>());
        } else {
          return arg->as<T>();
        }
      }
    }

    explicit context(raw_context&& raw) : raw_(std::move(raw)) {}
    raw_context raw_;

    friend class function;
  };
  static_assert(std::is_move_constructible_v<context>);

 private:
  object::ptr invoke(raw_context) final;
};

namespace detail {

template <typename F>
using function_return_t = std::invoke_result_t<F, function::context>;

template <typename F, typename T>
constexpr inline bool is_function_returning =
    std::is_same_v<function_return_t<F>, T>;

// This class could be moved to the body of make_function(...).
// However, MSVC fails to compile that.
template <
    typename F,
    WHISKER_DSL_REQUIRES(
        std::is_same_v<detail::function_return_t<F>, object::ptr>)>
class make_function_delegate final : public function {
 public:
  make_function_delegate(std::string name, F&& impl)
      : name_(std::move(name)), impl_(std::forward<F>(impl)) {}

  object::ptr invoke(context ctx) final { return impl_(std::move(ctx)); }

  std::string describe_type() const final {
    if (name_.empty()) {
      // The name can be empty if one is not provided via
      // dsl::make_function(...). That's fine because the name does not affect
      // execution, only a small subset of debug / diagnostics information in
      // case there is an error.
      return "<function>";
    }
    return fmt::format("<function {}>", name_);
  }

  void print_to(
      tree_printer::scope scope, const object_print_options&) const final {
    scope.println("{}", describe_type());
  }

 private:
  std::string name_;
  F impl_;
};

} // namespace detail

/**
 * Creates a named function object with the provided implementation. This
 * function provides terse syntax for dsl::function in cases where the
 * implementing class is only used once.
 *
 * Example:
 *
 *     dsl::make_function(
 *         "i64_eq", [](dsl::function::context ctx) -> object::ptr {
 *           ctx.declare_arity(2);
 *           ctx.declare_named_arguments({});
 *           i64 a = ctx.argument<i64>(0);
 *           i64 b = ctx.argument<i64>(1);
 *           return manage_owned<object>(whisker::make::boolean(a == b));
 *         });
 *
 *     {{ (i64_eq 42 42) }}
 *     {{! Produces true }}
 *
 * The name is used for debugging only. There is an overload that omits the name
 * which can be used in the common case.
 */
template <
    typename F,
    WHISKER_DSL_REQUIRES(detail::is_function_returning<F, object::ptr>)>
function::ptr make_function(std::string name, F&& function) {
  return std::make_shared<detail::make_function_delegate<F>>(
      std::move(name), std::forward<F>(function));
}

/**
 * An overload of make_function that allows the function to return any whisker
 * object.
 *
 * The returned object is wrapped via manage_owned<object>(...) for all types
 * except boolean and null (which are manage_static(...)).
 *
 * Example:
 *
 *     dsl::make_function(
 *         "i64_eq", [](dsl::function::context ctx) -> whisker::boolean {
 *           ctx.declare_arity(2);
 *           ctx.declare_named_arguments({});
 *           i64 a = ctx.argument<i64>(0);
 *           i64 b = ctx.argument<i64>(1);
 *           return a == b;
 *         });
 *
 *     {{ (i64_eq 42 42) }}
 *     {{! Produces true }}
 */
template <
    typename F,
    WHISKER_DSL_REQUIRES(
        whisker::is_any_object_type<detail::function_return_t<F>> ||
        detail::is_function_returning<F, object>)>
function::ptr make_function(std::string name, F&& function) {
  return make_function(
      std::move(name),
      [f = F(std::forward<F>(function))](function::context ctx) {
        if constexpr (detail::is_function_returning<F, boolean>) {
          return manage_as_static(
              f(std::move(ctx)) ? whisker::make::true_ : whisker::make::false_);
        } else if constexpr (detail::is_function_returning<F, null>) {
          f(std::move(ctx));
          return manage_as_static(whisker::make::null);
        } else {
          return manage_owned<object>(f(std::move(ctx)));
        }
      });
}

/**
 * Creates an function object without a name for debugging. It will be seen as
 * anonymous when debug printing. Omitting the name does not affect how the
 * function behaves, nor how name lookup works in Whisker.
 */
template <typename F>
function::ptr make_function(F&& function) {
  return make_function("" /* name */, std::forward<F>(function));
}

namespace detail {

template <typename T>
struct by_value {
  using type = T;
  using optional_type = std::optional<T>;
};

template <typename T>
struct by_ptr {
  using type = managed_ptr<T>;
  using optional_type = type;
};

template <>
struct function_argument_result<i64> : by_value<i64> {};

template <>
struct function_argument_result<f64> : by_value<f64> {};

template <>
struct function_argument_result<boolean> : by_value<boolean> {};

template <>
struct function_argument_result<string> : by_ptr<string> {};

template <>
struct function_argument_result<array> : by_value<array_like> {};

template <>
struct function_argument_result<map> : by_value<map_like> {};

template <typename T>
struct function_argument_result<native_handle<T>> : by_value<native_handle<T>> {
};

} // namespace detail

#undef WHISKER_DSL_REQUIRES

} // namespace whisker::dsl
