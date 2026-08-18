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

#include <map>
#include <string>
#include <utility>
#include <vector>

#include <thrift/lib/cpp/protocol/TType.h>
#include <thrift/lib/cpp2/protocol/Cpp2Ops.h>
#include <thrift/lib/cpp2/protocol/detail/protocol_methods.h>
#include <thrift/lib/thrift/SerializableDynamic.h>

namespace apache::thrift {

class Dynamic;

template <typename ThriftType>
struct BasicSerializableDynamicAdapter {
  static SerializableDynamic fromThrift(ThriftType value) {
    return SerializableDynamic(fromThriftValue(std::move(value)));
  }

  static folly::dynamic fromThriftValue(ThriftType value) {
    return detail::fromThriftDynamic(std::move(value));
  }

  static ThriftType toThrift(const SerializableDynamic& value) {
    return toThrift(*value);
  }

  static ThriftType toThrift(const folly::dynamic& value) {
    ThriftType result;
    switch (value.type()) {
      case folly::dynamic::Type::NULLT:
        break;
      case folly::dynamic::Type::BOOL:
        if constexpr (requires { result.set_boolean(value.asBool()); }) {
          result.set_boolean(value.asBool());
        } else {
          result.set_b(value.asBool());
        }
        break;
      case folly::dynamic::Type::INT64:
        if constexpr (requires { result.set_integer(value.asInt()); }) {
          result.set_integer(value.asInt());
        } else {
          result.set_i(value.asInt());
        }
        break;
      case folly::dynamic::Type::DOUBLE:
        if constexpr (requires { result.set_doubl(value.asDouble()); }) {
          result.set_doubl(value.asDouble());
        } else {
          result.set_d(value.asDouble());
        }
        break;
      case folly::dynamic::Type::STRING:
        if constexpr (requires { result.set_str(value.getString()); }) {
          result.set_str(value.getString());
        } else {
          result.set_s(value.getString());
        }
        break;
      case folly::dynamic::Type::ARRAY: {
        std::vector<ThriftType> items;
        items.reserve(value.size());
        for (const auto& item : value) {
          items.push_back(toThrift(item));
        }
        if constexpr (requires { result.set_arr(std::move(items)); }) {
          result.set_arr(std::move(items));
        } else {
          result.set_l(std::move(items));
        }
        break;
      }
      case folly::dynamic::Type::OBJECT: {
        std::map<std::string, ThriftType> items;
        for (const auto& item : value.items()) {
          items.emplace(item.first.asString(), toThrift(item.second));
        }
        if constexpr (requires { result.set_object(std::move(items)); }) {
          result.set_object(std::move(items));
        } else {
          result.set_o(std::move(items));
        }
        break;
      }
    }
    return result;
  }

  template <typename Protocol>
  static uint32_t write(Protocol& protocol, const SerializableDynamic& value) {
    uint32_t xfer = protocol.writeStructBegin("Dynamic");
    switch (value->type()) {
      case folly::dynamic::Type::NULLT:
        break;
      case folly::dynamic::Type::BOOL:
        xfer += protocol.writeFieldBegin("boolean", protocol::T_BOOL, 1);
        xfer += protocol.writeBool(value->asBool());
        xfer += protocol.writeFieldEnd();
        break;
      case folly::dynamic::Type::INT64:
        xfer += protocol.writeFieldBegin("integer", protocol::T_I64, 2);
        xfer += protocol.writeI64(value->asInt());
        xfer += protocol.writeFieldEnd();
        break;
      case folly::dynamic::Type::DOUBLE:
        xfer += protocol.writeFieldBegin("doubl", protocol::T_DOUBLE, 3);
        xfer += protocol.writeDouble(value->asDouble());
        xfer += protocol.writeFieldEnd();
        break;
      case folly::dynamic::Type::STRING:
        xfer += protocol.writeFieldBegin("str", protocol::T_STRING, 4);
        xfer += protocol.writeString(value->getString());
        xfer += protocol.writeFieldEnd();
        break;
      case folly::dynamic::Type::ARRAY:
        xfer += protocol.writeFieldBegin("arr", protocol::T_LIST, 5);
        xfer += protocol.writeListBegin(protocol::T_STRUCT, value->size());
        for (const auto& item : *value) {
          xfer += write(protocol, SerializableDynamic(item));
        }
        xfer += protocol.writeListEnd();
        xfer += protocol.writeFieldEnd();
        break;
      case folly::dynamic::Type::OBJECT:
        xfer += protocol.writeFieldBegin("object", protocol::T_MAP, 6);
        xfer += protocol.writeMapBegin(
            protocol::T_STRING, protocol::T_STRUCT, value->size());
        for (const auto& item : value->items()) {
          xfer += protocol.writeString(item.first.asString());
          xfer += write(protocol, SerializableDynamic(item.second));
        }
        xfer += protocol.writeMapEnd();
        xfer += protocol.writeFieldEnd();
        break;
    }
    xfer += protocol.writeFieldStop();
    xfer += protocol.writeStructEnd();
    return xfer;
  }

