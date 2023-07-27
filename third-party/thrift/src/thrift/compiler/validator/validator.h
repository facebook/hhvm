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

#include <memory>
#include <utility>
#include <vector>

#include <thrift/compiler/ast/visitor.h>
#include <thrift/compiler/diagnostic.h>

namespace apache {
namespace thrift {
namespace compiler {

class t_program;

// NOTE: Use thrift/compiler/sema/ast_validator.h instead.
class validator : virtual public visitor {
 public:
  static void validate(t_program* program, diagnostics_engine& diags);

  using visitor::visit;

 protected:
  template <typename... T>
  void report_error(
      diagnostic_location loc, fmt::format_string<T...> msg, T&&... args) {
    diags_->error(loc, msg, std::forward<T>(args)...);
  }

 private:
  template <typename T, typename... Args>
  friend std::unique_ptr<T> make_validator(diagnostics_engine&, Args&&...);

  diagnostics_engine* diags_ = nullptr;
};

template <typename T, typename... Args>
std::unique_ptr<T> make_validator(diagnostics_engine& diags, Args&&... args) {
  auto ptr = std::unique_ptr<T>(new T(std::forward<Args>(args)...));
  ptr->diags_ = &diags;
  return ptr;
}

template <typename T, typename... Args>
void run_validator(
    diagnostics_engine& diags, t_program* program, Args&&... args) {
  make_validator<T>(diags, std::forward<Args>(args)...)->traverse(program);
}

class validator_list {
 public:
  explicit validator_list(diagnostics_engine& diags) : diags_(diags) {}

  template <typename T, typename... Args>
  void add(Args&&... args) {
    auto ptr = make_validator<T>(diags_, std::forward<Args>(args)...);
    validators_.push_back(std::move(ptr));
  }

  void traverse(t_program* program);

 private:
  diagnostics_engine& diags_;
  std::vector<std::unique_ptr<validator>> validators_;
};

} // namespace compiler
} // namespace thrift
} // namespace apache
