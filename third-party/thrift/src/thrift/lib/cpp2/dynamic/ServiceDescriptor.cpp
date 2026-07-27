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

#include <thrift/lib/cpp2/dynamic/ServiceDescriptor.h>

#include <thrift/lib/cpp2/dynamic/ServiceCatalog.h>

#include <optional>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace apache::thrift::dynamic {

const ServiceDescriptor* FOLLY_NULLABLE
ServiceCatalog::getServiceByName(std::string_view name) const {
  for (const ServiceDescriptor* service : services()) {
    if (service != nullptr && service->serviceName() == name) {
      return service;
    }
  }
  return nullptr;
}

ServiceDescriptor::RpcStruct ServiceDescriptor::Function::requestEnvelope()
    const {
  RpcStruct envelope;
  envelope.fields.reserve(params.size());
  for (const auto& param : params) {
    envelope.fields.emplace_back(
        type_system::FieldIdentity{param.id, param.name},
        type_system::PresenceQualifier::UNQUALIFIED,
        param.type,
        std::nullopt,
        type_system::AnnotationsMap{});
  }
  return envelope;
}

ServiceDescriptor::RpcStruct ServiceDescriptor::Function::responseEnvelope()
    const {
  RpcStruct envelope;
  envelope.fields.reserve(exceptions.size() + (responseType ? 1 : 0));
  if (responseType.has_value()) {
    envelope.fields.emplace_back(
        type_system::FieldIdentity{FieldId{0}, "success"},
        type_system::PresenceQualifier::OPTIONAL_,
        *responseType,
        std::nullopt,
        type_system::AnnotationsMap{});
  }
  for (const auto& ex : exceptions) {
    envelope.fields.emplace_back(
        type_system::FieldIdentity{ex.id, ex.name},
        type_system::PresenceQualifier::OPTIONAL_,
        ex.type,
        std::nullopt,
        type_system::AnnotationsMap{});
  }
  return envelope;
}

const ServiceDescriptor::Function& ServiceDescriptor::getFunction(
    std::string_view uri) const {
  for (const auto& fn : functions()) {
    if (fn.uri == uri) {
      return fn;
    }
  }
  throw std::invalid_argument("Function not found: " + std::string(uri));
}

const ServiceDescriptor::Function& ServiceDescriptor::getFunctionByName(
    std::string_view name) const {
  for (const auto& fn : functions()) {
    if (fn.name == name) {
      return fn;
    }
  }
  throw std::invalid_argument("Function not found: " + std::string(name));
}

const ServiceDescriptor::Interaction& ServiceDescriptor::getInteraction(
    std::string_view uri) const {
  for (const auto& interaction : interactions()) {
    if (interaction.uri == uri) {
      return interaction;
    }
  }
  throw std::invalid_argument("Interaction not found: " + std::string(uri));
}

const ServiceDescriptor::Function& ServiceDescriptor::Interaction::getFunction(
    std::string_view functionUri) const {
  for (const auto& fn : functions) {
    if (fn.uri == functionUri) {
      return fn;
    }
  }
  throw std::invalid_argument(
      "Function not found: " + std::string(functionUri));
}

const ServiceDescriptor::Function&
ServiceDescriptor::Interaction::getFunctionByName(
    std::string_view functionName) const {
  for (const auto& fn : functions) {
    if (fn.name == functionName) {
      return fn;
    }
  }
  throw std::invalid_argument(
      "Function not found: " + std::string(functionName));
}

} // namespace apache::thrift::dynamic
