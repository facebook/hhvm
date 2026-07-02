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

#include <thrift/lib/cpp2/dynamic/ServiceDescriptorBuilder.h>

namespace apache::thrift::dynamic {

namespace {

class BuiltServiceDescriptor final : public ServiceDescriptor {
 public:
  BuiltServiceDescriptor(
      std::shared_ptr<const type_system::TypeSystem> typeSystem,
      std::string serviceName,
      std::vector<Function> functions,
      std::vector<DynamicValue> annotations)
      : typeSystem_(std::move(typeSystem)),
        serviceName_(std::move(serviceName)),
        functions_(std::move(functions)),
        annotations_(std::move(annotations)) {}

  std::string_view serviceName() const override { return serviceName_; }
  folly::span<const Function> functions() const override { return functions_; }

  std::shared_ptr<const type_system::TypeSystem> getTypeSystem()
      const override {
    return typeSystem_;
  }

  folly::span<const DynamicValue> annotations() const override {
    return annotations_;
  }

 private:
  std::shared_ptr<const type_system::TypeSystem> typeSystem_;
  std::string serviceName_;
  std::vector<Function> functions_;
  std::vector<DynamicValue> annotations_;
};

} // namespace

ServiceDescriptorBuilder::FunctionBuilder::FunctionBuilder(std::string name)
    : name_(std::move(name)) {}

ServiceDescriptorBuilder::FunctionBuilder&
ServiceDescriptorBuilder::FunctionBuilder::addParam(
    std::string name,
    FieldId id,
    type_system::TypeRef type,
    std::vector<DynamicValue> annotations) {
  params_.push_back(
      ServiceDescriptor::Param{
          std::move(name), id, type, std::move(annotations)});
  return *this;
}

ServiceDescriptorBuilder::FunctionBuilder&
ServiceDescriptorBuilder::FunctionBuilder::setResponseType(
    type_system::TypeRef type) {
  responseType_ = type;
  return *this;
}

ServiceDescriptorBuilder::FunctionBuilder&
ServiceDescriptorBuilder::FunctionBuilder::addException(
    std::string name,
    FieldId id,
    type_system::TypeRef type,
    std::vector<DynamicValue> annotations) {
  exceptions_.push_back(
      ServiceDescriptor::Exception{
          std::move(name), id, type, std::move(annotations)});
  return *this;
}

ServiceDescriptorBuilder::FunctionBuilder&
ServiceDescriptorBuilder::FunctionBuilder::setStream(
    type_system::TypeRef payloadType) {
  stream_ = ServiceDescriptor::Stream{
      .payloadType = payloadType,
      .exceptions = {},
  };
  rpcKind_ = RpcKind::Stream;
  return *this;
}

ServiceDescriptorBuilder::FunctionBuilder&
ServiceDescriptorBuilder::FunctionBuilder::setSink(
    type_system::TypeRef payloadType, type_system::TypeRef finalResponseType) {
  sink_ = ServiceDescriptor::Sink{
      .payloadType = payloadType,
      .finalResponseType = std::make_optional(finalResponseType),
      .clientExceptions = {},
      .serverExceptions = {},
  };
  rpcKind_ = RpcKind::Sink;
  return *this;
}

ServiceDescriptorBuilder::FunctionBuilder&
ServiceDescriptorBuilder::FunctionBuilder::setBidirectionalStream(
    type_system::TypeRef streamPayloadType,
    type_system::TypeRef sinkPayloadType) {
  stream_ = ServiceDescriptor::Stream{
      .payloadType = streamPayloadType,
      .exceptions = {},
  };
  sink_ = ServiceDescriptor::Sink{
      .payloadType = sinkPayloadType,
      .finalResponseType = std::nullopt,
      .clientExceptions = {},
      .serverExceptions = {},
  };
  rpcKind_ = RpcKind::BidirectionalStream;
  return *this;
}

ServiceDescriptorBuilder::FunctionBuilder&
ServiceDescriptorBuilder::FunctionBuilder::setOneWay() {
  rpcKind_ = RpcKind::OneWay;
  return *this;
}

ServiceDescriptorBuilder::FunctionBuilder&
ServiceDescriptorBuilder::FunctionBuilder::setQualifier(
    FunctionQualifier qualifier) {
  qualifier_ = qualifier;
  return *this;
}

ServiceDescriptorBuilder::FunctionBuilder&
ServiceDescriptorBuilder::FunctionBuilder::setCreatesInteraction(bool creates) {
  createsInteraction_ = creates;
  return *this;
}

ServiceDescriptorBuilder::FunctionBuilder&
ServiceDescriptorBuilder::FunctionBuilder::setIsPerforms(bool performs) {
  isPerforms_ = performs;
  return *this;
}

ServiceDescriptorBuilder::FunctionBuilder&
ServiceDescriptorBuilder::FunctionBuilder::addAnnotation(DynamicValue value) {
  annotations_.push_back(std::move(value));
  return *this;
}

ServiceDescriptorBuilder::FunctionBuilder&
ServiceDescriptorBuilder::FunctionBuilder::setDocBlock(std::string doc) {
  docBlock_ = std::move(doc);
  return *this;
}

ServiceDescriptorBuilder::ServiceDescriptorBuilder(
    std::shared_ptr<const type_system::TypeSystem> typeSystem,
    std::string serviceName)
    : typeSystem_(std::move(typeSystem)),
      serviceName_(std::move(serviceName)) {}

ServiceDescriptorBuilder::FunctionBuilder&
ServiceDescriptorBuilder::addFunction(std::string name) {
  functionBuilders_.emplace_back(FunctionBuilder(std::move(name)));
  return functionBuilders_.back();
}

ServiceDescriptorBuilder& ServiceDescriptorBuilder::addServiceAnnotation(
    DynamicValue value) {
  serviceAnnotations_.push_back(std::move(value));
  return *this;
}

std::unique_ptr<ServiceDescriptor> ServiceDescriptorBuilder::build() {
  std::vector<ServiceDescriptor::Function> functions;
  for (auto& fb : functionBuilders_) {
    ServiceDescriptor::Function fn;
    fn.name = std::move(fb.name_);
    fn.params = std::move(fb.params_);
    fn.responseType = fb.responseType_;
    fn.exceptions = std::move(fb.exceptions_);
    fn.stream = std::move(fb.stream_);
    fn.sink = std::move(fb.sink_);
    fn.qualifier = fb.qualifier_;
    fn.rpcKind = fb.rpcKind_;
    fn.createsInteraction = fb.createsInteraction_;
    fn.isPerforms = fb.isPerforms_;
    fn.annotations = std::move(fb.annotations_);
    fn.docBlock = std::move(fb.docBlock_);
    functions.push_back(std::move(fn));
  }
  return std::make_unique<BuiltServiceDescriptor>(
      std::move(typeSystem_),
      std::move(serviceName_),
      std::move(functions),
      std::move(serviceAnnotations_));
}

} // namespace apache::thrift::dynamic
