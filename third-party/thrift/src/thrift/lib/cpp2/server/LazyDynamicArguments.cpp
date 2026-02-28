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

#include <thrift/lib/cpp2/server/LazyDynamicArguments.h>

#include <glog/logging.h>

#include <fmt/core.h>

#include <folly/container/F14Map.h>

#include <thrift/lib/cpp2/dynamic/Serialization.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>
#include <thrift/lib/cpp2/runtime/SchemaRegistry.h>
#include <thrift/lib/cpp2/schema/SyntaxGraph.h>

namespace apache::thrift {

LazyDynamicArguments::LazyDynamicArguments(
    const folly::IOBuf* serializedRequestBuffer,
    const syntax_graph::FunctionNode* functionNode,
    protocol::PROTOCOL_TYPES protocolType)
    : serializedRequestBuffer_(serializedRequestBuffer),
      functionNode_(functionNode),
      protocolType_(protocolType) {}

std::size_t LazyDynamicArguments::count() const {
  return functionNode_->params().size();
}

std::string_view LazyDynamicArguments::name(std::size_t index) const {
  const auto& params = functionNode_->params();
  if (index >= params.size()) {
    throw std::out_of_range(
        fmt::format(
            "Argument index {} out of range, function has {} parameters",
            index,
            params.size()));
  }
  return params[index].name();
}

const LazyDynamicArguments::ArgsMap& LazyDynamicArguments::ensureDeserialized()
    const {
  return cachedArgs_.try_emplace_with([this]() {
    ArgsMap result;

    const auto& params = functionNode_->params();
    if (params.empty()) {
      return result;
    }

    auto deserializeWithProtocol = [&](auto& reader) {
      reader.setInput(serializedRequestBuffer_);

      std::string structName;
      reader.readStructBegin(structName);

      while (true) {
        std::string fieldName;
        protocol::TType fieldType;
        int16_t fieldId;
        reader.readFieldBegin(fieldName, fieldType, fieldId);

        if (fieldType == protocol::T_STOP) {
          break;
        }

        // Find the parameter with this field ID
        const syntax_graph::FunctionParam* matchedParam = nullptr;
        for (const auto& param : params) {
          if (param.id() == FieldId{fieldId}) {
            matchedParam = &param;
            break;
          }
        }

        if (matchedParam) {
          // Convert syntax_graph::TypeRef to type_system::TypeRef
          auto typeSystemTypeRef =
              SchemaRegistry::get().getTypeSystemTypeRef(matchedParam->type());

          // Deserialize the value
          auto value =
              dynamic::deserializeValue(reader, typeSystemTypeRef, nullptr);
          result.emplace(FieldId{fieldId}, std::move(value));
        } else {
          // Unknown field, skip it
          reader.skip(fieldType);
        }

        reader.readFieldEnd();
      }

      reader.readStructEnd();
    };

    switch (protocolType_) {
      case protocol::T_COMPACT_PROTOCOL: {
        CompactProtocolReader reader;
        deserializeWithProtocol(reader);
        break;
      }
      case protocol::T_BINARY_PROTOCOL: {
        BinaryProtocolReader reader;
        deserializeWithProtocol(reader);
        break;
      }
      default:
        // Unsupported protocol, leave result empty
        break;
    }

    // Fill in default values for any missing parameters
    for (const auto& param : params) {
      if (result.find(param.id()) == result.end()) {
        auto typeSystemTypeRef =
            SchemaRegistry::get().getTypeSystemTypeRef(param.type());
        result.emplace(
            param.id(), dynamic::DynamicValue::makeDefault(typeSystemTypeRef));
      }
    }

    return result;
  });
}

dynamic::DynamicConstRef LazyDynamicArguments::get(std::size_t index) const {
  const auto& params = functionNode_->params();
  if (index >= params.size()) {
    throw std::out_of_range(
        fmt::format(
            "Argument index {} out of range, function has {} parameters",
            index,
            params.size()));
  }

  const auto& argsMap = ensureDeserialized();

  // Get the field ID for this parameter index
  FieldId fieldId = params[index].id();
  auto it = argsMap.find(fieldId);
  // Parameter should always be present (either deserialized or default value)
  DCHECK(it != argsMap.end());
  return dynamic::DynamicConstRef(it->second);
}

dynamic::DynamicConstRef LazyDynamicArguments::get(
    std::string_view paramName) const {
  // Find parameter by name
  const auto& params = functionNode_->params();
  for (std::size_t i = 0; i < params.size(); ++i) {
    if (params[i].name() == paramName) {
      return get(i);
    }
  }

  throw std::out_of_range(
      fmt::format("Parameter '{}' not found in function", paramName));
}

} // namespace apache::thrift
