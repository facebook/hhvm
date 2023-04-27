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

#include <thrift/lib/cpp2/util/DebugString.h>

#include <unordered_map>
#include <fmt/core.h>
#include <folly/MapUtil.h>
#include <folly/String.h>
#include <thrift/lib/cpp2/protocol/BinaryProtocol.h>
#include <thrift/lib/cpp2/protocol/CompactProtocol.h>

namespace apache {
namespace thrift {
namespace {

using apache::thrift::protocol::TProtocolException;
using apache::thrift::protocol::TType;

const char* typeLabel(TType fieldType) {
  switch (fieldType) {
    case TType::T_STOP:
      return "stop";
    case TType::T_BOOL:
      return "bool";
    case TType::T_BYTE:
      return "byte";
    case TType::T_I16:
      return "i16";
    case TType::T_I32:
      return "i32";
    case TType::T_I64:
      return "i64";
    case TType::T_DOUBLE:
      return "double";
    case TType::T_FLOAT:
      return "float";
    case TType::T_STRING:
      return "string";
    case TType::T_UTF8:
      return "utf8";
    case TType::T_STRUCT:
      return "struct";
    case TType::T_LIST:
      return "list";
    case TType::T_SET:
      return "set";
    case TType::T_MAP:
      return "map";
    default:
      break;
  }
  return "";
}

std::string lookupTypeStringIfEmpty(std::string typeStr, TType type) {
  return typeStr.empty() ? typeLabel(type) : typeStr;
}

folly::StringPiece trimAngleBrackets(folly::StringPiece str) {
  if (!str.empty() && str.front() == '<' && str.back() == '>') {
    str.pop_front();
    str.pop_back();
  }
  return str;
}

TType parseSimpleTypeLabel(folly::StringPiece label) {
  label = trimAngleBrackets(label);
  // if type is "map<K, V>", shorten to "map" for the lookup.
  label = label.subpiece(0, label.find('<'));
  static const std::unordered_map<folly::StringPiece, TType, folly::Hash>
      kTable = {
          {"stop", TType::T_STOP},
          {"bool", TType::T_BOOL},
          {"byte", TType::T_BYTE},
          {"i16", TType::T_I16},
          {"i32", TType::T_I32},
          {"i64", TType::T_I64},
          {"double", TType::T_DOUBLE},
          {"float", TType::T_FLOAT},
          {"string", TType::T_STRING},
          {"utf8", TType::T_UTF8},
          {"struct", TType::T_STRUCT},
          {"list", TType::T_LIST},
          {"set", TType::T_SET},
          {"map", TType::T_MAP}};
  return folly::get_default(kTable, label, TType::T_STOP);
}

folly::StringPiece getInnerTypeLabel(folly::StringPiece label) {
  label = trimAngleBrackets(label);
  // Remove any characters before the first '<'
  while (!label.empty() && label.front() != '<') {
    label.pop_front();
  }
  return label;
}

std::pair<folly::StringPiece, folly::StringPiece> splitMapKeyValueType(
    folly::StringPiece str) {
  str = trimAngleBrackets(str);
  int ltCount = 0;
  size_t i = 0;
  size_t sz = str.size();
  while (i < sz) {
    char c = str[i++];
    if (c == '<') {
      ltCount++;
    } else if (c == '>') {
      ltCount--;
    }
    if (c == ',' && ltCount == 0 && i > 0) {
      folly::StringPiece k = str.subpiece(0, i - 1);
      folly::StringPiece v = folly::ltrimWhitespace(str.subpiece(i + 1));
      return std::make_pair(k, v);
    }
  }
  folly::StringPiece k, v;
  return std::make_pair(k, v);
}

class Tokenizer {
 public:
  explicit Tokenizer(folly::StringPiece text) : text_(text) {}
  folly::StringPiece getNextToken();
  static bool isspace(char x) {
    return x == ' ' || x == '\n' || x == '\r' || x == '\t';
  }

