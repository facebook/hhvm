/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "CarbonProtocolReader.h"

namespace carbon {

namespace {
// Zero-length types: booleans and undefined types (>= 0xe) where skip() is a
// no-op. Early return prevents timeout on malformed input with huge sizes.
constexpr bool isZeroLengthFieldType(FieldType ft) {
  return ft == FieldType::True || ft == FieldType::False ||
      static_cast<uint8_t>(ft) > static_cast<uint8_t>(FieldType::Float);
}
} // namespace

void CarbonProtocolReader::skipLinearContainer() {
  skipLinearContainerItems(readLinearContainerFieldSizeAndInnerType());
}

void CarbonProtocolReader::skipLinearContainerItems(
    std::pair<FieldType, uint32_t> pr) {
  const auto [fieldType, len] = pr;
  if (isZeroLengthFieldType(fieldType)) {
    return;
  }
  for (uint32_t i = 0; i < len; ++i) {
    skip(fieldType);
  }
}

void CarbonProtocolReader::skipKVContainer() {
  skipKVContainerItems(readKVContainerFieldSizeAndInnerTypes());
}

void CarbonProtocolReader::skipKVContainerItems(
    std::pair<std::pair<FieldType, FieldType>, uint32_t> pr) {
  const auto [keyType, valType] = pr.first;
  const auto len = pr.second;
  if (isZeroLengthFieldType(keyType) && isZeroLengthFieldType(valType)) {
    return;
  }
  for (uint32_t i = 0; i < len; ++i) {
    skip(keyType);
    skip(valType);
  }
}

void CarbonProtocolReader::skip(const FieldType ft) {
  switch (ft) {
    case FieldType::True:
    case FieldType::False: {
      break;
    }
    case FieldType::Int8: {
      readRaw<int8_t>();
      break;
    }
    case FieldType::Int16: {
      readRaw<int16_t>();
      break;
    }
    case FieldType::Int32: {
      readRaw<int32_t>();
      break;
    }
    case FieldType::Int64: {
      readRaw<int64_t>();
      break;
    }
    case FieldType::Double: {
      readRaw<double>();
      break;
    }
    case FieldType::Float: {
      readRaw<float>();
      break;
    }
    case FieldType::Binary: {
      readRaw<std::string>();
      break;
    }
    case FieldType::List: {
      skipLinearContainer();
      break;
    }
    case FieldType::Struct: {
      readStructBegin();
      while (true) {
        const auto fieldType = readFieldHeader().first;
        if (fieldType == FieldType::Stop) {
          break;
        }
        skip(fieldType);
      }
      readStructEnd();
      break;
    }
    case FieldType::Set: {
      skipLinearContainer();
      break;
    }
    case FieldType::Map: {
      skipKVContainer();
      break;
    }
    default: {
      break;
    }
  }
}

} // namespace carbon
