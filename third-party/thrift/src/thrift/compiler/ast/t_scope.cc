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

#include <stdio.h>

#include <iterator>
#include <sstream>
#include <vector>

#include <thrift/compiler/ast/t_const.h>
#include <thrift/compiler/ast/t_enum_value.h>
#include <thrift/compiler/ast/t_program.h>
#include <thrift/compiler/ast/t_scope.h>

namespace apache {
namespace thrift {
namespace compiler {
namespace {

std::string join_strings_by_commas(
    const std::unordered_set<std::string>& strs) {
  std::ostringstream stream;
  std::copy(
      strs.begin(),
      strs.end(),
      std::ostream_iterator<std::string>(stream, ", "));
  std::string joined_str = stream.str();
  // Remove the last comma.
  return joined_str.empty() ? joined_str
                            : joined_str.substr(0, joined_str.size() - 2);
}

std::vector<std::string> split_string_by_periods(const std::string& str) {
  std::istringstream iss(str);
  std::vector<std::string> tokens;
  std::string token;
  while (std::getline(iss, token, '.')) {
    if (!token.empty()) {
      tokens.push_back(token);
    }
  }
  return tokens;
}

} // namespace

t_type_ref t_scope::ref_type(
    const t_program& program,
    const std::string& name,
    const source_range& range) {
  std::string scopedName = name.find_first_of('.') != std::string::npos
      ? name
      : program.scope_name(name);
  // Try to resolve the type.
  const t_type* type = find_type(scopedName);
  // TODO(afuller): Why are interactions special? They should just be another
  // declared type.
  if (type == nullptr) {
    type = find_interaction(scopedName);
  }
  if (type != nullptr) {
    return *type; // We found the type!
  }

  /*
   Either this type isn't yet declared, or it's never
   declared. Either way allow it and we'll figure it out
   during generation.
  */
  // NOTE(afuller): This assumes that, since the type was referenced by name, it
  // is safe to create a dummy typedef to use as a proxy for the original type.
  // However, this actually breaks dynamic casts.
  // TODO(afuller): Merge t_placeholder_typedef into t_type_ref and remove const
  // cast.
  auto ph = std::make_unique<t_placeholder_typedef>(
      const_cast<t_program*>(&program), scopedName);
  ph->set_src_range(range);
  return add_placeholder_typedef(std::move(ph));
}

const t_named* t_scope::add_def(const t_named& node) {
  if (!node.uri().empty()) {
    auto result = defs_.emplace(node.uri(), &node);
    if (!result.second) {
      return result.first->second;
    }
  }
  // TODO(afuller): Also index the defs by name, and scoped name.
  return nullptr; // Success.
}

void t_scope::add_constant(std::string name, const t_const* constant) {
  if (constant && constant->get_value()->is_enum()) {
    const std::string& enum_value_name =
        constant->get_value()->get_enum_value()->get_name();
    std::vector<std::string> name_split = split_string_by_periods(name);
    if (enum_value_name.compare("UNKNOWN") &&
        (constants_.find(name) != constants_.end()) && constants_[name]) {
      redefined_enum_values_.insert(name);
    }
    if (name_split.size() == 3) {
      auto name_with_enum = name_split[1] + '.' + name_split[2];
      enum_values_[enum_value_name].insert(name_with_enum);
    }
  }
  constants_[std::move(name)] = constant;
}

std::string t_scope::get_fully_qualified_enum_value_names(
    const std::string& name) {
  // Get just the enum value name from name, which is
  // PROGRAM_NAME.ENUM_VALUE_NAME.
  auto name_split = split_string_by_periods(name);
  if (name_split.empty()) {
    return "";
  }
  return join_strings_by_commas(
      enum_values_[name_split[name_split.size() - 1]]);
}

void t_scope::dump() const {
  for (const auto& item : types_) {
    printf("%s => %s\n", item.first.c_str(), item.second->get_name().c_str());
  }
}

} // namespace compiler
} // namespace thrift
} // namespace apache
