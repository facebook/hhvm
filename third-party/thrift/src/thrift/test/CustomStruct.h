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

#include <string>

#include <folly/Conv.h>
#include <thrift/lib/cpp2/Thrift.h>
#include <thrift/lib/cpp2/protocol/Cpp2Ops.h>

namespace thrift::test {

/**
 * We serialize the first four bytes of the data_ string into the intData
 * field and the rest into the stringData field of the thrift struct.
 */
class MyCustomStruct {
 public:
  MyCustomStruct() : data_(4, '\0') {}
  /* implicit */ MyCustomStruct(const std::string& data) : data_(data) {}

  void __fbthrift_clear() { data_.clear(); }

  bool operator==(const MyCustomStruct& other) const {
    return data_ == other.data_;
  }
  bool operator<(const MyCustomStruct& other) const {
    return data_ < other.data_;
  }

  std::string data_;
};

/**
 * If data_ is parseable into an int, we serialize it as the intData field of
 * the thrift union; otherwise as the stringData field.
 */
class MyCustomUnion {
 public:
  MyCustomUnion() {}
  /* implicit */ MyCustomUnion(const std::string& data) : data_(data) {}

  void __fbthrift_clear() { data_.clear(); }

  bool operator==(const MyCustomUnion& other) const {
    return data_ == other.data_;
  }
  bool operator<(const MyCustomUnion& other) const {
    return data_ < other.data_;
  }

  std::string data_;
};

} // namespace thrift::test

////////////////////////////////////////////////////////////////////////////////

namespace apache::thrift {

template <>
class Cpp2Ops<::thrift::test::MyCustomStruct> {
 public:
  template <class Protocol>
  static uint32_t write(
      Protocol* p, const ::thrift::test::MyCustomStruct* obj) {
    uint32_t xfer = 0;
    assert(obj->data_.size() >= sizeof(int));
    const int prefix = *reinterpret_cast<const int*>(&obj->data_[0]);
    std::string suffix = obj->data_.substr(sizeof(int));
    xfer += p->writeStructBegin("MyStruct");
    xfer += p->writeFieldBegin("stringData", protocol::T_STRING, 1);
    xfer += p->writeString(suffix);
    xfer += p->writeFieldEnd();
    xfer += p->writeFieldBegin("intData", protocol::T_I32, 2);
    xfer += p->writeI32(prefix);
    xfer += p->writeFieldEnd();
    xfer += p->writeFieldStop();
    xfer += p->writeStructEnd();
    return xfer;
  }

  template <class Protocol>
  static uint32_t serializedSize(
      const Protocol* p, const ::thrift::test::MyCustomStruct* obj) {
    uint32_t xfer = 0;
    assert(obj->data_.size() >= sizeof(int));
    const int prefix = *reinterpret_cast<const int*>(&obj->data_[0]);
    std::string suffix = obj->data_.substr(sizeof(int));
    xfer += p->serializedStructSize("MyStruct");
    xfer += p->serializedFieldSize("stringData", protocol::T_STRING, 1);
    xfer += p->serializedSizeString(suffix);
    xfer +=
        p->serializedFieldSize("intData", apache::thrift::protocol::T_I32, 2);
    xfer += p->serializedSizeI32(prefix);
    xfer += p->serializedSizeStop();
    return xfer;
  }

  template <class Protocol>
  static uint32_t serializedSizeZC(
      const Protocol* p, const ::thrift::test::MyCustomStruct* obj) {
    return serializedSize(p, obj);
  }

  template <class Protocol>
  static void read(Protocol* iprot, ::thrift::test::MyCustomStruct* obj) {
    std::string fname;
    protocol::TType ftype;
    int16_t fid;
    std::string suffix;
    int prefix;

    iprot->readStructBegin(fname);

    while (true) {
      iprot->readFieldBegin(fname, ftype, fid);
      if (ftype == apache::thrift::protocol::T_STOP) {
        break;
      }
      switch (fid) {
        case 1:
          if (ftype == apache::thrift::protocol::T_STRING) {
            iprot->readString(suffix);
          } else {
            iprot->skip(ftype);
          }
          break;
        case 2:
          if (ftype == apache::thrift::protocol::T_I32) {
            iprot->readI32(prefix);
          } else {
            iprot->skip(ftype);
          }
          break;
        default:
          iprot->skip(ftype);
          break;
      }
      iprot->readFieldEnd();
    }
    iprot->readStructEnd();

    obj->data_ =
        std::string(reinterpret_cast<const char*>(&prefix), sizeof(int)) +
        suffix;
  }

  static constexpr apache::thrift::protocol::TType thriftType() {
    return apache::thrift::protocol::T_STRUCT;
  }
};

////////////////////////////////////////////////////////////////////////////////

template <>
class Cpp2Ops<::thrift::test::MyCustomUnion> {
 public:
  template <class Protocol>
  static uint32_t write(Protocol* p, const ::thrift::test::MyCustomUnion* obj) {
    uint32_t xfer = 0;
    xfer += p->writeStructBegin("MyStruct");
    try {
      int i = folly::to<int>(obj->data_);
      xfer += p->writeFieldBegin("intData", protocol::T_I32, 2);
      xfer += p->writeI32(i);
      xfer += p->writeFieldEnd();
    } catch (const std::range_error&) {
      xfer += p->writeFieldBegin("stringData", protocol::T_STRING, 1);
      xfer += p->writeString(obj->data_);
      xfer += p->writeFieldEnd();
    }
    xfer += p->writeFieldStop();
    xfer += p->writeStructEnd();
    return xfer;
  }

  template <class Protocol>
  static uint32_t serializedSize(
      const Protocol* p, const ::thrift::test::MyCustomUnion* obj) {
    uint32_t xfer = 0;
    xfer += p->serializedStructSize("MyStruct");
    try {
      int i = folly::to<int>(obj->data_);
      xfer += p->serializedFieldSize("intData", protocol::T_I32, 2);
      xfer += p->serializedSizeI32(i);
    } catch (const std::range_error&) {
      xfer += p->serializedFieldSize("stringData", protocol::T_STRING, 1);
      xfer += p->serializedSizeString(obj->data_);
    }
    xfer += p->serializedSizeStop();
    return xfer;
  }

  template <class Protocol>
  static uint32_t serializedSizeZC(
      const Protocol* p, const ::thrift::test::MyCustomUnion* obj) {
    return serializedSize(p, obj);
  }

  template <class Protocol>
  static void read(Protocol* iprot, ::thrift::test::MyCustomUnion* obj) {
    std::string fname;
    protocol::TType ftype;
    int16_t fid;

    iprot->readStructBegin(fname);

    iprot->readFieldBegin(fname, ftype, fid);
    if (ftype != apache::thrift::protocol::T_STOP) {
      switch (fid) {
        case 1:
          if (ftype == apache::thrift::protocol::T_STRING) {
            iprot->readString(obj->data_);

          } else {
            iprot->skip(ftype);
          }
          break;
        case 2:
          if (ftype == apache::thrift::protocol::T_I32) {
            int i;
            iprot->readI32(i);
            obj->data_ = folly::to<std::string>(i);
          } else {
            iprot->skip(ftype);
          }
          break;
        default:
          iprot->skip(ftype);
          break;
      }
      iprot->readFieldEnd();
      iprot->readFieldBegin(fname, ftype, fid);
      iprot->readFieldEnd();
    }

    iprot->readStructEnd();
  }

  static constexpr apache::thrift::protocol::TType thriftType() {
    return apache::thrift::protocol::T_STRUCT;
  }
};
} // namespace apache::thrift
