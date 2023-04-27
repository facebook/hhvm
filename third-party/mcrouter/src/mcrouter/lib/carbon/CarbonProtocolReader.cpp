/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * This source code is licensed under the MIT license found in the
 * LICENSE file in the root directory of this source tree.
 */

#include "CarbonProtocolReader.h"

namespace carbon {
void CarbonProtocolReader::skipLinearContainer() {
  const auto pr = readLinearContainerFieldSizeAndInnerType();
  skipLinearContainerItems(pr);
}

void CarbonProtocolReader::skipLinearContainerItems(
    std::pair<FieldType, uint32_t> pr) {
  const auto fieldType = pr.first;
  const auto len = pr.second;
  for (uint32_t i = 0; i < len; ++i) {
    skip(fieldType);
  }
}

void CarbonProtocolReader::skipKVContainer() {
  const auto pr = readKVContainerFieldSizeAndInnerTypes();
  skipKVContainerItems(pr);
}

void CarbonProtocolReader::skipKVContainerItems(
    std::pair<std::pair<FieldType, FieldType>, uint32_t> pr) {
  const auto len = pr.second;
  const auto keyType = pr.first.first;
  const auto valType = pr.first.second;
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
