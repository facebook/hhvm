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

namespace whisker::dsl {
namespace detail {
/**
 * Determines the result of trying to access a typed argument via
 * argument<T>(...) or named_argument<T>(...).
 */
template <typename T>
struct function_argument_result;
} // namespace detail

/**
 * A polymorphic native_handle is intended to be used with polymorphic types to
 * extract native_handle arguments from an inheritence chain.
 *
 * Since a native_handle uses type erasure, dynamic_cast-based traversal of
 * polymorphic types cannot be performed.
 *
 * This class allows expressing a relevant subset of types from a hierarchy of
 * types as potential alternatives. The argument will be checked against only
 * those types. Note that this limits the API to strictly known types at the
 * callsite.
 *
 * Example:
 *
 *     struct Base() {
 *       virtual ~Base() = default;
 *       virtual std::string describe() const = 0;
 *     };
 *     dsl::make_function(
 *         "describe", [](dsl::function::context ctx) -> string {
 *       using base_handle = dsl::polymorphic_native_handle<
 *           Base,
 *           Derived1,
 *           Derived2,
 *           DerivedN...>;
 *       ctx.declare_arity(1);
 *       ctx.declare_named_arguments({});
 *       auto base = ctx.argument<base_handle>(0);
 *       return base->description();
 *     });
 *
 */
template <typename Base, typename... SubClass>
struct polymorphic_native_handle {
  static_assert(std::is_polymorphic_v<Base>);
  static_assert((std::is_base_of_v<Base, SubClass> && ...));

  using element_type = Base;

  /**
   * Tries to match a dynamically-typed native handle against each of the
   * provided sub-classes, short-circuiting at the earliest match.
   *
   * Returns an empty optional if no match is found.
   */
  static std::optional<native_handle<Base>> try_as(
      const native_handle<>& handle) {
    if (std::optional<native_handle<Base>> converted = handle.try_as<Base>()) {
      return std::move(*converted);
    }
    std::optional<native_handle<Base>> result;
    (
        [&] {
          if (result.has_value()) {
            // match already found
            return;
          }
          if (std::optional<native_handle<SubClass>> converted =
                  handle.try_as<SubClass>()) {
            managed_ptr<Base> upcasted =
                std::static_pointer_cast<const Base>(converted->ptr());
            result = native_handle<Base>(
                std::move(upcasted), std::move(*converted).proto());
          }
        }(),
        ...);
    return result;
  }
};

namespace detail {
template <typename...>
struct flatten_poly_handle;
}

/**
 * This template helper is intended to describe polymorphic_native_handle<...>
 * type hierarchies in a clean, descriptive manner.
 *
 * Example:
 *
 *     struct t_A { ... };
 *     struct t_B : t_A { ... };
 *     struct t_B2 : t_B { ... };
 *     struct t_C : t_A { ... };
 *     struct t_C2 : t_C { ... };
 *
 *     // All of the names below are type aliases to
 *     // polymorphic_native_handle<...> types.
 *
 *     namespace polymorphic_handles {
 *     template <typename... Cases>
 *     using handle = make_polymorphic_native_handle<Cases...>;
 *
 *     using C2 = handle<t_C2>;       // (t_C2)
 *     using C = handle<t_C, C2>;     // (t_C, t_C2)
 *     using B2 = handle<t_B2>;       // (t_B2)
 *     using B = handle<t_B, B2>;     // (t_B, t_B2)
 *     using A = handle<t_A, B, C>;   // (t_A, t_B, t_B2, t_C, t_C2)
 *     }
 *
 * All of the type aliases above are some specialization of
 * polymorphic_native_handle<...> describing all possible paths in the
 * hierarchy.
 *
 * You may notice that this API requires declaring the type hierarchy in the
 * inverse direction of the inheritance hierarchy as written in C++.
 */
template <typename... Cases>
using make_polymorphic_native_handle =
    typename detail::flatten_poly_handle<Cases...>::type;

/**
 * A class providing ergonomic APIs (sugar) for implementing native_functions.
 *
 * Features include:
 *   - arguments validation (type checking)
 *   - diagnostics formatting APIs
 *
 * Example:
 *
 *     class i64_eq : public function {
 *       object invoke(context ctx) override {
 *         ctx.declare_arity(2);
 *         ctx.declare_named_arguments({});
 *         i64 a = ctx.argument<i64>(0);
 *         i64 b = ctx.argument<i64>(1);
 *         return whisker::make::boolean(a == b);
 *       }
 *     };
 *
 *     {{ (i64_eq 42 42) }}
 *     {{! Produces true }}
 *
 * Example (variadic, named arguments):
 *
 *     class str_concat : public function {
 *       object invoke(context ctx) override {
 *         ctx.declare_named_arguments({"sep"});
 *         const std::string sep = [&] {
 *           auto arg = ctx.named_argument<string>("sep", context::optional);
 *           return arg.has_value() ? std::string{*arg} : "";
 *         }();
 *         string result;
 *         for (std::size_t i = 0; i < ctx.arity(); ++i) {
 *           if (i != 0) {
 *             result += sep;
 *           }
 *           result += ctx.argument<string>(i);
 *         }
 *         return whisker::make::string(std::move(result));
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
  virtual object invoke(context) = 0;

  using raw_context = native_function::context;
  class context {
   public:
    explicit context(raw_context&& raw) : raw_(std::move(raw)) {}

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
     * Returns a reference to the implicit `self` argument, checked against the
     * desired type `T`.
     *
     * If the argument is not of the correct type, then this throws an error.
     */
    template <typename T>
    argument_result_t<T> self() const {
      return extract_argument<T>(
          raw().self(), [] { return std::string("'self' argument"); });
    }

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
     * an error. Otherwise, returns empty optional.
     */
    std::optional<object> named_argument(
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
      std::optional<object> arg = this->named_argument(name, presence);
      if (!arg.has_value()) {
        assert(presence == named_argument_presence::optional);
        // either nullptr or empty optional
        return {};
      }
      return extract_argument<T>(
          *arg, [name] { return fmt::format("named argument '{}'", name); });
    }

