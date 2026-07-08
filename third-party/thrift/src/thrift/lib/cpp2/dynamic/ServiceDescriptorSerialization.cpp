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

#include <thrift/lib/cpp2/dynamic/ServiceDescriptorSerialization.h>

#include <thrift/lib/cpp2/dynamic/AnnotationValue.h>
#include <thrift/lib/cpp2/dynamic/SerializableTypeSystemBuilder.h>
#include <thrift/lib/cpp2/dynamic/TypeSystemBuilder.h>
#include <thrift/lib/cpp2/dynamic/TypeSystemDigest.h>

namespace apache::thrift::dynamic {

using type_system::FieldIdentity;
using type_system::SerializableTypeSystemBuilder;
using type_system::TypeSystem;

namespace {

class DeserializedServiceDescriptor final : public ServiceDescriptor {
 public:
  DeserializedServiceDescriptor(
      std::shared_ptr<const TypeSystem> typeSystem,
      std::string serviceName,
      std::vector<Function> functions,
      std::vector<Interaction> interactions,
      std::vector<DynamicValue> annotations)
      : typeSystem_(std::move(typeSystem)),
        serviceName_(std::move(serviceName)),
        functions_(std::move(functions)),
        interactions_(std::move(interactions)),
        annotations_(std::move(annotations)) {}

  std::string_view serviceName() const override { return serviceName_; }
  folly::span<const Function> functions() const override { return functions_; }
  folly::span<const Interaction> interactions() const override {
    return interactions_;
  }

  folly::span<const DynamicValue> annotations() const override {
    return annotations_;
  }

 private:
  const TypeSystem& typeSystem() const override { return *typeSystem_; }

