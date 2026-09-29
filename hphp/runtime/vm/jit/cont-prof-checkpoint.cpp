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

#include "hphp/runtime/vm/jit/cont-prof-checkpoint.h"

#include <cstddef>
#include <cstdint>
#include <cstring>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include <folly/FileUtil.h>
#include <folly/Range.h>

#include "hphp/runtime/vm/jit/cont-prof-serde.h"

namespace HPHP::jit {

namespace {

using EncodedRecordCount = uint32_t;
using EncodedRecordSize = uint32_t;

constexpr size_t kMaxCheckpointBytes = 256U << 20;
constexpr EncodedRecordCount kMaxCheckpointRecords = 100'000;

struct Writer {
  explicit Writer(size_t maxBytes) : m_maxBytes{maxBytes} {}

  template<class T>
  bool writeValue(T value) {
    static_assert(std::is_unsigned_v<T>);
    return writeBytes(reinterpret_cast<const uint8_t*>(&value), sizeof(value));
  }

  bool writeBytes(const uint8_t* data, size_t size) {
    if (size > m_maxBytes - m_bytes.size()) return false;
    if (size != 0) {
      m_bytes.insert(m_bytes.end(), data, data + size);
    }
    return true;
  }

  template<class T>
  bool replaceValue(size_t offset, T value) {
    static_assert(std::is_unsigned_v<T>);
    if (offset > m_bytes.size() ||
        sizeof(value) > m_bytes.size() - offset) {
      return false;
    }

    std::memcpy(m_bytes.data() + offset, &value, sizeof(value));
    return true;
  }

  size_t remaining() const { return m_maxBytes - m_bytes.size(); }

  std::vector<uint8_t> takeBytes() && { return std::move(m_bytes); }

private:
  size_t m_maxBytes;
  std::vector<uint8_t> m_bytes;
};

struct Reader {
  explicit Reader(folly::ByteRange bytes)
    : m_data{bytes.data()}
    , m_remaining{bytes.size()} {}

  template<class T>
  bool readValue(T& value) {
    static_assert(std::is_unsigned_v<T>);
    return readBytes(reinterpret_cast<uint8_t*>(&value), sizeof(value));
  }

  bool readBytes(uint8_t* output, size_t size) {
    if (size > m_remaining) return false;

    if (size != 0) {
      std::memcpy(output, m_data, size);
      m_data += size;
      m_remaining -= size;
    }

    return true;
  }

  std::optional<folly::ByteRange> readRange(size_t size) {
    if (size > m_remaining) return std::nullopt;

    auto const result = folly::ByteRange{m_data, size};
    m_data += size;
    m_remaining -= size;
    return result;
  }

  size_t remaining() const { return m_remaining; }

private:
  const uint8_t* m_data;
  size_t m_remaining;
};

}

std::optional<std::vector<uint8_t>>
serializeContProfCheckpoint(const std::vector<ContProfProfileRecord>& records) {
  // TODO(jtwarren): add support for selecting hottest and truncating the rest.
  if (records.empty() || records.size() > kMaxCheckpointRecords) {
    return std::nullopt;
  }

  Writer writer{kMaxCheckpointBytes};

  if (!writer.writeValue(EncodedRecordCount{0})) return std::nullopt;

  EncodedRecordCount recordCount{};
  for (size_t i = 0; i < records.size(); ++i) {
    if (i != 0 &&
        !(records[i - 1].header.funcKey < records[i].header.funcKey)) {
      return std::nullopt;
    }

    auto const encoded = serializeContProfProfileRecord(records[i]);
    if (!encoded || sizeof(EncodedRecordSize) > writer.remaining() ||
        encoded->size() > writer.remaining() - sizeof(EncodedRecordSize)) {
      continue;
    }

    if (!writer.writeValue(static_cast<EncodedRecordSize>(encoded->size())) ||
        !writer.writeBytes(encoded->data(), encoded->size())) {
      return std::nullopt;
    }

    ++recordCount;
  }

  if (recordCount == 0 ||
      !writer.replaceValue(0, recordCount)) {
    return std::nullopt;
  }

  return std::move(writer).takeBytes();
}

std::optional<std::vector<ContProfProfileRecord>>
deserializeContProfCheckpoint(folly::ByteRange encoded) {
  if (encoded.size() < sizeof(EncodedRecordCount) ||
      encoded.size() > kMaxCheckpointBytes) {
    return std::nullopt;
  }

  Reader reader{encoded};

  EncodedRecordCount recordCount{};
  if (!reader.readValue(recordCount) || recordCount == 0 ||
      recordCount > kMaxCheckpointRecords) {
    return std::nullopt;
  }

  std::vector<ContProfProfileRecord> records;
  records.reserve(recordCount);

  for (EncodedRecordCount i = 0; i < recordCount; ++i) {
    EncodedRecordSize recordSize{};
    if (!reader.readValue(recordSize) || recordSize == 0) return std::nullopt;

    auto const frame = reader.readRange(recordSize);
    if (!frame) return std::nullopt;

    auto record = deserializeContProfProfileRecord(*frame);
    if (!record) return std::nullopt;

    if (!records.empty() &&
        !(records.back().header.funcKey < record->header.funcKey)) {
      return std::nullopt;
    }

    records.push_back(std::move(*record));
  }

  if (reader.remaining() != 0) return std::nullopt;
  return records;
}

bool writeContProfCheckpointFile(
  const std::string& path,
  const std::vector<ContProfProfileRecord>& records
) {
  if (path.empty()) return false;

  auto const encoded = serializeContProfCheckpoint(records);
  if (!encoded) return false;

  auto const contents = folly::StringPiece{
    reinterpret_cast<const char*>(encoded->data()),
    encoded->size()
  };

  return folly::writeFileAtomicNoThrow(
    path,
    contents,
    folly::WriteFileAtomicOptions{}
  ) == 0;
}

std::optional<std::vector<ContProfProfileRecord>>
readContProfCheckpointFile(const std::string& path) {
  if (path.empty()) return std::nullopt;

  std::vector<uint8_t> encoded;
  if (!folly::readFile(path.c_str(), encoded, kMaxCheckpointBytes + 1) ||
      encoded.size() > kMaxCheckpointBytes) {
    return std::nullopt;
  }

  return deserializeContProfCheckpoint(
    folly::ByteRange{encoded.data(), encoded.size()}
  );
}

}
