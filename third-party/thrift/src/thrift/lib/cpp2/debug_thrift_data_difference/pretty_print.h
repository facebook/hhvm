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

#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

#include <thrift/lib/cpp2/reflection/indenter.h>

#include <folly/Traits.h>

#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/debug_thrift_data_difference/detail/pretty_print-inl-pre.h>
#include <thrift/lib/cpp2/type/NativeType.h>

namespace facebook {
namespace thrift {

/**
 * Pretty-prints an object to the given output stream using Thrift's
 * always-on-reflection support. Adapters are supported without their
 * customisation.
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */
template <typename Tag, typename OutputStream, typename T>
void pretty_print(
    OutputStream&& out,
    T&& what,
    std::string indentation = "  ",
    std::string margin = std::string()) {
  using impl = detail::pretty_print_impl<Tag>;
  auto indenter = apache::thrift::make_indenter(
      std::forward<OutputStream>(out),
      std::move(indentation),
      std::move(margin));
  impl::print(indenter, std::forward<T>(what));
}
template <typename OutputStream, typename T>
void pretty_print(
    OutputStream&& out,
    T&& what,
    std::string indentation = "  ",
    std::string margin = std::string()) {
  using Tag = apache::thrift::type::infer_tag<T>;
  pretty_print<Tag>(
      std::forward<OutputStream>(out),
      std::forward<T>(what),
      std::move(indentation),
      std::move(margin));
}

/**
 * Pretty-prints an object to a string using Thrift's always-on-reflection
 * support. Adapters are supported without their customisation.
 *
 * @author: Marcelo Juchem <marcelo@fb.com>
 */
template <typename Tag, typename... Args>
std::string pretty_string(Args&&... args) {
  std::ostringstream out;
  pretty_print<Tag>(out, std::forward<Args>(args)...);
  return out.str();
}
template <typename... Args>
std::string pretty_string(Args&&... args) {
  std::ostringstream out;
  pretty_print(out, std::forward<Args>(args)...);
  return out.str();
}

} // namespace thrift
} // namespace facebook

#include <thrift/lib/cpp2/debug_thrift_data_difference/detail/pretty_print-inl-post.h>
