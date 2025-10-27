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

#include <thrift/compiler/ast/t_type.h>

namespace apache::thrift::compiler {

class t_container : public t_type {
 public:
  /**
   * The subset of t_type::type values that are containers.
   */
  enum class type {
    t_list = int(t_type::type::t_list),
    t_set = int(t_type::type::t_set),
    t_map = int(t_type::type::t_map),
  };

  virtual type container_type() const = 0;

  ~t_container() override;

 protected:
  t_container() = default;

  // TODO(T227540797): Remove everything below here. It is provided only for
  // backwards compatibility.
 public:
  t_type::type get_type_value() const override {
    return static_cast<t_type::type>(container_type());
  }
};

} // namespace apache::thrift::compiler
