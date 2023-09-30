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
#include <utility>

#include <thrift/compiler/ast/t_throws.h>
#include <thrift/compiler/ast/t_type.h>

namespace apache {
namespace thrift {
namespace compiler {

class t_stream_response : public t_type {
 public:
  explicit t_stream_response(t_type_ref elem_type)
      : elem_type_(std::move(elem_type)) {}

  const t_type_ref& elem_type() const { return elem_type_; }

  // Returns nullptr when the throws clause is absent.
  t_throws* exceptions() { return exceptions_.get(); }
  const t_throws* exceptions() const { return exceptions_.get(); }
  // Use nullptr to indicate an absent throws clause.
  void set_exceptions(std::unique_ptr<t_throws> exceptions) {
    exceptions_ = std::move(exceptions);
  }

  void set_first_response_type(t_type_ref first_response_type) {
    first_response_type_ = std::move(first_response_type);
  }

  const t_type_ref& first_response_type() const { return first_response_type_; }

  std::string get_full_name() const override {
    std::string result = "stream<" + elem_type_->get_full_name() + ">";
    if (!first_response_type_.empty()) {
      result = first_response_type_->get_full_name() + ", " + result;
    }
    return result;
  }

 private:
  t_type_ref elem_type_;
  std::unique_ptr<t_throws> exceptions_;
  t_type_ref first_response_type_;

  // TODO(afuller): Remove everything below here. It is provided only for
  // backwards compatibility.
 public:
  explicit t_stream_response(
      const t_type* elem_type, std::unique_ptr<t_throws> throws = nullptr)
      : t_stream_response(t_type_ref::from_req_ptr(elem_type)) {
    set_exceptions(std::move(throws));
  }

  void set_first_response_type(const t_type* first_response_type) {
    set_first_response_type(t_type_ref::from_ptr(first_response_type));
  }
  const t_type* get_elem_type() const { return elem_type().get_type(); }
  const t_type* get_first_response_type() const {
    return first_response_type().get_type();
  }
  bool has_first_response() const { return !first_response_type_.empty(); }

  bool is_streamresponse() const override { return true; }
  type get_type_value() const override { return type::t_stream; }
};

} // namespace compiler
} // namespace thrift
} // namespace apache
