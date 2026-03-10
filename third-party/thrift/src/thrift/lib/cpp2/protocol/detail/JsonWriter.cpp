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

#include <thrift/lib/cpp2/protocol/detail/JsonWriter.h>

#include <cstring>
#include <fmt/format.h>
#include <re2/re2.h>
#include <folly/Conv.h>
#include <folly/Exception.h>
#include <folly/json.h>

namespace apache::thrift::json5::detail {
namespace {
bool isValidJavascriptIdentifier(std::string_view s) {
  static const RE2 regex("^[a-zA-Z_$][a-zA-Z0-9_$]*$");
  return RE2::FullMatch(s, regex);
}
} // namespace

// -- helpers ----------------------------------------------------------

bool JsonWriter::inContainer(ContainerType type) const {
  return !containerStack_.empty() && containerStack_.back() == type;
}

std::uint32_t JsonWriter::appendNewlineAndIndent() {
  std::uint32_t n = 0;
  if (options_.indentWidth > 0) {
    out_.value().write('\n');
    n += 1;
    size_t spaces = containerStack_.size() * options_.indentWidth;
    out_.value().ensure(spaces);
    memset(out_.value().writableData(), ' ', spaces);
    out_.value().append(spaces);
    n += spaces;
  }
  return n;
}

std::uint32_t JsonWriter::appendComma() {
  if (!containerStack_.empty()) {
    out_.value().write(',');
    return 1;
  }
  return 0;
}

char JsonWriter::back() {
  return out_.value().tail<folly::io::CursorAccess::PRIVATE>(1).read<char>();
}

char JsonWriter::getOpenBracket(ContainerType type) {
  if (type == ContainerType::List) {
    return '[';
  }
  CHECK_THROW(type == ContainerType::Object, std::logic_error);
  return '{';
}
char JsonWriter::getCloseBracket(ContainerType type) {
  if (type == ContainerType::List) {
    return ']';
  }
  CHECK_THROW(type == ContainerType::Object, std::logic_error);
  return '}';
}

bool JsonWriter::trailingComma(ContainerType type) const {
  if (type == ContainerType::List) {
    return options_.listTrailingComma;
  }
  CHECK_THROW(type == ContainerType::Object, std::logic_error);
  return options_.objectTrailingComma;
}

// Called before writing any JSON value to insert indentation if needed.
std::uint32_t JsonWriter::beginValue() {
  std::uint32_t n = 0;
  if (inContainer(ContainerType::List)) {
    // Only add indentation when writing value in a list. Why? If it's in a
    // object, we should indent when writing object name, not when writing
    // object value.
    n += appendNewlineAndIndent();
  } else if (inContainer(ContainerType::Object)) {
    CHECK_THROW(lastWrittenValueWasObjectName_, std::runtime_error);
    lastWrittenValueWasObjectName_ = false;
  }
  return n;
}

// -- containers -------------------------------------------------------

std::uint32_t JsonWriter::openContainer(ContainerType type) {
  std::uint32_t n = 0;
  n += beginValue();
  containerStack_.push_back(type);
  out_.value().write(getOpenBracket(type));
  n += 1;
  return n;
}

std::uint32_t JsonWriter::closeContainer(ContainerType type) {
  // Verify the container type matches the corresponding openContainer call.
  CHECK_THROW(inContainer(type), std::runtime_error);

  // A container must not be closed immediately after an object name (a value
  // is expected first).
  CHECK_THROW(!lastWrittenValueWasObjectName_, std::runtime_error);

  CHECK_THROW(
      back() == ',' || back() == getOpenBracket(type), std::logic_error);
  std::int64_t n = 0;
  containerStack_.pop_back();
  if (!trailingComma(type) && back() == ',') {
    out_.value().trimEnd(1);
    n -= 1;
  }
  if (back() != getOpenBracket(type)) {
    n += appendNewlineAndIndent();
  }
  out_.value().write(getCloseBracket(type));
  n += 1;
  n += appendComma();
  return n;
}

// -- scalars ----------------------------------------------------------

template <std::floating_point T>
std::uint32_t JsonWriter::writeFloatingPoint(T t) {
  CHECK_THROW(options_.allowNanInf || std::isfinite(t), std::invalid_argument);
  std::uint32_t n = 0;
  n += beginValue();
  constexpr auto mode = std::is_same_v<T, float>
      ? folly::DtoaMode::SHORTEST_SINGLE
      : folly::DtoaMode::SHORTEST;
  if (std::isnan(t) && std::signbit(t)) {
    constexpr folly::StringPiece signedNan = "-NaN";
    n += signedNan.size();
    out_.value().push(signedNan);
  } else {
    std::string tmp;
    folly::toAppend(
        t,
        &tmp,
        mode,
        0,
        folly::DtoaFlags::EMIT_TRAILING_DECIMAL_POINT |
            folly::DtoaFlags::EMIT_TRAILING_ZERO_AFTER_POINT);
    out_.value().push(folly::StringPiece(tmp));
    n += tmp.size();
  }
  n += appendComma();
  return n;
}

template std::uint32_t JsonWriter::writeFloatingPoint(float);
template std::uint32_t JsonWriter::writeFloatingPoint(double);

std::uint32_t JsonWriter::writeIntegral(std::integral auto t) {
  std::uint32_t n = 0;
  n += beginValue();
  auto s = fmt::format("{}", t);
  out_.value().push(folly::StringPiece(s));
  n += s.size();
  n += appendComma();
  return n;
}

template std::uint32_t JsonWriter::writeIntegral(bool);
template std::uint32_t JsonWriter::writeIntegral(std::int8_t);
template std::uint32_t JsonWriter::writeIntegral(std::int16_t);
template std::uint32_t JsonWriter::writeIntegral(std::int32_t);
template std::uint32_t JsonWriter::writeIntegral(std::int64_t);

// -- strings ----------------------------------------------------------

std::uint32_t JsonWriter::writeObjectName(std::string_view s) {
  CHECK_THROW(inContainer(ContainerType::Object), std::runtime_error);

  // Consecutive object names without an intervening value are not allowed.
  CHECK_THROW(!lastWrittenValueWasObjectName_, std::runtime_error);

  std::uint32_t n = 0;
  lastWrittenValueWasObjectName_ = true;
  n += appendNewlineAndIndent();
  if (options_.unquoteObjectName && isValidJavascriptIdentifier(s)) {
    out_.value().push(folly::StringPiece(s));
    n += s.size();
  } else {
    std::string tmp;
    folly::json::escapeString(s, tmp, {.validate_utf8 = true});
    out_.value().push(folly::StringPiece(tmp));
    n += tmp.size();
  }
  auto sep = options_.indentWidth > 0 ? ": " : ":";
  out_.value().push(folly::StringPiece(sep));
  n += strlen(sep);
  return n;
}

std::uint32_t JsonWriter::writeString(std::string_view s) {
  std::uint32_t n = 0;
  n += beginValue();
  std::string tmp;
  folly::json::escapeString(s, tmp, {.validate_utf8 = true});
  out_.value().push(folly::StringPiece(tmp));
  n += tmp.size();
  n += appendComma();
  return n;
}

} // namespace apache::thrift::json5::detail
