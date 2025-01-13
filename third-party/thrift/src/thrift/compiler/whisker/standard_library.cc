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

#include <thrift/compiler/whisker/standard_library.h>

namespace w = whisker::make;

namespace whisker {

namespace {

class named_native_function : public native_function {
 public:
  explicit named_native_function(std::string_view name)
      : name_(std::move(name)) {}

  void print_to(
      tree_printer::scope scope, const object_print_options&) const final {
    scope.println("<function {}>", name_);
  }

 private:
  std::string_view name_;
};

map::value_type create_array_functions() {
  map array_functions;

  {
    /**
     * Creates an array with the provided arguments in order. This function can
     * be used to form an "array literal".
     *
     * Name: array.of
     *
     * Arguments:
     *   0 or more objects (variadic)
     *
     * Returns:
     *   [array] provided arguments in order.
     */
    class array_of : public named_native_function {
     public:
      array_of() : named_native_function("array.of") {}

      object::ptr invoke(context ctx) override {
        ctx.declare_named_arguments({});
        array result;
        result.reserve(ctx.arity());
        for (const object::ptr& arg : ctx.arguments()) {
          result.emplace_back(object(*arg));
        }
        return manage_owned<object>(w::array(std::move(result)));
      }
    };
    array_functions["of"] = w::make_native_function<array_of>();
  }

  {
    /**
     * Produces the length of an array or array-like object.
     *
     * Name: array.len
     *
     * Arguments:
     *   - [array] — The array to find length of.
     *
     * Returns:
     *   [i64] length of the provided array.
     */
    class array_len : public named_native_function {
     public:
      array_len() : named_native_function("array.len") {}

      object::ptr invoke(context ctx) override {
        ctx.declare_named_arguments({});
        ctx.declare_arity(1);
        auto len = i64(ctx.argument<array>(0).size());
        return manage_owned<object>(w::i64(len));
      }
    };
    array_functions["len"] = w::make_native_function<array_len>();
  }

  {
    /**
     * Checks an array for emptiness.
     *
     * Name: array.empty?
     *
     * Arguments:
     *   - [array] — The array to check for emptiness.
     *
     * Returns:
     *   [boolean] indicating whether the array is empty.
     */
    class array_empty : public named_native_function {
     public:
      array_empty() : named_native_function("array.empty?") {}

      object::ptr invoke(context ctx) override {
        ctx.declare_named_arguments({});
        ctx.declare_arity(1);
        return manage_as_static(
            ctx.argument<array>(0).size() == 0 ? w::true_ : w::false_);
      }
    };
    array_functions["empty?"] = w::make_native_function<array_empty>();
  }

  {
    /**
     * Gets the object from an array at a given index. If the index is negative,
     * or larger than the size of the array, then an error is thrown.
     *
     * Name: array.at
     *
     * Arguments:
     *   - [whisker::array] — The array to get from
     *   - [i64] — The index of the item to get
     *
     * Returns:
     *   [object] the item at the given index.
     */
    class array_at : public named_native_function {
     public:
      array_at() : named_native_function("array.at") {}

      object::ptr invoke(context ctx) override {
        ctx.declare_named_arguments({});
        ctx.declare_arity(2);

        auto a = ctx.argument<array>(0);
        auto index = ctx.argument<i64>(1);

        if (index < 0 || std::size_t(index) >= a.size()) {
          ctx.error(
              "Index '{}' is out of bounds (size is {}).", index, a.size());
        }
        return a.at(index);
      }
    };
    array_functions["at"] = w::make_native_function<array_at>();
  }

  return map::value_type{"array", std::move(array_functions)};
}

map::value_type create_string_functions() {
  map string_functions;

  {
    /**
     * Produces the length of string.
     *
     * Name: string.len
     *
     * Arguments:
     *   - [string] — The string to find length of.
     *
     * Returns:
     *   [i64] length of the provided string.
     */
    class string_len : public named_native_function {
     public:
      string_len() : named_native_function("string.len") {}

      object::ptr invoke(context ctx) override {
        ctx.declare_named_arguments({});
        ctx.declare_arity(1);
        auto len = i64(ctx.argument<string>(0)->length());
        return manage_owned<object>(w::i64(len));
      }
    };
    string_functions["len"] = w::make_native_function<string_len>();
  }

  return map::value_type{"string", std::move(string_functions)};
}

map::value_type create_int_functions() {
  map int_functions;

  class i64_binary_predicate : public named_native_function {
   public:
    using named_native_function::named_native_function;

    virtual boolean invoke(i64 lhs, i64 rhs) const = 0;

    object::ptr invoke(context ctx) final {
      ctx.declare_named_arguments({});
      ctx.declare_arity(2);
      return manage_as_static(
          invoke(ctx.argument<i64>(0), ctx.argument<i64>(1)) ? w::true_
                                                             : w::false_);
    }
  };

  // The naming format used here matches the "operator" module in Python:
  //    https://docs.python.org/3/library/operator.html
  //
  // For functions returning boolean, "?" is added to the end of the name, as
  // convention for Whisker.

  {
    /**
     * Checks if one i64 is lesser than another.
     *
     * Name: int.lt?
     *
     * Arguments:
     *   - [i64] — The left-hand side of the comparison.
     *   - [i64] — The right-hand side of the comparison.
     *
     * Returns:
     *   [boolean] indicating whether the first number is lesser than the
     *             second.
     */
    class int_lt final : public i64_binary_predicate {
     public:
      int_lt() : i64_binary_predicate("int.lt?") {}

      bool invoke(i64 lhs, i64 rhs) const final { return lhs < rhs; }
    };
    int_functions["lt?"] = w::make_native_function<int_lt>();
  }

  {
    /**
     * Checks if one i64 is lesser or equal to than another.
     *
     * Name: int.le?
     *
     * Arguments:
     *   - [i64] — The left-hand side of the comparison.
     *   - [i64] — The right-hand side of the comparison.
     *
     * Returns:
     *   [boolean] indicating whether the first number is lesser than or equal
     *             to the second.
     */
    class int_le final : public i64_binary_predicate {
     public:
      int_le() : i64_binary_predicate("int.le?") {}

      bool invoke(i64 lhs, i64 rhs) const final { return lhs <= rhs; }
    };
    int_functions["le?"] = w::make_native_function<int_le>();
  }

  {
    /**
     * Checks two i64 values for equality.
     *
     * Name: int.eq?
     *
     * Arguments:
     *   - [i64] — The left-hand side of the comparison.
     *   - [i64] — The right-hand side of the comparison.
     *
     * Returns:
     *   [boolean] indicating whether the two values are equal.
     */
    class int_eq final : public i64_binary_predicate {
     public:
      int_eq() : i64_binary_predicate("int.eq?") {}

      bool invoke(i64 lhs, i64 rhs) const final { return lhs == rhs; }
    };
    int_functions["eq?"] = w::make_native_function<int_eq>();
  }

  {
    /**
     * Checks two i64 values for inequality.
     *
     * Name: int.ne?
     *
     * Arguments:
     *   - [i64] — The left-hand side of the comparison.
     *   - [i64] — The right-hand side of the comparison.
     *
     * Returns:
     *   [boolean] indicating whether the two values are not equal.
     */
    class int_ne final : public i64_binary_predicate {
     public:
      int_ne() : i64_binary_predicate("int.ne?") {}

      bool invoke(i64 lhs, i64 rhs) const final { return lhs != rhs; }
    };
    int_functions["ne?"] = w::make_native_function<int_ne>();
  }

  {
    /**
     * Checks if one i64 is greater or equal to than another.
     *
     * Name: int.ge?
     *
     * Arguments:
     *   - [i64] — The left-hand side of the comparison.
     *   - [i64] — The right-hand side of the comparison.
     *
     * Returns:
     *   [boolean] indicating whether the first number is greater than or equal
     *             to the second.
     */
    class int_ge final : public i64_binary_predicate {
     public:
      int_ge() : i64_binary_predicate("int.ge?") {}

      bool invoke(i64 lhs, i64 rhs) const final { return lhs >= rhs; }
    };
    int_functions["ge?"] = w::make_native_function<int_ge>();
  }

  {
    /**
     * Checks if one i64 is greater than another.
     *
     * Name: int.gt?
     *
     * Arguments:
     *   - [i64] — The left-hand side of the comparison.
     *   - [i64] — The right-hand side of the comparison.
     *
     * Returns:
     *   [boolean] indicating whether the first number is greater than the
     *             second.
     */
    class int_gt final : public i64_binary_predicate {
     public:
      int_gt() : i64_binary_predicate("int.gt?") {}

      bool invoke(i64 lhs, i64 rhs) const final { return lhs > rhs; }
    };
    int_functions["gt?"] = w::make_native_function<int_gt>();
  }

  {
    /**
     * Adds numbers together.
     *
     * Name: int.add
     *
     * Arguments:
     *   - [i64...] — numbers to add together.
     *
     * Returns:
     *   [i64] the sum of the provided numbers. If there are no arguments, then
     *         returns 0.
     */
    class int_add final : public named_native_function {
     public:
      int_add() : named_native_function("int.add") {}

      object::ptr invoke(context ctx) override {
        ctx.declare_named_arguments({});
        i64 result = 0;
        for (std::size_t i = 0; i < ctx.arity(); ++i) {
          result += ctx.argument<i64>(i);
        }
        return manage_owned<object>(w::i64(result));
      }
    };
    int_functions["add"] = w::make_native_function<int_add>();
  }

  {
    /**
     * Negates the provided number.
     *
     * Name: int.neg
     *
     * Arguments:
     *   - [i64] — number to negate
     *
     * Returns:
     *   [i64] the negative of the provided number.
     */
    class int_neg final : public named_native_function {
     public:
      int_neg() : named_native_function("int.neg") {}

      object::ptr invoke(context ctx) override {
        ctx.declare_named_arguments({});
        ctx.declare_arity(1);
        return manage_owned<object>(w::i64(-ctx.argument<i64>(0)));
      }
    };
    int_functions["neg"] = w::make_native_function<int_neg>();
  }

  {
    /**
     * Subtracts one number from another.
     *
     * Name: int.sub
     *
     * Arguments:
     *   - [i64] — number to subtract from.
     *   - [i64] — amount to subtract.
     *
     * Returns:
     *   [i64] the difference of the two numbers.
     */
    class int_sub final : public named_native_function {
     public:
      int_sub() : named_native_function("int.sub") {}

      object::ptr invoke(context ctx) override {
        ctx.declare_named_arguments({});
        ctx.declare_arity(2);
        return manage_owned<object>(
            w::i64(ctx.argument<i64>(0) - ctx.argument<i64>(1)));
      }
    };
    int_functions["sub"] = w::make_native_function<int_sub>();
  }

  return map::value_type{"int", std::move(int_functions)};
}

} // namespace

void load_standard_library(map& module) {
  module.emplace(create_array_functions());
  module.emplace(create_string_functions());
  module.emplace(create_int_functions());
}

} // namespace whisker
