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
#include <thrift/compiler/ast/t_type.h>

namespace apache {
namespace thrift {
namespace compiler {

/**
 * A list is a lightweight container type that just wraps another data type.
 */
class t_list final : public t_container {
 public:
  explicit t_list(t_type_ref elem_type) : elem_type_(std::move(elem_type)) {}

  const t_type_ref& elem_type() const { return elem_type_; }
  t_type_ref& elem_type() { return elem_type_; }

  type container_type() const override { return type::t_list; }
  std::string get_full_name() const override {
    return "list<" + elem_type_->get_full_name() + ">";
  }

 private:
  t_type_ref elem_type_;

  // TODO(afuller): Delete everything below here. It is only provided for
  // backwards compatibility.
 public:
  explicit t_list(const t_type* elem_type)
      : t_list(t_type_ref::from_req_ptr(elem_type)) {}
  const t_type* get_elem_type() const { return elem_type().get_type(); }
};

} // namespace compiler
} // namespace thrift
} // namespace apache