 private:
  folly::StringPiece text_;
};

folly::StringPiece Tokenizer::getNextToken() {
  enum {
    INITIAL_WHITESPACE = 0,
    REGULAR_TOKEN, // anything not inside "" or inside <>
    IN_QUOTES, // inside ""
    IN_QUOTES_IN_ESCAPE, // inside "", and saw \ escape
    IN_LT_TAG, // inside <>, with ltCount unmatched <'s
    TOKEN_DONE
  };
  int8_t state = INITIAL_WHITESPACE;
  int8_t ltCount = 0;

  size_t tokenStartPos = 0, tokenEndPos = 0;
  size_t pos = 0;
  size_t sz = text_.size();
  while (pos < sz && state != TOKEN_DONE) {
    char c = text_[pos++];
    switch (state) {
      case INITIAL_WHITESPACE:
        if (isspace(c)) {
          continue;
        }
        tokenStartPos = pos - 1;
        if (c == '<') { // inmatched '<' tag
          state = IN_LT_TAG;
          ltCount++;
        } else if (c == '"') { // unmatched quote
          state = IN_QUOTES;
        } else if (c == '[' || c == '{' || c == '}' || c == ']') {
          tokenEndPos = pos;
          state = TOKEN_DONE;
        } else {
          state = REGULAR_TOKEN;
        }
        break;
      case REGULAR_TOKEN: // Anything not inside "" or <>
        if (c == '"' || isspace(c)) {
          // Stop the token on seeing "" or space.
          tokenEndPos = pos - 1;
          state = TOKEN_DONE;
        }
        if (c == '<') {
          // Keep attached <> as part of token, e.g. list<i32>
          state = IN_LT_TAG;
          ltCount++;
        }
        break;
      case IN_QUOTES:
        if (c == '\\') {
          state = IN_QUOTES_IN_ESCAPE;
        } else if (c == '"') {
          tokenEndPos = pos; // end token
          state = TOKEN_DONE;
        }
        break;
      case IN_QUOTES_IN_ESCAPE:
        state = IN_QUOTES;
        break;
      case IN_LT_TAG:
        if (c == '>' && --ltCount == 0) {
          tokenEndPos = pos; // end token
          state = TOKEN_DONE;
        } else if (c == '<') {
          ++ltCount;
        }
        break;
    }
  }
  if (state == REGULAR_TOKEN) {
    tokenEndPos = pos;
    state = TOKEN_DONE;
  }
  if (state != TOKEN_DONE) {
    text_.clear();
    return text_;
  }
  assert(tokenEndPos <= sz);
  folly::StringPiece ret =
      text_.subpiece(tokenStartPos, tokenEndPos - tokenStartPos);
  text_.advance(tokenEndPos);
  // clear out ','
  if (ret.endsWith(',')) {
    ret.pop_back();
  } else if (text_.startsWith(',')) {
    text_.pop_front();
  }
  return ret;
}

struct RenderState {
  DebugStringParams params;
  std::string indentStr;

