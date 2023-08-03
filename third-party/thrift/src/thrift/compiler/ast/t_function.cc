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

#include <thrift/compiler/ast/t_function.h>

#include <stdexcept>

#include <thrift/compiler/ast/t_sink.h>
#include <thrift/compiler/ast/t_stream.h>

namespace apache {
namespace thrift {
namespace compiler {

t_function::t_function(
    t_type_ref return_type,
    std::string name,
    std::unique_ptr<t_paramlist> paramlist,
    t_function_qualifier qualifier)
    : t_named(nullptr, std::move(name)),
      paramlist_(std::move(paramlist)),
      qualifier_(qualifier) {
  assert(paramlist_);
  set_return_type(return_type);
}

const t_throws* t_function::get_stream_xceptions() const {
  if (const auto* stream =
          dynamic_cast<const t_stream_response*>(sink_or_stream_.get())) {
    return t_throws::or_empty(stream->exceptions());
  }
  return t_throws::no_exceptions();
}
const t_throws* t_function::get_sink_xceptions() const {
  if (const auto* sink = dynamic_cast<const t_sink*>(sink_or_stream_.get())) {
    return t_throws::or_empty(sink->sink_exceptions());
  }
  return t_throws::no_exceptions();
}
const t_throws* t_function::get_sink_final_response_xceptions() const {
  if (const auto* sink = dynamic_cast<const t_sink*>(sink_or_stream_.get())) {
    return t_throws::or_empty(sink->final_response_exceptions());
  }
  return t_throws::no_exceptions();
}

} // namespace compiler
} // namespace thrift
} // namespace apache
