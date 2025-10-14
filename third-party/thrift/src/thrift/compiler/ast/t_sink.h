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
#include <utility>

#include <thrift/compiler/ast/t_throws.h>
#include <thrift/compiler/ast/t_type.h>

namespace apache::thrift::compiler {

/**
 * A sink contains the types for stream of responses and a final response.
 *
 * Exceptions throw during the stream or instead of the final response can also
 * be specified.
 */
class t_sink : public t_node {
 public:
  explicit t_sink(t_type_ref elem_type, t_type_ref final_response_type)
      : elem_type_(elem_type), final_response_type_(final_response_type) {}

  const t_type_ref& elem_type() const { return elem_type_; }
  const t_type_ref& final_response_type() const { return final_response_type_; }
  t_type_ref& elem_type() { return elem_type_; }
  t_type_ref& final_response_type() { return final_response_type_; }

  // Returns nullptr when throws clause is absent.
  t_throws* sink_exceptions() { return sink_exceptions_.get(); }
  const t_throws* sink_exceptions() const { return sink_exceptions_.get(); }
  // Use nullptr to indicate an absent throws clause.
  void set_sink_exceptions(std::unique_ptr<t_throws> sink_exceptions) {
    sink_exceptions_ = std::move(sink_exceptions);
  }

  // Returns nullptr when throws clause is absent.
  t_throws* final_response_exceptions() {
    return final_response_exceptions_.get();
  }
  const t_throws* final_response_exceptions() const {
    return final_response_exceptions_.get();
  }
  // Use nullptr to indicate an absent throws clause.
  void set_final_response_exceptions(
      std::unique_ptr<t_throws> final_response_exceptions) {
    final_response_exceptions_ = std::move(final_response_exceptions);
  }

 private:
  t_type_ref elem_type_;
  std::unique_ptr<t_throws> sink_exceptions_;
  t_type_ref final_response_type_;
  std::unique_ptr<t_throws> final_response_exceptions_;

 public:
  // TODO(T227540797): Delete everything below here. It is only provided for
  // backwards compatibility.
  const t_type* get_elem_type() const { return elem_type().get_type(); }
  const t_type* get_final_response_type() const {
    return final_response_type().get_type();
  }
};

} // namespace apache::thrift::compiler
