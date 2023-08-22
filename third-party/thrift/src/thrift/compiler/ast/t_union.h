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

#include <thrift/compiler/ast/t_struct.h>

namespace apache {
namespace thrift {
namespace compiler {

/**
 * Represents a union definition.
 */
// TODO(afuller): Inherit from t_structured instead.
class t_union : public t_struct {
 public:
  using t_struct::t_struct;

  // TODO(afuller): Remove everything below this comment. It is only provided
  // for backwards compatibility.
 public:
  bool is_union() const override { return true; }

 private:
  friend class t_struct;
  t_union* clone_DO_NOT_USE() const override {
    auto clone = std::make_unique<t_union>(program_, name_);
    clone_structured(clone.get());
    return clone.release();
  }
};

} // namespace compiler
} // namespace thrift
} // namespace apache
