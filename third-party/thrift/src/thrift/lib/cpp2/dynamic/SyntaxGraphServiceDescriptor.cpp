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

#include <thrift/lib/cpp2/dynamic/SyntaxGraphServiceDescriptor.h>

#include <thrift/lib/cpp2/dynamic/AnnotationValue.h>
#include <thrift/lib/cpp2/dynamic/DynamicValue.h>

namespace apache::thrift::dynamic {

using apache::thrift::syntax_graph::SyntaxGraph;
using apache::thrift::type_system::TypeSystem;

namespace {

FunctionQualifier convertQualifier(type::FunctionQualifier q) {
  switch (q) {
    case type::FunctionQualifier::Idempotent:
      return FunctionQualifier::Idempotent;
    case type::FunctionQualifier::ReadOnly:
      return FunctionQualifier::ReadOnly;
    // Oneway is captured as an RpcKind, not a qualifier.
    case type::FunctionQualifier::OneWay:
    case type::FunctionQualifier::Unspecified:
      return FunctionQualifier::Unspecified;
  }
  return FunctionQualifier::Unspecified;
}

RpcKind determineRpcKind(const syntax_graph::FunctionNode& fn) {
  if (fn.qualifier() == type::FunctionQualifier::OneWay) {
    return RpcKind::OneWay;
  }
  const auto& response = fn.response();
  if (response.bidirectionalStream() != nullptr) {
    return RpcKind::BidirectionalStream;
  }
  if (response.stream() != nullptr) {
    return RpcKind::Stream;
  }
  if (response.sink() != nullptr) {
    return RpcKind::Sink;
  }
  return RpcKind::Unary;
}

ServiceDescriptor::Exception makeException(
    const syntax_graph::FunctionException& ex, const SyntaxGraph& syntaxGraph) {
  return ServiceDescriptor::Exception{
      .name = std::string(ex.name()),
      .id = ex.id(),
      .type = type_system::TypeRef(
          syntaxGraph
              .asTypeSystemDefinitionRef(ex.type().asException().definition())
              .asStruct()),
  };
}

ServiceDescriptor::Function makeFunction(
    const syntax_graph::FunctionNode& fn, const SyntaxGraph& syntaxGraph) {
  ServiceDescriptor::Function function;
  function.name = std::string(fn.name());

  for (const auto& param : fn.params()) {
    function.params.push_back(
        ServiceDescriptor::Param{
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

  if (const auto* bidi = fn.response().bidirectionalStream()) {
    ServiceDescriptor::Stream s{
        .payloadType =
            syntaxGraph.asTypeSystemTypeRef(bidi->streamPayloadType()),
        .exceptions = {},
    };
    for (const auto& ex : bidi->streamExceptions()) {
      s.exceptions.push_back(makeException(ex, syntaxGraph));
    }
    function.stream = std::move(s);
    ServiceDescriptor::Sink sk{
        .payloadType = syntaxGraph.asTypeSystemTypeRef(bidi->sinkPayloadType()),
        .finalResponseType = std::nullopt,
        .clientExceptions = {},
        .serverExceptions = {},
    };
    for (const auto& ex : bidi->sinkExceptions()) {
      sk.clientExceptions.push_back(makeException(ex, syntaxGraph));
    }
    function.sink = std::move(sk);
  } else if (const auto* stream = fn.response().stream()) {
    ServiceDescriptor::Stream s{
        .payloadType = syntaxGraph.asTypeSystemTypeRef(stream->payloadType()),
        .exceptions = {},
    };
    for (const auto& ex : stream->exceptions()) {
      s.exceptions.push_back(makeException(ex, syntaxGraph));
    }
    function.stream = std::move(s);
  } else if (const auto* sink = fn.response().sink()) {
    ServiceDescriptor::Sink sk{
        .payloadType = syntaxGraph.asTypeSystemTypeRef(sink->payloadType()),
        .finalResponseType = std::make_optional(
            syntaxGraph.asTypeSystemTypeRef(sink->finalResponseType())),
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

  function.qualifier = convertQualifier(fn.qualifier());
  function.rpcKind = determineRpcKind(fn);
  function.createsInteraction = fn.response().interaction() != nullptr;
  function.isPerforms = fn.isPerforms();

  if (auto doc = fn.docBlock()) {
    function.docBlock = std::string(*doc);
  }

  for (const auto& ann : fn.annotations()) {
    function.annotations.push_back(toDynamicValue(
        ann.value(), syntaxGraph.asTypeSystemTypeRef(ann.type())));
  }

  return function;
}

void collectFunctions(
    const syntax_graph::ServiceNode& service,
    const SyntaxGraph& syntaxGraph,
    std::vector<ServiceDescriptor::Function>& functions) {
  if (const auto* base = service.baseService()) {
    collectFunctions(*base, syntaxGraph, functions);
  }
  for (const auto& fn : service.functions()) {
    functions.push_back(makeFunction(fn, syntaxGraph));
  }
}

} // namespace

SyntaxGraphServiceDescriptor::SyntaxGraphServiceDescriptor(
    std::shared_ptr<const SyntaxGraph> syntaxGraph,
    const syntax_graph::ServiceNode& service)
    : syntaxGraph_(std::move(syntaxGraph)) {
  serviceName_ = std::string(service.definition().name());
  collectFunctions(service, *syntaxGraph_, functions_);
}

std::string_view SyntaxGraphServiceDescriptor::serviceName() const {
  return serviceName_;
}

folly::span<const ServiceDescriptor::Function>
SyntaxGraphServiceDescriptor::functions() const {
  return functions_;
}

std::shared_ptr<const TypeSystem> SyntaxGraphServiceDescriptor::getTypeSystem()
    const {
  return std::shared_ptr<const TypeSystem>(
      syntaxGraph_, &syntaxGraph_->asTypeSystem());
}

} // namespace apache::thrift::dynamic