  std::shared_ptr<const TypeSystem> typeSystem_;
  std::string serviceName_;
  std::vector<Function> functions_;
  std::vector<Interaction> interactions_;
  std::vector<DynamicValue> annotations_;
};

void addTypeRefDefinitions(
    type_system::TypeRef type, SerializableTypeSystemBuilder& builder) {
  switch (type.kind()) {
    case type_system::TypeRef::Kind::STRUCT:
      builder.addDefinition(type.asStruct().uri());
      break;
    case type_system::TypeRef::Kind::UNION:
      builder.addDefinition(type.asUnion().uri());
      break;
    case type_system::TypeRef::Kind::ENUM:
      builder.addDefinition(type.asEnum().uri());
      break;
    case type_system::TypeRef::Kind::OPAQUE_ALIAS:
      builder.addDefinition(type.asOpaqueAlias().uri());
      break;
    case type_system::TypeRef::Kind::LIST:
      addTypeRefDefinitions(type.asList().elementType(), builder);
      break;
    case type_system::TypeRef::Kind::SET:
      addTypeRefDefinitions(type.asSet().elementType(), builder);
      break;
    case type_system::TypeRef::Kind::MAP:
      addTypeRefDefinitions(type.asMap().keyType(), builder);
      addTypeRefDefinitions(type.asMap().valueType(), builder);
      break;
    // Primitives and Any carry no nested type definitions to register.
    case type_system::TypeRef::Kind::BOOL:
    case type_system::TypeRef::Kind::BYTE:
    case type_system::TypeRef::Kind::I16:
    case type_system::TypeRef::Kind::I32:
    case type_system::TypeRef::Kind::I64:
    case type_system::TypeRef::Kind::FLOAT:
    case type_system::TypeRef::Kind::DOUBLE:
    case type_system::TypeRef::Kind::STRING:
    case type_system::TypeRef::Kind::BINARY:
    case type_system::TypeRef::Kind::ANY:
      break;
  }
}

// Register the annotation struct types so the serialized universe is
// self-contained: deserializeAnnotations resolves each annotation's URI against
// it. Mirrors serializeAnnotations by skipping facebook.com/thrift/annotation/
// definitions, which are intentionally not bundled.
void addAnnotationTypeDefinitions(
    folly::span<const DynamicValue> annotations,
    SerializableTypeSystemBuilder& builder) {
  for (const auto& annotation : annotations) {
    const auto& uri = annotation.type().asStruct().uri();
    if (uri.starts_with("facebook.com/thrift/annotation/")) {
      continue;
    }
    builder.addDefinition(uri);
  }
}

void addFunctionTypeDefinitions(
    const ServiceDescriptor::Function& fn,
    SerializableTypeSystemBuilder& builder) {
  addAnnotationTypeDefinitions(fn.annotations, builder);
  for (const auto& param : fn.params) {
    addTypeRefDefinitions(param.type, builder);
    addAnnotationTypeDefinitions(param.annotations, builder);
  }
  if (fn.responseType) {
    addTypeRefDefinitions(*fn.responseType, builder);
  }
  for (const auto& ex : fn.exceptions) {
    addTypeRefDefinitions(ex.type, builder);
    addAnnotationTypeDefinitions(ex.annotations, builder);
  }
  if (fn.stream) {
    addTypeRefDefinitions(fn.stream->payloadType, builder);
    for (const auto& ex : fn.stream->exceptions) {
      addTypeRefDefinitions(ex.type, builder);
      addAnnotationTypeDefinitions(ex.annotations, builder);
    }
  }
  if (fn.sink) {
    addTypeRefDefinitions(fn.sink->payloadType, builder);
    if (fn.sink->finalResponseType) {
      addTypeRefDefinitions(*fn.sink->finalResponseType, builder);
    }
    for (const auto& ex : fn.sink->clientExceptions) {
      addTypeRefDefinitions(ex.type, builder);
      addAnnotationTypeDefinitions(ex.annotations, builder);
    }
    for (const auto& ex : fn.sink->serverExceptions) {
      addTypeRefDefinitions(ex.type, builder);
      addAnnotationTypeDefinitions(ex.annotations, builder);
    }
  }
}

type_system::FunctionQualifier toSerializableQualifier(FunctionQualifier q) {
  switch (q) {
    case FunctionQualifier::Unspecified:
      return type_system::FunctionQualifier::Unspecified;
    case FunctionQualifier::Idempotent:
      return type_system::FunctionQualifier::Idempotent;
    case FunctionQualifier::ReadOnly:
      return type_system::FunctionQualifier::ReadOnly;
  }
  return type_system::FunctionQualifier::Unspecified;
}

FunctionQualifier fromSerializableQualifier(type_system::FunctionQualifier q) {
  switch (q) {
    case type_system::FunctionQualifier::Idempotent:
      return FunctionQualifier::Idempotent;
    case type_system::FunctionQualifier::ReadOnly:
      return FunctionQualifier::ReadOnly;
    case type_system::FunctionQualifier::Unspecified:
      return FunctionQualifier::Unspecified;
  }
  return FunctionQualifier::Unspecified;
}

type_system::RpcKind toSerializableRpcKind(RpcKind kind) {
  switch (kind) {
    case RpcKind::Unary:
      return type_system::RpcKind::Unary;
    case RpcKind::OneWay:
      return type_system::RpcKind::OneWay;
    case RpcKind::Stream:
      return type_system::RpcKind::Stream;
    case RpcKind::Sink:
      return type_system::RpcKind::Sink;
    case RpcKind::BidirectionalStream:
      return type_system::RpcKind::BidirectionalStream;
  }
  return type_system::RpcKind::Unary;
}

RpcKind fromSerializableRpcKind(type_system::RpcKind kind) {
  switch (kind) {
    case type_system::RpcKind::Unary:
      return RpcKind::Unary;
    case type_system::RpcKind::OneWay:
      return RpcKind::OneWay;
    case type_system::RpcKind::Stream:
      return RpcKind::Stream;
    case type_system::RpcKind::Sink:
      return RpcKind::Sink;
    case type_system::RpcKind::BidirectionalStream:
      return RpcKind::BidirectionalStream;
  }
  return RpcKind::Unary;
}

std::string makeFunctionUri(
    std::string_view interfaceUri, std::string_view name) {
  if (interfaceUri.empty()) {
    return std::string(name);
  }
  std::string result(interfaceUri);
  result.push_back('/');
  result.append(name);
  return result;
}

std::string toDigestBytes(const type_system::TypeSystemDigest& digest) {
  return std::string(
      reinterpret_cast<const char*>(digest.data()), digest.size());
}

type_system::SerializableParameter toSerializableParam(
    const ServiceDescriptor::Param& param) {
  type_system::SerializableParameter result;
  result.identity() = FieldIdentity{param.id, std::string(param.name)};
  result.type() = param.type.id();
  result.annotations() = serializeAnnotations(param.annotations);
  return result;
}

type_system::SerializableException toSerializableException(
    const ServiceDescriptor::Exception& ex) {
  type_system::SerializableException result;
  result.identity() = FieldIdentity{ex.id, std::string(ex.name)};
  result.type() = ex.type.id();
  result.annotations() = serializeAnnotations(ex.annotations);
  return result;
}

type_system::SerializableFunction toSerializableFunction(
    const ServiceDescriptor::Function& fn) {
  type_system::SerializableFunction result;
  result.name() = fn.name;
  result.qualifier() = toSerializableQualifier(fn.qualifier);
  result.rpcKind() = toSerializableRpcKind(fn.rpcKind);
  result.annotations() = serializeAnnotations(fn.annotations);

  for (const auto& param : fn.params) {
    result.params()->push_back(toSerializableParam(param));
  }

  type_system::SerializableFunctionResponse response;
  if (fn.responseType) {
    response.initialResponseType() = fn.responseType->id();
  }
  if (fn.createdInteractionUri.has_value()) {
    response.createsInteraction() = *fn.createdInteractionUri;
  }

  if (fn.stream && fn.sink) {
    type_system::SerializableBidirectionalStream bidi;
    bidi.streamPayloadType() = fn.stream->payloadType.id();
    bidi.sinkPayloadType() = fn.sink->payloadType.id();
    for (const auto& ex : fn.stream->exceptions) {
      bidi.streamExceptions()->push_back(toSerializableException(ex));
    }
    for (const auto& ex : fn.sink->clientExceptions) {
      bidi.sinkExceptions()->push_back(toSerializableException(ex));
    }
    type_system::SerializableStreamingResponse streaming;
    streaming.bidirectionalStream_ref() = std::move(bidi);
    response.streaming() = std::move(streaming);
  } else if (fn.stream) {
    type_system::SerializableStream stream;
    stream.payloadType() = fn.stream->payloadType.id();
    for (const auto& ex : fn.stream->exceptions) {
      stream.exceptions()->push_back(toSerializableException(ex));
    }
    type_system::SerializableStreamingResponse streaming;
    streaming.serverStream_ref() = std::move(stream);
    response.streaming() = std::move(streaming);
  } else if (fn.sink) {
    type_system::SerializableSink sink;
    sink.payloadType() = fn.sink->payloadType.id();
    if (fn.sink->finalResponseType) {
      sink.finalResponseType() = fn.sink->finalResponseType->id();
    }
    for (const auto& ex : fn.sink->clientExceptions) {
      sink.clientExceptions()->push_back(toSerializableException(ex));
    }
    for (const auto& ex : fn.sink->serverExceptions) {
      sink.serverExceptions()->push_back(toSerializableException(ex));
    }
    type_system::SerializableStreamingResponse streaming;
    streaming.clientSink_ref() = std::move(sink);
    response.streaming() = std::move(streaming);
  }

  result.response() = std::move(response);

  for (const auto& ex : fn.exceptions) {
    result.exceptions()->push_back(toSerializableException(ex));
  }

  return result;
}

ServiceDescriptor::Exception fromSerializableException(
    const type_system::SerializableException& ex,
    const TypeSystem& typeSystem) {
  return ServiceDescriptor::Exception{
      .name = std::string(ex.identity()->name()),
      .id = ex.identity()->id(),
      .type = typeSystem.resolveTypeId(*ex.type()),
      .annotations = deserializeAnnotations(*ex.annotations(), typeSystem),
  };
}

ServiceDescriptor::Function fromSerializableFunction(
    const type_system::SerializableFunction& serFn,
    std::string_view interfaceUri,
    const TypeSystem& typeSystem) {
  ServiceDescriptor::Function fn;
  fn.name = *serFn.name();
  fn.uri = makeFunctionUri(interfaceUri, fn.name);
  fn.qualifier = fromSerializableQualifier(*serFn.qualifier());
  fn.rpcKind = fromSerializableRpcKind(*serFn.rpcKind());
  fn.annotations = deserializeAnnotations(*serFn.annotations(), typeSystem);

  for (const auto& param : *serFn.params()) {
    fn.params.push_back(
        ServiceDescriptor::Param{
            .name = std::string(param.identity()->name()),
            .id = param.identity()->id(),
            .type = typeSystem.resolveTypeId(*param.type()),
            .annotations =
                deserializeAnnotations(*param.annotations(), typeSystem),
        });
  }

  const auto& response = *serFn.response();
  if (response.initialResponseType().has_value()) {
    fn.responseType = typeSystem.resolveTypeId(*response.initialResponseType());
  }

  for (const auto& ex : *serFn.exceptions()) {
    fn.exceptions.push_back(fromSerializableException(ex, typeSystem));
  }

  if (response.streaming().has_value()) {
    const auto& streaming = *response.streaming();
    if (streaming.getType() ==
        type_system::SerializableStreamingResponse::Type::bidirectionalStream) {
      const auto& bidi = *streaming.bidirectionalStream_ref();
      ServiceDescriptor::Stream s{
          .payloadType = typeSystem.resolveTypeId(*bidi.streamPayloadType()),
          .exceptions = {},
      };
      for (const auto& ex : *bidi.streamExceptions()) {
        s.exceptions.push_back(fromSerializableException(ex, typeSystem));
      }
      fn.stream = std::move(s);
      ServiceDescriptor::Sink sk{
          .payloadType = typeSystem.resolveTypeId(*bidi.sinkPayloadType()),
          .finalResponseType = std::nullopt,
          .clientExceptions = {},
          .serverExceptions = {},
      };
      for (const auto& ex : *bidi.sinkExceptions()) {
        sk.clientExceptions.push_back(
            fromSerializableException(ex, typeSystem));
      }
      fn.sink = std::move(sk);
    } else if (
        streaming.getType() ==
        type_system::SerializableStreamingResponse::Type::serverStream) {
      const auto& stream = *streaming.serverStream_ref();
      ServiceDescriptor::Stream s{
          .payloadType = typeSystem.resolveTypeId(*stream.payloadType()),
          .exceptions = {},
      };
      for (const auto& ex : *stream.exceptions()) {
        s.exceptions.push_back(fromSerializableException(ex, typeSystem));
      }
      fn.stream = std::move(s);
    } else if (
        streaming.getType() ==
        type_system::SerializableStreamingResponse::Type::clientSink) {
      const auto& sink = *streaming.clientSink_ref();
      ServiceDescriptor::Sink sk{
          .payloadType = typeSystem.resolveTypeId(*sink.payloadType()),
          .finalResponseType = std::nullopt,
          .clientExceptions = {},
          .serverExceptions = {},
      };
      if (sink.finalResponseType().has_value()) {
        sk.finalResponseType =
            typeSystem.resolveTypeId(*sink.finalResponseType());
      }
      for (const auto& ex : *sink.clientExceptions()) {
        sk.clientExceptions.push_back(
            fromSerializableException(ex, typeSystem));
      }
      for (const auto& ex : *sink.serverExceptions()) {
        sk.serverExceptions.push_back(
            fromSerializableException(ex, typeSystem));
      }
      fn.sink = std::move(sk);
    }
  }

  if (response.createsInteraction().has_value()) {
    fn.createdInteractionUri = std::string(*response.createsInteraction());
  }

  return fn;
}

std::string_view extractServiceName(type_system::UriView uri) {
  auto pos = uri.rfind('/');
  if (pos != std::string_view::npos) {
    return uri.substr(pos + 1);
  }
  return uri;
}

bool containsInteractionUri(
    folly::span<const ServiceDescriptor::Interaction> interactions,
    std::string_view uri) {
  for (const auto& interaction : interactions) {
    if (interaction.uri == uri) {
      return true;
    }
  }
  return false;
}

void validateCreatedInteractions(
    folly::span<const ServiceDescriptor::Function> functions,
    folly::span<const ServiceDescriptor::Interaction> interactions) {
  for (const auto& fn : functions) {
    if (fn.createdInteractionUri.has_value() &&
        !containsInteractionUri(interactions, *fn.createdInteractionUri)) {
      throw std::invalid_argument(
          "Function creates an interaction that is not in the catalog: " +
          fn.name);
    }
  }
}

type_system::SerializableInteractionDefinition toSerializableInteraction(
    const ServiceDescriptor::Interaction& interaction) {
  type_system::SerializableInteractionDefinition result;
  for (const auto& fn : interaction.functions) {
    result.functions()->push_back(toSerializableFunction(fn));
  }
  result.annotations() = serializeAnnotations(interaction.annotations);
  return result;
}

} // namespace

type_system::SerializableServiceCatalog toSerializable(
    const ServiceDescriptor& descriptor, type_system::UriView serviceUri) {
  const auto& typeSystem = descriptor.typeSystem();
  auto interactions = descriptor.interactions();
  validateCreatedInteractions(descriptor.functions(), interactions);
  for (const auto& interaction : interactions) {
    validateCreatedInteractions(interaction.functions, interactions);
  }

  auto builder = SerializableTypeSystemBuilder::withoutSourceInfo(typeSystem);
  for (const auto& fn : descriptor.functions()) {
    addFunctionTypeDefinitions(fn, builder);
  }
  for (const auto& interaction : interactions) {
    for (const auto& fn : interaction.functions) {
      addFunctionTypeDefinitions(fn, builder);
    }
    addAnnotationTypeDefinitions(interaction.annotations, builder);
  }
  addAnnotationTypeDefinitions(descriptor.annotations(), builder);

  type_system::SerializableServiceCatalog result;
  result.types() = std::move(*std::move(builder).build());
  result.typesDigest() =
      toDigestBytes(type_system::TypeSystemHasher{}(*result.types()));

  type_system::SerializableServiceDefinition serviceDef;
  for (const auto& fn : descriptor.functions()) {
    serviceDef.functions()->push_back(toSerializableFunction(fn));
  }
  serviceDef.annotations() = serializeAnnotations(descriptor.annotations());

  type_system::SerializableRpcInterfaceDefinition interfaceDef;
  interfaceDef.serviceDef_ref() = std::move(serviceDef);
  result.interfaces()[std::string(serviceUri)] = std::move(interfaceDef);

  for (const auto& interaction : interactions) {
    type_system::SerializableRpcInterfaceDefinition interactionInterfaceDef;
    interactionInterfaceDef.interactionDef_ref() =
        toSerializableInteraction(interaction);
    result.interfaces()[interaction.uri] = std::move(interactionInterfaceDef);
  }

  return result;
}

std::unique_ptr<ServiceDescriptor> fromSerializable(
    type_system::SerializableServiceCatalog serialized,
    type_system::UriView serviceUri) {
  if (!serialized.types_ref().has_value()) {
    throw std::invalid_argument(
        "SerializableServiceCatalog has no inline types to rebuild from");
  }
  auto typeSystem =
      type_system::fromSerializable(std::move(*serialized.types_ref()));
  if (typeSystem == nullptr) {
    throw std::invalid_argument(
        "SerializableServiceCatalog has an unresolvable TypeSystem");
  }

  auto it = serialized.interfaces()->find(std::string(serviceUri));
  if (it == serialized.interfaces()->end()) {
    throw std::invalid_argument(
        "Service URI not found: " + std::string(serviceUri));
  }

  const auto& interfaceDef = it->second;
  if (interfaceDef.getType() !=
      type_system::SerializableRpcInterfaceDefinition::Type::serviceDef) {
    throw std::invalid_argument(
        "URI does not point to a service: " + std::string(serviceUri));
  }

  const auto& serviceDef = *interfaceDef.serviceDef_ref();

  std::vector<ServiceDescriptor::Function> functions;
  for (const auto& serFn : *serviceDef.functions()) {
    functions.push_back(
        fromSerializableFunction(serFn, serviceUri, *typeSystem));
  }

  std::vector<ServiceDescriptor::Interaction> interactions;
  for (const auto& [uri, def] : *serialized.interfaces()) {
    if (def.getType() !=
        type_system::SerializableRpcInterfaceDefinition::Type::interactionDef) {
      continue;
    }
    const auto& interactionDef = *def.interactionDef_ref();
    ServiceDescriptor::Interaction interaction{
        .name = std::string(extractServiceName(uri)),
        .uri = uri,
        .functions = {},
        .annotations =
            deserializeAnnotations(*interactionDef.annotations(), *typeSystem),
    };
    for (const auto& serFn : *interactionDef.functions()) {
      interaction.functions.push_back(
          fromSerializableFunction(serFn, uri, *typeSystem));
    }
    interactions.push_back(std::move(interaction));
  }

  auto annotations =
      deserializeAnnotations(*serviceDef.annotations(), *typeSystem);

  return std::make_unique<DeserializedServiceDescriptor>(
      std::shared_ptr<const TypeSystem>(std::move(typeSystem)),
      std::string(extractServiceName(serviceUri)),
      std::move(functions),
      std::move(interactions),
      std::move(annotations));
}

} // namespace apache::thrift::dynamic
