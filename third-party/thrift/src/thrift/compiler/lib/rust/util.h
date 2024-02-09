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

#include <map>
#include <string>
#include <boost/optional.hpp>

#include <thrift/compiler/ast/t_struct.h>
#include <thrift/compiler/ast/t_typedef.h>

namespace apache {
namespace thrift {
namespace compiler {
namespace rust {

struct rust_crate {
  std::string name;
  boost::optional<std::string> multifile_module;
  std::string label;

  std::string import_name() const;
};

struct rust_crate_map {
  bool multifile_mode = false;
  std::map<std::string, rust_crate> cratemap;
};

rust_crate_map load_crate_map(const std::string& path);

std::string mangle(const std::string& name);
std::string mangle_type(const std::string& name);
std::string snakecase(const std::string& name);
std::string camelcase(const std::string& name);
std::string quote(const std::string& data, bool do_backslash);

constexpr auto kRustOrdUri = "facebook.com/thrift/annotation/rust/Ord";
constexpr auto kRustBoxUri = "facebook.com/thrift/annotation/rust/Box";
constexpr auto kRustTypeUri = "facebook.com/thrift/annotation/rust/Type";
constexpr auto kRustNewTypeUri = "facebook.com/thrift/annotation/rust/NewType";
constexpr auto kRustAdapterUri = "facebook.com/thrift/annotation/rust/Adapter";
constexpr auto kRustDeriveUri = "facebook.com/thrift/annotation/rust/Derive";
constexpr auto kRustServiceExnUri =
    "facebook.com/thrift/annotation/rust/ServiceExn";
constexpr auto kRustExhaustiveUri =
    "facebook.com/thrift/annotation/rust/Exhaustive";
constexpr auto kRustArcUri = "facebook.com/thrift/annotation/rust/Arc";
constexpr auto kRustRequestContextUri =
    "facebook.com/thrift/annotation/rust/RequestContext";
constexpr auto kRustCopyUri = "facebook.com/thrift/annotation/rust/Copy";
constexpr auto kRustNameUri = "facebook.com/thrift/annotation/rust/Name";

bool get_annotation_property_bool(
    const t_const* annotation, const std::string& key);
std::string get_annotation_property_string(
    const t_const* annotation, const std::string& key);

std::string named_rust_name(const t_named* name); // mangle()
std::string type_rust_name(const t_type* type_); // mangle_type()
inline std::string typedef_rust_name(const t_typedef* typedef_) {
  return type_rust_name(typedef_);
}
inline std::string struct_rust_name(const t_structured* struct_) {
  return type_rust_name(struct_);
}

} // namespace rust
} // namespace compiler
} // namespace thrift
} // namespace apache
