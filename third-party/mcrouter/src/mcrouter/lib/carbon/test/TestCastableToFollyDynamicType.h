/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once
#include "folly/json/dynamic.h"

/* Class which implements a function-style cast to 'folly::dynamic'
 * for testing in CarbonMessageConversionUtilsTest.cpp.
 *    tests std::is_convertible<T, folly::dynamic>::value toDynamic#()
 * function used in carbon::ConvertToFollyDynamic()
 */

namespace carbon {
namespace test {

class CastableToFollyDynamicType {
 public:
  int val_ = 0;
  CastableToFollyDynamicType(int val = 0) : val_(val) {}
  operator folly::dynamic() const {
    return val_;
  }
};
} // namespace test

// Serialization traits are required for types included in CarbonTest.idl
//  These traits are utilized in auto generated code.
//  see: CarbonTestMessages.cpp
template <>
struct SerializationTraits<carbon::test::CastableToFollyDynamicType> {
  static constexpr carbon::FieldType kWireType = carbon::FieldType::Struct;

  template <class Reader>
  static carbon::test::CastableToFollyDynamicType read(Reader&& reader) {
    carbon::test::CastableToFollyDynamicType readType;
    reader.readStructBegin();
    while (true) {
      const auto pr = reader.readFieldHeader();
      const auto fieldType = pr.first;
      const auto fieldId = pr.second;

      if (fieldType == carbon::FieldType::Stop) {
        break;
      }

      switch (fieldId) {
        case 1: {
          reader.readRawInto(readType.val_);
          break;
        }
        default: {
          reader.skip(fieldType);
          break;
        }
      }
    }
    reader.readStructEnd();
    return readType;
  }

  template <class Writer>
  static void write(
      const carbon::test::CastableToFollyDynamicType& writeType,
      Writer&& writer) {
    writer.writeStructBegin();
    writer.writeField(1 /* field id */, writeType.val_);
    writer.writeFieldStop();
    writer.writeStructEnd();
  }

  static bool isEmpty(
      const carbon::test::CastableToFollyDynamicType& writeType) {
    return writeType.val_ == 0;
  }
};

} // namespace carbon
