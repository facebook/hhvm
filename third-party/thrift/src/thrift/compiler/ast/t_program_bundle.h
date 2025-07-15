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

#include <algorithm>
#include <map>
#include <memory>
#include <vector>

#include <thrift/compiler/ast/node_list.h>
#include <thrift/compiler/ast/t_program.h>

namespace apache::thrift::compiler {

/**
 * Used to encapsulate a root t_program along with all included dependencies.
 */
class t_program_bundle {
 public:
  explicit t_program_bundle(
      std::unique_ptr<t_program> root_program,
      t_program_bundle* already_parsed = nullptr) {
    if (already_parsed) {
      programs_by_full_path_ = already_parsed->programs_by_full_path_;
    }
    add_program(std::move(root_program));
  }

  const t_program* root_program() const { return programs_[0].get(); }
  t_program* root_program() { return programs_[0].get(); }

  node_list_view<t_program> programs() { return programs_; }
  node_list_view<const t_program> programs() const { return programs_; }
  void add_program(std::unique_ptr<t_program> program) {
    programs_by_full_path_[program->full_path()] = program.get();
    programs_.emplace_back(std::move(program));
  }

  void add_implicit_includes(std::unique_ptr<t_program_bundle> inc) {
    for (const auto& [path, prog] : inc->programs_by_full_path_) {
      programs_by_full_path_[path] = prog;
    }
    std::move(
        inc->programs_.begin(),
        inc->programs_.end(),
        std::back_inserter(implicit_includes_));
  }

  t_program* find_program_by_full_path(std::string_view full_path) {
    if (auto itr = programs_by_full_path_.find(full_path);
        itr != programs_by_full_path_.end()) {
      return itr->second;
    }
    return nullptr;
  }

 private:
  node_list<t_program> programs_;
  node_list<t_program> implicit_includes_;
  std::map<std::string, t_program*, std::less<>> programs_by_full_path_;

  // TODO(afuller): Delete everything below here. It is only provided for
  // backwards compatibility.
 public:
  t_program* get_root_program() const { return programs_[0].get(); }
};

} // namespace apache::thrift::compiler
