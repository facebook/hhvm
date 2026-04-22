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

#include <thrift/lib/cpp2/dynamic/SyntaxGraphServiceSchema.h>

namespace apache::thrift::dynamic {

using apache::thrift::syntax_graph::SyntaxGraph;
using apache::thrift::type_system::TypeSystem;

namespace {

DynamicServiceSchema::Exception makeException(
    const syntax_graph::FunctionException& ex, const SyntaxGraph& syntaxGraph) {
  return DynamicServiceSchema::Exception{
      .name = std::string(ex.name()),
      .id = ex.id(),
      .type = type_system::TypeRef(
          syntaxGraph
              .asTypeSystemDefinitionRef(ex.type().asException().definition())
              .asStruct()),
  };
}

DynamicServiceSchema::Function makeFunction(
    const syntax_graph::FunctionNode& fn, const SyntaxGraph& syntaxGraph) {
  DynamicServiceSchema::Function function;
  function.name = std::string(fn.name());

  for (const auto& param : fn.params()) {
    function.params.push_back(
        DynamicServiceSchema::Param{
            .name = std::string(param.name()),
            .id = FieldId{static_cast<int16_t>(param.id())},
            .type = syntaxGraph.asTypeSystemTypeRef(param.type()),
        });
  }

  const auto* responseType = fn.response().type();
  if (responseType != nullptr) {
    function.responseType = syntaxGraph.asTypeSystemTypeRef(*responseType);
  }

  for (const auto& ex : fn.exceptions()) {
    function.exceptions.push_back(makeException(ex, syntaxGraph));
  }

  if (const auto* stream = fn.response().stream()) {
    DynamicServiceSchema::Stream s{
        .payloadType = syntaxGraph.asTypeSystemTypeRef(stream->payloadType()),
        .exceptions = {},
    };
    for (const auto& ex : stream->exceptions()) {
      s.exceptions.push_back(makeException(ex, syntaxGraph));
    }
    function.stream = std::move(s);
  } else if (const auto* sink = fn.response().sink()) {
    DynamicServiceSchema::Sink sk{
        .payloadType = syntaxGraph.asTypeSystemTypeRef(sink->payloadType()),
        .finalResponseType =
            syntaxGraph.asTypeSystemTypeRef(sink->finalResponseType()),
        .clientExceptions = {},
        .serverExceptions = {},
    };
    for (const auto& ex : sink->clientExceptions()) {
      sk.clientExceptions.push_back(makeException(ex, syntaxGraph));
    }
    for (const auto& ex : sink->serverExceptions()) {
      sk.serverExceptions.push_back(makeException(ex, syntaxGraph));
    }
    function.sink = std::move(sk);
  }

  return function;
}

void collectFunctions(
    const syntax_graph::ServiceNode& service,
    const SyntaxGraph& syntaxGraph,
    std::vector<DynamicServiceSchema::Function>& functions) {
  if (const auto* base = service.baseService()) {
    collectFunctions(*base, syntaxGraph, functions);
  }
  for (const auto& fn : service.functions()) {
    functions.push_back(makeFunction(fn, syntaxGraph));
  }
}

} // namespace

SyntaxGraphServiceSchema::SyntaxGraphServiceSchema(
    std::shared_ptr<const SyntaxGraph> syntaxGraph,
    const syntax_graph::ServiceNode& service)
    : syntaxGraph_(std::move(syntaxGraph)) {
  serviceName_ = std::string(service.definition().name());
  collectFunctions(service, *syntaxGraph_, functions_);
}

std::string_view SyntaxGraphServiceSchema::serviceName() const {
  return serviceName_;
}

folly::span<const DynamicServiceSchema::Function>
SyntaxGraphServiceSchema::functions() const {
  return functions_;
}

std::shared_ptr<const TypeSystem> SyntaxGraphServiceSchema::getTypeSystem()
    const {
  return std::shared_ptr<const TypeSystem>(
      syntaxGraph_, &syntaxGraph_->asTypeSystem());
}

} // namespace apache::thrift::dynamic
