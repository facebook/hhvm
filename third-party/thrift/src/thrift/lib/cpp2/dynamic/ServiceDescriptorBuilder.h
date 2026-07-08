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

#include <thrift/lib/cpp2/dynamic/ServiceDescriptor.h>

namespace apache::thrift::dynamic {

class ServiceDescriptorBuilder {
 public:
  class FunctionBuilder {
   public:
    FunctionBuilder& addParam(
        std::string name,
        FieldId id,
        type_system::TypeRef type,
        std::vector<DynamicValue> annotations = {});
    FunctionBuilder& setResponseType(type_system::TypeRef type);
    FunctionBuilder& addException(
        std::string name,
        FieldId id,
        type_system::TypeRef type,
        std::vector<DynamicValue> annotations = {});
    FunctionBuilder& setStream(type_system::TypeRef payloadType);
    FunctionBuilder& setSink(
        type_system::TypeRef payloadType,
        type_system::TypeRef finalResponseType);
    FunctionBuilder& setBidirectionalStream(
        type_system::TypeRef streamPayloadType,
        type_system::TypeRef sinkPayloadType);
    FunctionBuilder& setOneWay();
    FunctionBuilder& setQualifier(FunctionQualifier qualifier);
    FunctionBuilder& setCreatedInteractionUri(std::string interactionUri);
    FunctionBuilder& setIsPerforms(bool performs);
    FunctionBuilder& addAnnotation(DynamicValue value);
    FunctionBuilder& setDocBlock(std::string doc);

   private:
    friend class ServiceDescriptorBuilder;
    explicit FunctionBuilder(std::string name);

    std::string name_;
    std::vector<ServiceDescriptor::Param> params_;
    std::optional<type_system::TypeRef> responseType_;
    std::vector<ServiceDescriptor::Exception> exceptions_;
    std::optional<ServiceDescriptor::Stream> stream_;
    std::optional<ServiceDescriptor::Sink> sink_;
    FunctionQualifier qualifier_ = FunctionQualifier::Unspecified;
    RpcKind rpcKind_ = RpcKind::Unary;
    std::optional<std::string> createdInteractionUri_;
    bool isPerforms_ = false;
    std::vector<DynamicValue> annotations_;
    std::optional<std::string> docBlock_;
  };

  class InteractionBuilder {
   public:
    FunctionBuilder& addFunction(std::string name);
    InteractionBuilder& addAnnotation(DynamicValue value);

   private:
    friend class ServiceDescriptorBuilder;
    InteractionBuilder(std::string name, std::string uri);

    std::string name_;
    std::string uri_;
    std::vector<FunctionBuilder> functionBuilders_;
    std::vector<DynamicValue> annotations_;
  };

  ServiceDescriptorBuilder(
      std::shared_ptr<const type_system::TypeSystem> typeSystem,
      std::string serviceName,
      std::string serviceUri = {});

  FunctionBuilder& addFunction(std::string name);
  InteractionBuilder& addInteraction(std::string name, std::string uri);
  ServiceDescriptorBuilder& addServiceAnnotation(DynamicValue value);

  std::unique_ptr<ServiceDescriptor> build();

 private:
  static std::string makeFunctionUri(
      std::string_view interfaceUri, std::string_view name);
  static ServiceDescriptor::Function buildFunction(
      FunctionBuilder& fb, std::string_view interfaceUri);

  std::shared_ptr<const type_system::TypeSystem> typeSystem_;
  std::string serviceName_;
  std::string serviceUri_;
  std::vector<FunctionBuilder> functionBuilders_;
  std::vector<InteractionBuilder> interactionBuilders_;
  std::vector<DynamicValue> serviceAnnotations_;
};

} // namespace apache::thrift::dynamic
