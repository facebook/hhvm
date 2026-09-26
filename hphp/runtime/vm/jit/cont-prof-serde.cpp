/*
   +----------------------------------------------------------------------+
   | HipHop for PHP                                                       |
   +----------------------------------------------------------------------+
   | Copyright (c) 2010-present Facebook, Inc. (http://www.facebook.com)  |
   +----------------------------------------------------------------------+
   | This source file is subject to version 3.01 of the PHP license,      |
   | that is bundled with this package in the file LICENSE, and is        |
   | available through the world-wide-web at the following url:           |
   | http://www.php.net/license/3_01.txt                                  |
   | If you did not receive a copy of the PHP license and are unable to   |
   | obtain it through the world-wide-web, please send a note to          |
   | license@php.net so we can mail you a copy immediately.               |
   +----------------------------------------------------------------------+
*/

#include "hphp/runtime/vm/jit/cont-prof-serde.h"

#include <array>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "hphp/util/build-info.h"

namespace HPHP::jit {

namespace {

using EncodedStartKind = uint8_t;
using EncodedEntryArgs = uint32_t;
using EncodedRegionLength = uint32_t;
using EncodedExecutionCount = uint64_t;

constexpr size_t kEncodedProfileTranslationSize =
  sizeof(EncodedStartKind) +
  sizeof(EncodedEntryArgs) +
  sizeof(EncodedRegionLength) +
  sizeof(EncodedExecutionCount);
using EncodedSHA1 = std::array<uint32_t, SHA1::kQNumWords>;
static_assert(sizeof(EncodedSHA1) == SHA1::kStrLen / 2);

EncodedSHA1 encodeSHA1(const SHA1& hash) {
  EncodedSHA1 encoded{};
  hash.nbo(encoded.data());
  return encoded;
}

struct Writer {
  void writeByte(uint8_t value) { writeBytes(&value, sizeof(value)); }

  template<class T>
  void writeValue(T value) {
    static_assert(std::is_unsigned_v<T>);
    writeBytes(&value, sizeof(value));
  }

  void writeBytes(const void* data, size_t size) {
    if (size != 0) {
      auto const bytes = static_cast<const uint8_t*>(data);
      m_bytes.insert(m_bytes.end(), bytes, bytes + size);
    }
  }

  void writeString(folly::StringPiece value) {
    writeValue(value.size());
    writeBytes(value.data(), value.size());
  }

  void writeOptionalString(const std::optional<std::string>& value) {
    writeByte(value ? 1 : 0);
    if (value) writeString(folly::StringPiece{*value});
  }

  std::vector<uint8_t> takeBytes() && { return std::move(m_bytes); }

private:
  std::vector<uint8_t> m_bytes;
};

struct Reader {
  explicit Reader(folly::ByteRange bytes)
    : m_data{bytes.data()}
    , m_remaining{bytes.size()} {}

  bool readByte(uint8_t& value) { return readBytes(&value, sizeof(value)); }

  template<class T>
  bool readValue(T& value) {
    static_assert(std::is_unsigned_v<T>);
    return readBytes(&value, sizeof(value));
  }

  bool readBytes(void* output, size_t size) {
    if (size > m_remaining) return false;

    if (size != 0) {
      std::memcpy(output, m_data, size);
      m_data += size;
      m_remaining -= size;
    }

    return true;
  }

  bool readString(std::string& value) {
    size_t size{};
    if (!readValue(size) || size > m_remaining) return false;

    value.resize(size);
    return readBytes(value.data(), size);
  }

  bool readOptionalString(std::optional<std::string>& value) {
    uint8_t present{};
    if (!readByte(present) || present > 1) return false;

    if (!present) {
      value.reset();
      return true;
    }

    std::string decoded;
    if (!readString(decoded)) return false;
    value = std::move(decoded);
    return true;
  }

