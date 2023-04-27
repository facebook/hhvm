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

#include <boost/optional.hpp>

#include <thrift/compiler/ast/t_throws.h>
#include <thrift/compiler/ast/t_type.h>

namespace apache {
namespace thrift {
namespace compiler {

/**
 * A sink contains the types for stream of responses and a final response.
 *
 * Exceptions throw during the stream or instead of the final response can also
 * be specified.
 */
class t_sink : public t_templated_type {
 public:
  explicit t_sink(t_type_ref sink_type, t_type_ref final_response_type)
      : sink_type_(std::move(sink_type)),
        final_response_type_(std::move(final_response_type)) {}

  const t_type_ref& sink_type() const { return sink_type_; }
  const t_type_ref& final_response_type() const { return final_response_type_; }

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

  void set_first_response_type(t_type_ref first_response) {
    first_response_type_ = std::move(first_response);
  }
  const t_type_ref& first_response_type() const { return first_response_type_; }

  std::string get_full_name() const override {
    std::string result = "sink<" + sink_type_->get_full_name() + ", " +
        final_response_type_->get_full_name() + ">";
    if (!first_response_type_.empty()) {
      result += ", " + first_response_type_->get_full_name();
    }
    return result;
  }

 private:
  t_type_ref sink_type_;
  std::unique_ptr<t_throws> sink_exceptions_;
  t_type_ref final_response_type_;
  std::unique_ptr<t_throws> final_response_exceptions_;
  t_type_ref first_response_type_;

 public:
  // TODO(afuller): Delete everything below here. It is only provided for
  // backwards compatibility.

  explicit t_sink(
      const t_type* sink_type,
      std::unique_ptr<t_throws> sink_exceptions,
      const t_type* final_response_type,
      std::unique_ptr<t_throws> final_response_exceptions)
      : t_sink(
            t_type_ref::from_req_ptr(sink_type),
            t_type_ref::from_req_ptr(final_response_type)) {
    set_sink_exceptions(std::move(sink_exceptions));
    set_final_response_exceptions(std::move(final_response_exceptions));
  }

  void set_first_response(const t_type* first_response) {
    set_first_response_type(t_type_ref::from_ptr(first_response));
  }

  bool sink_has_first_response() const { return !first_response_type_.empty(); }
  t_throws* get_final_response_xceptions() const {
    return final_response_exceptions_.get();
  }
  t_throws* get_sink_xceptions() const { return sink_exceptions_.get(); }
  const t_type* get_sink_type() const { return sink_type().get_type(); }
  const t_type* get_first_response_type() const {
    return first_response_type_.get_type();
  }
  const t_type* get_final_response_type() const {
    return final_response_type().get_type();
  }

  bool is_sink() const override { return true; }
  type get_type_value() const override { return type::t_sink; }
};

} // namespace compiler
} // namespace thrift
} // namespace apache
