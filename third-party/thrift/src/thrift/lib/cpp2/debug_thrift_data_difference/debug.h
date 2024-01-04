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

#include <string>

#include <folly/Range.h>

#include <thrift/lib/cpp2/debug_thrift_data_difference/detail/debug-inl-pre.h>

#include <thrift/lib/cpp2/debug_thrift_data_difference/pretty_print.h>

#include <thrift/lib/cpp2/type/NativeType.h>

namespace facebook {
namespace thrift {

/**
 * Compares two objects for difference of underlying Thrift data,
 * field-by-fields, in a DFS traversal. Returns a boolean that tells whether
 * they're different in that sense or not. Please follow template
 * specialistation of adapted types to find more information on how those are
 * handled. Legacy reflection is not required.
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
template <typename Tag, typename T, typename Callback>
bool debug_thrift_data_difference(
    T const& lhs, T const& rhs, Callback&& callback, std::string path = "$") {
  return detail::debug_thrift_data_difference<Tag>(
      path, lhs, rhs, std::forward<Callback>(callback));
}
template <typename T, typename Callback>
bool debug_thrift_data_difference(
    T const& lhs, T const& rhs, Callback&& callback, std::string path = "$") {
  using Tag = apache::thrift::type::infer_tag<T>;
  return detail::debug_thrift_data_difference<Tag>(
      path, lhs, rhs, std::forward<Callback>(callback));
}

/**
 * A handy callback for `debug_thrift_data_difference()` that outputs to a given
 * stream.
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

  template <typename Tag, typename T>
  void operator()(
      Tag,
      T const* lhs,
      T const* rhs,
      folly::StringPiece path,
      folly::StringPiece message) const {
    out_ << path << ": " << message;
    if (lhs) {
      out_ << "\n"
           << "  " << lhs_ << ":\n";
      facebook::thrift::pretty_print<Tag>(out_, *lhs, "  ", "    ");
    }
    if (rhs) {
      out_ << "\n"
           << "  " << rhs_ << ":\n";
      facebook::thrift::pretty_print<Tag>(out_, *rhs, "  ", "    ");
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
 *  bool const differs = debug_thrift_data_difference(
 *    lhs,
 *    rhs,
 *    make_debug_output_callback(std::cout)
 *  );
 *
 *  EXPECT_TRUE(
 *    debug_thrift_data_difference(
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
} // namespace facebook

#include <thrift/lib/cpp2/debug_thrift_data_difference/detail/debug-inl-post.h>