  template <typename Tag, typename Protocol>
  static uint32_t encode(Protocol& protocol, const SerializableDynamic& value) {
    return write(protocol, value);
  }

  template <bool ZeroCopy, typename Protocol>
  static uint32_t serializedSize(
      Protocol& protocol, const SerializableDynamic& value) {
    uint32_t xfer = protocol.serializedStructSize("Dynamic");
    switch (value->type()) {
      case folly::dynamic::Type::NULLT:
        break;
      case folly::dynamic::Type::BOOL:
        xfer += protocol.serializedFieldSize("boolean", protocol::T_BOOL, 1);
        xfer += protocol.serializedSizeBool(value->asBool());
        break;
      case folly::dynamic::Type::INT64:
        xfer += protocol.serializedFieldSize("integer", protocol::T_I64, 2);
        xfer += protocol.serializedSizeI64(value->asInt());
        break;
      case folly::dynamic::Type::DOUBLE:
        xfer += protocol.serializedFieldSize("doubl", protocol::T_DOUBLE, 3);
        xfer += protocol.serializedSizeDouble(value->asDouble());
        break;
      case folly::dynamic::Type::STRING:
        xfer += protocol.serializedFieldSize("str", protocol::T_STRING, 4);
        xfer += protocol.serializedSizeString(value->getString());
        break;
      case folly::dynamic::Type::ARRAY:
        xfer += protocol.serializedFieldSize("arr", protocol::T_LIST, 5);
        xfer +=
            protocol.serializedSizeListBegin(protocol::T_STRUCT, value->size());
        for (const auto& item : *value) {
          xfer += serializedSize<ZeroCopy>(protocol, SerializableDynamic(item));
        }
        xfer += protocol.serializedSizeListEnd();
        break;
      case folly::dynamic::Type::OBJECT:
        xfer += protocol.serializedFieldSize("object", protocol::T_MAP, 6);
        xfer += protocol.serializedSizeMapBegin(
            protocol::T_STRING, protocol::T_STRUCT, value->size());
        for (const auto& item : value->items()) {
          xfer += protocol.serializedSizeString(item.first.asString());
          xfer += serializedSize<ZeroCopy>(
              protocol, SerializableDynamic(item.second));
        }
        xfer += protocol.serializedSizeMapEnd();
        break;
    }
    xfer += protocol.serializedSizeStop();
    return xfer;
  }

  template <bool ZeroCopy, typename Tag, typename Protocol>
  static uint32_t serializedSize(
      Protocol& protocol, const SerializableDynamic& value) {
    return serializedSize<ZeroCopy>(protocol, value);
  }

