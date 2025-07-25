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

#include <string>

#include <thrift/compiler/ast/t_interface.h>

namespace apache::thrift::compiler {

class t_program;

/**
 * Represents a service definition.
 */
class t_service : public t_interface {
 public:
  t_service(
      const t_program* program,
      std::string name,
      const t_service* extends = nullptr)
      : t_interface(program, std::move(name)), extends_(extends) {}

  const t_service* extends() const { return extends_; }

  // The source range of the base service identifier if present.
  void set_extends_range(source_range rng) { extends_range_ = rng; }
  source_range extends_range() const { return extends_range_; }

 private:
  const t_service* extends_ = nullptr;
  source_range extends_range_;

  // TODO(T227540797): Remove everything below this comment. It is only provided
  // for backwards compatibility.
 public:
  type get_type_value() const override { return type::t_service; }
};

} // namespace apache::thrift::compiler
