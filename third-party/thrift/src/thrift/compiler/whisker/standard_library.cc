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

#include <thrift/compiler/whisker/dsl.h>

#include <fmt/core.h>

namespace w = whisker::make;

namespace whisker {

namespace {

map::value_type create_array_functions() {
  map array_functions;

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
  array_functions["of"] =
      dsl::make_function("array.of", [](dsl::function::context ctx) -> array {
        ctx.declare_named_arguments({});
        array result;
        result.reserve(ctx.arity());
        for (const object::ptr& arg : ctx.raw().positional_arguments()) {
          result.emplace_back(object(*arg));
        }
        return result;
      });

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
  array_functions["len"] =
      dsl::make_function("array.len", [](dsl::function::context ctx) -> i64 {
        ctx.declare_named_arguments({});
        ctx.declare_arity(1);
        return i64(ctx.argument<array>(0).size());
      });

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
  array_functions["empty?"] = dsl::make_function(
      "array.empty?", [](dsl::function::context ctx) -> boolean {
        ctx.declare_named_arguments({});
        ctx.declare_arity(1);
        return ctx.argument<array>(0).size() == 0;
      });

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
  array_functions["at"] = dsl::make_function(
      "array.at", [](dsl::function::context ctx) -> object::ptr {
        ctx.declare_named_arguments({});
        ctx.declare_arity(2);

        auto a = ctx.argument<array>(0);
        auto index = ctx.argument<i64>(1);

        if (index < 0 || std::size_t(index) >= a.size()) {
          throw ctx.make_error(
              "Index '{}' is out of bounds (size is {}).", index, a.size());
        }
        return a.at(index);
      });

  return map::value_type{"array", std::move(array_functions)};
}

map::value_type create_map_functions() {
  map map_functions;

  /**
   * Returns a view of the provided map's items. The output is an array where
   * each element is a map with "key" and "value" properties.
   *
   * This function fails if the provided map is not enumerable:
   * - whisker::map objects are enumerable.
   * - map-like native objects are enumerable iff map_like::keys() is provided.
   *
   * The order of items in the produced array matches the enumeration above.
   * For whisker::map, properties are sorted lexicographically by name.
   *
   * Name: map.items
   *
   * Arguments:
   *   - [map] — The map to enumerate
   *
   * Returns:
   *   [array] items (key-value pairs) of the provided map.
   */
  map_functions["items"] = dsl::make_function(
      "map.items", [](dsl::function::context ctx) -> object::ptr {
        ctx.declare_named_arguments({});
        ctx.declare_arity(1);

        dsl::map_like m = ctx.argument<map>(0);

        class items_view
            : public native_object,
              public native_object::array_like,
              public std::enable_shared_from_this<native_object::array_like> {
         public:
          explicit items_view(
              dsl::map_like&& m, std::vector<std::string>&& keys)
              : map_like_(std::move(m)), keys_(std::move(keys)) {}

          native_object::array_like::ptr as_array_like() const override {
            return shared_from_this();
          }

          std::size_t size() const final { return keys_.size(); }
          object::ptr at(std::size_t index) const final {
            const std::string& property_name = keys_.at(index);
            auto value = map_like_.lookup_property(property_name);
            // The name is guaranteed to exist because it was enumerated
            assert(value != nullptr);
            return manage_owned<object>(w::map({
                {"key", w::string(property_name)},
                {"value", w::proxy(value)},
            }));
          }

          void print_to(
              tree_printer::scope scope,
              const object_print_options& options) const final {
            default_print_to("<map.items view>", std::move(scope), options);
          }

         private:
          dsl::map_like map_like_;
          std::vector<std::string> keys_;
        };

        auto keys = m.keys();
        if (!keys.has_value()) {
          throw ctx.make_error(
              "map-like object does not have enumerable properties.");
        }
        auto view =
            w::make_native_object<items_view>(std::move(m), std::move(*keys));
        return manage_owned<object>(std::move(view));
      });

  /**
   * Determines if the provided map contains a given key.
   *
   * Name: map.has_key?
   *
   * Arguments:
   *   - [map] — The map to check for key
   *   - [string] — The key to check in the map
   *
   * Returns:
   *   [boolean] if the provided key is in the map.
   */
  map_functions["has_key?"] = dsl::make_function(
      "map.has_key?", [](dsl::function::context ctx) -> boolean {
        ctx.declare_named_arguments({});
        ctx.declare_arity(2);

        dsl::map_like m = ctx.argument<map>(0);
        managed_ptr<std::string> key = ctx.argument<string>(1);
        return m.lookup_property(*key) != nullptr;
      });

  return map::value_type{"map", std::move(map_functions)};
}

map::value_type create_string_functions() {
  map string_functions;

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
  string_functions["len"] =
      dsl::make_function("string.len", [](dsl::function::context ctx) -> i64 {
        ctx.declare_named_arguments({});
        ctx.declare_arity(1);
        return i64(ctx.argument<string>(0)->length());
      });

  /**
   * Produces the provided string concatenated together.
   *
   * Name: string.concat
   *
   * Arguments:
   *   - [string...] — The strings to concatenate together (variadic).
   *
   * Returns:
   *   [string] the combined string
   */
  string_functions["concat"] = dsl::make_function(
      "string.concat", [](dsl::function::context ctx) -> string {
        ctx.declare_named_arguments({});
        std::string result;
        for (std::size_t i = 0; i < ctx.arity(); ++i) {
          result += *ctx.argument<string>(i);
        }
        return result;
      });

  return map::value_type{"string", std::move(string_functions)};
}

map::value_type create_int_functions() {
  map int_functions;

  const auto make_i64_binary_predicate = [](std::string name,
                                            auto&& predicate) {
    using P = std::decay_t<decltype(predicate)>;
    return dsl::make_function(
        std::move(name),
        [p = P(predicate)](dsl::function::context ctx) -> boolean {
          ctx.declare_named_arguments({});
          ctx.declare_arity(2);
          return p(ctx.argument<i64>(0), ctx.argument<i64>(1));
        });
  };

  // The naming format used here matches the "operator" module in Python:
  //    https://docs.python.org/3/library/operator.html
  //
  // For functions returning boolean, "?" is added to the end of the name, as
  // convention for Whisker.

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
  int_functions["lt?"] = make_i64_binary_predicate(
      "int.lt?", [](i64 lhs, i64 rhs) -> bool { return lhs < rhs; });

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
  int_functions["le?"] = make_i64_binary_predicate(
      "int.le?", [](i64 lhs, i64 rhs) -> bool { return lhs <= rhs; });

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
  int_functions["eq?"] = make_i64_binary_predicate(
      "int.eq?", [](i64 lhs, i64 rhs) -> bool { return lhs == rhs; });

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
  int_functions["ne?"] = make_i64_binary_predicate(
      "int.ne?", [](i64 lhs, i64 rhs) -> bool { return lhs != rhs; });

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
  int_functions["ge?"] = make_i64_binary_predicate(
      "int.ge?", [](i64 lhs, i64 rhs) -> bool { return lhs >= rhs; });

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
  int_functions["gt?"] = make_i64_binary_predicate(
      "int.gt?", [](i64 lhs, i64 rhs) -> bool { return lhs > rhs; });

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
  int_functions["add"] =
      dsl::make_function("int.add", [](dsl::function::context ctx) -> i64 {
        ctx.declare_named_arguments({});
        i64 result = 0;
        for (std::size_t i = 0; i < ctx.arity(); ++i) {
          result += ctx.argument<i64>(i);
        }
        return result;
      });

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
  int_functions["neg"] =
      dsl::make_function("int.neg", [](dsl::function::context ctx) -> i64 {
        ctx.declare_named_arguments({});
        ctx.declare_arity(1);
        return -ctx.argument<i64>(0);
      });

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
  int_functions["sub"] =
      dsl::make_function("int.sub", [](dsl::function::context ctx) -> i64 {
        ctx.declare_named_arguments({});
        ctx.declare_arity(2);
        return ctx.argument<i64>(0) - ctx.argument<i64>(1);
      });

  return map::value_type{"int", std::move(int_functions)};
}

map::value_type create_object_functions() {
  map object_functions;

  /**
   * Returns true if two objects are equal in value, otherwise false.
   *
   * Value equality between two (unordered) pair of objects is defined as
   * follows:
   *   - {null, null} → equal
   *   - {i64, i64} → equal if same value
   *   - {f64, f64} → equal if same value
   *   - {string, string} → equal if same value
   *   - {boolean, boolean} → equal if same value
   *   - {array, array}
   *        → equal if all corresponding elements are equal (recursive)
   *   - {array, native_object::array_like}
   *        → equal if all corresponding elements are equal (recursive)
   *   - {map, map}
   *        → equal if all key-value pairs are equal between the two maps
   *          (recursive)
   *   - {map, native_object::map_like}
   *        → true if native_object::keys() presents enumerable property keys
   *          and each key-value pairs are equal between the two maps
   *          (recursive)
   *   - {native_object, native_object}
   *        → equal if both are equivalent array-like or equivalent map-like
   *   - {native_function, native_function} → equal if same pointer to function
   *   - {native_handle, native_handle} → equal if same pointer to handle
   *   - all other pair of objects are NOT equal
   *
   * Name: object.eq?
   *
   * Arguments:
   *   - [object] — The left-hand side of the comparison.
   *   - [object] — The right-hand side of the comparison.
   *
   * Returns:
   *   [boolean] if the two objects are equal.
   */
  object_functions["eq?"] = dsl::make_function(
      "object.eq?", [](dsl::function::context ctx) -> boolean {
        ctx.declare_named_arguments({});
        ctx.declare_arity(2);
        const object::ptr& lhs = ctx.raw().positional_arguments()[0];
        const object::ptr& rhs = ctx.raw().positional_arguments()[1];
        return *lhs == *rhs;
      });

  return map::value_type{"object", std::move(object_functions)};
}

} // namespace

void load_standard_library(map& module) {
  module.emplace(create_array_functions());
  module.emplace(create_map_functions());
  module.emplace(create_string_functions());
  module.emplace(create_int_functions());
  module.emplace(create_object_functions());
}

} // namespace whisker