  size_t remaining() const { return m_remaining; }

private:
  const uint8_t* m_data;
  size_t m_remaining;
};

}

std::optional<std::vector<uint8_t>>
serializeContProfFuncKey(const ContProfFuncKey& key) {
  if (!isValidContProfFuncKey(key)) return std::nullopt;

  auto const encodedBytecodeUnitHash = encodeSHA1(key.bytecodeUnitHash);

  Writer writer;
  writer.writeString(repoSchemaId());
  writer.writeString(key.resolutionUnitPath);
  writer.writeOptionalString(key.bytecodeUnitPath);
  writer.writeBytes(
    encodedBytecodeUnitHash.data(),
    sizeof(encodedBytecodeUnitHash)
  );
  writer.writeString(key.functionName);
  writer.writeOptionalString(key.className);
  writer.writeOptionalString(key.closureContextName);

  return std::move(writer).takeBytes();
}

std::optional<ContProfFuncKey>
deserializeContProfFuncKey(folly::ByteRange encoded) {
  Reader reader{encoded};

  std::string schema;
  if (!reader.readString(schema) || schema != repoSchemaId()) {
    return std::nullopt;
  }

  ContProfFuncKey key{};
  EncodedSHA1 encodedBytecodeUnitHash{};

  if (!reader.readString(key.resolutionUnitPath) ||
      !reader.readOptionalString(key.bytecodeUnitPath) ||
      !reader.readBytes(encodedBytecodeUnitHash.data(),
                        sizeof(encodedBytecodeUnitHash)) ||
      !reader.readString(key.functionName) ||
      !reader.readOptionalString(key.className) ||
      !reader.readOptionalString(key.closureContextName) ||
      reader.remaining() != 0) {
    return std::nullopt;
  }

  key.bytecodeUnitHash = SHA1{
    encodedBytecodeUnitHash.data(),
    sizeof(encodedBytecodeUnitHash)
  };
  if (!isValidContProfFuncKey(key)) return std::nullopt;

  return key;
}

std::optional<std::vector<uint8_t>>
serializeContProfProfileRecord(const ContProfProfileRecord& record) {
  if (!isValidContProfProfileRecord(record)) return std::nullopt;

  auto const funcKey = serializeContProfFuncKey(record.header.funcKey);
  if (!funcKey) return std::nullopt;

  Writer writer;
  writer.writeValue(funcKey->size());
  writer.writeBytes(funcKey->data(), funcKey->size());
  writer.writeValue(record.header.capturedAtMs);
  writer.writeValue(record.translations.size());

  for (auto const& translation : record.translations) {
    writer.writeValue(static_cast<EncodedStartKind>(translation.startKind));
    writer.writeValue(translation.numEntryArgs);
    writer.writeValue(translation.regionLength);
    writer.writeValue(translation.executionCount);
  }

  return std::move(writer).takeBytes();
}

std::optional<ContProfProfileRecord>
deserializeContProfProfileRecord(folly::ByteRange encoded) {
  Reader reader{encoded};

  size_t funcKeySize{};
  if (!reader.readValue(funcKeySize) || funcKeySize == 0 ||
      funcKeySize > reader.remaining()) {
    return std::nullopt;
  }

  std::vector<uint8_t> funcKeyBytes(funcKeySize);
  if (!reader.readBytes(funcKeyBytes.data(), funcKeyBytes.size())) {
    return std::nullopt;
  }

  auto funcKey = deserializeContProfFuncKey(
    folly::ByteRange{funcKeyBytes.data(), funcKeyBytes.size()}
  );
  if (!funcKey) return std::nullopt;

  ContProfProfileRecord record{};
  record.header.funcKey = std::move(*funcKey);

  size_t translationCount{};
  if (!reader.readValue(record.header.capturedAtMs) ||
      !reader.readValue(translationCount) || translationCount == 0 ||
      translationCount > reader.remaining() / kEncodedProfileTranslationSize) {
    return std::nullopt;
  }

  record.translations.reserve(translationCount);

  for (size_t i = 0; i < translationCount; ++i) {
    ContProfProfileTranslation translation{};
    EncodedStartKind startKind{};

    if (!reader.readValue(startKind) ||
        !reader.readValue(translation.numEntryArgs) ||
        !reader.readValue(translation.regionLength) ||
        !reader.readValue(translation.executionCount)) {
      return std::nullopt;
    }

    switch (startKind) {
      case static_cast<EncodedStartKind>(ContProfStartKind::FuncEntry):
        translation.startKind = ContProfStartKind::FuncEntry;
        break;
      case static_cast<EncodedStartKind>(
          ContProfStartKind::NamedParamsFuncEntry):
        translation.startKind =
          ContProfStartKind::NamedParamsFuncEntry;
        break;
      default:
        return std::nullopt;
    }

    record.translations.push_back(translation);
  }

  if (reader.remaining() != 0 || !isValidContProfProfileRecord(record)) {
    return std::nullopt;
  }

  return record;
}

}
