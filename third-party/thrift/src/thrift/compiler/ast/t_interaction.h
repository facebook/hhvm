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
#include <vector>

#include <thrift/compiler/ast/t_service.h>

namespace apache {
namespace thrift {
namespace compiler {

class t_program;

/**
 * An interaction is a service with context.
 */
// TODO(afuller): Inherit from t_interface directly.
class t_interaction : public t_service {
 public:
  using t_service::t_service;

  // TODO(afuller): Remove everything below this comment. It is only provided
  // for backwards compatibility.
 public:
  bool is_serial() const { return has_annotation("serial"); }
};

} // namespace compiler
} // namespace thrift
} // namespace apache
