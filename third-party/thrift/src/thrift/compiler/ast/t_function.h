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

#include <thrift/compiler/ast/t_named.h>
#include <thrift/compiler/ast/t_paramlist.h>
#include <thrift/compiler/ast/t_sink.h>
#include <thrift/compiler/ast/t_stream.h>
#include <thrift/compiler/ast/t_throws.h>
#include <thrift/compiler/ast/t_type.h>

namespace apache {
namespace thrift {
namespace compiler {

class t_interaction;

enum class t_function_qualifier {
  none,
  oneway,
  idempotent,
  readonly,
};

/**
 * A Thrift function declaration.
 */
class t_function final : public t_named {
 public:
  t_function(
      t_program* program,
      t_type_ref return_type,
      std::string name,
      std::unique_ptr<t_paramlist> params = {},
      std::unique_ptr<t_node> sink_or_stream = {},
      t_type_ref interaction = {});

  const t_type* return_type() const;
  void set_return_type(t_type_ref ret) {
    response_pos_ = return_types_.size();
    return_types_.push_back(ret);
  }
  bool has_return_type() const { return response_pos_ != -1; }

  t_node* sink_or_stream() { return sink_or_stream_.get(); }
  const t_node* sink_or_stream() const { return sink_or_stream_.get(); }

  const t_sink* sink() const {
    return dynamic_cast<const t_sink*>(sink_or_stream_.get());
  }
  const t_stream_response* stream() const {
    return dynamic_cast<const t_stream_response*>(sink_or_stream_.get());
  }

  t_paramlist& params() { return *params_; }
  const t_paramlist& params() const { return *params_; }

  // Returns the function qualifier.
  t_function_qualifier qualifier() const { return qualifier_; }
  void set_qualifier(t_function_qualifier qualifier) { qualifier_ = qualifier; }

  // The declared exceptions that function might throw.
  //
  // Returns nullptr when the throws clause is absent.
  t_throws* exceptions() { return exceptions_.get(); }
  const t_throws* exceptions() const { return exceptions_.get(); }
  // Use nullptr to indicate an absent throws clause.
  void set_exceptions(std::unique_ptr<t_throws> exceptions) {
    exceptions_ = std::move(exceptions);
  }

  // old syntax only
  bool is_interaction_constructor() const {
    return is_interaction_constructor_;
  }
  void set_is_interaction_constructor() { is_interaction_constructor_ = true; }
  bool is_interaction_member() const { return is_interaction_member_; }
  void set_is_interaction_member() { is_interaction_member_ = true; }

  // Returns an interaction created by this function or null if there is none.
  const t_type_ref& interaction() const { return interaction_; }
  void set_response_pos(uint8_t pos) { response_pos_ = pos; }

  std::vector<t_type_ref>& return_types() { return return_types_; }
  const std::vector<t_type_ref>& return_types() const { return return_types_; }

 private:
  std::vector<t_type_ref> return_types_;
  std::unique_ptr<t_node> sink_or_stream_;
  t_type_ref interaction_;
  std::unique_ptr<t_paramlist> params_;
  std::unique_ptr<t_throws> exceptions_;
  t_function_qualifier qualifier_ = t_function_qualifier::none;
  int8_t response_pos_ = -1;
  bool is_interaction_constructor_ = false;
  bool is_interaction_member_ = false;
};

} // namespace compiler
} // namespace thrift
} // namespace apache
