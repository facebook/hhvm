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

#include <thrift/lib/cpp2/protocol/test/Json5ToDynamic.h>

#include <folly/Overload.h>
#include <folly/io/IOBuf.h>

#include <thrift/lib/cpp2/protocol/detail/JsonReader.h>

namespace apache::thrift::json5::detail {

static folly::dynamic json5ToDynamic(Json5Reader& r) {
  switch (r.peekToken()) {
    case Json5Reader::Token::ObjectBegin: {
      folly::dynamic obj = folly::dynamic::object();
      r.readObjectBegin();
      while (r.peekToken() != Json5Reader::Token::ObjectEnd) {
        auto key = r.readObjectName();
        // When there are duplicated keys in a json object, today
        // folly::parseJson uses the last value. We mimic the behavior here.
        obj[key] = json5ToDynamic(r);
      }
      r.readObjectEnd();
      return obj;
    }
    case Json5Reader::Token::ListBegin: {
      folly::dynamic arr = folly::dynamic::array();
      r.readListBegin();
      while (r.peekToken() != Json5Reader::Token::ListEnd) {
        arr.push_back(json5ToDynamic(r));
      }
      r.readListEnd();
      return arr;
    }
    case Json5Reader::Token::Primitive:
      return folly::variant_match(
          r.readPrimitive(Json5Reader::FloatingPointPrecision::Double),
          [](std::monostate) -> folly::dynamic { return nullptr; },
          [](auto&& v) -> folly::dynamic { return std::move(v); });
    case Json5Reader::Token::ListEnd:
    case Json5Reader::Token::ObjectEnd:
    default:
      throw std::runtime_error("json5ToFollyDynamic: unexpected token");
  }
}

folly::dynamic json5ToDynamic(std::string_view input) {
  auto buf = folly::IOBuf::copyBuffer(input);
  Json5Reader reader;
  reader.setCursor(folly::io::Cursor(buf.get()));
  return json5ToDynamic(reader);
}

} // namespace apache::thrift::json5::detail
