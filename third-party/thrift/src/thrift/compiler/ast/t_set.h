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

#include <thrift/compiler/ast/t_container.h>

namespace apache::thrift::compiler {

/**
 * A set is a lightweight container type that just wraps another data type.
 *
 */
class t_set final : public t_container {
 public:
  explicit t_set(t_type_ref elem_type) : elem_type_(elem_type) {}

  const t_type_ref& elem_type() const { return elem_type_; }
  t_type_ref& elem_type() { return elem_type_; }

  std::string get_full_name() const override {
    return "set<" + elem_type_->get_full_name() + ">";
  }

  bool is_sealed() const override;

  ~t_set() override;

 private:
  t_type_ref elem_type_;
};

} // namespace apache::thrift::compiler
