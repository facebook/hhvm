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

#include <thrift/lib/cpp2/dynamic/DynamicServiceSchemaBuilder.h>

namespace apache::thrift::dynamic {

namespace {

class BuiltServiceSchema final : public DynamicServiceSchema {
 public:
  BuiltServiceSchema(
      std::shared_ptr<const type_system::TypeSystem> typeSystem,
      std::string serviceName,
      std::vector<Function> functions)
      : typeSystem_(std::move(typeSystem)),
        serviceName_(std::move(serviceName)),
        functions_(std::move(functions)) {}

  std::string_view serviceName() const override { return serviceName_; }
  folly::span<const Function> functions() const override { return functions_; }

  std::shared_ptr<const type_system::TypeSystem> getTypeSystem()
      const override {
    return typeSystem_;
  }

 private:
  std::shared_ptr<const type_system::TypeSystem> typeSystem_;
  std::string serviceName_;
  std::vector<Function> functions_;
};

} // namespace

DynamicServiceSchemaBuilder::FunctionBuilder::FunctionBuilder(std::string name)
    : name_(std::move(name)) {}

DynamicServiceSchemaBuilder::FunctionBuilder&
DynamicServiceSchemaBuilder::FunctionBuilder::addParam(
    std::string name, FieldId id, type_system::TypeRef type) {
  params_.push_back(DynamicServiceSchema::Param{std::move(name), id, type});
  return *this;
}

DynamicServiceSchemaBuilder::FunctionBuilder&
DynamicServiceSchemaBuilder::FunctionBuilder::setResponseType(
    type_system::TypeRef type) {
  responseType_ = type;
  return *this;
}

DynamicServiceSchemaBuilder::FunctionBuilder&
DynamicServiceSchemaBuilder::FunctionBuilder::addException(
    std::string name, FieldId id, type_system::TypeRef type) {
  exceptions_.push_back(
      DynamicServiceSchema::Exception{std::move(name), id, type});
  return *this;
}

DynamicServiceSchemaBuilder::FunctionBuilder&
DynamicServiceSchemaBuilder::FunctionBuilder::setStream(
    type_system::TypeRef payloadType) {
  stream_ = DynamicServiceSchema::Stream{
      .payloadType = payloadType,
      .exceptions = {},
  };
  return *this;
}

DynamicServiceSchemaBuilder::FunctionBuilder&
DynamicServiceSchemaBuilder::FunctionBuilder::setSink(
    type_system::TypeRef payloadType, type_system::TypeRef finalResponseType) {
  sink_ = DynamicServiceSchema::Sink{
      .payloadType = payloadType,
      .finalResponseType = finalResponseType,
      .clientExceptions = {},
      .serverExceptions = {},
  };
  return *this;
}

DynamicServiceSchemaBuilder::DynamicServiceSchemaBuilder(
    std::shared_ptr<const type_system::TypeSystem> typeSystem,
    std::string serviceName)
    : typeSystem_(std::move(typeSystem)),
      serviceName_(std::move(serviceName)) {}

DynamicServiceSchemaBuilder::FunctionBuilder&
DynamicServiceSchemaBuilder::addFunction(std::string name) {
  functionBuilders_.emplace_back(FunctionBuilder(std::move(name)));
  return functionBuilders_.back();
}

std::unique_ptr<DynamicServiceSchema> DynamicServiceSchemaBuilder::build() {
  std::vector<DynamicServiceSchema::Function> functions;
  for (auto& fb : functionBuilders_) {
    DynamicServiceSchema::Function fn;
    fn.name = std::move(fb.name_);
    fn.params = std::move(fb.params_);
    fn.responseType = fb.responseType_;
    fn.exceptions = std::move(fb.exceptions_);
    fn.stream = std::move(fb.stream_);
    fn.sink = std::move(fb.sink_);
    functions.push_back(std::move(fn));
  }
  return std::make_unique<BuiltServiceSchema>(
      std::move(typeSystem_), std::move(serviceName_), std::move(functions));
}

} // namespace apache::thrift::dynamic
