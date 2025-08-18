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

#include <thrift/compiler/ast/t_named.h>
#include <thrift/compiler/ast/t_paramlist.h>
#include <thrift/compiler/ast/t_sink.h>
#include <thrift/compiler/ast/t_stream.h>
#include <thrift/compiler/ast/t_throws.h>
#include <thrift/compiler/ast/t_type.h>

namespace apache::thrift::compiler {

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
      const t_program* program,
      t_type_ref return_type,
      std::string name,
      std::unique_ptr<t_paramlist> params = {},
      std::unique_ptr<t_sink> sink = {},
      std::unique_ptr<t_stream> stream = {},
      t_type_ref interaction = {});
  ~t_function() override;

  // Returns the function qualifier.
  t_function_qualifier qualifier() const { return qualifier_; }
  void set_qualifier(t_function_qualifier qualifier) { qualifier_ = qualifier; }

  // Returns an interaction created by this function or null if there is none.
  // It is represented as a type for legacy reasons.
  const t_type_ref& interaction() const { return interaction_; }
  t_type_ref& interaction() { return interaction_; }

  // Returns the function's return type. The return type can be omitted if
  // there is a stream or interaction in the return clause of a function
  // definition. In this case return_type() returns the void type for
  // convenience.
  const t_type_ref& return_type() const { return return_type_; }
  t_type_ref& return_type() { return return_type_; }

  // Returns true if the function returns void, is oneway, or returns a
  // stream/sink/interaction with no initial response type specified.
  bool has_void_initial_response() const { return return_type_->is_void(); }

  t_node* sink_or_stream() {
    return sink_ ? static_cast<t_node*>(sink_.get())
                 : static_cast<t_node*>(stream_.get());
  }
  const t_node* sink_or_stream() const {
    return sink_ ? static_cast<const t_node*>(sink_.get())
                 : static_cast<const t_node*>(stream_.get());
  }

  const t_sink* sink() const { return sink_.get(); }
  const t_stream* stream() const { return stream_.get(); }

  t_paramlist& params() { return *params_; }
  const t_paramlist& params() const { return *params_; }

  // Returns the exceptions declared in the throws clause or or null if there
  // is no throws clause.
  t_throws* exceptions() { return exceptions_.get(); }
  const t_throws* exceptions() const { return exceptions_.get(); }
  void set_exceptions(std::unique_ptr<t_throws> exceptions) {
    exceptions_ = std::move(exceptions);
  }

  // old syntax only
  bool is_interaction_constructor() const {
    return is_interaction_constructor_;
  }
  void set_is_interaction_constructor() { is_interaction_constructor_ = true; }

 private:
  t_type_ref return_type_;
  std::unique_ptr<t_paramlist> params_;
  std::unique_ptr<t_throws> exceptions_;
  t_function_qualifier qualifier_ = t_function_qualifier::none;
  std::unique_ptr<t_sink> sink_;
  std::unique_ptr<t_stream> stream_;
  t_type_ref interaction_;
  bool is_interaction_constructor_ = false;
};

} // namespace apache::thrift::compiler