  template <typename Protocol>
  static void read(Protocol& protocol, SerializableDynamic& value) {
    std::string name;
    protocol::TType type;
    int16_t id;
    protocol.readStructBegin(name);
    protocol.readFieldBegin(name, type, id);
    if (type == protocol::T_STOP) {
      value = nullptr;
    } else {
      if (type == protocol::T_VOID) {
        if (name == "boolean") {
          id = 1;
          type = protocol::T_BOOL;
        } else if (name == "integer") {
          id = 2;
          type = protocol::T_I64;
        } else if (name == "doubl") {
          id = 3;
          type = protocol::T_DOUBLE;
        } else if (name == "str") {
          id = 4;
          type = protocol::T_STRING;
        } else if (name == "arr") {
          id = 5;
          type = protocol::T_LIST;
        } else if (name == "object") {
          id = 6;
          type = protocol::T_MAP;
        }
      }
      switch (id) {
        case 1: {
          if (type == protocol::T_BOOL) {
            bool result;
            protocol.readBool(result);
            value = result;
          } else {
            protocol.skip(type);
          }
          break;
        }
        case 2: {
          if (type == protocol::T_I64) {
            int64_t result;
            protocol.readI64(result);
            value = result;
          } else {
            protocol.skip(type);
          }
          break;
        }
        case 3: {
          if (type == protocol::T_DOUBLE) {
            double result;
            protocol.readDouble(result);
            value = result;
          } else {
            protocol.skip(type);
          }
          break;
        }
        case 4: {
          if (type == protocol::T_STRING) {
            std::string result;
            protocol.readString(result);
            value = std::move(result);
          } else {
            protocol.skip(type);
          }
          break;
        }
        case 5: {
          if (type == protocol::T_LIST) {
            folly::dynamic result = folly::dynamic::array;
            protocol::TType elementType;
            uint32_t size;
            protocol.readListBegin(elementType, size);
            auto readItem = [&] {
              SerializableDynamic item;
              read(protocol, item);
              result.push_back(std::move(*item));
            };
            if (protocol.kOmitsContainerSizes()) {
              while (protocol.peekList()) {
                readItem();
              }
            } else {
              result.reserve(size);
              for (uint32_t i = 0; i < size; ++i) {
                readItem();
              }
            }
            protocol.readListEnd();
            value = std::move(result);
          } else {
            protocol.skip(type);
          }
          break;
        }
        case 6: {
          if (type == protocol::T_MAP) {
            folly::dynamic result = folly::dynamic::object;
            protocol::TType keyType;
            protocol::TType valueType;
            uint32_t size;
            protocol.readMapBegin(keyType, valueType, size);
            auto readItem = [&] {
              std::string key;
              SerializableDynamic item;
              protocol.readString(key);
              read(protocol, item);
              result[std::move(key)] = std::move(*item);
            };
            if (protocol.kOmitsContainerSizes()) {
              while (protocol.peekMap()) {
                readItem();
              }
            } else {
              for (uint32_t i = 0; i < size; ++i) {
                readItem();
              }
            }
            protocol.readMapEnd();
            value = std::move(result);
          } else {
            protocol.skip(type);
          }
          break;
        }
        default:
          protocol.skip(type);
          break;
      }
      protocol.readFieldEnd();
      protocol.readFieldBegin(name, type, id);
      protocol.readFieldEnd();
    }
    protocol.readStructEnd();
  }

  template <typename Tag, typename Protocol>
  static void decode(Protocol& protocol, SerializableDynamic& value) {
    read(protocol, value);
  }
};

using SerializableDynamicAdapter = BasicSerializableDynamicAdapter<Dynamic>;

} // namespace apache::thrift

namespace apache::thrift::detail::pm {

template <typename ExpectedTag>
struct protocol_methods<type_class::variant, SerializableDynamic, ExpectedTag> {
  template <typename Protocol>
  static void read(Protocol& protocol, SerializableDynamic& value) {
    SerializableDynamicAdapter::read(protocol, value);
  }

  template <typename Protocol>
  static std::size_t write(
      Protocol& protocol, const SerializableDynamic& value) {
    return SerializableDynamicAdapter::write(protocol, value);
  }

  template <bool ZeroCopy, typename Protocol>
  static std::size_t serializedSize(
      Protocol& protocol, const SerializableDynamic& value) {
    return SerializableDynamicAdapter::template serializedSize<ZeroCopy>(
        protocol, value);
  }
};

} // namespace apache::thrift::detail::pm

namespace apache::thrift {

template <>
class Cpp2Ops<SerializableDynamic> {
 public:
  static constexpr protocol::TType thriftType() { return protocol::T_STRUCT; }

  template <typename Protocol>
  static uint32_t write(Protocol* protocol, const SerializableDynamic* value) {
    return SerializableDynamicAdapter::write(*protocol, *value);
  }

  template <typename Protocol>
  static void read(Protocol* protocol, SerializableDynamic* value) {
    SerializableDynamicAdapter::read(*protocol, *value);
  }

  template <typename Protocol>
  static uint32_t serializedSize(
      Protocol* protocol, const SerializableDynamic* value) {
    return SerializableDynamicAdapter::template serializedSize<false>(
        *protocol, *value);
  }

  template <typename Protocol>
  static uint32_t serializedSizeZC(
      Protocol* protocol, const SerializableDynamic* value) {
    return SerializableDynamicAdapter::template serializedSize<true>(
        *protocol, *value);
  }
};

} // namespace apache::thrift
