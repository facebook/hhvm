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

#include <iterator>

namespace w = whisker::make;

namespace whisker {

namespace {

map::raw::value_type create_array_functions() {
  map::raw array_functions;

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
  array_functions["of"] = dsl::make_function(
      "array.of", [](dsl::function::context ctx) -> array::ptr {
        ctx.declare_named_arguments({});
        array::raw result;
        result.reserve(ctx.arity());
        for (const object& arg : ctx.raw().positional_arguments()) {
          result.emplace_back(arg);
        }
        return array::of(std::move(result));
      });

  /**
   * Produces the length of an array.
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
        return i64(ctx.argument<array>(0)->size());
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
        return ctx.argument<array>(0)->size() == 0;
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
  array_functions["at"] =
      dsl::make_function("array.at", [](dsl::function::context ctx) -> object {
        ctx.declare_named_arguments({});
        ctx.declare_arity(2);

        auto a = ctx.argument<array>(0);
        auto index = ctx.argument<i64>(1);

        if (index < 0 || std::size_t(index) >= a->size()) {
          throw ctx.make_error(
              "Index '{}' is out of bounds (size is {}).", index, a->size());
        }
        return a->at(index);
      });

  /**
   * Returns a view of the provided array's items paired with their index. The
   * output is an array where each element is a tuple of `(index, item)`.
   *
   * The order of items produced in the output array matches the input array.
   *
   * Name: array.enumerate
   *
   * Arguments:
   *   - [array] — The array to enumerate
   *
   * Named arguments:
   *   - [with_first: boolean] — If true, the 3rd element of each tuple is set
   *     to the value of `index == 0`.
   *   - [with_last: boolean] — If true, the last element of each tuple is set
   *     to the value of `index == <size of array> - 1`. If `with_first` is
   *     true, then this is the 4th element. Otherwise, it is the 3rd element.
   *
   * Returns:
   *   [array] enumerated pairs `(index, item)` of the provided array.
   */
  array_functions["enumerate"] = dsl::make_function(
      "array.enumerate", [](dsl::function::context ctx) -> object {
        ctx.declare_named_arguments({"with_first", "with_last"});
        ctx.declare_arity(1);

        auto arr = ctx.argument<array>(0);
        boolean with_first =
            ctx.named_argument<boolean>(
                   "with_first", dsl::function::context::optional)
                .value_or(false);
        boolean with_last =
            ctx.named_argument<boolean>(
                   "with_last", dsl::function::context::optional)
                .value_or(false);

        class enumerate_view : public array {
         public:
          explicit enumerate_view(
              array::ptr&& a, bool with_first, bool with_last)
              : array_(std::move(a)),
                with_first_(with_first),
                with_last_(with_last) {}

          std::size_t size() const final { return array_->size(); }
          object at(std::size_t index) const final {
            array::raw result{
                w::i64(static_cast<i64>(index)),
                array_->at(index),
            };
            if (with_first_) {
              result.emplace_back(w::boolean(index == 0));
            }
            if (with_last_) {
              result.emplace_back(w::boolean(index == array_->size() - 1));
            }
            return w::array(std::move(result));
          }

          void print_to(
              tree_printer::scope& scope,
              const object_print_options& options) const final {
            default_print_to("<array.enumerate view>", scope, options);
          }

         private:
          array::ptr array_;
          bool with_first_;
          bool with_last_;
        };

        return w::make_array<enumerate_view>(
            std::move(arr), with_first, with_last);
      });

  return map::raw::value_type{"array", w::map(std::move(array_functions))};
}

map::raw::value_type create_map_functions() {
  map::raw map_functions;

  /**
   * Returns a view of the provided map's items. The output is an array where
   * each element is a map with "key" and "value" properties.
   *
   * This function fails if the provided map is not enumerable.
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
  map_functions["items"] =
      dsl::make_function("map.items", [](dsl::function::context ctx) -> object {
        ctx.declare_named_arguments({});
        ctx.declare_arity(1);

        map::ptr m = ctx.argument<map>(0);

        class items_view : public array {
         public:
          explicit items_view(map::ptr&& m, std::vector<std::string>&& keys)
              : map_(std::move(m)), keys_(std::move(keys)) {}

          std::size_t size() const final { return keys_.size(); }
          object at(std::size_t index) const final {
            const std::string& property_name = keys_.at(index);
            auto value = map_->lookup_property(property_name);
            // The name is guaranteed to exist because it was enumerated
            assert(value.has_value());
            return w::map({
                {"key", w::string(property_name)},
                {"value", std::move(*value)},
            });
          }

          void print_to(
              tree_printer::scope& scope,
              const object_print_options& options) const final {
            default_print_to("<map.items view>", scope, options);
          }

         private:
          map::ptr map_;
          std::vector<std::string> keys_;
        };

        auto keys = m->keys();
        if (!keys.has_value()) {
          throw ctx.make_error("map does not have enumerable properties.");
        }
        return w::make_array<items_view>(
            std::move(m),
            std::vector<std::string>(
                std::make_move_iterator(keys->begin()),
                std::make_move_iterator(keys->end())));
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

        map::ptr m = ctx.argument<map>(0);
        std::string key = ctx.argument<string>(1);
        return m->lookup_property(key).has_value();
      });

  return map::raw::value_type{"map", w::map(std::move(map_functions))};
}

map::raw::value_type create_string_functions() {
  map::raw string_functions;

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
        return i64(ctx.argument<string>(0).length());
      });

  /**
   * Checks a string for emptiness.
   *
   * Name: string.empty?
   *
   * Arguments:
   *   - [string] — The string to check for emptiness.
   *
   * Returns:
   *   [boolean] indicating whether the string is empty.
   */
  string_functions["empty?"] = dsl::make_function(
      "string.empty?", [](dsl::function::context ctx) -> boolean {
        ctx.declare_named_arguments({});
        ctx.declare_arity(1);
        return ctx.argument<string>(0).empty();
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
          result += ctx.argument<string>(i);
        }
        return result;
      });

  return map::raw::value_type{"string", w::map(std::move(string_functions))};
}

map::raw::value_type create_int_functions() {
  map::raw int_functions;

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

  return map::raw::value_type{"int", w::map(std::move(int_functions))};
}

map::raw::value_type create_object_functions() {
  map::raw object_functions;

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
   *   - {map, map}
   *        → equal if all both maps have enumerable property keys and each
   *          key-value pairs are equal between the two maps (recursive)
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
        const object& lhs = ctx.raw().positional_arguments()[0];
        const object& rhs = ctx.raw().positional_arguments()[1];
        return lhs == rhs;
      });

  return map::raw::value_type{"object", w::map(std::move(object_functions))};
}

} // namespace

void load_standard_library(map::raw& module) {
  module.emplace(create_array_functions());
  module.emplace(create_map_functions());
  module.emplace(create_string_functions());
  module.emplace(create_int_functions());
  module.emplace(create_object_functions());
}

} // namespace whisker
