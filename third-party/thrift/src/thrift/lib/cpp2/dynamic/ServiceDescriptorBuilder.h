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

#include <thrift/lib/cpp2/dynamic/ServiceDescriptor.h>

namespace apache::thrift::dynamic {

class ServiceDescriptorBuilder {
 public:
  class FunctionBuilder {
   public:
    FunctionBuilder& addParam(
        std::string name, FieldId id, type_system::TypeRef type);
    FunctionBuilder& setResponseType(type_system::TypeRef type);
    FunctionBuilder& addException(
        std::string name, FieldId id, type_system::TypeRef type);
    FunctionBuilder& setStream(type_system::TypeRef payloadType);
    FunctionBuilder& setSink(
        type_system::TypeRef payloadType,
        type_system::TypeRef finalResponseType);
    FunctionBuilder& setBidirectionalStream(
        type_system::TypeRef streamPayloadType,
        type_system::TypeRef sinkPayloadType);

   private:
    friend class ServiceDescriptorBuilder;
    explicit FunctionBuilder(std::string name);

    std::string name_;
    std::vector<ServiceDescriptor::Param> params_;
    std::optional<type_system::TypeRef> responseType_;
    std::vector<ServiceDescriptor::Exception> exceptions_;
    std::optional<ServiceDescriptor::Stream> stream_;
    std::optional<ServiceDescriptor::Sink> sink_;
  };

  ServiceDescriptorBuilder(
      std::shared_ptr<const type_system::TypeSystem> typeSystem,
      std::string serviceName);

  FunctionBuilder& addFunction(std::string name);

  std::unique_ptr<ServiceDescriptor> build();

 private:
  std::shared_ptr<const type_system::TypeSystem> typeSystem_;
  std::string serviceName_;
  std::vector<FunctionBuilder> functionBuilders_;
};

} // namespace apache::thrift::dynamic
