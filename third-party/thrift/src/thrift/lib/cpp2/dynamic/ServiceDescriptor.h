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

#include <optional>
#include <string>
#include <string_view>
#include <vector>

#include <folly/container/span.h>
#include <thrift/lib/cpp2/dynamic/DynamicValue.h>

namespace apache::thrift::dynamic {

enum class FunctionQualifier {
  Unspecified,
  Idempotent,
  ReadOnly,
};

enum class RpcKind {
  Unary,
  OneWay,
  Stream,
  Sink,
  BidirectionalStream,
};

class ServiceDescriptor {
 public:
  struct Param {
    std::string name;
    FieldId id;
    type_system::TypeRef type;
  };

  struct Exception {
    std::string name;
    FieldId id;
    type_system::TypeRef type;
  };

  struct Stream {
    type_system::TypeRef payloadType;
    std::vector<Exception> exceptions;
  };

  struct Sink {
    type_system::TypeRef payloadType;
    std::optional<type_system::TypeRef> finalResponseType;
    std::vector<Exception> clientExceptions;
    std::vector<Exception> serverExceptions;
  };

  struct Function {
    std::string name;
    std::vector<Param> params;
    std::optional<type_system::TypeRef> responseType;
    std::vector<Exception> exceptions;
    // Both stream and sink are set for bidirectional-streaming methods.
    std::optional<Stream> stream;
    std::optional<Sink> sink;
    FunctionQualifier qualifier = FunctionQualifier::Unspecified;
    RpcKind rpcKind = RpcKind::Unary;
    bool createsInteraction = false;
    bool isPerforms = false;
    // Structured annotations as schema-typed values; each value's type() is the
    // annotation's struct type.
    std::vector<DynamicValue> annotations;
    std::optional<std::string> docBlock;
  };

  virtual ~ServiceDescriptor() = default;

  virtual std::string_view serviceName() const = 0;
  virtual folly::span<const Function> functions() const = 0;
  virtual std::shared_ptr<const type_system::TypeSystem> getTypeSystem()
      const = 0;

  const Function& getFunction(std::string_view name) const;
};

} // namespace apache::thrift::dynamic
