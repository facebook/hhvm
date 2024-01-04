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

#ifndef THRIFT_FATAL_DEBUG_H_
#define THRIFT_FATAL_DEBUG_H_ 1

#include <string>

#include <folly/Range.h>

/**
 * READ ME FIRST: this header enhances Thrift with debugging utilities.
 *
 * Please refer to the top of `thrift/lib/cpp2/reflection/reflection.h` on how
 * to enable compile-time reflection for Thrift types. The present header relies
 * on it for its functionality.
 *
 * TROUBLESHOOTING:
 *  - make sure you've followed the instructions on `reflection.h` to enable
 *    generation of compile-time reflection;
 *  - make sure you've included the metadata for your Thrift types, as specified
 *    in `reflection.h`.
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */

#include <thrift/lib/cpp2/reflection/internal/debug-inl-pre.h>

#include <thrift/lib/cpp2/reflection/pretty_print.h>

namespace apache {
namespace thrift {

/**
 * Compares two objects for equality, field-by-fields, in a DFS traversal.
 * Returns a boolean that tells whether they're equal or not.
 *
 * Once a mismatch has been found, the callback is called as if with the
 * following signature:
 *
 *  void operator ()(
 *    T const* lhs,
 *    T const* rhs,
 *    folly::StringPiece path,
 *    folly::StringPiece message
 *  ) const;
 *
 *  lhs: the left-hand side mismatched field or nullptr if the field is not
 *     present in the lhs object
 *  rhs: the right-hand side mismatched field or nullptr if the field is not
 *     present in the rhs object
 *  path: the path in the DFS where the mismatch happened
 *  message: a message explaining the mismatch
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */
template <typename TC, typename T, typename Callback>
[[deprecated(
    "Use facebook::thrift::debug_thrift_data_difference supporting adapters and not requiring old reflection")]] bool
debug_equals(
    T const& lhs, T const& rhs, Callback&& callback, std::string path = "$") {
  return apache::thrift::detail::debug_equals<TC>(path, lhs, rhs, callback);
}
template <typename T, typename Callback>
[[deprecated(
    "Use facebook::thrift::debug_thrift_data_difference supporting adapters and not requiring old reflection")]] bool
debug_equals(
    T const& lhs, T const& rhs, Callback&& callback, std::string path = "$") {
  using TC = type_class_of_thrift_class_enum_t<T>;
  return apache::thrift::detail::debug_equals<TC>(path, lhs, rhs, callback);
}

/**
 * A handy callback for `debug_equals()` that outputs to a given stream.
 *
 * See `make_debug_output_callback` for a convenient way to create an instance
 * of this callback.
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */
template <typename Output>
struct debug_output_callback {
  explicit debug_output_callback(
      Output& out, folly::StringPiece lhs, folly::StringPiece rhs)
      : out_(out), lhs_(lhs), rhs_(rhs) {}

  template <typename TC, typename T>
  void operator()(
      TC,
      T const* lhs,
      T const* rhs,
      folly::StringPiece path,
      folly::StringPiece message) const {
    out_ << path << ": " << message;
    if (lhs) {
      out_ << "\n"
           << "  " << lhs_ << ":\n";
      detail::pretty_print<TC>(out_, *lhs, "  ", "    ");
    }
    if (rhs) {
      out_ << "\n"
           << "  " << rhs_ << ":\n";
      detail::pretty_print<TC>(out_, *rhs, "  ", "    ");
    }
    out_ << "\n";
  }

 private:
  Output& out_;
  folly::StringPiece lhs_;
  folly::StringPiece rhs_;
};

/**
 * A convenient way to create an instance of `debug_output_callback`.
 *
 * Example:
 *
 *  bool const equals = debug_equals(
 *    lhs,
 *    rhs,
 *    make_debug_output_callback(std::cout)
 *  );
 *
 *  EXPECT_TRUE(
 *    debug_equals(
 *      lhs,
 *      rhs,
 *      make_debug_output_callback(LOG(ERROR))
 *    )
 *  );
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */

template <typename Output>
debug_output_callback<Output> make_debug_output_callback(
    Output& output,
    folly::StringPiece lhs = "lhs",
    folly::StringPiece rhs = "rhs") {
  return debug_output_callback<Output>(output, lhs, rhs);
}

} // namespace thrift
} // namespace apache

#include <thrift/lib/cpp2/reflection/internal/debug-inl-post.h>

#endif // THRIFT_FATAL_DEBUG_H_
