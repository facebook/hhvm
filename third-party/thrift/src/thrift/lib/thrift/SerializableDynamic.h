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

#ifndef THRIFT_UTIL_SERIALIZABLEDYNAMIC_H
#define THRIFT_UTIL_SERIALIZABLEDYNAMIC_H

#include <folly/dynamic.h>

#include <thrift/lib/cpp/protocol/TType.h>
#include <thrift/lib/cpp2/protocol/Cpp2Ops.h>

namespace apache {
namespace thrift {

/**
 *
 */
class SerializableDynamic {
 public:
  SerializableDynamic() : value_(nullptr) {}

  /* implicit */ SerializableDynamic(folly::dynamic value)
      : value_(std::move(value)) {}

  SerializableDynamic& operator=(folly::dynamic value) {
    value_ = std::move(value);
    return *this;
  }

  const folly::dynamic& operator*() const { return value_; }
  folly::dynamic& operator*() { return value_; }

  const folly::dynamic& value() const { return value_; }
  folly::dynamic& value() { return value_; }

  const folly::dynamic* operator->() const { return &value_; }
  folly::dynamic* operator->() { return &value_; }

  bool operator==(const SerializableDynamic& other) const {
    return value_ == other.value_;
  }

  bool operator<(const SerializableDynamic& other) const {
    return value_ < other.value_;
  }

  template <typename Protocol_>
  uint32_t read(Protocol_* iprot) {
    uint32_t xfer = 0;
    std::string fname;
    protocol::TType ftype;
    int16_t fid;

    xfer += iprot->readStructBegin(fname);
    xfer += iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == protocol::T_STOP) {
      __fbthrift_clear();
    } else {
      switch (fid) {
        case 1: {
          if (ftype == protocol::T_BOOL) {
            bool value;
            xfer += iprot->readBool(value);
            value_ = value;
          } else {
            xfer += iprot->skip(ftype);
          }
          break;
        }
        case 2: {
          if (ftype == protocol::T_I64) {
            int64_t value;
            xfer += iprot->readI64(value);
            value_ = value;
          } else {
            xfer += iprot->skip(ftype);
          }
          break;
        }
        case 3: {
          if (ftype == protocol::T_DOUBLE) {
            double value;
            xfer += iprot->readDouble(value);
            value_ = value;
          } else {
            xfer += iprot->skip(ftype);
          }
          break;
        }
        case 4: {
          if (ftype == protocol::T_STRING) {
            std::string value;
            xfer += iprot->readString(value);
            value_ = std::move(value);
          } else {
            xfer += iprot->skip(ftype);
          }
          break;
        }
        case 5: {
          if (ftype == protocol::T_LIST) {
            value_ = folly::dynamic::array;
            uint32_t size;
            bool sizeUnknown;
            protocol::TType etype;
            xfer += iprot->readListBegin(etype, size, sizeUnknown);
            for (uint32_t i = 0; i < size; ++i) {
              SerializableDynamic item;
              xfer += item.read(iprot);
              value_.push_back(std::move(item.value_));
            }
            xfer += iprot->readListEnd();
          } else {
            xfer += iprot->skip(ftype);
          }
          break;
        }
        case 6: {
          if (ftype == protocol::T_MAP) {
            value_ = folly::dynamic::object;
            uint32_t size;
            bool sizeUnknown;
            protocol::TType ktype;
            protocol::TType vtype;
            xfer += iprot->readMapBegin(ktype, vtype, size, sizeUnknown);
            for (uint32_t i = 0; i < size; ++i) {
              std::string key;
              xfer += iprot->readString(key);
              SerializableDynamic val;
              xfer += val.read(iprot);
              value_[std::move(key)] = std::move(val.value_);
            }
            xfer += iprot->readMapEnd();
          } else {
            xfer += iprot->skip(ftype);
          }
          break;
        }
        default: {
          xfer += iprot->skip(ftype);
          break;
        }
      }
      xfer += iprot->readFieldEnd();
      xfer += iprot->readFieldBegin(fname, ftype, fid);
      xfer += iprot->readFieldEnd();
    }
    xfer += iprot->readStructEnd();

    return xfer;
  }