    /**
     * Creates a eval_error instance that can be thrown to indicate an error in
     * function evaluation.
     *
     * Calling this function will prevent further evaluation of this function
     * and cause text rendering to fail.
     */
    template <typename... T>
    eval_error make_error(fmt::format_string<T...> msg, T&&... args) const {
      return eval_error{fmt::format(msg, std::forward<T>(args)...)};
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
        const object& arg, DescribeArgumentFunc&& describe_argument) const {
      if constexpr (std::is_same_v<T, array>) {
        if (arg.is_array()) {
          return arg.as_array();
        }
        throw make_error(
            "Expected type of {} to be `array`, but found `{}`.",
            describe_argument(),
            arg.describe_type());
      } else if constexpr (std::is_same_v<T, map>) {
        if (arg.is_map()) {
          return arg.as_map();
        }
        throw make_error(
            "Expected type of {} to be `map`, but found `{}`.",
            describe_argument(),
            arg.describe_type());
      } else if constexpr (whisker::detail::is_specialization_v<
                               T,
                               polymorphic_native_handle>) {
        // polymorpic_native_handle<T, ...> (class hierarchy match)
        using element_type = typename T::element_type;
        const auto abort = [&] {
          return make_error(
              "Expected type of {} to be `{}` (polymorphic), but found `{}`.",
              describe_argument(),
              native_handle<element_type>::describe_class_type(),
              arg.describe_type());
        };

        if (!arg.is_native_handle()) {
          throw abort();
        }
        const native_handle<>& handle = arg.as_native_handle();
        if (std::optional<native_handle<element_type>> converted =
                T::try_as(handle)) {
          return std::move(*converted);
        }
        throw abort();
      } else if constexpr (whisker::detail::
                               is_specialization_v<T, native_handle>) {
        // native_handle<T> (exact match)
        using element_type = typename T::element_type;
        const auto abort = [&] {
          return make_error(
              "Expected type of {} to be `{}`, but found `{}`.",
              describe_argument(),
              T::describe_class_type(),
              arg.describe_type());
        };

        if (!arg.is_native_handle()) {
          throw abort();
        }
        const native_handle<>& handle = arg.as_native_handle();
        if constexpr (std::is_same_v<element_type, void>) {
          return handle;
        } else {
          if (auto converted = handle.try_as<element_type>()) {
            return std::move(*converted);
          }
          throw abort();
        }
      } else {
        // Primitive types
        static_assert(
            std::is_same_v<T, boolean> || std::is_same_v<T, i64> ||
            std::is_same_v<T, f64> || std::is_same_v<T, string> ||
            std::is_same_v<T, null>);
        if (!arg.is<T>()) {
          throw make_error(
              "Expected type of {} to be `{}`, but found `{}`.",
              describe_argument(),
              describe_primitive_type<T>(),
              arg.describe_type());
        }
        return arg.as<T>();
      }
    }

