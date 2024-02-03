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

const std::string typedef_rust_name(const t_typedef* t);
const std::string struct_rust_name(const t_structured* strct);

} // namespace rust
} // namespace compiler
} // namespace thrift
} // namespace apache