  template <typename Protocol_>
  uint32_t write(Protocol_* p) const {
    uint32_t xfer = 0;
    xfer += p->writeStructBegin("Dynamic");
    switch (value_.type()) {
      case folly::dynamic::Type::NULLT:
        break;

      case folly::dynamic::Type::BOOL:
        xfer += p->writeFieldBegin("boolean", protocol::T_BOOL, 1);
        xfer += p->writeBool(value_.asBool());
        xfer += p->writeFieldEnd();
        break;

      case folly::dynamic::Type::INT64:
        xfer += p->writeFieldBegin("integer", protocol::T_I64, 2);
        xfer += p->writeI64(value_.asInt());
        xfer += p->writeFieldEnd();
        break;

      case folly::dynamic::Type::DOUBLE:
        xfer += p->writeFieldBegin("doubl", protocol::T_DOUBLE, 3);
        xfer += p->writeDouble(value_.asDouble());
        xfer += p->writeFieldEnd();
        break;

      case folly::dynamic::Type::STRING:
        xfer += p->writeFieldBegin("str", protocol::T_STRING, 4);
        xfer += p->writeString(value_.getString());
        xfer += p->writeFieldEnd();
        break;

      case folly::dynamic::Type::ARRAY:
        xfer += p->writeFieldBegin("arr", protocol::T_LIST, 5);
        xfer += p->writeListBegin(protocol::T_STRUCT, value_.size());
        for (const auto& item : value_) {
          SerializableDynamic wrappedItem(item);
          xfer += wrappedItem.write(p);
        }
        xfer += p->writeListEnd();
        xfer += p->writeFieldEnd();
        break;

      case folly::dynamic::Type::OBJECT:
        xfer += p->writeFieldBegin("object", protocol::T_MAP, 6);
        xfer += p->writeMapBegin(
            protocol::T_STRING, protocol::T_STRUCT, value_.size());
        for (const auto& item : value_.items()) {
          SerializableDynamic wrappedItem(item.second);
          xfer += p->writeString(item.first.asString());
          xfer += wrappedItem.write(p);
        }
        xfer += p->writeMapEnd();
        xfer += p->writeFieldEnd();
        break;
    }

    xfer += p->writeFieldStop();
    xfer += p->writeStructEnd();
    return xfer;
  }

 private:
  folly::dynamic value_;
  friend class ::apache::thrift::Cpp2Ops<SerializableDynamic>;
  friend struct apache::thrift::detail::st::struct_private_access;

  void __fbthrift_clear() { value_ = nullptr; }
};

////////////////////////////////////////////////////////////////////////////////

template <>
class Cpp2Ops<SerializableDynamic> {
 public:
  static void clear(SerializableDynamic* obj) { apache::thrift::clear(*obj); }

  static constexpr protocol::TType thriftType() { return protocol::T_STRUCT; }

  template <class Protocol_>
  static uint32_t write(Protocol_* p, const SerializableDynamic* obj) {
    return obj->write(p);
  }

  template <class Protocol_>
  static uint32_t serializedSize(
      const Protocol_* p, const SerializableDynamic* obj) {
    uint32_t xfer = 0;
    xfer += p->serializedStructSize("Variant");
    switch (obj->value_.type()) {
      case folly::dynamic::Type::NULLT:
        break;

      case folly::dynamic::Type::BOOL:
        xfer += p->serializedFieldSize("boolean", protocol::T_BOOL, 1);
        xfer += p->serializedSizeBool(obj->value_.asBool());
        break;

      case folly::dynamic::Type::INT64:
        xfer += p->serializedFieldSize("integer", protocol::T_I64, 2);
        xfer += p->serializedSizeI64(obj->value_.asInt());
        break;

      case folly::dynamic::Type::DOUBLE:
        xfer += p->serializedFieldSize("doubl", protocol::T_DOUBLE, 3);
        xfer += p->serializedSizeDouble(obj->value_.asDouble());
        break;

      case folly::dynamic::Type::STRING:
        xfer += p->serializedFieldSize("str", protocol::T_STRING, 4);
        xfer += p->serializedSizeString(obj->value_.getString());
        break;

      case folly::dynamic::Type::ARRAY:
        xfer += p->serializedFieldSize("arr", protocol::T_LIST, 5);
        xfer +=
            p->serializedSizeListBegin(protocol::T_STRUCT, obj->value_.size());
        for (const auto& item : obj->value_) {
          SerializableDynamic wrappedItem(item);
          xfer += Cpp2Ops<SerializableDynamic>::serializedSize(p, &wrappedItem);
        }
        xfer += p->serializedSizeListEnd();
        break;

      case folly::dynamic::Type::OBJECT:
        xfer += p->serializedFieldSize("object", protocol::T_MAP, 6);
        xfer += p->serializedSizeMapBegin(
            protocol::T_STRING, protocol::T_STRUCT, obj->value_.size());
        for (const auto& item : obj->value_.items()) {
          SerializableDynamic wrappedItem(item.second);
          xfer += p->serializedSizeString(item.first.asString());
          xfer += Cpp2Ops<SerializableDynamic>::serializedSize(p, &wrappedItem);
        }
        xfer += p->serializedSizeMapEnd();
        break;
    }
    xfer += p->serializedSizeStop();
    return xfer;
  }