    raw_context raw_;
  };
  static_assert(std::is_move_constructible_v<context>);

 private:
  object invoke(raw_context) final;
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
    std::enable_if_t< //
        std::is_same_v<detail::function_return_t<F>, object>,
        int> = 0>
class make_function_delegate final : public function {
 public:
  make_function_delegate(std::string name, F&& impl)
      : name_(std::move(name)), impl_(std::forward<F>(impl)) {}

  object invoke(context ctx) final { return impl_(std::move(ctx)); }

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
      tree_printer::scope& scope, const object_print_options&) const final {
    scope.print("{}", describe_type());
  }

 private:
  std::string name_;
  std::decay_t<F> impl_;
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
 *         "i64_eq", [](dsl::function::context ctx) -> object {
 *           ctx.declare_arity(2);
 *           ctx.declare_named_arguments({});
 *           i64 a = ctx.argument<i64>(0);
 *           i64 b = ctx.argument<i64>(1);
 *           return whisker::make::boolean(a == b);
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
    std::enable_if_t<detail::is_function_returning<F, object>, int> = 0>
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
    std::enable_if_t<
        whisker::is_any_object_type<detail::function_return_t<F>>,
        int> = 0>
function::ptr make_function(std::string name, F&& function) {
  return make_function(
      std::move(name),
      [f = std::decay_t<F>(std::forward<F>(function))](
          function::context ctx) -> object {
        if constexpr (detail::is_function_returning<F, boolean>) {
          return f(std::move(ctx)) ? whisker::make::true_
                                   : whisker::make::false_;
        } else if constexpr (detail::is_function_returning<F, null>) {
          f(std::move(ctx));
          return whisker::make::null;
        } else {
          return object(f(std::move(ctx)));
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

/**
 * A class that provides an ergonomic way to build prototype objects.
 *
 * The provided handle type must be either:
 *   - native_handle<S>, or
 *   - polymorphic_native_handle<S, ...>
 *
 * The handle's element type (S) is called the "self type" of the prototype. The
 * prototype is designed to operate as members of the self type only.
 *
 * Example (property):
 *
 *    struct Foo {
 *      whisker::i64 woah() const { return 42; }
 *    };
 *    using foo_handle = native_handle<Foo>;
 *    prototype_builder<foo_handle> def;
 *    def.property("woah", [](const Foo& self) { return self.woah(); });
 *    prototype_ptr<Foo> proto = std::move(def).make();
 *
 *    {{ foo.woah }}
 *    {{! Produces 42 }}
 *
 * Example (function):
 *
 *    struct Bar {
 *      i64 add1(i64 x) const { return x + 1; }
 *    };
 *    using bar_handle = native_handle<Bar>;
 *    prototype_builder<bar_handle> def;
 *    def.function("add1", [](const Bar& self, function::context ctx) {
 *      return self.add1(ctx.argument<i64>(0));
 *    });
 *    prototype_ptr<Bar> proto = std::move(def).make();
 *
 *    {{ (bar.add1 41) }}
 *    {{! Produces 42 }}
 */
template <typename Handle>
class prototype_builder {
 public:
  using self_type = typename Handle::element_type;
  using result = typename prototype<self_type>::ptr;

  template <
      typename Parent,
      std::enable_if_t<std::is_base_of_v<Parent, self_type>, int> = 0>
  explicit prototype_builder(
      prototype_ptr<Parent> parent, const std::string_view& name = "")
      : parent_(std::move(parent)), name_(name) {
    assert(parent_ != nullptr);
  }

  explicit prototype_builder(const std::string_view& name)
      : parent_(nullptr), name_(name) {}

  prototype_builder() = default;

  /**
   * Registers a property descriptor with the provided name.
   *
   * Throws:
   *   - `std::runtime_error` if there is another descriptor with the same name.
   */
  template <typename F>
  void property(std::string name, F&& function) {
    try_emplace(
        std::move(name),
        dsl::make_function([f = std::decay_t<F>(std::forward<F>(function))](
                               function::context ctx) {
          native_handle<self_type> self = ctx.self<Handle>();
          return f(*self);
        }));
  }

  /**
   * Registers a "member function" descriptor with the provided name. The
   * underlying descriptor is a fixed_object descriptor which is a
   * native_function instance.
   *
   * Throws:
   *   - `std::runtime_error` if there is another descriptor with the same name.
   */
  template <typename F>
  void function(std::string name, F&& function) {
    auto fn =
        dsl::make_function([f = std::decay_t<F>(std::forward<F>(function))](
                               function::context ctx) {
          native_handle<self_type> self = ctx.self<Handle>();
          return f(*self, std::move(ctx));
        });
    try_emplace(
        std::move(name),
        prototype<>::fixed_object(
            whisker::make::native_function(std::move(fn))));
  }

  /**
   * Finalizes the prototype, and returns the resultant object. No further
   * changes can be made to this prototype builder.
   */
  result make() && {
    return std::make_shared<basic_prototype<self_type>>(
        std::move(descriptors_), std::move(parent_), std::move(name_));
  }

  // Define a prototype which extends from a parent prototype of a super type.
  // To extend the functionality of a prototype for the same type, use
  // `patches`.
  template <
      typename Parent,
      std::enable_if_t<
          (std::is_base_of_v<Parent, self_type> &&
           !std::is_same_v<Parent, self_type>),
          int> = 0>
  static prototype_builder extends(
      prototype_ptr<Parent> parent, const std::string_view& name = "") {
    return prototype_builder{std::move(parent), name};
  }

  // Define a prototype which extends from a parent prototype of the same type.
  // This will return an extension builder which accepts the name (if any) of
  // the parent.
  static prototype_builder patches(prototype_ptr<self_type> parent) {
    const std::string_view name = parent->name();
    auto builder = prototype_builder{std::move(parent), name};

    return builder;
  }

 private:
  void try_emplace(std::string name, prototype<>::descriptor descriptor) {
    auto [_, inserted] = descriptors_.emplace(name, std::move(descriptor));
    if (!inserted) {
      throw std::runtime_error(
          fmt::format("Descriptor named '{}' already exists.", name));
    }
  }

  prototype<>::ptr parent_;
  prototype<>::descriptors_map descriptors_;

  /**
   * Optional explicit name.
   * The name can be used in contexts like loops which implicitly change the
   * scope from the parent object to the looped type. By using an explicit name
   * for the prototype, the type of the template variable can be explicitly
   * specified for readability and as an assertion of the type being operated
   * on. Names can also be used to explicitly access parent prototype members
   * on child objects.
   */
  std::string_view name_;
};

/**
 * A helper function for `prototype_builder<Handle>` where the creation of the
 * builder object and materializing an instance are hidden from the user.
 *
 * The user provided a function will be called with a
 * `prototype_builder<Handle>`.
 *
 * The primary benefit of this function is to avoid the creation (and thus
 * naming) of a temporary prototype_builder object.
 */
template <typename Handle, typename Parent, typename F>
typename prototype_builder<Handle>::result make_prototype(
    prototype_ptr<Parent> parent, F&& build) {
  prototype_builder<Handle> builder{std::move(parent)};
  std::invoke(std::forward<F>(build), builder);
  return std::move(builder).make();
}

template <typename Handle, typename F>
typename prototype_builder<Handle>::result make_prototype(F&& build) {
  prototype_builder<Handle> builder;
  std::invoke(std::forward<F>(build), builder);
  return std::move(builder).make();
}

namespace detail {

template <typename T>
struct by_value {
  using type = T;
  using optional_type = std::optional<T>;
};

template <typename T>
struct by_managed_ptr {
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
struct function_argument_result<string> : by_value<string> {};

template <>
struct function_argument_result<array> : by_managed_ptr<array> {};

template <>
struct function_argument_result<map> : by_managed_ptr<map> {};

template <typename T>
struct function_argument_result<native_handle<T>> : by_value<native_handle<T>> {
};

template <typename Base, typename... SubClasses>
struct function_argument_result<polymorphic_native_handle<Base, SubClasses...>>
    : by_value<native_handle<Base>> {};

} // namespace detail

// flatten_poly_handle<...> works by recursively expanding
// polymorphic_native_handle<...> types that are present in its template
// parameter pack.
//
// To achieve this, we pattern match (using template specialization) against the
// following patterns (poly<...> represents polymorphic_native_handle<...>).
//
// First are the base cases (one param only):
//   1. (T)       → poly<T>
//   2. (poly<T>) → poly<T>
//
// Then are the 4 permutations of (T, <poly>) pairs, and recursion:
//   3. (T,          U,          rest...) → flatten(poly<T, U>,       rest...)
//   4. (T,          poly<U...>, rest...) → flatten(poly<T, U...>,    rest...)
//   5. (poly<T...>, U,          rest...) → flatten(poly<T..., U>,    rest...)
//   6. (poly<T...>, poly<U...>, rest...) → flatten(poly<T..., U...>, rest...)
//
// The end result is a type where all poly<...> parameter packs to any degree of
// depth is flattened to a single poly<...>.
namespace detail {

template <typename... T>
using poly = polymorphic_native_handle<T...>;
template <typename... T>
using flatten = typename flatten_poly_handle<T...>::type;

// (1): (T) → poly<T>
template <typename T>
struct flatten_poly_handle<T> {
  using type = poly<T>;
};
// (2): (poly<T>) → poly<T>
template <typename... T>
struct flatten_poly_handle<poly<T...>> {
  using type = poly<T...>;
};

// (3): (T, U, rest...) → flatten(poly<T, U>, rest...)
template <typename T, typename U, typename... Rest>
struct flatten_poly_handle<T, U, Rest...> {
  using type = flatten<poly<T, U>, Rest...>;
};

// (4): (T, poly<U...>, rest...) → flatten(poly<T, U...>, rest...)
template <typename T, typename... U, typename... Rest>
struct flatten_poly_handle<T, poly<U...>, Rest...> {
  using type = flatten<poly<T, U...>, Rest...>;
};

// (5): (poly<T...>, U, rest....) → flatten(poly<T..., U>, rest...)
template <typename... T, typename U, typename... Rest>
struct flatten_poly_handle<poly<T...>, U, Rest...> {
  using type = flatten<poly<T..., U>, Rest...>;
};

// (6) (poly<T...>, poly<U...>, rest...) → flatten(poly<T..., U...>, rest...)
template <typename... T, typename... U, typename... Rest>
struct flatten_poly_handle<poly<T...>, poly<U...>, Rest...> {
  using type = flatten<poly<T...>, U..., Rest...>;
};

} // namespace detail
} // namespace whisker::dsl
