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

#include <thrift/lib/cpp2/op/Testing.h>

#include <folly/portability/GTest.h>

namespace apache {
namespace thrift {
namespace test {
using type::AnyValue;
using type::BaseType;
using type::Protocol;
using type::Ref;
using type::Type;

const Protocol kFollyToStringProtocol =
    Protocol::fromName(thriftType("FollyToString"));

const type::Protocol& Number1Serializer::getProtocol() const {
  static auto kInst = type::Protocol::fromName(thriftType("Number1"));
  return kInst;
}

void MultiSerializer::encode(
    type::ConstRef value, folly::io::QueueAppender&& appender) const {
  switch (value.type().base_type()) {
    case BaseType::I32:
      ++intEncCount;
      FollyToStringSerializer<type::i32_t>().encode(value, std::move(appender));
      break;
    case BaseType::Double:
      ++dblEncCount;
      FollyToStringSerializer<type::double_t>().encode(
          value, std::move(appender));
      break;
    default:
      folly::throw_exception<std::bad_any_cast>();
  }
}

void MultiSerializer::encode(
    const type::AnyValue& value, folly::io::QueueAppender&& appender) const {
  switch (value.type().base_type()) {
    case BaseType::I32:
      ++intEncCount;
      FollyToStringSerializer<type::i32_t>().encode(value, std::move(appender));
      break;
    case BaseType::Double:
      ++dblEncCount;
      FollyToStringSerializer<type::double_t>().encode(
          value, std::move(appender));
      break;
    default:
      folly::throw_exception<std::bad_any_cast>();
  }
}

void MultiSerializer::decode(
    const Type& type, folly::io::Cursor& cursor, AnyValue& value) const {
  // It is being decoded into an anyvalues.
  ++anyDecCount;
  switch (type.base_type()) {
    case BaseType::I32:
      ++intDecCount;
      FollyToStringSerializer<type::i32_t>().decode(type, cursor, value);
      break;
    case BaseType::Double:
      ++dblDecCount;
      FollyToStringSerializer<type::double_t>().decode(type, cursor, value);
      break;
    default:
      folly::throw_exception<std::bad_any_cast>();
  }
}

void MultiSerializer::decode(folly::io::Cursor& cursor, Ref value) const {
  switch (value.type().base_type()) {
    case BaseType::I32:
      ++intDecCount;
      FollyToStringSerializer<type::i32_t>().decode(cursor, value);
      break;
    case BaseType::Double:
      ++dblDecCount;
      FollyToStringSerializer<type::double_t>().decode(cursor, value);
      break;
    default:
      folly::throw_exception<std::bad_any_cast>();
  }
}

void MultiSerializer::checkAndResetInt(size_t enc, size_t dec) const {
  EXPECT_EQ(intEncCount, enc);
  EXPECT_EQ(intDecCount, dec);
  intEncCount = 0;
  intDecCount = 0;
}
void MultiSerializer::checkAndResetDbl(size_t enc, size_t dec) const {
  EXPECT_EQ(dblEncCount, enc);
  EXPECT_EQ(dblDecCount, dec);
  dblEncCount = 0;
  dblDecCount = 0;
}
void MultiSerializer::checkAndResetAny(size_t dec) const {
  EXPECT_EQ(anyDecCount, dec);
  anyDecCount = 0;
}
void MultiSerializer::checkAndResetAll() const {
  checkAndResetInt(0, 0);
  checkAndResetDbl(0, 0);
  checkAndResetAny(0);
}
void MultiSerializer::checkAnyDec() const {
  checkAndResetAny(1);
  checkAndResetAll();
}
void MultiSerializer::checkIntEnc() const {
  checkAndResetInt(1, 0);
  checkAndResetAll();
}
void MultiSerializer::checkIntDec() const {
  checkAndResetInt(0, 1);
  checkAndResetAll();
}
void MultiSerializer::checkDblEnc() const {
  checkAndResetDbl(1, 0);
  checkAndResetAll();
}
void MultiSerializer::checkDblDec() const {
  checkAndResetDbl(0, 1);
  checkAndResetAll();
}
void MultiSerializer::checkAnyIntDec() const {
  checkAndResetAny(1);
  checkAndResetInt(0, 1);
  checkAndResetAll();
}
void MultiSerializer::checkAnyDblDec() const {
  checkAndResetAny(1);
  checkAndResetDbl(0, 1);
  checkAndResetAll();
}

} // namespace test
} // namespace thrift
} // namespace apache
