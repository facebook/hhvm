/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#pragma once

#include <folly/io/IOBuf.h>
#include <string>
#include <vector>

#include "mcrouter/lib/carbon/Fields.h"

namespace carbon {
namespace test {

typedef struct {
  std::string name;
  std::vector<int> points;
} UserType;
} // namespace test

template <>
struct SerializationTraits<carbon::test::UserType> {
  static constexpr carbon::FieldType kWireType = carbon::FieldType::Struct;

  template <class Reader>
  static carbon::test::UserType read(Reader&& reader) {
    carbon::test::UserType readType;
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
          reader.readRawInto(readType.name);
          break;
        }
        case 2: {
          reader.readRawInto(readType.points);
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
  static void write(const carbon::test::UserType& writeType, Writer&& writer) {
    writer.writeStructBegin();
    writer.writeField(1 /* field id */, writeType.name);
    writer.writeField(2 /* field id */, writeType.points);
    writer.writeFieldStop();
    writer.writeStructEnd();
  }

  static bool isEmpty(const carbon::test::UserType& writeType) {
    return writeType.name.empty() && writeType.points.empty();
  }
};
} // namespace carbon