  template <class Protocol_>
  static uint32_t serializedSizeZC(
      const Protocol_* p, const SerializableDynamic* obj) {
    return serializedSize(p, obj);
  }

  template <class Protocol_>
  static void read(Protocol_* iprot, SerializableDynamic* obj) {
    std::string fname;
    protocol::TType ftype;
    int16_t fid;

    iprot->readStructBegin(fname);
    iprot->readFieldBegin(fname, ftype, fid);
    if (ftype == protocol::T_STOP) {
      apache::thrift::clear(*obj);
    } else {
      switch (fid) {
        case 1: {
          if (ftype == protocol::T_BOOL) {
            bool value;
            iprot->readBool(value);
            obj->value_ = value;
          } else {
            iprot->skip(ftype);
          }
          break;
        }
        case 2: {
          if (ftype == protocol::T_I64) {
            int64_t value;
            iprot->readI64(value);
            obj->value_ = value;
          } else {
            iprot->skip(ftype);
          }
          break;
        }
        case 3: {
          if (ftype == protocol::T_DOUBLE) {
            double value;
            iprot->readDouble(value);
            obj->value_ = value;
          } else {
            iprot->skip(ftype);
          }
          break;
        }
        case 4: {
          if (ftype == protocol::T_STRING) {
            std::string value;
            iprot->readString(value);
            obj->value_ = std::move(value);
          } else {
            iprot->skip(ftype);
          }
          break;
        }
        case 5: {
          if (ftype == protocol::T_LIST) {
            obj->value_ = folly::dynamic::array;
            uint32_t size;
            protocol::TType etype;
            iprot->readListBegin(etype, size);
            obj->value_.reserve(size);
            for (uint32_t i = 0; i < size; ++i) {
              SerializableDynamic item;
              Cpp2Ops<SerializableDynamic>::read(iprot, &item);
              obj->value_.push_back(std::move(item.value_));
            }
            iprot->readListEnd();
          } else {
            iprot->skip(ftype);
          }
          break;
        }
        case 6: {
          if (ftype == protocol::T_MAP) {
            obj->value_ = folly::dynamic::object;
            uint32_t size;
            protocol::TType ktype;
            protocol::TType vtype;
            iprot->readMapBegin(ktype, vtype, size);
            for (uint32_t i = 0; i < size; ++i) {
              std::string key;
              iprot->readString(key);
              SerializableDynamic val;
              Cpp2Ops<SerializableDynamic>::read(iprot, &val);
              obj->value_[std::move(key)] = std::move(val.value_);
            }
            iprot->readMapEnd();
          } else {
            iprot->skip(ftype);
          }
          break;
        }
        default: {
          iprot->skip(ftype);
          break;
        }
      }
      iprot->readFieldEnd();
      iprot->readFieldBegin(fname, ftype, fid);
      iprot->readFieldEnd();
    }
    iprot->readStructEnd();
  }
};
} // namespace thrift
} // namespace apache

#endif // THRIFT_UTIL_SERIALIZABLEDYNAMIC_H
