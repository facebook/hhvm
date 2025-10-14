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

#include <thrift/compiler/ast/t_throws.h>
#include <thrift/compiler/ast/t_type.h>

namespace apache::thrift::compiler {

class t_stream : public t_node {
 public:
  explicit t_stream(t_type_ref elem_type) : elem_type_(elem_type) {}

  const t_type_ref& elem_type() const { return elem_type_; }
  t_type_ref& elem_type() { return elem_type_; }

  // Returns the exceptions declared in the throws clause or or null if there
  // is no throws clause.
  t_throws* exceptions() { return exceptions_.get(); }
  const t_throws* exceptions() const { return exceptions_.get(); }
  void set_exceptions(std::unique_ptr<t_throws> exceptions) {
    exceptions_ = std::move(exceptions);
  }

 private:
  t_type_ref elem_type_;
  std::unique_ptr<t_throws> exceptions_;
};

} // namespace apache::thrift::compiler
