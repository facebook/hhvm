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

#include <thrift/lib/cpp2/protocol/detail/Json5ProtocolWriter.h>

#include <cmath>
#include <concepts>

#include <boost/locale/utf.hpp>
#include <fmt/core.h>
#include <folly/Exception.h>
#include <folly/Unicode.h>
#include <folly/base64.h>

namespace apache::thrift::json5::detail {

namespace {

bool isValidUtf8(std::string_view str) {
  const char* begin = str.data();
  const char* end = begin + str.size();

  while (begin != end) {
    // Read next codepoint and advance pointer
    boost::locale::utf::code_point c =
        boost::locale::utf::utf_traits<char>::decode(begin, end);
    if (c == boost::locale::utf::illegal ||
        c == boost::locale::utf::incomplete) {
      return false;
    }
  }
  return true;
}

} // namespace

void Json5ProtocolWriter::setOutput(folly::IOBufQueue* queue, size_t growth) {
  writer_.setOutput(folly::io::QueueAppender(queue, growth));
}

void Json5ProtocolWriter::setOutput(folly::io::QueueAppender&& output) {
  writer_.setOutput(std::move(output));
}

std::uint32_t Json5ProtocolWriter::beginWriteValue() {
  auto* map = containerStack_.mapState();
  if (map == nullptr ||
      map->encodedAs != CompoundTypeTracker::MapForm::KeyValueArray) {
    return 0;
  }
  switch (map->expectingToken) {
    case CompoundTypeTracker::MapToken::Key: {
      // We are writing a complex key, we need to write `{"key":`
      //
      //   [                [{"key":
      //    ^        =>              ^
      // before                    after
      std::uint32_t xfer = writer_.writeObjectBegin();
      return xfer + writer_.writeObjectName("key");
    }
    case CompoundTypeTracker::MapToken::Value:
      // We are writing a value after a complex key, we need to write `"value":`
      //
      //   [{"key": ...,                [{"key": ..., "value":
      //                ^        =>                           ^
      //             before                                 after
      return writer_.writeObjectName("value");
  }
}

std::uint32_t Json5ProtocolWriter::endWriteValue() {
  auto* map = containerStack_.mapState();
  if (map == nullptr) {
    return 0;
  }

  std::uint32_t xfer = 0;
  if (map->encodedAs == CompoundTypeTracker::MapForm::KeyValueArray &&
      map->expectingToken == CompoundTypeTracker::MapToken::Value) {
    // We wrote a value after a complex key, we need to write `}`:
    //
    //   [{"key": ..., "value": ...         [{"key": ..., "value": ...},
    //                             ^    =>                             ^
    //                          before                               after
    xfer += writer_.writeObjectEnd();
  }

  containerStack_.toggleExpectingKeyValue();

  return xfer;
}

std::uint32_t Json5ProtocolWriter::writeStructBegin(const char* /*name*/) {
  std::uint32_t xfer = beginWriteValue();
  containerStack_.beginStruct();
  return xfer + writer_.writeObjectBegin();
}

std::uint32_t Json5ProtocolWriter::writeStructEnd() {
  containerStack_.endStruct();
  std::uint32_t xfer = writer_.writeObjectEnd();
  return xfer + endWriteValue();
}

std::uint32_t Json5ProtocolWriter::writeFieldBegin(
    const char* name, protocol::TType, int16_t) {
  return writer_.writeObjectName(name);
}

std::uint32_t Json5ProtocolWriter::writeFieldEnd() {
  return 0;
}

std::uint32_t Json5ProtocolWriter::writeFieldStop() {
  return 0;
}

std::uint32_t Json5ProtocolWriter::writeListBegin(protocol::TType, uint32_t) {
  std::uint32_t xfer = beginWriteValue();
  containerStack_.beginList();
  return xfer + writer_.writeListBegin();
}

std::uint32_t Json5ProtocolWriter::writeListEnd() {
  containerStack_.endList();
  std::uint32_t xfer = writer_.writeListEnd();
  return xfer + endWriteValue();
}

std::uint32_t Json5ProtocolWriter::writeSetBegin(protocol::TType, uint32_t) {
  std::uint32_t xfer = beginWriteValue();
  containerStack_.beginSet();
  return xfer + writer_.writeListBegin();
}

std::uint32_t Json5ProtocolWriter::writeSetEnd() {
  containerStack_.endSet();
  std::uint32_t xfer = writer_.writeListEnd();
  return xfer + endWriteValue();
}

std::uint32_t Json5ProtocolWriter::writeMapBegin(bool objectForm) {
  std::uint32_t xfer = beginWriteValue();
  auto form = objectForm ? CompoundTypeTracker::MapForm::Object
                         : CompoundTypeTracker::MapForm::KeyValueArray;
  containerStack_.beginMap(form);
  if (form == CompoundTypeTracker::MapForm::KeyValueArray) {
    return xfer + writer_.writeListBegin();
  }
  return xfer + writer_.writeObjectBegin();
}

std::uint32_t Json5ProtocolWriter::writeMapEnd() {
  auto* map = containerStack_.mapState();
  CHECK_THROW(map != nullptr, std::logic_error);

  // Map must end after a complete key-value pair (i.e., expecting next key).
  CHECK_THROW(
      map->expectingToken == CompoundTypeTracker::MapToken::Key,
      std::logic_error);

  auto encodedAs = map->encodedAs;
  containerStack_.endMap();
  std::uint32_t xfer = encodedAs == CompoundTypeTracker::MapForm::KeyValueArray
      ? writer_.writeListEnd()
      : writer_.writeObjectEnd();
  return xfer + endWriteValue();
}

template <class Tag, class T>
std::optional<std::uint32_t> Json5ProtocolWriter::maybeWriteSimpleMapKey(
    const T& value) {
  if (!containerStack_.inObjectMapExpectingKey()) {
    return std::nullopt;
  }

  if constexpr (std::is_same_v<Tag, type::string_t>) {
    return writer_.writeObjectName(value);
  } else {
    // For non-string keys, use Json5Protocol to stringify the value.
    return writer_.writeObjectName(toJsonImpl<Tag>(value, options_));
  }
}

std::uint32_t Json5ProtocolWriter::writeBool(bool value) {
  std::uint32_t xfer = beginWriteValue();
  if (auto size = maybeWriteSimpleMapKey<type::bool_t>(value)) {
    xfer += *size;
  } else {
    xfer += writer_.writeBool(value);
  }
  return xfer + endWriteValue();
}

std::uint32_t Json5ProtocolWriter::writeByte(int8_t value) {
  std::uint32_t xfer = beginWriteValue();
  if (auto size = maybeWriteSimpleMapKey<type::byte_t>(value)) {
    xfer += *size;
  } else {
    xfer += writer_.writeByte(value);
  }
  return xfer + endWriteValue();
}

std::uint32_t Json5ProtocolWriter::writeI16(int16_t value) {
  std::uint32_t xfer = beginWriteValue();
  if (auto size = maybeWriteSimpleMapKey<type::i16_t>(value)) {
    xfer += *size;
  } else {
    xfer += writer_.writeI16(value);
  }
  return xfer + endWriteValue();
}

std::uint32_t Json5ProtocolWriter::writeI32(int32_t value) {
  std::uint32_t xfer = beginWriteValue();
  if (auto size = maybeWriteSimpleMapKey<type::i32_t>(value)) {
    xfer += *size;
  } else {
    xfer += writer_.writeI32(value);
  }
  return xfer + endWriteValue();
}

std::uint32_t Json5ProtocolWriter::writeI64(int64_t value) {
  std::uint32_t xfer = beginWriteValue();
  if (auto size = maybeWriteSimpleMapKey<type::i64_t>(value)) {
    xfer += *size;
  } else {
    xfer += writer_.writeI64(value);
  }
  return xfer + endWriteValue();
}

std::optional<std::string_view> Json5ProtocolWriter::encodeNanInfAsString(
    std::floating_point auto f) const {
  if (options_.allowNanInf) {
    return {};
  }
  if (std::isnan(f)) {
    return std::signbit(f) ? "-NaN" : "NaN";
  }
  if (std::isinf(f)) {
    return f > 0 ? "Infinity" : "-Infinity";
  }
  return {};
}

std::uint32_t Json5ProtocolWriter::writeFloat(float value) {
  if (auto str = encodeNanInfAsString(value)) {
    return writeString(*str);
  }
  std::uint32_t xfer = beginWriteValue();
  if (auto size = maybeWriteSimpleMapKey<type::float_t>(value)) {
    xfer += *size;
  } else {
    xfer += writer_.writeFloat(value);
  }
  return xfer + endWriteValue();
}

std::uint32_t Json5ProtocolWriter::writeDouble(double value) {
  if (auto str = encodeNanInfAsString(value)) {
    return writeString(*str);
  }
  std::uint32_t xfer = beginWriteValue();
  if (auto size = maybeWriteSimpleMapKey<type::double_t>(value)) {
    xfer += *size;
  } else {
    xfer += writer_.writeDouble(value);
  }
  return xfer + endWriteValue();
}

std::uint32_t Json5ProtocolWriter::writeString(folly::StringPiece str) {
  std::uint32_t xfer = beginWriteValue();
  if (auto size = maybeWriteSimpleMapKey<type::string_t>(str)) {
    xfer += *size;
  } else {
    xfer += writer_.writeString(str);
  }
  return xfer + endWriteValue();
}

std::uint32_t Json5ProtocolWriter::writeBinary(folly::StringPiece str) {
  std::uint32_t xfer = beginWriteValue();
  xfer += writer_.writeObjectBegin();
  if (isValidUtf8(str)) {
    xfer += writer_.writeObjectName("utf-8");
    xfer += writer_.writeString(str);
  } else {
    xfer += writer_.writeObjectName("base64url");
    xfer += writer_.writeString(folly::base64URLEncode(str));
  }
  xfer += writer_.writeObjectEnd();
  return xfer + endWriteValue();
}

std::uint32_t Json5ProtocolWriter::writeBinary(folly::ByteRange str) {
  return writeBinary(folly::StringPiece(str));
}

std::uint32_t Json5ProtocolWriter::writeBinary(
    const std::unique_ptr<folly::IOBuf>& buf) {
  return writeBinary(buf ? buf->toString() : "");
}

std::uint32_t Json5ProtocolWriter::writeBinary(const folly::IOBuf& buf) {
  return writeBinary(buf.toString());
}

std::uint32_t Json5ProtocolWriter::writeEnum(
    std::string_view name, std::int32_t value) {
  return writeString(
      name.empty() ? fmt::format("({})", value)
                   : fmt::format("{} ({})", name, value));
}

} // namespace apache::thrift::json5::detail
