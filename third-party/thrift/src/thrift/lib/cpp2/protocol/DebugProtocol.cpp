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

#include <thrift/lib/cpp2/protocol/DebugProtocol.h>

#include <folly/Conv.h>
#include <folly/String.h>

FOLLY_GFLAGS_DEFINE_bool(
    thrift_cpp2_debug_skip_list_indices,
    false,
    "Wether to skip indices when debug-printing lists (unless overridden)");
FOLLY_GFLAGS_DEFINE_int64(
    thrift_cpp2_debug_string_limit,
    256,
    "Limit on string size when debug-printing thrift, 0 is no limit");

namespace apache::thrift {

DebugProtocolWriter::DebugProtocolWriter(
    ExternalBufferSharing /*sharing*/, Options options)
    : out_(nullptr, 0), options_(options) {}

namespace {

std::string fieldTypeName(TType type) {
  switch (type) {
    case TType::T_STOP:
      return "stop";
    case TType::T_VOID:
      return "void";
    case TType::T_BOOL:
      return "bool";
    case TType::T_BYTE:
      return "byte";
    case TType::T_I16:
      return "i16";
    case TType::T_I32:
      return "i32";
    case TType::T_U64:
      return "u64";
    case TType::T_I64:
      return "i64";
    case TType::T_DOUBLE:
      return "double";
    case TType::T_FLOAT:
      return "float";
    case TType::T_STRING:
      return "string";
    case TType::T_STRUCT:
      return "struct";
    case TType::T_MAP:
      return "map";
    case TType::T_SET:
      return "set";
    case TType::T_LIST:
      return "list";
    case TType::T_UTF8:
      return "utf8";
    case TType::T_UTF16:
      return "utf16";
    case TType::T_STREAM:
      return "stream";
    default:
      return fmt::format("unknown({})", int(type));
  }
}

const size_t kIndent = 2;

} // namespace

void DebugProtocolWriter::indentUp() {
  indent_.append(kIndent, ' ');
}

void DebugProtocolWriter::indentDown() {
  CHECK_GE(indent_.size(), kIndent);
  indent_.erase(indent_.size() - kIndent);
}

void DebugProtocolWriter::pushState(ItemType t) {
  indentUp();
  writeState_.emplace_back(t);
}

void DebugProtocolWriter::popState() {
  CHECK(!writeState_.empty());
  writeState_.pop_back();
  indentDown();
}

void DebugProtocolWriter::startItem() {
  if (writeState_.empty()) { // top level
    return;
  }
  auto& ws = writeState_.back();
  switch (ws.type) {
    case STRUCT:
      break;
    case SET:
    case MAP_KEY:
      writeIndent();
      break;
    case MAP_VALUE:
      writePlain(" -> ");
      break;
    case LIST:
      if (options_.skipListIndices) {
        writeIndent();
      } else {
        writeIndented("[{}] = ", ws.index);
      }
      break;
  }
}

void DebugProtocolWriter::endItem() {
  if (writeState_.empty()) { // top level
    return;
  }
  auto& ws = writeState_.back();
  ++ws.index;
  switch (ws.type) {
    case LIST:
    case STRUCT:
    case SET:
      writePlain(",\n");
      break;
    case MAP_KEY:
      ws.type = MAP_VALUE;
      break;
    case MAP_VALUE:
      ws.type = MAP_KEY;
      writePlain(",\n");
  }
}

void DebugProtocolWriter::setOutput(
    folly::IOBufQueue* storage, size_t maxGrowth) {
  // Allocate 16KB at a time; leave some room for the IOBuf overhead
  constexpr size_t kDesiredGrowth = (1 << 14) - 64;
  out_.reset(storage, std::min(kDesiredGrowth, maxGrowth));
}

void DebugProtocolWriter::setOutput(folly::io::QueueAppender&& output) {
  out_ = std::move(output);
}

uint32_t DebugProtocolWriter::writeMessageBegin(
    const std::string& name, MessageType messageType, int32_t /*seqid*/) {
  std::string mtype;
  switch (messageType) {
    case MessageType::T_CALL:
      mtype = "call";
      break;
    case MessageType::T_REPLY:
      mtype = "reply";
      break;
    case MessageType::T_EXCEPTION:
      mtype = "exn";
      break;
    case MessageType::T_ONEWAY:
      mtype = "oneway";
      break;
  }

  writeIndented("({}) {}(", mtype, name);
  indentUp();
  return 0;
}

uint32_t DebugProtocolWriter::writeMessageEnd() {
  indentDown();
  writeIndented(")\n");
  return 0;
}

uint32_t DebugProtocolWriter::writeStructBegin(const char* name) {
  startItem();
  writePlain("{} {{\n", name);
  pushState(STRUCT);
  return 0;
}

uint32_t DebugProtocolWriter::writeStructEnd() {
  popState();
  writeIndented("}}");
  endItem();
  return 0;
}

uint32_t DebugProtocolWriter::writeFieldBegin(
    const char* name, TType fieldType, int16_t fieldId) {
  writeIndent();
  if (!options_.skipFieldId) {
    writePlain("{:0d}: ", fieldId);
  }
  writePlain("{}", name);
  if (!options_.skipFieldType) {
    writePlain(" ({})", fieldTypeName(fieldType));
  }
  writePlain(" = ");
  return 0;
}

uint32_t DebugProtocolWriter::writeFieldEnd() {
  return 0;
}
uint32_t DebugProtocolWriter::writeFieldStop() {
  return 0;
}

uint32_t DebugProtocolWriter::writeMapBegin(
    TType keyType, TType valueType, uint32_t size) {
  startItem();
  writePlain(
      "map<{},{}>[{}] {{\n",
      fieldTypeName(keyType),
      fieldTypeName(valueType),
      size);
  pushState(MAP_KEY);
  return 0;
}

uint32_t DebugProtocolWriter::writeMapEnd() {
  popState();
  writeIndented("}}");
  endItem();
  return 0;
}

uint32_t DebugProtocolWriter::writeListBegin(TType elemType, uint32_t size) {
  startItem();
  writePlain("list<{}>[{}] {{\n", fieldTypeName(elemType), size);
  pushState(LIST);
  return 0;
}

uint32_t DebugProtocolWriter::writeListEnd() {
  popState();
  writeIndented("}}");
  endItem();
  return 0;
}

uint32_t DebugProtocolWriter::writeSetBegin(TType elemType, uint32_t size) {
  startItem();
  writePlain("set<{}>[{}] {{\n", fieldTypeName(elemType), size);
  pushState(SET);
  return 0;
}

uint32_t DebugProtocolWriter::writeSetEnd() {
  popState();
  writeIndented("}}");
  endItem();
  return 0;
}

uint32_t DebugProtocolWriter::writeBool(bool v) {
  writeItem("{}", v);
  return 0;
}

uint32_t DebugProtocolWriter::writeByte(int8_t v) {
  writeItem("0x{:x}", uint8_t(v));
  return 0;
}

uint32_t DebugProtocolWriter::writeI16(int16_t v) {
  writeItem("{}", v);
  return 0;
}

uint32_t DebugProtocolWriter::writeI32(int32_t v) {
  writeItem("{}", v);
  return 0;
}

uint32_t DebugProtocolWriter::writeI64(int64_t v) {
  writeItem("{}", v);
  return 0;
}

uint32_t DebugProtocolWriter::writeFloat(float v) {
  writeItem("{}", v);
  return 0;
}

uint32_t DebugProtocolWriter::writeDouble(double v) {
  writeItem("{}", v);
  return 0;
}

uint32_t DebugProtocolWriter::writeString(folly::StringPiece str) {
  return writeBinary(str);
}

uint32_t DebugProtocolWriter::writeBinary(folly::StringPiece str) {
  return writeBinary(folly::ByteRange(str));
}

uint32_t DebugProtocolWriter::writeBinary(folly::ByteRange v) {
  writeByteRange(v);
  return 0;
}

uint32_t DebugProtocolWriter::writeBinary(
    const std::unique_ptr<folly::IOBuf>& str) {
  if (str) {
    writeByteRange(folly::ByteRange(str->clone()->coalesce()));
  }
  return 0;
}

uint32_t DebugProtocolWriter::writeBinary(const folly::IOBuf& str) {
  writeByteRange(folly::ByteRange(str.clone()->coalesce()));
  return 0;
}

uint32_t DebugProtocolWriter::serializedMessageSize(
    const std::string& /*name*/) {
  return 0;
}

uint32_t DebugProtocolWriter::serializedFieldSize(
    const char* /*name*/, TType /*fieldName*/, int16_t /*fieldId*/) {
  return 0;
}

uint32_t DebugProtocolWriter::serializedStructSize(const char* /*name*/) {
  return 0;
}

uint32_t DebugProtocolWriter::serializedSizeMapBegin(
    TType /*keyType*/, TType /*valType*/, uint32_t /*size*/) {
  return 0;
}

uint32_t DebugProtocolWriter::serializedSizeMapEnd() {
  return 0;
}

uint32_t DebugProtocolWriter::serializedSizeListBegin(
    TType /*elemType*/, uint32_t /*size*/) {
  return 0;
}

uint32_t DebugProtocolWriter::serializedSizeListEnd() {
  return 0;
}

uint32_t DebugProtocolWriter::serializedSizeSetBegin(
    TType /*elemType*/, uint32_t /*size*/) {
  return 0;
}

uint32_t DebugProtocolWriter::serializedSizeSetEnd() {
  return 0;
}

uint32_t DebugProtocolWriter::serializedSizeStop() {
  return 0;
}

uint32_t DebugProtocolWriter::serializedSizeBool(bool /*value*/) {
  return 0;
}

uint32_t DebugProtocolWriter::serializedSizeByte(int8_t /*byte*/) {
  return 0;
}

uint32_t DebugProtocolWriter::serializedSizeI16(int16_t /*i16*/) {
  return 0;
}

uint32_t DebugProtocolWriter::serializedSizeI32(int32_t /*i32*/) {
  return 0;
}

uint32_t DebugProtocolWriter::serializedSizeI64(int64_t /*i64*/) {
  return 0;
}

uint32_t DebugProtocolWriter::serializedSizeDouble(double /*dub*/) {
  return 0;
}

uint32_t DebugProtocolWriter::serializedSizeFloat(float /*flt*/) {
  return 0;
}

uint32_t DebugProtocolWriter::serializedSizeString(folly::StringPiece str) {
  return serializedSizeBinary(str);
}

uint32_t DebugProtocolWriter::serializedSizeBinary(folly::StringPiece str) {
  return serializedSizeBinary(folly::ByteRange(str));
}

uint32_t DebugProtocolWriter::serializedSizeBinary(folly::ByteRange) {
  return 0;
}

uint32_t DebugProtocolWriter::serializedSizeBinary(
    const std::unique_ptr<folly::IOBuf>&) {
  return 0;
}

uint32_t DebugProtocolWriter::serializedSizeBinary(const folly::IOBuf&) {
  return 0;
}

uint32_t DebugProtocolWriter::serializedSizeZCBinary(folly::StringPiece str) {
  return serializedSizeBinary(folly::ByteRange(str));
}

uint32_t DebugProtocolWriter::serializedSizeZCBinary(folly::ByteRange) {
  return 0;
}

uint32_t DebugProtocolWriter::serializedSizeZCBinary(
    const std::unique_ptr<folly::IOBuf>&) {
  return 0;
}

uint32_t DebugProtocolWriter::serializedSizeZCBinary(const folly::IOBuf&) {
  return 0;
}

void DebugProtocolWriter::writeByteRange(folly::ByteRange v) {
  static constexpr size_t kStringPrefixSize = 128;
  const size_t limit = options_.stringLengthLimit;

  auto str = folly::StringPiece(v);

  folly::StringPiece toShow = limit != 0 && str.size() > limit
      ? str.subpiece(0, std::min(kStringPrefixSize, limit))
      : str;
  std::string printed = folly::cEscape<std::string>(toShow);
  if (toShow.size() < str.size()) {
    folly::toAppend("[...](", str.size(), ")", &printed);
  }

  writeItem("\"{}\"", printed);
}

} // namespace apache::thrift
