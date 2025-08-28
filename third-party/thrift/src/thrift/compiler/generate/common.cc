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

#include <thrift/compiler/generate/common.h>

#include <regex>
#include <stdexcept>
#include <string>
#include <vector>

#include <thrift/compiler/ast/t_list.h>
#include <thrift/compiler/ast/t_map.h>
#include <thrift/compiler/ast/t_set.h>
#include <thrift/compiler/ast/uri.h>

namespace apache::thrift::compiler {

std::vector<std::string> split_namespace(const std::string& s) {
  std::string token = ".";
  std::size_t last_match = 0;
  std::size_t next_match = s.find(token);

  std::vector<std::string> output;
  while (next_match != std::string::npos) {
    output.push_back(s.substr(last_match, next_match - last_match));
    last_match = next_match + 1;
    next_match = s.find(token, last_match);
  }
  if (!s.empty()) {
    output.push_back(s.substr(last_match));
  }

  return output;
}

namespace {

template <typename F>
void visit_type(const t_type* type, F&& visitor) {
  visitor(type);
  const auto* true_type = type->get_true_type();
  if (const auto* tmap = dynamic_cast<const t_map*>(true_type)) {
    visit_type(tmap->get_key_type(), std::forward<F>(visitor));
    visit_type(tmap->get_val_type(), std::forward<F>(visitor));
  } else if (const auto* tlist = dynamic_cast<const t_list*>(true_type)) {
    visit_type(tlist->get_elem_type(), std::forward<F>(visitor));
  } else if (const auto* tset = dynamic_cast<const t_set*>(true_type)) {
    visit_type(tset->get_elem_type(), std::forward<F>(visitor));
  }
}

} // namespace

std::unordered_set<const t_type*> collect_types(const t_structured* strct) {
  std::unordered_set<const t_type*> types;
  for (const auto& field : strct->fields()) {
    visit_type(&field.type().deref(), [&](const t_type* type) {
      types.emplace(type);
    });
  }
  return types;
}

std::string program_cache_id(const t_program* prog, std::string ns) {
  // DEVNOTE: Don't use the program name as part of the cache id, as
  // program names (i.e. the .thrift filename without the extension) are
  // not unique within a thrift compiler run. Instead, rely on the
  // `t_program` instance being unique.
  return std::to_string(reinterpret_cast<std::intptr_t>(prog)) + ns;
}

} // namespace apache::thrift::compiler
