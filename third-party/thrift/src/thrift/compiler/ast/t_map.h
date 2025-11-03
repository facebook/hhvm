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
 * A map is a lightweight container type that just wraps another two data
 * types.
 */
class t_map final : public t_container {
 public:
  t_map(t_type_ref key_type, t_type_ref val_type)
      : key_type_(key_type), val_type_(val_type) {}

  const t_type_ref& key_type() const { return key_type_; }
  const t_type_ref& val_type() const { return val_type_; }
  t_type_ref& key_type() { return key_type_; }
  t_type_ref& val_type() { return val_type_; }

  std::string get_full_name() const override {
    return "map<" + key_type_->get_full_name() + ", " +
        val_type_->get_full_name() + ">";
  }

  ~t_map() override;

 private:
  t_type_ref key_type_;
  t_type_ref val_type_;
};

} // namespace apache::thrift::compiler
