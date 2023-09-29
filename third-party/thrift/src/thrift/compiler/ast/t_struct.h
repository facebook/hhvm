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
#include <cassert>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <thrift/compiler/ast/t_structured.h>
#include <thrift/compiler/ast/t_type.h>

namespace apache {
namespace thrift {
namespace compiler {

// Forward declare that puppy
class t_program;

/**
 * Represents a struct definition.
 */
class t_struct : public t_structured {
 public:
  t_struct(const t_program* program, std::string name)
      : t_structured(program, std::move(name)) {}

  // TODO(afuller): Remove everything below this comment. It is only provided
  // for backwards compatibility.
 public:
  bool is_struct() const override { return !is_exception(); }
  type get_type_value() const override { return type::t_structured; }

  using t_structured::clone_DO_NOT_USE;

 protected:
  friend class t_structured;
  virtual t_struct* clone_DO_NOT_USE() const {
    auto clone = std::make_unique<t_struct>(program_, name_);
    clone_structured(clone.get());
    return clone.release();
  }
};

} // namespace compiler
} // namespace thrift
} // namespace apache
