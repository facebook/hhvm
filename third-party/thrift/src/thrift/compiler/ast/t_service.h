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
#include <string>
#include <vector>

#include <thrift/compiler/ast/t_interface.h>

namespace apache {
namespace thrift {
namespace compiler {

class t_program;

/**
 * Represents a service definition.
 */
class t_service : public t_interface {
 public:
  explicit t_service(
      t_program* program, std::string name, const t_service* extends = nullptr)
      : t_interface(program, std::move(name)), extends_(extends) {}

  const t_service* extends() const { return extends_; }

 private:
  const t_service* extends_ = nullptr;

  // TODO(afuller): Remove everything below this comment. It is only provided
  // for backwards compatibility.
 public:
  const t_service* get_extends() const { return extends_; }
  void set_extends(const t_service* extends) { extends_ = extends; }
  type get_type_value() const override { return type::t_service; }
  bool is_service() const override { return true; }
};

} // namespace compiler
} // namespace thrift
} // namespace apache