  void incIndent() {
    if (!params.oneLine) {
      indentStr.append(params.indentAmount, ' ');
    }
  }
  void decIndent() {
    if (!params.oneLine) {
      assert(indentStr.size() >= params.indentAmount);
      indentStr.resize(indentStr.size() - params.indentAmount);
    }
  }
  void wrapLine(std::string* outVal) const {
    if (params.oneLine) {
      *outVal += ' ';
    } else {
      *outVal += fmt::format("\n{}", indentStr);
    }
  }
};

template <class ProtocolReader>
void toDebugStringForType(
    TType fieldType,
    ProtocolReader* inProtoReader,
    RenderState& rs,
    std::string* outType,
    std::string* outVal) {
  switch (fieldType) {
    case TType::T_BOOL: {
      bool val;
      inProtoReader->readBool(val);
      *outType = "bool";
      *outVal = val ? "true" : "false";
      break;
    }
    case TType::T_BYTE: {
      int8_t val;
      inProtoReader->readByte(val);
      *outType = "byte";
      *outVal = fmt::format("{}", val);
      break;
    }
    case TType::T_I16: {
      int16_t val;
      inProtoReader->readI16(val);
      *outType = "i16";
      *outVal = fmt::format("{}", val);
      break;
    }
    case TType::T_I32: {
      int32_t val;
      inProtoReader->readI32(val);
      *outType = "i32";
      *outVal = fmt::format("{}", val);
      break;
    }
    case TType::T_I64: {
      int64_t val;
      inProtoReader->readI64(val);
      *outType = "i64";
      *outVal = fmt::format("{}", val);
      break;
    }
    case TType::T_DOUBLE: {
      double val;
      inProtoReader->readDouble(val);
      *outType = "double";
      *outVal = fmt::format("{}", val);
      return;
    }
    case TType::T_FLOAT: {
      float val;
      inProtoReader->readFloat(val);
      *outType = "float";
      *outVal = fmt::format("{}", val);
      return;
    }
    case TType::T_STRING:
    case TType::T_UTF8: {
      std::string val, valOut;
      inProtoReader->readString(val);
      folly::humanify(val, valOut);
      *outType = "string";
      *outVal = fmt::format("\"{}\"", valOut);
      break;
    }
    case TType::T_STRUCT: {
      std::string nameIgnored;
      inProtoReader->readStructBegin(nameIgnored);
      *outVal = "{";
      rs.incIndent();
      TType elemType;
      int16_t fieldId;
      std::string valStr;
      for (;;) {
        inProtoReader->readFieldBegin(nameIgnored, elemType, fieldId);
        if (elemType == TType::T_STOP) {
          break;
        }
        toDebugStringForType(elemType, inProtoReader, rs, outType, &valStr);
        rs.wrapLine(outVal);
        *outVal += fmt::format("{}: {} = {}", fieldId, *outType, valStr);
        inProtoReader->readFieldEnd();
      }
      inProtoReader->readStructEnd();
      rs.decIndent();
      rs.wrapLine(outVal);
      *outVal += '}';
      *outType = "struct";
      break;
    }
    case TType::T_SET:
    case TType::T_LIST: {
      TType elemTypeT;
      uint32_t size = 0;
      if (fieldType == TType::T_LIST) {
        inProtoReader->readListBegin(elemTypeT, size);
        *outVal = "[";
      } else {
        inProtoReader->readSetBegin(elemTypeT, size);
        *outVal = "{";
      }
      rs.incIndent();
      std::string valStr, valTypeS;
      std::string* valTypeP = outType ? &valTypeS : nullptr;

      constexpr int kMaxLineTarget = 80;
      int lineLen = kMaxLineTarget;
      for (uint32_t num = 0; num < size; ++num) {
        toDebugStringForType(elemTypeT, inProtoReader, rs, valTypeP, &valStr);
        if (!rs.params.oneLine && lineLen + valStr.size() >= kMaxLineTarget) {
          rs.wrapLine(outVal);
          *outVal += fmt::format("{},", valStr);
          lineLen = rs.indentStr.size() + valStr.size();
        } else {
          *outVal += fmt::format(" {},", valStr);
          lineLen += valStr.size() + strlen(" ,");
        }
      }
      const char* thisType;
      const char* finalChar;
      if (fieldType == TType::T_LIST) {
        inProtoReader->readListEnd();
        thisType = "list";
        finalChar = "]";
      } else {
        inProtoReader->readSetEnd();
        thisType = "set";
        finalChar = "}";
      }
      rs.decIndent();
      rs.wrapLine(outVal);
      *outVal += finalChar;
      *outType = fmt::format(
          "{}<{}>", thisType, lookupTypeStringIfEmpty(valTypeS, elemTypeT));
      break;
    }
    case TType::T_MAP: {
      TType keyTypeT, valTypeT;
      uint32_t size = 0;
      inProtoReader->readMapBegin(keyTypeT, valTypeT, size);
      *outVal = "{";
      rs.incIndent();
      std::string keyStr, valStr, keyTypeS, valTypeS;
      std::string* keyTypeP = outType ? &keyTypeS : nullptr;
      std::string* valTypeP = outType ? &valTypeS : nullptr;

      for (uint32_t num = 0; num < size; ++num) {
        toDebugStringForType(keyTypeT, inProtoReader, rs, keyTypeP, &keyStr);
        toDebugStringForType(valTypeT, inProtoReader, rs, valTypeP, &valStr);
        rs.wrapLine(outVal);
        *outVal += fmt::format("{} : {},", keyStr, valStr);
      }
      inProtoReader->readMapEnd();
      rs.decIndent();
      rs.wrapLine(outVal);
      *outVal += "}";
      *outType = fmt::format(
          "map<{}, {}>",
          lookupTypeStringIfEmpty(keyTypeS, keyTypeT),
          lookupTypeStringIfEmpty(valTypeS, valTypeT));
      break;
    }
    default:
      TProtocolException::throwInvalidSkipType(fieldType);
      break;
  }
}

template <class ProtocolWriter>
void parseField(
    TType expectedType,
    folly::StringPiece extendedExpectedType,
    Tokenizer& tokenizer,
    ProtocolWriter* outProtoWriter) {
  auto getNextOrThrow = [&]() -> folly::StringPiece {
    auto sp = tokenizer.getNextToken();
    if (sp.empty()) {
      throw TProtocolException(
          TProtocolException::INVALID_DATA, "Unexpected end of data");
    }
    return sp;
  };

  switch (expectedType) {
    case TType::T_BOOL: {
      auto tok = getNextOrThrow();
      if (tok == "true") {
        if (outProtoWriter) {
          outProtoWriter->writeBool(true);
        }
      } else if (tok == "false") {
        if (outProtoWriter) {
          outProtoWriter->writeBool(false);
        }
      } else {
        throw TProtocolException(
            TProtocolException::INVALID_DATA,
            fmt::format("Bad bool token {}", tok));
      }
      break;
    }
    case TType::T_BYTE: {
      int8_t val = folly::to<int8_t>(getNextOrThrow());
      if (outProtoWriter) {
        outProtoWriter->writeByte(val);
      }
      break;
    }
    case TType::T_I16: {
      int16_t val = folly::to<int16_t>(getNextOrThrow());
      if (outProtoWriter) {
        outProtoWriter->writeI16(val);
      }
      break;
    }
    case TType::T_I32: {
      int32_t val = folly::to<int32_t>(getNextOrThrow());
      if (outProtoWriter) {
        outProtoWriter->writeI32(val);
      }
      break;
    }
    case TType::T_I64: {
      int64_t val = folly::to<int64_t>(getNextOrThrow());
      if (outProtoWriter) {
        outProtoWriter->writeI64(val);
      }
      break;
    }
    case TType::T_DOUBLE: {
      double val = folly::to<double>(getNextOrThrow());
      if (outProtoWriter) {
        outProtoWriter->writeDouble(val);
      }
      break;
    }
    case TType::T_FLOAT: {
      float val = folly::to<float>(getNextOrThrow());
      if (outProtoWriter) {
        outProtoWriter->writeFloat(val);
      }
      break;
    }
    case TType::T_STRING: {
      auto tok = getNextOrThrow();
      if (tok.size() < 2 || tok.front() != '"' || tok.back() != '"') {
        throw TProtocolException(
            TProtocolException::INVALID_DATA,
            fmt::format("Bad string token {}", tok));
      }
      tok.pop_back();
      tok.pop_front();
      std::string unescaped;
      folly::cUnescape(tok, unescaped);
      if (outProtoWriter) {
        outProtoWriter->writeString(unescaped);
      }
      break;
    }
    case TType::T_STRUCT: {
      auto tok = getNextOrThrow();
      if (tok == "struct") {
        tok = getNextOrThrow();
      }
      if (tok != "{") {
        throw TProtocolException(
            TProtocolException::INVALID_DATA,
            fmt::format("Expected struct '{{' at {}", tok));
      }
      if (outProtoWriter) {
        outProtoWriter->writeStructBegin("");
      }
      for (;;) {
        tok = getNextOrThrow();
        if (tok == "}") {
          if (outProtoWriter) {
            outProtoWriter->writeFieldStop();
            outProtoWriter->writeStructEnd();
          }
          break;
        }
        if (tok.back() != ':') {
          throw TProtocolException(
              TProtocolException::INVALID_DATA,
              fmt::format("Expected tag: {}", tok));
        }
        tok.pop_back();
        int16_t tagnum = folly::to<int16_t>(tok);
        folly::StringPiece fullType = getNextOrThrow();
        TType typeId = parseSimpleTypeLabel(fullType);
        if (typeId == TType::T_STOP) {
          throw TProtocolException(
              TProtocolException::INVALID_DATA,
              fmt::format("Unexpected type {}", fullType));
        }
        if (getNextOrThrow() != "=") {
          throw TProtocolException(
              TProtocolException::INVALID_DATA, "Missing = delim in struct");
        }
        if (outProtoWriter) {
          outProtoWriter->writeFieldBegin("", typeId, tagnum);
        }
        parseField(typeId, fullType, tokenizer, outProtoWriter);
        if (outProtoWriter) {
          outProtoWriter->writeFieldEnd();
        }
      }
      break;
    }
    case TType::T_LIST:
    case TType::T_SET: {
      folly::StringPiece elemTypeStr = getInnerTypeLabel(extendedExpectedType);
      TType elemTypeId = parseSimpleTypeLabel(elemTypeStr);
      if (elemTypeId == TType::T_STOP) {
        throw TProtocolException(
            TProtocolException::INVALID_DATA,
            fmt::format("Cannot parse element type in {}", elemTypeStr));
      }
      folly::StringPiece openDelim, closeDelim;
      if (expectedType == TType::T_LIST) {
        openDelim = "[";
        closeDelim = "]";
      } else {
        openDelim = "{";
        closeDelim = "}";
      }
      if (getNextOrThrow() != openDelim) {
        throw TProtocolException(
            TProtocolException::INVALID_DATA, "Missing open delimiter");
      }

      // Count the number of elements. Unfortunately, the involves parsing the
      // data. We parse to a null writer, and copy the tokenizer.
      size_t numElems = 0;
      auto tokenizerCopy = tokenizer; // cheap
      for (;;) {
        auto peekTokenizer = tokenizerCopy;
        auto delim = peekTokenizer.getNextToken();
        if (delim.empty()) {
          throw TProtocolException(
              TProtocolException::INVALID_DATA, "Premature end of stream");
        }
        if (delim == closeDelim) {
          break;
        }
        parseField(
            elemTypeId,
            elemTypeStr,
            tokenizerCopy,
            static_cast<ProtocolWriter*>(nullptr));
        numElems++;
      }

      if (outProtoWriter) {
        if (expectedType == TType::T_LIST) {
          outProtoWriter->writeListBegin(elemTypeId, numElems);
        } else {
          outProtoWriter->writeSetBegin(elemTypeId, numElems);
        }
      }

      // Parse the actual elements
      for (size_t i = 0; i < numElems; ++i) {
        parseField(elemTypeId, elemTypeStr, tokenizer, outProtoWriter);
      }

      if (getNextOrThrow() != closeDelim) {
        throw TProtocolException(
            TProtocolException::INVALID_DATA, "Missing closing delimiter");
      }

      if (outProtoWriter) {
        if (expectedType == TType::T_LIST) {
          outProtoWriter->writeListEnd();
        } else {
          outProtoWriter->writeSetEnd();
        }
      }
      break;
    }

    case TType::T_MAP: {
      std::pair<folly::StringPiece, folly::StringPiece> keyValSplit =
          splitMapKeyValueType(getInnerTypeLabel(extendedExpectedType));
      TType keyTypeId = parseSimpleTypeLabel(keyValSplit.first);
      TType valTypeId = parseSimpleTypeLabel(keyValSplit.second);
      if (keyTypeId == TType::T_STOP || valTypeId == TType::T_STOP) {
        throw TProtocolException(
            TProtocolException::INVALID_DATA,
            fmt::format("Cannot parse map types: {}", extendedExpectedType));
      }
      if (getNextOrThrow() != "{") {
        throw TProtocolException(
            TProtocolException::INVALID_DATA, "Missing { delimiter in map");
      }

      // Need to count the elements, which requires a pre-pass.
      // Parse with a tokenizerCopy and discard the results.
      auto tokenizerCopy = tokenizer; // cheap
      size_t numElems = 0;
      for (;;) {
        auto peekTokenizer = tokenizerCopy;
        auto delim = peekTokenizer.getNextToken();
        if (delim.empty()) {
          throw TProtocolException(
              TProtocolException::INVALID_DATA, "Premature end of stream");
        }
        if (delim == "}") {
          break;
        }
        parseField(
            keyTypeId,
            keyValSplit.first,
            tokenizerCopy,
            static_cast<ProtocolWriter*>(nullptr));
        if (tokenizerCopy.getNextToken() != ":") {
          throw TProtocolException(
              TProtocolException::INVALID_DATA, "Missing : delimiter");
        }
        parseField(
            valTypeId,
            keyValSplit.second,
            tokenizerCopy,
            static_cast<ProtocolWriter*>(nullptr));
        numElems++;
      }

      if (outProtoWriter) {
        outProtoWriter->writeMapBegin(keyTypeId, valTypeId, numElems);
      }

      // Now, go back and actually parse the map.
      for (size_t i = 0; i < numElems; ++i) {
        parseField(keyTypeId, keyValSplit.first, tokenizer, outProtoWriter);
        if (getNextOrThrow() != ":") {
          throw TProtocolException(
              TProtocolException::INVALID_DATA, "Missing : delimiter");
        }
        parseField(valTypeId, keyValSplit.second, tokenizer, outProtoWriter);
      }
      if (getNextOrThrow() != "}") {
        throw TProtocolException(
            TProtocolException::INVALID_DATA, "Missing } delimiter");
      }
      if (outProtoWriter) {
        outProtoWriter->writeMapEnd();
      }
      break;
    }
    default:
      TProtocolException::throwInvalidSkipType(expectedType);
      break;
  }
}
} // namespace

template <class ProtocolReader>
std::string toDebugString(
    ProtocolReader& inProtoReader, DebugStringParams params) {
  RenderState rs;
  rs.params = params;
  std::string outType, outVal;
  toDebugStringForType(
      protocol::TType::T_STRUCT, &inProtoReader, rs, &outType, &outVal);
  return fmt::format("{} {}", outType, outVal);
}

template <class ProtocolWriter>
void fromDebugString(folly::StringPiece text, ProtocolWriter& outProtoWriter) {
  Tokenizer tok(text);
  parseField(protocol::TType::T_STRUCT, "", tok, &outProtoWriter);
}

// Explicit instantiations.
// Currently, we instantiate Compact and Binary, but we could instantiate
// others if it makes sense.
template std::string toDebugString<CompactProtocolReader>(
    CompactProtocolReader& inProtoReader, DebugStringParams p);
template std::string toDebugString<BinaryProtocolReader>(
    BinaryProtocolReader& inProtoReader, DebugStringParams p);
template void fromDebugString<class CompactProtocolWriter>(
    folly::StringPiece text, CompactProtocolWriter& outProtoWriter);
template void fromDebugString<class BinaryProtocolWriter>(
    folly::StringPiece text, BinaryProtocolWriter& outProtoWriter);

} // namespace thrift
} // namespace apache
