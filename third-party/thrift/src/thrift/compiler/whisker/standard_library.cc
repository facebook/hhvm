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
        return object::owned(w::array(std::move(result)));
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
        return object::owned(w::i64(len));
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
        return object::as_static(
            ctx.argument<array>(0).size() == 0 ? w::true_ : w::false_);
      }
    };
    array_functions["empty?"] = w::make_native_function<array_empty>();
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
        return object::owned(w::i64(len));
      }
    };
    string_functions["len"] = w::make_native_function<string_len>();
  }

  return map::value_type{"string", std::move(string_functions)};
}

} // namespace

void load_standard_library(map& module) {
  module.emplace(create_array_functions());
  module.emplace(create_string_functions());
}

} // namespace whisker
