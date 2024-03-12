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

/* Generated SBE (Simple Binary Encoding) message codec */
#ifndef _FACEBOOK_SBE_TEST_CUSTOMERRESPONSE_CXX_H_
#define _FACEBOOK_SBE_TEST_CUSTOMERRESPONSE_CXX_H_

#if __cplusplus >= 201103L
#define SBE_CONSTEXPR constexpr
#define SBE_NOEXCEPT noexcept
#else
#define SBE_CONSTEXPR
#define SBE_NOEXCEPT
#endif

#if __cplusplus >= 201703L
#include <string_view>
#define SBE_NODISCARD [[nodiscard]]
#else
#define SBE_NODISCARD
#endif

#if !defined(__STDC_LIMIT_MACROS)
#define __STDC_LIMIT_MACROS 1
#endif

#include <cstdint>
#include <cstring>
#include <iomanip>
#include <limits>
#include <ostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <vector>

#if defined(WIN32) || defined(_WIN32)
#define SBE_BIG_ENDIAN_ENCODE_16(v) _byteswap_ushort(v)
#define SBE_BIG_ENDIAN_ENCODE_32(v) _byteswap_ulong(v)
#define SBE_BIG_ENDIAN_ENCODE_64(v) _byteswap_uint64(v)
#define SBE_LITTLE_ENDIAN_ENCODE_16(v) (v)
#define SBE_LITTLE_ENDIAN_ENCODE_32(v) (v)
#define SBE_LITTLE_ENDIAN_ENCODE_64(v) (v)
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#define SBE_BIG_ENDIAN_ENCODE_16(v) __builtin_bswap16(v)
#define SBE_BIG_ENDIAN_ENCODE_32(v) __builtin_bswap32(v)
#define SBE_BIG_ENDIAN_ENCODE_64(v) __builtin_bswap64(v)
#define SBE_LITTLE_ENDIAN_ENCODE_16(v) (v)
#define SBE_LITTLE_ENDIAN_ENCODE_32(v) (v)
#define SBE_LITTLE_ENDIAN_ENCODE_64(v) (v)
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#define SBE_LITTLE_ENDIAN_ENCODE_16(v) __builtin_bswap16(v)
#define SBE_LITTLE_ENDIAN_ENCODE_32(v) __builtin_bswap32(v)
#define SBE_LITTLE_ENDIAN_ENCODE_64(v) __builtin_bswap64(v)
#define SBE_BIG_ENDIAN_ENCODE_16(v) (v)
#define SBE_BIG_ENDIAN_ENCODE_32(v) (v)
#define SBE_BIG_ENDIAN_ENCODE_64(v) (v)
#else
#error \
    "Byte Ordering of platform not determined. Set __BYTE_ORDER__ manually before including this file."
#endif

#if !defined(SBE_BOUNDS_CHECK_EXPECT)
#if defined(SBE_NO_BOUNDS_CHECK)
#define SBE_BOUNDS_CHECK_EXPECT(exp, c) (false)
#elif defined(_MSC_VER)
#define SBE_BOUNDS_CHECK_EXPECT(exp, c) (exp)
#else
#define SBE_BOUNDS_CHECK_EXPECT(exp, c) (__builtin_expect(exp, c))
#endif

#endif

#define SBE_FLOAT_NAN std::numeric_limits<float>::quiet_NaN()
#define SBE_DOUBLE_NAN std::numeric_limits<double>::quiet_NaN()
#define SBE_NULLVALUE_INT8 (std::numeric_limits<std::int8_t>::min)()
#define SBE_NULLVALUE_INT16 (std::numeric_limits<std::int16_t>::min)()
#define SBE_NULLVALUE_INT32 (std::numeric_limits<std::int32_t>::min)()
#define SBE_NULLVALUE_INT64 (std::numeric_limits<std::int64_t>::min)()
#define SBE_NULLVALUE_UINT8 (std::numeric_limits<std::uint8_t>::max)()
#define SBE_NULLVALUE_UINT16 (std::numeric_limits<std::uint16_t>::max)()
#define SBE_NULLVALUE_UINT32 (std::numeric_limits<std::uint32_t>::max)()
#define SBE_NULLVALUE_UINT64 (std::numeric_limits<std::uint64_t>::max)()

#include "ByteVarStringEncoding.h"
#include "GroupSizeEncoding.h"
#include "MessageHeader.h"
#include "ShortVarStringEncoding.h"

namespace facebook {
namespace sbe {
namespace test {

class CustomerResponse {
 private:
  /**
   * The states in which a encoder/decoder/codec can live.
   *
   * <p>The state machine diagram below, encoded in the dot language, describes
   * the valid state transitions according to the order in which fields may be
   * accessed safely. Tools such as PlantUML and Graphviz can render it.
   *
   * <pre>{@code
   *   digraph G {
   *       NOT_WRAPPED -> V0_BLOCK [label="  wrap(version=0)  "];
   *       V0_BLOCK -> V0_BLOCK [label="  Index(?)  "];
   *       V0_BLOCK -> V0_BLOCK [label="  CustomerIdLength()  "];
   *       V0_BLOCK -> V0_CUSTOMERID_DONE [label="  CustomerId(?)  "];
   *       V0_CUSTOMERID_DONE -> V0_CUSTOMERID_DONE [label="  FirstNameLength()
   * "]; V0_CUSTOMERID_DONE -> V0_FIRSTNAME_DONE [label="  FirstName(?)  "];
   *       V0_FIRSTNAME_DONE -> V0_FIRSTNAME_DONE [label="  LastNameLength() "];
   *       V0_FIRSTNAME_DONE -> V0_LASTNAME_DONE [label="  LastName(?)  "];
   *       V0_LASTNAME_DONE -> V0_LASTNAME_DONE [label="  CompanyLength()  "];
   *       V0_LASTNAME_DONE -> V0_COMPANY_DONE [label="  Company(?)  "];
   *       V0_COMPANY_DONE -> V0_COMPANY_DONE [label="  CityLength()  "];
   *       V0_COMPANY_DONE -> V0_CITY_DONE [label="  City(?)  "];
   *       V0_CITY_DONE -> V0_CITY_DONE [label="  CountryLength()  "];
   *       V0_CITY_DONE -> V0_COUNTRY_DONE [label="  Country(?)  "];
   *       V0_COUNTRY_DONE -> V0_COUNTRY_DONE [label="  Phone1Length()  "];
   *       V0_COUNTRY_DONE -> V0_PHONE1_DONE [label="  Phone1(?)  "];
   *       V0_PHONE1_DONE -> V0_PHONE1_DONE [label="  Phone2Length()  "];
   *       V0_PHONE1_DONE -> V0_PHONE2_DONE [label="  Phone2(?)  "];
   *       V0_PHONE2_DONE -> V0_PHONE2_DONE [label="  EmailLength()  "];
   *       V0_PHONE2_DONE -> V0_EMAIL_DONE [label="  Email(?)  "];
   *       V0_EMAIL_DONE -> V0_EMAIL_DONE [label="  SubscriptionDateLength() "];
   *       V0_EMAIL_DONE -> V0_SUBSCRIPTIONDATE_DONE [label="
   * SubscriptionDate(?)  "]; V0_SUBSCRIPTIONDATE_DONE ->
   * V0_SUBSCRIPTIONDATE_DONE [label="  WebSiteLength()  "];
   *       V0_SUBSCRIPTIONDATE_DONE -> V0_WEBSITE_DONE [label="  WebSite(?)  "];
   *   }
   * }</pre>
   */
  enum class CodecState {
    NOT_WRAPPED = 0,
    V0_BLOCK = 1,
    V0_CUSTOMERID_DONE = 2,
    V0_FIRSTNAME_DONE = 3,
    V0_LASTNAME_DONE = 4,
    V0_COMPANY_DONE = 5,
    V0_CITY_DONE = 6,
    V0_COUNTRY_DONE = 7,
    V0_PHONE1_DONE = 8,
    V0_PHONE2_DONE = 9,
    V0_EMAIL_DONE = 10,
    V0_SUBSCRIPTIONDATE_DONE = 11,
    V0_WEBSITE_DONE = 12,
  };

  static const std::string STATE_NAME_LOOKUP[13];
  static const std::string STATE_TRANSITIONS_LOOKUP[13];

  static std::string codecStateName(CodecState state) {
    return STATE_NAME_LOOKUP[static_cast<int>(state)];
  }

  static std::string codecStateTransitions(CodecState state) {
    return STATE_TRANSITIONS_LOOKUP[static_cast<int>(state)];
  }

  char* m_buffer = nullptr;
  std::uint64_t m_bufferLength = 0;
  std::uint64_t m_offset = 0;
  std::uint64_t m_position = 0;
  std::uint64_t m_actingBlockLength = 0;
  std::uint64_t m_actingVersion = 0;
  CodecState m_codecState = CodecState::NOT_WRAPPED;

  CodecState codecState() const { return m_codecState; }

  CodecState* codecStatePtr() { return &m_codecState; }

  void codecState(CodecState newState) { m_codecState = newState; }

  inline std::uint64_t* sbePositionPtr() SBE_NOEXCEPT { return &m_position; }

 public:
  static constexpr std::uint16_t SBE_BLOCK_LENGTH =
      static_cast<std::uint16_t>(8);
  static constexpr std::uint16_t SBE_TEMPLATE_ID =
      static_cast<std::uint16_t>(3);
  static constexpr std::uint16_t SBE_SCHEMA_ID = static_cast<std::uint16_t>(1);
  static constexpr std::uint16_t SBE_SCHEMA_VERSION =
      static_cast<std::uint16_t>(0);
  static constexpr const char* SBE_SEMANTIC_VERSION = "5.2";

  enum MetaAttribute { EPOCH, TIME_UNIT, SEMANTIC_TYPE, PRESENCE };

  union sbe_float_as_uint_u {
    float fp_value;
    std::uint32_t uint_value;
  };

  union sbe_double_as_uint_u {
    double fp_value;
    std::uint64_t uint_value;
  };

  using messageHeader = MessageHeader;

  class AccessOrderError : public std::logic_error {
   public:
    explicit AccessOrderError(const std::string& msg) : std::logic_error(msg) {}
  };

  CustomerResponse() = default;

  CustomerResponse(
      char* buffer,
      const std::uint64_t offset,
      const std::uint64_t bufferLength,
      const std::uint64_t actingBlockLength,
      const std::uint64_t actingVersion,
      CodecState codecState)
      : m_buffer(buffer),
        m_bufferLength(bufferLength),
        m_offset(offset),
        m_position(sbeCheckPosition(offset + actingBlockLength)),
        m_actingBlockLength(actingBlockLength),
        m_actingVersion(actingVersion),
        m_codecState(codecState) {}

  CustomerResponse(
      char* buffer,
      const std::uint64_t offset,
      const std::uint64_t bufferLength,
      const std::uint64_t actingBlockLength,
      const std::uint64_t actingVersion)
      : m_buffer(buffer),
        m_bufferLength(bufferLength),
        m_offset(offset),
        m_position(sbeCheckPosition(offset + actingBlockLength)),
        m_actingBlockLength(actingBlockLength),
        m_actingVersion(actingVersion) {}

  CustomerResponse(char* buffer, const std::uint64_t bufferLength)
      : CustomerResponse(
            buffer, 0, bufferLength, sbeBlockLength(), sbeSchemaVersion()) {}

  CustomerResponse(
      char* buffer,
      const std::uint64_t bufferLength,
      const std::uint64_t actingBlockLength,
      const std::uint64_t actingVersion)
      : CustomerResponse(
            buffer, 0, bufferLength, actingBlockLength, actingVersion) {}

  SBE_NODISCARD static SBE_CONSTEXPR std::uint16_t sbeBlockLength()
      SBE_NOEXCEPT {
    return static_cast<std::uint16_t>(8);
  }

  SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t sbeBlockAndHeaderLength()
      SBE_NOEXCEPT {
    return messageHeader::encodedLength() + sbeBlockLength();
  }

  SBE_NODISCARD static SBE_CONSTEXPR std::uint16_t sbeTemplateId()
      SBE_NOEXCEPT {
    return static_cast<std::uint16_t>(3);
  }

  SBE_NODISCARD static SBE_CONSTEXPR std::uint16_t sbeSchemaId() SBE_NOEXCEPT {
    return static_cast<std::uint16_t>(1);
  }

  SBE_NODISCARD static SBE_CONSTEXPR std::uint16_t sbeSchemaVersion()
      SBE_NOEXCEPT {
    return static_cast<std::uint16_t>(0);
  }

  SBE_NODISCARD static const char* sbeSemanticVersion() SBE_NOEXCEPT {
    return "5.2";
  }

  SBE_NODISCARD static SBE_CONSTEXPR const char* sbeSemanticType()
      SBE_NOEXCEPT {
    return "";
  }

  SBE_NODISCARD std::uint64_t offset() const SBE_NOEXCEPT { return m_offset; }

  CustomerResponse& wrapForEncode(
      char* buffer,
      const std::uint64_t offset,
      const std::uint64_t bufferLength) {
    m_buffer = buffer;
    m_bufferLength = bufferLength;
    m_offset = offset;
    m_actingBlockLength = sbeBlockLength();
    m_actingVersion = sbeSchemaVersion();
    m_position = sbeCheckPosition(m_offset + m_actingBlockLength);
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    codecState(CodecState::V0_BLOCK);
#endif
    return *this;
  }

  CustomerResponse& wrapAndApplyHeader(
      char* buffer,
      const std::uint64_t offset,
      const std::uint64_t bufferLength) {
    messageHeader hdr(buffer, offset, bufferLength, sbeSchemaVersion());

    hdr.blockLength(sbeBlockLength())
        .templateId(sbeTemplateId())
        .schemaId(sbeSchemaId())
        .version(sbeSchemaVersion());

    m_buffer = buffer;
    m_bufferLength = bufferLength;
    m_offset = offset + messageHeader::encodedLength();
    m_actingBlockLength = sbeBlockLength();
    m_actingVersion = sbeSchemaVersion();
    m_position = sbeCheckPosition(m_offset + m_actingBlockLength);
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    codecState(CodecState::V0_BLOCK);
#endif
    return *this;
  }

  CustomerResponse& wrapForDecode(
      char* buffer,
      const std::uint64_t offset,
      const std::uint64_t actingBlockLength,
      const std::uint64_t actingVersion,
      const std::uint64_t bufferLength) {
    m_buffer = buffer;
    m_bufferLength = bufferLength;
    m_offset = offset;
    m_actingBlockLength = actingBlockLength;
    m_actingVersion = actingVersion;
    m_position = sbeCheckPosition(m_offset + m_actingBlockLength);
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    codecState(CodecState::V0_BLOCK);
#endif
    return *this;
  }

  CustomerResponse& sbeRewind() {
    return wrapForDecode(
        m_buffer,
        m_offset,
        m_actingBlockLength,
        m_actingVersion,
        m_bufferLength);
  }

  SBE_NODISCARD std::uint64_t sbePosition() const SBE_NOEXCEPT {
    return m_position;
  }

  // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
  std::uint64_t sbeCheckPosition(const std::uint64_t position) {
    if (SBE_BOUNDS_CHECK_EXPECT((position > m_bufferLength), false)) {
      throw std::runtime_error("buffer too short [E100]");
    }
    return position;
  }

  void sbePosition(const std::uint64_t position) {
    m_position = sbeCheckPosition(position);
  }

  SBE_NODISCARD std::uint64_t encodedLength() const SBE_NOEXCEPT {
    return sbePosition() - m_offset;
  }

  SBE_NODISCARD std::uint64_t decodeLength() const {
    CustomerResponse skipper(
        m_buffer,
        m_offset,
        m_bufferLength,
        sbeBlockLength(),
        m_actingVersion,
        m_codecState);
    skipper.skip();
    return skipper.encodedLength();
  }

  SBE_NODISCARD const char* buffer() const SBE_NOEXCEPT { return m_buffer; }

  SBE_NODISCARD char* buffer() SBE_NOEXCEPT { return m_buffer; }

  SBE_NODISCARD std::uint64_t bufferLength() const SBE_NOEXCEPT {
    return m_bufferLength;
  }

  SBE_NODISCARD std::uint64_t actingVersion() const SBE_NOEXCEPT {
    return m_actingVersion;
  }

  void checkEncodingIsComplete() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    switch (m_codecState) {
      case CodecState::V0_WEBSITE_DONE:
        return;
      default:
        throw AccessOrderError(
            std::string("Not fully encoded, current state: ") +
            codecStateName(m_codecState) +
            ", allowed transitions: " + codecStateTransitions(m_codecState));
    }
#endif
  }

  SBE_NODISCARD static const char* IndexMetaAttribute(
      const MetaAttribute metaAttribute) SBE_NOEXCEPT {
    switch (metaAttribute) {
      case MetaAttribute::PRESENCE:
        return "required";
      default:
        return "";
    }
  }

  static SBE_CONSTEXPR std::uint16_t indexId() SBE_NOEXCEPT { return 1; }

  SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t indexSinceVersion()
      SBE_NOEXCEPT {
    return 0;
  }

  SBE_NODISCARD bool indexInActingVersion() SBE_NOEXCEPT { return true; }

  SBE_NODISCARD static SBE_CONSTEXPR std::size_t indexEncodingOffset()
      SBE_NOEXCEPT {
    return 0;
  }

 private:
  void onIndexAccessed() const {
    if (codecState() == CodecState::NOT_WRAPPED) {
      throw AccessOrderError(
          std::string("Illegal field access order. ") +
          "Cannot access field \"Index\" in state: " +
          codecStateName(codecState()) +
          ". Expected one of these transitions: [" +
          codecStateTransitions(codecState()) +
          "]. Please see the diagram in the docs of the enum CustomerResponse::CodecState.");
    }
  }

 public:
  static SBE_CONSTEXPR std::int64_t indexNullValue() SBE_NOEXCEPT {
    return SBE_NULLVALUE_INT64;
  }

  static SBE_CONSTEXPR std::int64_t indexMinValue() SBE_NOEXCEPT {
    return INT64_C(-9223372036854775807);
  }

  static SBE_CONSTEXPR std::int64_t indexMaxValue() SBE_NOEXCEPT {
    return INT64_C(9223372036854775807);
  }

  static SBE_CONSTEXPR std::size_t indexEncodingLength() SBE_NOEXCEPT {
    return 8;
  }

  SBE_NODISCARD std::int64_t index() const {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onIndexAccessed();
#endif
    std::int64_t val;
    std::memcpy(&val, m_buffer + m_offset + 0, sizeof(std::int64_t));
    return SBE_LITTLE_ENDIAN_ENCODE_64(val);
  }

  CustomerResponse& index(const std::int64_t value) {
    std::int64_t val = SBE_LITTLE_ENDIAN_ENCODE_64(value);
    std::memcpy(m_buffer + m_offset + 0, &val, sizeof(std::int64_t));
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onIndexAccessed();
#endif
    return *this;
  }

  SBE_NODISCARD static const char* CustomerIdMetaAttribute(
      const MetaAttribute metaAttribute) SBE_NOEXCEPT {
    switch (metaAttribute) {
      case MetaAttribute::PRESENCE:
        return "required";
      default:
        return "";
    }
  }

  static const char* customerIdCharacterEncoding() SBE_NOEXCEPT {
    return "UTF-8";
  }

  static SBE_CONSTEXPR std::uint64_t customerIdSinceVersion() SBE_NOEXCEPT {
    return 0;
  }

  bool customerIdInActingVersion() SBE_NOEXCEPT { return true; }

  static SBE_CONSTEXPR std::uint16_t customerIdId() SBE_NOEXCEPT { return 2; }

  static SBE_CONSTEXPR std::uint64_t customerIdHeaderLength() SBE_NOEXCEPT {
    return 1;
  }

 private:
  void onCustomerIdLengthAccessed() const {
    switch (codecState()) {
      case CodecState::V0_BLOCK:
        break;
      default:
        throw AccessOrderError(
            std::string("Illegal field access order. ") +
            "Cannot decode length of var data \"CustomerId\" in state: " +
            codecStateName(codecState()) +
            ". Expected one of these transitions: [" +
            codecStateTransitions(codecState()) +
            "]. Please see the diagram in the docs of the enum CustomerResponse::CodecState.");
    }
  }

 public:
  SBE_NODISCARD std::uint8_t customerIdLength() const {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onCustomerIdLengthAccessed();
#endif
    std::uint8_t length;
    std::memcpy(&length, m_buffer + sbePosition(), sizeof(std::uint8_t));
    return (length);
  }

 private:
  void onCustomerIdAccessed() {
    switch (codecState()) {
      case CodecState::V0_BLOCK:
        codecState(CodecState::V0_CUSTOMERID_DONE);
        break;
      default:
        throw AccessOrderError(
            std::string("Illegal field access order. ") +
            "Cannot access field \"CustomerId\" in state: " +
            codecStateName(codecState()) +
            ". Expected one of these transitions: [" +
            codecStateTransitions(codecState()) +
            "]. Please see the diagram in the docs of the enum CustomerResponse::CodecState.");
    }
  }

 public:
  std::uint64_t skipCustomerId() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onCustomerIdAccessed();
#endif
    std::uint64_t lengthOfLengthField = 1;
    std::uint64_t lengthPosition = sbePosition();
    std::uint8_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint8_t));
    std::uint64_t dataLength = (lengthFieldValue);
    sbePosition(lengthPosition + lengthOfLengthField + dataLength);
    return dataLength;
  }

  SBE_NODISCARD const char* customerId() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onCustomerIdAccessed();
#endif
    std::uint8_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + sbePosition(), sizeof(std::uint8_t));
    const char* fieldPtr = m_buffer + sbePosition() + 1;
    sbePosition(sbePosition() + 1 + (lengthFieldValue));
    return fieldPtr;
  }

  std::uint64_t getCustomerId(char* dst, const std::uint64_t length) {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onCustomerIdAccessed();
#endif
    std::uint64_t lengthOfLengthField = 1;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint8_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint8_t));
    std::uint64_t dataLength = (lengthFieldValue);
    std::uint64_t bytesToCopy = length < dataLength ? length : dataLength;
    std::uint64_t pos = sbePosition();
    sbePosition(pos + dataLength);
    std::memcpy(dst, m_buffer + pos, static_cast<std::size_t>(bytesToCopy));
    return bytesToCopy;
  }

  CustomerResponse& putCustomerId(const char* src, const std::uint8_t length) {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onCustomerIdAccessed();
#endif
    std::uint64_t lengthOfLengthField = 1;
    std::uint64_t lengthPosition = sbePosition();
    std::uint8_t lengthFieldValue = (length);
    sbePosition(lengthPosition + lengthOfLengthField);
    std::memcpy(
        m_buffer + lengthPosition, &lengthFieldValue, sizeof(std::uint8_t));
    if (length != std::uint8_t(0)) {
      std::uint64_t pos = sbePosition();
      sbePosition(pos + length);
      std::memcpy(m_buffer + pos, src, length);
    }
    return *this;
  }

  std::string getCustomerIdAsString() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onCustomerIdAccessed();
#endif
    std::uint64_t lengthOfLengthField = 1;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint8_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint8_t));
    std::uint64_t dataLength = (lengthFieldValue);
    std::uint64_t pos = sbePosition();
    const std::string result(m_buffer + pos, dataLength);
    sbePosition(pos + dataLength);
    return result;
  }

  std::string getCustomerIdAsJsonEscapedString() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onCustomerIdAccessed();
#endif
    std::ostringstream oss;
    std::string s = getCustomerIdAsString();

    for (const auto c : s) {
      switch (c) {
        case '"':
          oss << "\\\"";
          break;
        case '\\':
          oss << "\\\\";
          break;
        case '\b':
          oss << "\\b";
          break;
        case '\f':
          oss << "\\f";
          break;
        case '\n':
          oss << "\\n";
          break;
        case '\r':
          oss << "\\r";
          break;
        case '\t':
          oss << "\\t";
          break;

        default:
          if ('\x00' <= c && c <= '\x1f') {
            oss << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                << (int)(c);
          } else {
            oss << c;
          }
      }
    }

    return oss.str();
  }

#if __cplusplus >= 201703L
  std::string_view getCustomerIdAsStringView() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onCustomerIdAccessed();
#endif
    std::uint64_t lengthOfLengthField = 1;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint8_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint8_t));
    std::uint64_t dataLength = (lengthFieldValue);
    std::uint64_t pos = sbePosition();
    const std::string_view result(m_buffer + pos, dataLength);
    sbePosition(pos + dataLength);
    return result;
  }
#endif

  CustomerResponse& putCustomerId(const std::string& str) {
    if (str.length() > 254) {
      throw std::runtime_error("std::string too long for length type [E109]");
    }
    return putCustomerId(str.data(), static_cast<std::uint8_t>(str.length()));
  }

#if __cplusplus >= 201703L
  CustomerResponse& putCustomerId(const std::string_view str) {
    if (str.length() > 254) {
      throw std::runtime_error("std::string too long for length type [E109]");
    }
    return putCustomerId(str.data(), static_cast<std::uint8_t>(str.length()));
  }
#endif

  SBE_NODISCARD static const char* FirstNameMetaAttribute(
      const MetaAttribute metaAttribute) SBE_NOEXCEPT {
    switch (metaAttribute) {
      case MetaAttribute::PRESENCE:
        return "required";
      default:
        return "";
    }
  }

  static const char* firstNameCharacterEncoding() SBE_NOEXCEPT {
    return "UTF-8";
  }

  static SBE_CONSTEXPR std::uint64_t firstNameSinceVersion() SBE_NOEXCEPT {
    return 0;
  }

  bool firstNameInActingVersion() SBE_NOEXCEPT { return true; }

  static SBE_CONSTEXPR std::uint16_t firstNameId() SBE_NOEXCEPT { return 3; }

  static SBE_CONSTEXPR std::uint64_t firstNameHeaderLength() SBE_NOEXCEPT {
    return 2;
  }

 private:
  void onFirstNameLengthAccessed() const {
    switch (codecState()) {
      case CodecState::V0_CUSTOMERID_DONE:
        break;
      default:
        throw AccessOrderError(
            std::string("Illegal field access order. ") +
            "Cannot decode length of var data \"FirstName\" in state: " +
            codecStateName(codecState()) +
            ". Expected one of these transitions: [" +
            codecStateTransitions(codecState()) +
            "]. Please see the diagram in the docs of the enum CustomerResponse::CodecState.");
    }
  }

 public:
  SBE_NODISCARD std::uint16_t firstNameLength() const {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onFirstNameLengthAccessed();
#endif
    std::uint16_t length;
    std::memcpy(&length, m_buffer + sbePosition(), sizeof(std::uint16_t));
    return SBE_LITTLE_ENDIAN_ENCODE_16(length);
  }

 private:
  void onFirstNameAccessed() {
    switch (codecState()) {
      case CodecState::V0_CUSTOMERID_DONE:
        codecState(CodecState::V0_FIRSTNAME_DONE);
        break;
      default:
        throw AccessOrderError(
            std::string("Illegal field access order. ") +
            "Cannot access field \"FirstName\" in state: " +
            codecStateName(codecState()) +
            ". Expected one of these transitions: [" +
            codecStateTransitions(codecState()) +
            "]. Please see the diagram in the docs of the enum CustomerResponse::CodecState.");
    }
  }

 public:
  std::uint64_t skipFirstName() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onFirstNameAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    sbePosition(lengthPosition + lengthOfLengthField + dataLength);
    return dataLength;
  }

  SBE_NODISCARD const char* firstName() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onFirstNameAccessed();
#endif
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + sbePosition(), sizeof(std::uint16_t));
    const char* fieldPtr = m_buffer + sbePosition() + 2;
    sbePosition(
        sbePosition() + 2 + SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue));
    return fieldPtr;
  }

  std::uint64_t getFirstName(char* dst, const std::uint64_t length) {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onFirstNameAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    std::uint64_t bytesToCopy = length < dataLength ? length : dataLength;
    std::uint64_t pos = sbePosition();
    sbePosition(pos + dataLength);
    std::memcpy(dst, m_buffer + pos, static_cast<std::size_t>(bytesToCopy));
    return bytesToCopy;
  }

  CustomerResponse& putFirstName(const char* src, const std::uint16_t length) {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onFirstNameAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    std::uint16_t lengthFieldValue = SBE_LITTLE_ENDIAN_ENCODE_16(length);
    sbePosition(lengthPosition + lengthOfLengthField);
    std::memcpy(
        m_buffer + lengthPosition, &lengthFieldValue, sizeof(std::uint16_t));
    if (length != std::uint16_t(0)) {
      std::uint64_t pos = sbePosition();
      sbePosition(pos + length);
      std::memcpy(m_buffer + pos, src, length);
    }
    return *this;
  }

  std::string getFirstNameAsString() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onFirstNameAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    std::uint64_t pos = sbePosition();
    const std::string result(m_buffer + pos, dataLength);
    sbePosition(pos + dataLength);
    return result;
  }

  std::string getFirstNameAsJsonEscapedString() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onFirstNameAccessed();
#endif
    std::ostringstream oss;
    std::string s = getFirstNameAsString();

    for (const auto c : s) {
      switch (c) {
        case '"':
          oss << "\\\"";
          break;
        case '\\':
          oss << "\\\\";
          break;
        case '\b':
          oss << "\\b";
          break;
        case '\f':
          oss << "\\f";
          break;
        case '\n':
          oss << "\\n";
          break;
        case '\r':
          oss << "\\r";
          break;
        case '\t':
          oss << "\\t";
          break;

        default:
          if ('\x00' <= c && c <= '\x1f') {
            oss << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                << (int)(c);
          } else {
            oss << c;
          }
      }
    }

    return oss.str();
  }

#if __cplusplus >= 201703L
  std::string_view getFirstNameAsStringView() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onFirstNameAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    std::uint64_t pos = sbePosition();
    const std::string_view result(m_buffer + pos, dataLength);
    sbePosition(pos + dataLength);
    return result;
  }
#endif

  CustomerResponse& putFirstName(const std::string& str) {
    if (str.length() > 65534) {
      throw std::runtime_error("std::string too long for length type [E109]");
    }
    return putFirstName(str.data(), static_cast<std::uint16_t>(str.length()));
  }

#if __cplusplus >= 201703L
  CustomerResponse& putFirstName(const std::string_view str) {
    if (str.length() > 65534) {
      throw std::runtime_error("std::string too long for length type [E109]");
    }
    return putFirstName(str.data(), static_cast<std::uint16_t>(str.length()));
  }
#endif

  SBE_NODISCARD static const char* LastNameMetaAttribute(
      const MetaAttribute metaAttribute) SBE_NOEXCEPT {
    switch (metaAttribute) {
      case MetaAttribute::PRESENCE:
        return "required";
      default:
        return "";
    }
  }

  static const char* lastNameCharacterEncoding() SBE_NOEXCEPT {
    return "UTF-8";
  }

  static SBE_CONSTEXPR std::uint64_t lastNameSinceVersion() SBE_NOEXCEPT {
    return 0;
  }

  bool lastNameInActingVersion() SBE_NOEXCEPT { return true; }

  static SBE_CONSTEXPR std::uint16_t lastNameId() SBE_NOEXCEPT { return 4; }

  static SBE_CONSTEXPR std::uint64_t lastNameHeaderLength() SBE_NOEXCEPT {
    return 2;
  }

 private:
  void onLastNameLengthAccessed() const {
    switch (codecState()) {
      case CodecState::V0_FIRSTNAME_DONE:
        break;
      default:
        throw AccessOrderError(
            std::string("Illegal field access order. ") +
            "Cannot decode length of var data \"LastName\" in state: " +
            codecStateName(codecState()) +
            ". Expected one of these transitions: [" +
            codecStateTransitions(codecState()) +
            "]. Please see the diagram in the docs of the enum CustomerResponse::CodecState.");
    }
  }

 public:
  SBE_NODISCARD std::uint16_t lastNameLength() const {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onLastNameLengthAccessed();
#endif
    std::uint16_t length;
    std::memcpy(&length, m_buffer + sbePosition(), sizeof(std::uint16_t));
    return SBE_LITTLE_ENDIAN_ENCODE_16(length);
  }

 private:
  void onLastNameAccessed() {
    switch (codecState()) {
      case CodecState::V0_FIRSTNAME_DONE:
        codecState(CodecState::V0_LASTNAME_DONE);
        break;
      default:
        throw AccessOrderError(
            std::string("Illegal field access order. ") +
            "Cannot access field \"LastName\" in state: " +
            codecStateName(codecState()) +
            ". Expected one of these transitions: [" +
            codecStateTransitions(codecState()) +
            "]. Please see the diagram in the docs of the enum CustomerResponse::CodecState.");
    }
  }

 public:
  std::uint64_t skipLastName() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onLastNameAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    sbePosition(lengthPosition + lengthOfLengthField + dataLength);
    return dataLength;
  }

  SBE_NODISCARD const char* lastName() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onLastNameAccessed();
#endif
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + sbePosition(), sizeof(std::uint16_t));
    const char* fieldPtr = m_buffer + sbePosition() + 2;
    sbePosition(
        sbePosition() + 2 + SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue));
    return fieldPtr;
  }

  std::uint64_t getLastName(char* dst, const std::uint64_t length) {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onLastNameAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    std::uint64_t bytesToCopy = length < dataLength ? length : dataLength;
    std::uint64_t pos = sbePosition();
    sbePosition(pos + dataLength);
    std::memcpy(dst, m_buffer + pos, static_cast<std::size_t>(bytesToCopy));
    return bytesToCopy;
  }

  CustomerResponse& putLastName(const char* src, const std::uint16_t length) {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onLastNameAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    std::uint16_t lengthFieldValue = SBE_LITTLE_ENDIAN_ENCODE_16(length);
    sbePosition(lengthPosition + lengthOfLengthField);
    std::memcpy(
        m_buffer + lengthPosition, &lengthFieldValue, sizeof(std::uint16_t));
    if (length != std::uint16_t(0)) {
      std::uint64_t pos = sbePosition();
      sbePosition(pos + length);
      std::memcpy(m_buffer + pos, src, length);
    }
    return *this;
  }

  std::string getLastNameAsString() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onLastNameAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    std::uint64_t pos = sbePosition();
    const std::string result(m_buffer + pos, dataLength);
    sbePosition(pos + dataLength);
    return result;
  }

  std::string getLastNameAsJsonEscapedString() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onLastNameAccessed();
#endif
    std::ostringstream oss;
    std::string s = getLastNameAsString();

    for (const auto c : s) {
      switch (c) {
        case '"':
          oss << "\\\"";
          break;
        case '\\':
          oss << "\\\\";
          break;
        case '\b':
          oss << "\\b";
          break;
        case '\f':
          oss << "\\f";
          break;
        case '\n':
          oss << "\\n";
          break;
        case '\r':
          oss << "\\r";
          break;
        case '\t':
          oss << "\\t";
          break;

        default:
          if ('\x00' <= c && c <= '\x1f') {
            oss << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                << (int)(c);
          } else {
            oss << c;
          }
      }
    }

    return oss.str();
  }

#if __cplusplus >= 201703L
  std::string_view getLastNameAsStringView() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onLastNameAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    std::uint64_t pos = sbePosition();
    const std::string_view result(m_buffer + pos, dataLength);
    sbePosition(pos + dataLength);
    return result;
  }
#endif

  CustomerResponse& putLastName(const std::string& str) {
    if (str.length() > 65534) {
      throw std::runtime_error("std::string too long for length type [E109]");
    }
    return putLastName(str.data(), static_cast<std::uint16_t>(str.length()));
  }

#if __cplusplus >= 201703L
  CustomerResponse& putLastName(const std::string_view str) {
    if (str.length() > 65534) {
      throw std::runtime_error("std::string too long for length type [E109]");
    }
    return putLastName(str.data(), static_cast<std::uint16_t>(str.length()));
  }
#endif

  SBE_NODISCARD static const char* CompanyMetaAttribute(
      const MetaAttribute metaAttribute) SBE_NOEXCEPT {
    switch (metaAttribute) {
      case MetaAttribute::PRESENCE:
        return "required";
      default:
        return "";
    }
  }

  static const char* companyCharacterEncoding() SBE_NOEXCEPT { return "UTF-8"; }

  static SBE_CONSTEXPR std::uint64_t companySinceVersion() SBE_NOEXCEPT {
    return 0;
  }

  bool companyInActingVersion() SBE_NOEXCEPT { return true; }

  static SBE_CONSTEXPR std::uint16_t companyId() SBE_NOEXCEPT { return 5; }

  static SBE_CONSTEXPR std::uint64_t companyHeaderLength() SBE_NOEXCEPT {
    return 2;
  }

 private:
  void onCompanyLengthAccessed() const {
    switch (codecState()) {
      case CodecState::V0_LASTNAME_DONE:
        break;
      default:
        throw AccessOrderError(
            std::string("Illegal field access order. ") +
            "Cannot decode length of var data \"Company\" in state: " +
            codecStateName(codecState()) +
            ". Expected one of these transitions: [" +
            codecStateTransitions(codecState()) +
            "]. Please see the diagram in the docs of the enum CustomerResponse::CodecState.");
    }
  }

 public:
  SBE_NODISCARD std::uint16_t companyLength() const {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onCompanyLengthAccessed();
#endif
    std::uint16_t length;
    std::memcpy(&length, m_buffer + sbePosition(), sizeof(std::uint16_t));
    return SBE_LITTLE_ENDIAN_ENCODE_16(length);
  }

 private:
  void onCompanyAccessed() {
    switch (codecState()) {
      case CodecState::V0_LASTNAME_DONE:
        codecState(CodecState::V0_COMPANY_DONE);
        break;
      default:
        throw AccessOrderError(
            std::string("Illegal field access order. ") +
            "Cannot access field \"Company\" in state: " +
            codecStateName(codecState()) +
            ". Expected one of these transitions: [" +
            codecStateTransitions(codecState()) +
            "]. Please see the diagram in the docs of the enum CustomerResponse::CodecState.");
    }
  }

 public:
  std::uint64_t skipCompany() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onCompanyAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    sbePosition(lengthPosition + lengthOfLengthField + dataLength);
    return dataLength;
  }

  SBE_NODISCARD const char* company() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onCompanyAccessed();
#endif
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + sbePosition(), sizeof(std::uint16_t));
    const char* fieldPtr = m_buffer + sbePosition() + 2;
    sbePosition(
        sbePosition() + 2 + SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue));
    return fieldPtr;
  }

  std::uint64_t getCompany(char* dst, const std::uint64_t length) {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onCompanyAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    std::uint64_t bytesToCopy = length < dataLength ? length : dataLength;
    std::uint64_t pos = sbePosition();
    sbePosition(pos + dataLength);
    std::memcpy(dst, m_buffer + pos, static_cast<std::size_t>(bytesToCopy));
    return bytesToCopy;
  }

  CustomerResponse& putCompany(const char* src, const std::uint16_t length) {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onCompanyAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    std::uint16_t lengthFieldValue = SBE_LITTLE_ENDIAN_ENCODE_16(length);
    sbePosition(lengthPosition + lengthOfLengthField);
    std::memcpy(
        m_buffer + lengthPosition, &lengthFieldValue, sizeof(std::uint16_t));
    if (length != std::uint16_t(0)) {
      std::uint64_t pos = sbePosition();
      sbePosition(pos + length);
      std::memcpy(m_buffer + pos, src, length);
    }
    return *this;
  }

  std::string getCompanyAsString() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onCompanyAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    std::uint64_t pos = sbePosition();
    const std::string result(m_buffer + pos, dataLength);
    sbePosition(pos + dataLength);
    return result;
  }

  std::string getCompanyAsJsonEscapedString() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onCompanyAccessed();
#endif
    std::ostringstream oss;
    std::string s = getCompanyAsString();

    for (const auto c : s) {
      switch (c) {
        case '"':
          oss << "\\\"";
          break;
        case '\\':
          oss << "\\\\";
          break;
        case '\b':
          oss << "\\b";
          break;
        case '\f':
          oss << "\\f";
          break;
        case '\n':
          oss << "\\n";
          break;
        case '\r':
          oss << "\\r";
          break;
        case '\t':
          oss << "\\t";
          break;

        default:
          if ('\x00' <= c && c <= '\x1f') {
            oss << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                << (int)(c);
          } else {
            oss << c;
          }
      }
    }

    return oss.str();
  }

#if __cplusplus >= 201703L
  std::string_view getCompanyAsStringView() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onCompanyAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    std::uint64_t pos = sbePosition();
    const std::string_view result(m_buffer + pos, dataLength);
    sbePosition(pos + dataLength);
    return result;
  }
#endif

  CustomerResponse& putCompany(const std::string& str) {
    if (str.length() > 65534) {
      throw std::runtime_error("std::string too long for length type [E109]");
    }
    return putCompany(str.data(), static_cast<std::uint16_t>(str.length()));
  }

#if __cplusplus >= 201703L
  CustomerResponse& putCompany(const std::string_view str) {
    if (str.length() > 65534) {
      throw std::runtime_error("std::string too long for length type [E109]");
    }
    return putCompany(str.data(), static_cast<std::uint16_t>(str.length()));
  }
#endif

  SBE_NODISCARD static const char* CityMetaAttribute(
      const MetaAttribute metaAttribute) SBE_NOEXCEPT {
    switch (metaAttribute) {
      case MetaAttribute::PRESENCE:
        return "required";
      default:
        return "";
    }
  }

  static const char* cityCharacterEncoding() SBE_NOEXCEPT { return "UTF-8"; }

  static SBE_CONSTEXPR std::uint64_t citySinceVersion() SBE_NOEXCEPT {
    return 0;
  }

  bool cityInActingVersion() SBE_NOEXCEPT { return true; }

  static SBE_CONSTEXPR std::uint16_t cityId() SBE_NOEXCEPT { return 6; }

  static SBE_CONSTEXPR std::uint64_t cityHeaderLength() SBE_NOEXCEPT {
    return 2;
  }

 private:
  void onCityLengthAccessed() const {
    switch (codecState()) {
      case CodecState::V0_COMPANY_DONE:
        break;
      default:
        throw AccessOrderError(
            std::string("Illegal field access order. ") +
            "Cannot decode length of var data \"City\" in state: " +
            codecStateName(codecState()) +
            ". Expected one of these transitions: [" +
            codecStateTransitions(codecState()) +
            "]. Please see the diagram in the docs of the enum CustomerResponse::CodecState.");
    }
  }

 public:
  SBE_NODISCARD std::uint16_t cityLength() const {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onCityLengthAccessed();
#endif
    std::uint16_t length;
    std::memcpy(&length, m_buffer + sbePosition(), sizeof(std::uint16_t));
    return SBE_LITTLE_ENDIAN_ENCODE_16(length);
  }

 private:
  void onCityAccessed() {
    switch (codecState()) {
      case CodecState::V0_COMPANY_DONE:
        codecState(CodecState::V0_CITY_DONE);
        break;
      default:
        throw AccessOrderError(
            std::string("Illegal field access order. ") +
            "Cannot access field \"City\" in state: " +
            codecStateName(codecState()) +
            ". Expected one of these transitions: [" +
            codecStateTransitions(codecState()) +
            "]. Please see the diagram in the docs of the enum CustomerResponse::CodecState.");
    }
  }

 public:
  std::uint64_t skipCity() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onCityAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    sbePosition(lengthPosition + lengthOfLengthField + dataLength);
    return dataLength;
  }

  SBE_NODISCARD const char* city() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onCityAccessed();
#endif
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + sbePosition(), sizeof(std::uint16_t));
    const char* fieldPtr = m_buffer + sbePosition() + 2;
    sbePosition(
        sbePosition() + 2 + SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue));
    return fieldPtr;
  }

  std::uint64_t getCity(char* dst, const std::uint64_t length) {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onCityAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    std::uint64_t bytesToCopy = length < dataLength ? length : dataLength;
    std::uint64_t pos = sbePosition();
    sbePosition(pos + dataLength);
    std::memcpy(dst, m_buffer + pos, static_cast<std::size_t>(bytesToCopy));
    return bytesToCopy;
  }

  CustomerResponse& putCity(const char* src, const std::uint16_t length) {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onCityAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    std::uint16_t lengthFieldValue = SBE_LITTLE_ENDIAN_ENCODE_16(length);
    sbePosition(lengthPosition + lengthOfLengthField);
    std::memcpy(
        m_buffer + lengthPosition, &lengthFieldValue, sizeof(std::uint16_t));
    if (length != std::uint16_t(0)) {
      std::uint64_t pos = sbePosition();
      sbePosition(pos + length);
      std::memcpy(m_buffer + pos, src, length);
    }
    return *this;
  }

  std::string getCityAsString() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onCityAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    std::uint64_t pos = sbePosition();
    const std::string result(m_buffer + pos, dataLength);
    sbePosition(pos + dataLength);
    return result;
  }

  std::string getCityAsJsonEscapedString() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onCityAccessed();
#endif
    std::ostringstream oss;
    std::string s = getCityAsString();

    for (const auto c : s) {
      switch (c) {
        case '"':
          oss << "\\\"";
          break;
        case '\\':
          oss << "\\\\";
          break;
        case '\b':
          oss << "\\b";
          break;
        case '\f':
          oss << "\\f";
          break;
        case '\n':
          oss << "\\n";
          break;
        case '\r':
          oss << "\\r";
          break;
        case '\t':
          oss << "\\t";
          break;

        default:
          if ('\x00' <= c && c <= '\x1f') {
            oss << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                << (int)(c);
          } else {
            oss << c;
          }
      }
    }

    return oss.str();
  }

#if __cplusplus >= 201703L
  std::string_view getCityAsStringView() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onCityAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    std::uint64_t pos = sbePosition();
    const std::string_view result(m_buffer + pos, dataLength);
    sbePosition(pos + dataLength);
    return result;
  }
#endif

  CustomerResponse& putCity(const std::string& str) {
    if (str.length() > 65534) {
      throw std::runtime_error("std::string too long for length type [E109]");
    }
    return putCity(str.data(), static_cast<std::uint16_t>(str.length()));
  }

#if __cplusplus >= 201703L
  CustomerResponse& putCity(const std::string_view str) {
    if (str.length() > 65534) {
      throw std::runtime_error("std::string too long for length type [E109]");
    }
    return putCity(str.data(), static_cast<std::uint16_t>(str.length()));
  }
#endif

  SBE_NODISCARD static const char* CountryMetaAttribute(
      const MetaAttribute metaAttribute) SBE_NOEXCEPT {
    switch (metaAttribute) {
      case MetaAttribute::PRESENCE:
        return "required";
      default:
        return "";
    }
  }

  static const char* countryCharacterEncoding() SBE_NOEXCEPT { return "UTF-8"; }

  static SBE_CONSTEXPR std::uint64_t countrySinceVersion() SBE_NOEXCEPT {
    return 0;
  }

  bool countryInActingVersion() SBE_NOEXCEPT { return true; }

  static SBE_CONSTEXPR std::uint16_t countryId() SBE_NOEXCEPT { return 7; }

  static SBE_CONSTEXPR std::uint64_t countryHeaderLength() SBE_NOEXCEPT {
    return 2;
  }

 private:
  void onCountryLengthAccessed() const {
    switch (codecState()) {
      case CodecState::V0_CITY_DONE:
        break;
      default:
        throw AccessOrderError(
            std::string("Illegal field access order. ") +
            "Cannot decode length of var data \"Country\" in state: " +
            codecStateName(codecState()) +
            ". Expected one of these transitions: [" +
            codecStateTransitions(codecState()) +
            "]. Please see the diagram in the docs of the enum CustomerResponse::CodecState.");
    }
  }

 public:
  SBE_NODISCARD std::uint16_t countryLength() const {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onCountryLengthAccessed();
#endif
    std::uint16_t length;
    std::memcpy(&length, m_buffer + sbePosition(), sizeof(std::uint16_t));
    return SBE_LITTLE_ENDIAN_ENCODE_16(length);
  }

 private:
  void onCountryAccessed() {
    switch (codecState()) {
      case CodecState::V0_CITY_DONE:
        codecState(CodecState::V0_COUNTRY_DONE);
        break;
      default:
        throw AccessOrderError(
            std::string("Illegal field access order. ") +
            "Cannot access field \"Country\" in state: " +
            codecStateName(codecState()) +
            ". Expected one of these transitions: [" +
            codecStateTransitions(codecState()) +
            "]. Please see the diagram in the docs of the enum CustomerResponse::CodecState.");
    }
  }

 public:
  std::uint64_t skipCountry() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onCountryAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    sbePosition(lengthPosition + lengthOfLengthField + dataLength);
    return dataLength;
  }

  SBE_NODISCARD const char* country() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onCountryAccessed();
#endif
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + sbePosition(), sizeof(std::uint16_t));
    const char* fieldPtr = m_buffer + sbePosition() + 2;
    sbePosition(
        sbePosition() + 2 + SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue));
    return fieldPtr;
  }

  std::uint64_t getCountry(char* dst, const std::uint64_t length) {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onCountryAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    std::uint64_t bytesToCopy = length < dataLength ? length : dataLength;
    std::uint64_t pos = sbePosition();
    sbePosition(pos + dataLength);
    std::memcpy(dst, m_buffer + pos, static_cast<std::size_t>(bytesToCopy));
    return bytesToCopy;
  }

  CustomerResponse& putCountry(const char* src, const std::uint16_t length) {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onCountryAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    std::uint16_t lengthFieldValue = SBE_LITTLE_ENDIAN_ENCODE_16(length);
    sbePosition(lengthPosition + lengthOfLengthField);
    std::memcpy(
        m_buffer + lengthPosition, &lengthFieldValue, sizeof(std::uint16_t));
    if (length != std::uint16_t(0)) {
      std::uint64_t pos = sbePosition();
      sbePosition(pos + length);
      std::memcpy(m_buffer + pos, src, length);
    }
    return *this;
  }

  std::string getCountryAsString() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onCountryAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    std::uint64_t pos = sbePosition();
    const std::string result(m_buffer + pos, dataLength);
    sbePosition(pos + dataLength);
    return result;
  }

  std::string getCountryAsJsonEscapedString() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onCountryAccessed();
#endif
    std::ostringstream oss;
    std::string s = getCountryAsString();

    for (const auto c : s) {
      switch (c) {
        case '"':
          oss << "\\\"";
          break;
        case '\\':
          oss << "\\\\";
          break;
        case '\b':
          oss << "\\b";
          break;
        case '\f':
          oss << "\\f";
          break;
        case '\n':
          oss << "\\n";
          break;
        case '\r':
          oss << "\\r";
          break;
        case '\t':
          oss << "\\t";
          break;

        default:
          if ('\x00' <= c && c <= '\x1f') {
            oss << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                << (int)(c);
          } else {
            oss << c;
          }
      }
    }

    return oss.str();
  }

#if __cplusplus >= 201703L
  std::string_view getCountryAsStringView() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onCountryAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    std::uint64_t pos = sbePosition();
    const std::string_view result(m_buffer + pos, dataLength);
    sbePosition(pos + dataLength);
    return result;
  }
#endif

  CustomerResponse& putCountry(const std::string& str) {
    if (str.length() > 65534) {
      throw std::runtime_error("std::string too long for length type [E109]");
    }
    return putCountry(str.data(), static_cast<std::uint16_t>(str.length()));
  }

#if __cplusplus >= 201703L
  CustomerResponse& putCountry(const std::string_view str) {
    if (str.length() > 65534) {
      throw std::runtime_error("std::string too long for length type [E109]");
    }
    return putCountry(str.data(), static_cast<std::uint16_t>(str.length()));
  }
#endif

  SBE_NODISCARD static const char* Phone1MetaAttribute(
      const MetaAttribute metaAttribute) SBE_NOEXCEPT {
    switch (metaAttribute) {
      case MetaAttribute::PRESENCE:
        return "required";
      default:
        return "";
    }
  }

  static const char* phone1CharacterEncoding() SBE_NOEXCEPT { return "UTF-8"; }

  static SBE_CONSTEXPR std::uint64_t phone1SinceVersion() SBE_NOEXCEPT {
    return 0;
  }

  bool phone1InActingVersion() SBE_NOEXCEPT { return true; }

  static SBE_CONSTEXPR std::uint16_t phone1Id() SBE_NOEXCEPT { return 8; }

  static SBE_CONSTEXPR std::uint64_t phone1HeaderLength() SBE_NOEXCEPT {
    return 2;
  }

 private:
  void onPhone1LengthAccessed() const {
    switch (codecState()) {
      case CodecState::V0_COUNTRY_DONE:
        break;
      default:
        throw AccessOrderError(
            std::string("Illegal field access order. ") +
            "Cannot decode length of var data \"Phone1\" in state: " +
            codecStateName(codecState()) +
            ". Expected one of these transitions: [" +
            codecStateTransitions(codecState()) +
            "]. Please see the diagram in the docs of the enum CustomerResponse::CodecState.");
    }
  }

 public:
  SBE_NODISCARD std::uint16_t phone1Length() const {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onPhone1LengthAccessed();
#endif
    std::uint16_t length;
    std::memcpy(&length, m_buffer + sbePosition(), sizeof(std::uint16_t));
    return SBE_LITTLE_ENDIAN_ENCODE_16(length);
  }

 private:
  void onPhone1Accessed() {
    switch (codecState()) {
      case CodecState::V0_COUNTRY_DONE:
        codecState(CodecState::V0_PHONE1_DONE);
        break;
      default:
        throw AccessOrderError(
            std::string("Illegal field access order. ") +
            "Cannot access field \"Phone1\" in state: " +
            codecStateName(codecState()) +
            ". Expected one of these transitions: [" +
            codecStateTransitions(codecState()) +
            "]. Please see the diagram in the docs of the enum CustomerResponse::CodecState.");
    }
  }

 public:
  std::uint64_t skipPhone1() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onPhone1Accessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    sbePosition(lengthPosition + lengthOfLengthField + dataLength);
    return dataLength;
  }

  SBE_NODISCARD const char* phone1() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onPhone1Accessed();
#endif
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + sbePosition(), sizeof(std::uint16_t));
    const char* fieldPtr = m_buffer + sbePosition() + 2;
    sbePosition(
        sbePosition() + 2 + SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue));
    return fieldPtr;
  }

  std::uint64_t getPhone1(char* dst, const std::uint64_t length) {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onPhone1Accessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    std::uint64_t bytesToCopy = length < dataLength ? length : dataLength;
    std::uint64_t pos = sbePosition();
    sbePosition(pos + dataLength);
    std::memcpy(dst, m_buffer + pos, static_cast<std::size_t>(bytesToCopy));
    return bytesToCopy;
  }

  CustomerResponse& putPhone1(const char* src, const std::uint16_t length) {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onPhone1Accessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    std::uint16_t lengthFieldValue = SBE_LITTLE_ENDIAN_ENCODE_16(length);
    sbePosition(lengthPosition + lengthOfLengthField);
    std::memcpy(
        m_buffer + lengthPosition, &lengthFieldValue, sizeof(std::uint16_t));
    if (length != std::uint16_t(0)) {
      std::uint64_t pos = sbePosition();
      sbePosition(pos + length);
      std::memcpy(m_buffer + pos, src, length);
    }
    return *this;
  }

  std::string getPhone1AsString() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onPhone1Accessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    std::uint64_t pos = sbePosition();
    const std::string result(m_buffer + pos, dataLength);
    sbePosition(pos + dataLength);
    return result;
  }

  std::string getPhone1AsJsonEscapedString() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onPhone1Accessed();
#endif
    std::ostringstream oss;
    std::string s = getPhone1AsString();

    for (const auto c : s) {
      switch (c) {
        case '"':
          oss << "\\\"";
          break;
        case '\\':
          oss << "\\\\";
          break;
        case '\b':
          oss << "\\b";
          break;
        case '\f':
          oss << "\\f";
          break;
        case '\n':
          oss << "\\n";
          break;
        case '\r':
          oss << "\\r";
          break;
        case '\t':
          oss << "\\t";
          break;

        default:
          if ('\x00' <= c && c <= '\x1f') {
            oss << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                << (int)(c);
          } else {
            oss << c;
          }
      }
    }

    return oss.str();
  }

#if __cplusplus >= 201703L
  std::string_view getPhone1AsStringView() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onPhone1Accessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    std::uint64_t pos = sbePosition();
    const std::string_view result(m_buffer + pos, dataLength);
    sbePosition(pos + dataLength);
    return result;
  }
#endif

  CustomerResponse& putPhone1(const std::string& str) {
    if (str.length() > 65534) {
      throw std::runtime_error("std::string too long for length type [E109]");
    }
    return putPhone1(str.data(), static_cast<std::uint16_t>(str.length()));
  }

#if __cplusplus >= 201703L
  CustomerResponse& putPhone1(const std::string_view str) {
    if (str.length() > 65534) {
      throw std::runtime_error("std::string too long for length type [E109]");
    }
    return putPhone1(str.data(), static_cast<std::uint16_t>(str.length()));
  }
#endif

  SBE_NODISCARD static const char* Phone2MetaAttribute(
      const MetaAttribute metaAttribute) SBE_NOEXCEPT {
    switch (metaAttribute) {
      case MetaAttribute::PRESENCE:
        return "required";
      default:
        return "";
    }
  }

  static const char* phone2CharacterEncoding() SBE_NOEXCEPT { return "UTF-8"; }

  static SBE_CONSTEXPR std::uint64_t phone2SinceVersion() SBE_NOEXCEPT {
    return 0;
  }

  bool phone2InActingVersion() SBE_NOEXCEPT { return true; }

  static SBE_CONSTEXPR std::uint16_t phone2Id() SBE_NOEXCEPT { return 9; }

  static SBE_CONSTEXPR std::uint64_t phone2HeaderLength() SBE_NOEXCEPT {
    return 2;
  }

 private:
  void onPhone2LengthAccessed() const {
    switch (codecState()) {
      case CodecState::V0_PHONE1_DONE:
        break;
      default:
        throw AccessOrderError(
            std::string("Illegal field access order. ") +
            "Cannot decode length of var data \"Phone2\" in state: " +
            codecStateName(codecState()) +
            ". Expected one of these transitions: [" +
            codecStateTransitions(codecState()) +
            "]. Please see the diagram in the docs of the enum CustomerResponse::CodecState.");
    }
  }

 public:
  SBE_NODISCARD std::uint16_t phone2Length() const {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onPhone2LengthAccessed();
#endif
    std::uint16_t length;
    std::memcpy(&length, m_buffer + sbePosition(), sizeof(std::uint16_t));
    return SBE_LITTLE_ENDIAN_ENCODE_16(length);
  }

 private:
  void onPhone2Accessed() {
    switch (codecState()) {
      case CodecState::V0_PHONE1_DONE:
        codecState(CodecState::V0_PHONE2_DONE);
        break;
      default:
        throw AccessOrderError(
            std::string("Illegal field access order. ") +
            "Cannot access field \"Phone2\" in state: " +
            codecStateName(codecState()) +
            ". Expected one of these transitions: [" +
            codecStateTransitions(codecState()) +
            "]. Please see the diagram in the docs of the enum CustomerResponse::CodecState.");
    }
  }

 public:
  std::uint64_t skipPhone2() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onPhone2Accessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    sbePosition(lengthPosition + lengthOfLengthField + dataLength);
    return dataLength;
  }

  SBE_NODISCARD const char* phone2() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onPhone2Accessed();
#endif
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + sbePosition(), sizeof(std::uint16_t));
    const char* fieldPtr = m_buffer + sbePosition() + 2;
    sbePosition(
        sbePosition() + 2 + SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue));
    return fieldPtr;
  }

  std::uint64_t getPhone2(char* dst, const std::uint64_t length) {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onPhone2Accessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    std::uint64_t bytesToCopy = length < dataLength ? length : dataLength;
    std::uint64_t pos = sbePosition();
    sbePosition(pos + dataLength);
    std::memcpy(dst, m_buffer + pos, static_cast<std::size_t>(bytesToCopy));
    return bytesToCopy;
  }

  CustomerResponse& putPhone2(const char* src, const std::uint16_t length) {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onPhone2Accessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    std::uint16_t lengthFieldValue = SBE_LITTLE_ENDIAN_ENCODE_16(length);
    sbePosition(lengthPosition + lengthOfLengthField);
    std::memcpy(
        m_buffer + lengthPosition, &lengthFieldValue, sizeof(std::uint16_t));
    if (length != std::uint16_t(0)) {
      std::uint64_t pos = sbePosition();
      sbePosition(pos + length);
      std::memcpy(m_buffer + pos, src, length);
    }
    return *this;
  }

  std::string getPhone2AsString() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onPhone2Accessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    std::uint64_t pos = sbePosition();
    const std::string result(m_buffer + pos, dataLength);
    sbePosition(pos + dataLength);
    return result;
  }

  std::string getPhone2AsJsonEscapedString() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onPhone2Accessed();
#endif
    std::ostringstream oss;
    std::string s = getPhone2AsString();

    for (const auto c : s) {
      switch (c) {
        case '"':
          oss << "\\\"";
          break;
        case '\\':
          oss << "\\\\";
          break;
        case '\b':
          oss << "\\b";
          break;
        case '\f':
          oss << "\\f";
          break;
        case '\n':
          oss << "\\n";
          break;
        case '\r':
          oss << "\\r";
          break;
        case '\t':
          oss << "\\t";
          break;

        default:
          if ('\x00' <= c && c <= '\x1f') {
            oss << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                << (int)(c);
          } else {
            oss << c;
          }
      }
    }

    return oss.str();
  }

#if __cplusplus >= 201703L
  std::string_view getPhone2AsStringView() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onPhone2Accessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    std::uint64_t pos = sbePosition();
    const std::string_view result(m_buffer + pos, dataLength);
    sbePosition(pos + dataLength);
    return result;
  }
#endif

  CustomerResponse& putPhone2(const std::string& str) {
    if (str.length() > 65534) {
      throw std::runtime_error("std::string too long for length type [E109]");
    }
    return putPhone2(str.data(), static_cast<std::uint16_t>(str.length()));
  }

#if __cplusplus >= 201703L
  CustomerResponse& putPhone2(const std::string_view str) {
    if (str.length() > 65534) {
      throw std::runtime_error("std::string too long for length type [E109]");
    }
    return putPhone2(str.data(), static_cast<std::uint16_t>(str.length()));
  }
#endif

  SBE_NODISCARD static const char* EmailMetaAttribute(
      const MetaAttribute metaAttribute) SBE_NOEXCEPT {
    switch (metaAttribute) {
      case MetaAttribute::PRESENCE:
        return "required";
      default:
        return "";
    }
  }

  static const char* emailCharacterEncoding() SBE_NOEXCEPT { return "UTF-8"; }

  static SBE_CONSTEXPR std::uint64_t emailSinceVersion() SBE_NOEXCEPT {
    return 0;
  }

  bool emailInActingVersion() SBE_NOEXCEPT { return true; }

  static SBE_CONSTEXPR std::uint16_t emailId() SBE_NOEXCEPT { return 10; }

  static SBE_CONSTEXPR std::uint64_t emailHeaderLength() SBE_NOEXCEPT {
    return 2;
  }

 private:
  void onEmailLengthAccessed() const {
    switch (codecState()) {
      case CodecState::V0_PHONE2_DONE:
        break;
      default:
        throw AccessOrderError(
            std::string("Illegal field access order. ") +
            "Cannot decode length of var data \"Email\" in state: " +
            codecStateName(codecState()) +
            ". Expected one of these transitions: [" +
            codecStateTransitions(codecState()) +
            "]. Please see the diagram in the docs of the enum CustomerResponse::CodecState.");
    }
  }

 public:
  SBE_NODISCARD std::uint16_t emailLength() const {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onEmailLengthAccessed();
#endif
    std::uint16_t length;
    std::memcpy(&length, m_buffer + sbePosition(), sizeof(std::uint16_t));
    return SBE_LITTLE_ENDIAN_ENCODE_16(length);
  }

 private:
  void onEmailAccessed() {
    switch (codecState()) {
      case CodecState::V0_PHONE2_DONE:
        codecState(CodecState::V0_EMAIL_DONE);
        break;
      default:
        throw AccessOrderError(
            std::string("Illegal field access order. ") +
            "Cannot access field \"Email\" in state: " +
            codecStateName(codecState()) +
            ". Expected one of these transitions: [" +
            codecStateTransitions(codecState()) +
            "]. Please see the diagram in the docs of the enum CustomerResponse::CodecState.");
    }
  }

 public:
  std::uint64_t skipEmail() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onEmailAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    sbePosition(lengthPosition + lengthOfLengthField + dataLength);
    return dataLength;
  }

  SBE_NODISCARD const char* email() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onEmailAccessed();
#endif
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + sbePosition(), sizeof(std::uint16_t));
    const char* fieldPtr = m_buffer + sbePosition() + 2;
    sbePosition(
        sbePosition() + 2 + SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue));
    return fieldPtr;
  }

  std::uint64_t getEmail(char* dst, const std::uint64_t length) {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onEmailAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    std::uint64_t bytesToCopy = length < dataLength ? length : dataLength;
    std::uint64_t pos = sbePosition();
    sbePosition(pos + dataLength);
    std::memcpy(dst, m_buffer + pos, static_cast<std::size_t>(bytesToCopy));
    return bytesToCopy;
  }

  CustomerResponse& putEmail(const char* src, const std::uint16_t length) {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onEmailAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    std::uint16_t lengthFieldValue = SBE_LITTLE_ENDIAN_ENCODE_16(length);
    sbePosition(lengthPosition + lengthOfLengthField);
    std::memcpy(
        m_buffer + lengthPosition, &lengthFieldValue, sizeof(std::uint16_t));
    if (length != std::uint16_t(0)) {
      std::uint64_t pos = sbePosition();
      sbePosition(pos + length);
      std::memcpy(m_buffer + pos, src, length);
    }
    return *this;
  }

  std::string getEmailAsString() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onEmailAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    std::uint64_t pos = sbePosition();
    const std::string result(m_buffer + pos, dataLength);
    sbePosition(pos + dataLength);
    return result;
  }

  std::string getEmailAsJsonEscapedString() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onEmailAccessed();
#endif
    std::ostringstream oss;
    std::string s = getEmailAsString();

    for (const auto c : s) {
      switch (c) {
        case '"':
          oss << "\\\"";
          break;
        case '\\':
          oss << "\\\\";
          break;
        case '\b':
          oss << "\\b";
          break;
        case '\f':
          oss << "\\f";
          break;
        case '\n':
          oss << "\\n";
          break;
        case '\r':
          oss << "\\r";
          break;
        case '\t':
          oss << "\\t";
          break;

        default:
          if ('\x00' <= c && c <= '\x1f') {
            oss << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                << (int)(c);
          } else {
            oss << c;
          }
      }
    }

    return oss.str();
  }

#if __cplusplus >= 201703L
  std::string_view getEmailAsStringView() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onEmailAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    std::uint64_t pos = sbePosition();
    const std::string_view result(m_buffer + pos, dataLength);
    sbePosition(pos + dataLength);
    return result;
  }
#endif

  CustomerResponse& putEmail(const std::string& str) {
    if (str.length() > 65534) {
      throw std::runtime_error("std::string too long for length type [E109]");
    }
    return putEmail(str.data(), static_cast<std::uint16_t>(str.length()));
  }

#if __cplusplus >= 201703L
  CustomerResponse& putEmail(const std::string_view str) {
    if (str.length() > 65534) {
      throw std::runtime_error("std::string too long for length type [E109]");
    }
    return putEmail(str.data(), static_cast<std::uint16_t>(str.length()));
  }
#endif

  SBE_NODISCARD static const char* SubscriptionDateMetaAttribute(
      const MetaAttribute metaAttribute) SBE_NOEXCEPT {
    switch (metaAttribute) {
      case MetaAttribute::PRESENCE:
        return "required";
      default:
        return "";
    }
  }

  static const char* subscriptionDateCharacterEncoding() SBE_NOEXCEPT {
    return "UTF-8";
  }

  static SBE_CONSTEXPR std::uint64_t subscriptionDateSinceVersion()
      SBE_NOEXCEPT {
    return 0;
  }

  bool subscriptionDateInActingVersion() SBE_NOEXCEPT { return true; }

  static SBE_CONSTEXPR std::uint16_t subscriptionDateId() SBE_NOEXCEPT {
    return 11;
  }

  static SBE_CONSTEXPR std::uint64_t subscriptionDateHeaderLength()
      SBE_NOEXCEPT {
    return 2;
  }

 private:
  void onSubscriptionDateLengthAccessed() const {
    switch (codecState()) {
      case CodecState::V0_EMAIL_DONE:
        break;
      default:
        throw AccessOrderError(
            std::string("Illegal field access order. ") +
            "Cannot decode length of var data \"SubscriptionDate\" in state: " +
            codecStateName(codecState()) +
            ". Expected one of these transitions: [" +
            codecStateTransitions(codecState()) +
            "]. Please see the diagram in the docs of the enum CustomerResponse::CodecState.");
    }
  }

 public:
  SBE_NODISCARD std::uint16_t subscriptionDateLength() const {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onSubscriptionDateLengthAccessed();
#endif
    std::uint16_t length;
    std::memcpy(&length, m_buffer + sbePosition(), sizeof(std::uint16_t));
    return SBE_LITTLE_ENDIAN_ENCODE_16(length);
  }

 private:
  void onSubscriptionDateAccessed() {
    switch (codecState()) {
      case CodecState::V0_EMAIL_DONE:
        codecState(CodecState::V0_SUBSCRIPTIONDATE_DONE);
        break;
      default:
        throw AccessOrderError(
            std::string("Illegal field access order. ") +
            "Cannot access field \"SubscriptionDate\" in state: " +
            codecStateName(codecState()) +
            ". Expected one of these transitions: [" +
            codecStateTransitions(codecState()) +
            "]. Please see the diagram in the docs of the enum CustomerResponse::CodecState.");
    }
  }

 public:
  std::uint64_t skipSubscriptionDate() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onSubscriptionDateAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    sbePosition(lengthPosition + lengthOfLengthField + dataLength);
    return dataLength;
  }

  SBE_NODISCARD const char* subscriptionDate() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onSubscriptionDateAccessed();
#endif
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + sbePosition(), sizeof(std::uint16_t));
    const char* fieldPtr = m_buffer + sbePosition() + 2;
    sbePosition(
        sbePosition() + 2 + SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue));
    return fieldPtr;
  }

  std::uint64_t getSubscriptionDate(char* dst, const std::uint64_t length) {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onSubscriptionDateAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    std::uint64_t bytesToCopy = length < dataLength ? length : dataLength;
    std::uint64_t pos = sbePosition();
    sbePosition(pos + dataLength);
    std::memcpy(dst, m_buffer + pos, static_cast<std::size_t>(bytesToCopy));
    return bytesToCopy;
  }

  CustomerResponse& putSubscriptionDate(
      const char* src, const std::uint16_t length) {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onSubscriptionDateAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    std::uint16_t lengthFieldValue = SBE_LITTLE_ENDIAN_ENCODE_16(length);
    sbePosition(lengthPosition + lengthOfLengthField);
    std::memcpy(
        m_buffer + lengthPosition, &lengthFieldValue, sizeof(std::uint16_t));
    if (length != std::uint16_t(0)) {
      std::uint64_t pos = sbePosition();
      sbePosition(pos + length);
      std::memcpy(m_buffer + pos, src, length);
    }
    return *this;
  }

  std::string getSubscriptionDateAsString() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onSubscriptionDateAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    std::uint64_t pos = sbePosition();
    const std::string result(m_buffer + pos, dataLength);
    sbePosition(pos + dataLength);
    return result;
  }

  std::string getSubscriptionDateAsJsonEscapedString() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onSubscriptionDateAccessed();
#endif
    std::ostringstream oss;
    std::string s = getSubscriptionDateAsString();

    for (const auto c : s) {
      switch (c) {
        case '"':
          oss << "\\\"";
          break;
        case '\\':
          oss << "\\\\";
          break;
        case '\b':
          oss << "\\b";
          break;
        case '\f':
          oss << "\\f";
          break;
        case '\n':
          oss << "\\n";
          break;
        case '\r':
          oss << "\\r";
          break;
        case '\t':
          oss << "\\t";
          break;

        default:
          if ('\x00' <= c && c <= '\x1f') {
            oss << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                << (int)(c);
          } else {
            oss << c;
          }
      }
    }

    return oss.str();
  }

#if __cplusplus >= 201703L
  std::string_view getSubscriptionDateAsStringView() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onSubscriptionDateAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    std::uint64_t pos = sbePosition();
    const std::string_view result(m_buffer + pos, dataLength);
    sbePosition(pos + dataLength);
    return result;
  }
#endif

  CustomerResponse& putSubscriptionDate(const std::string& str) {
    if (str.length() > 65534) {
      throw std::runtime_error("std::string too long for length type [E109]");
    }
    return putSubscriptionDate(
        str.data(), static_cast<std::uint16_t>(str.length()));
  }

#if __cplusplus >= 201703L
  CustomerResponse& putSubscriptionDate(const std::string_view str) {
    if (str.length() > 65534) {
      throw std::runtime_error("std::string too long for length type [E109]");
    }
    return putSubscriptionDate(
        str.data(), static_cast<std::uint16_t>(str.length()));
  }
#endif

  SBE_NODISCARD static const char* WebSiteMetaAttribute(
      const MetaAttribute metaAttribute) SBE_NOEXCEPT {
    switch (metaAttribute) {
      case MetaAttribute::PRESENCE:
        return "required";
      default:
        return "";
    }
  }

  static const char* webSiteCharacterEncoding() SBE_NOEXCEPT { return "UTF-8"; }

  static SBE_CONSTEXPR std::uint64_t webSiteSinceVersion() SBE_NOEXCEPT {
    return 0;
  }

  bool webSiteInActingVersion() SBE_NOEXCEPT { return true; }

  static SBE_CONSTEXPR std::uint16_t webSiteId() SBE_NOEXCEPT { return 12; }

  static SBE_CONSTEXPR std::uint64_t webSiteHeaderLength() SBE_NOEXCEPT {
    return 2;
  }

 private:
  void onWebSiteLengthAccessed() const {
    switch (codecState()) {
      case CodecState::V0_SUBSCRIPTIONDATE_DONE:
        break;
      default:
        throw AccessOrderError(
            std::string("Illegal field access order. ") +
            "Cannot decode length of var data \"WebSite\" in state: " +
            codecStateName(codecState()) +
            ". Expected one of these transitions: [" +
            codecStateTransitions(codecState()) +
            "]. Please see the diagram in the docs of the enum CustomerResponse::CodecState.");
    }
  }

 public:
  SBE_NODISCARD std::uint16_t webSiteLength() const {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onWebSiteLengthAccessed();
#endif
    std::uint16_t length;
    std::memcpy(&length, m_buffer + sbePosition(), sizeof(std::uint16_t));
    return SBE_LITTLE_ENDIAN_ENCODE_16(length);
  }

 private:
  void onWebSiteAccessed() {
    switch (codecState()) {
      case CodecState::V0_SUBSCRIPTIONDATE_DONE:
        codecState(CodecState::V0_WEBSITE_DONE);
        break;
      default:
        throw AccessOrderError(
            std::string("Illegal field access order. ") +
            "Cannot access field \"WebSite\" in state: " +
            codecStateName(codecState()) +
            ". Expected one of these transitions: [" +
            codecStateTransitions(codecState()) +
            "]. Please see the diagram in the docs of the enum CustomerResponse::CodecState.");
    }
  }

 public:
  std::uint64_t skipWebSite() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onWebSiteAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    sbePosition(lengthPosition + lengthOfLengthField + dataLength);
    return dataLength;
  }

  SBE_NODISCARD const char* webSite() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onWebSiteAccessed();
#endif
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + sbePosition(), sizeof(std::uint16_t));
    const char* fieldPtr = m_buffer + sbePosition() + 2;
    sbePosition(
        sbePosition() + 2 + SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue));
    return fieldPtr;
  }

  std::uint64_t getWebSite(char* dst, const std::uint64_t length) {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onWebSiteAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    std::uint64_t bytesToCopy = length < dataLength ? length : dataLength;
    std::uint64_t pos = sbePosition();
    sbePosition(pos + dataLength);
    std::memcpy(dst, m_buffer + pos, static_cast<std::size_t>(bytesToCopy));
    return bytesToCopy;
  }

  CustomerResponse& putWebSite(const char* src, const std::uint16_t length) {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onWebSiteAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    std::uint16_t lengthFieldValue = SBE_LITTLE_ENDIAN_ENCODE_16(length);
    sbePosition(lengthPosition + lengthOfLengthField);
    std::memcpy(
        m_buffer + lengthPosition, &lengthFieldValue, sizeof(std::uint16_t));
    if (length != std::uint16_t(0)) {
      std::uint64_t pos = sbePosition();
      sbePosition(pos + length);
      std::memcpy(m_buffer + pos, src, length);
    }
    return *this;
  }

  std::string getWebSiteAsString() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onWebSiteAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    std::uint64_t pos = sbePosition();
    const std::string result(m_buffer + pos, dataLength);
    sbePosition(pos + dataLength);
    return result;
  }

  std::string getWebSiteAsJsonEscapedString() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onWebSiteAccessed();
#endif
    std::ostringstream oss;
    std::string s = getWebSiteAsString();

    for (const auto c : s) {
      switch (c) {
        case '"':
          oss << "\\\"";
          break;
        case '\\':
          oss << "\\\\";
          break;
        case '\b':
          oss << "\\b";
          break;
        case '\f':
          oss << "\\f";
          break;
        case '\n':
          oss << "\\n";
          break;
        case '\r':
          oss << "\\r";
          break;
        case '\t':
          oss << "\\t";
          break;

        default:
          if ('\x00' <= c && c <= '\x1f') {
            oss << "\\u" << std::hex << std::setw(4) << std::setfill('0')
                << (int)(c);
          } else {
            oss << c;
          }
      }
    }

    return oss.str();
  }

#if __cplusplus >= 201703L
  std::string_view getWebSiteAsStringView() {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
    onWebSiteAccessed();
#endif
    std::uint64_t lengthOfLengthField = 2;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint16_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint16_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_16(lengthFieldValue);
    std::uint64_t pos = sbePosition();
    const std::string_view result(m_buffer + pos, dataLength);
    sbePosition(pos + dataLength);
    return result;
  }
#endif

  CustomerResponse& putWebSite(const std::string& str) {
    if (str.length() > 65534) {
      throw std::runtime_error("std::string too long for length type [E109]");
    }
    return putWebSite(str.data(), static_cast<std::uint16_t>(str.length()));
  }

#if __cplusplus >= 201703L
  CustomerResponse& putWebSite(const std::string_view str) {
    if (str.length() > 65534) {
      throw std::runtime_error("std::string too long for length type [E109]");
    }
    return putWebSite(str.data(), static_cast<std::uint16_t>(str.length()));
  }
#endif

  template <typename CharT, typename Traits>
  friend std::basic_ostream<CharT, Traits>& operator<<(
      std::basic_ostream<CharT, Traits>& builder,
      const CustomerResponse& _writer) {
    CustomerResponse writer(
        _writer.m_buffer,
        _writer.m_offset,
        _writer.m_bufferLength,
        _writer.m_actingBlockLength,
        _writer.m_actingVersion);

    builder << '{';
    builder << R"("Name": "CustomerResponse", )";
    builder << R"("sbeTemplateId": )";
    builder << writer.sbeTemplateId();
    builder << ", ";

    builder << R"("Index": )";
    builder << +writer.index();

    builder << ", ";
    builder << R"("CustomerId": )";
    builder << '"' << writer.getCustomerIdAsJsonEscapedString().c_str() << '"';

    builder << ", ";
    builder << R"("FirstName": )";
    builder << '"' << writer.getFirstNameAsJsonEscapedString().c_str() << '"';

    builder << ", ";
    builder << R"("LastName": )";
    builder << '"' << writer.getLastNameAsJsonEscapedString().c_str() << '"';

    builder << ", ";
    builder << R"("Company": )";
    builder << '"' << writer.getCompanyAsJsonEscapedString().c_str() << '"';

    builder << ", ";
    builder << R"("City": )";
    builder << '"' << writer.getCityAsJsonEscapedString().c_str() << '"';

    builder << ", ";
    builder << R"("Country": )";
    builder << '"' << writer.getCountryAsJsonEscapedString().c_str() << '"';

    builder << ", ";
    builder << R"("Phone1": )";
    builder << '"' << writer.getPhone1AsJsonEscapedString().c_str() << '"';

    builder << ", ";
    builder << R"("Phone2": )";
    builder << '"' << writer.getPhone2AsJsonEscapedString().c_str() << '"';

    builder << ", ";
    builder << R"("Email": )";
    builder << '"' << writer.getEmailAsJsonEscapedString().c_str() << '"';

    builder << ", ";
    builder << R"("SubscriptionDate": )";
    builder << '"' << writer.getSubscriptionDateAsJsonEscapedString().c_str()
            << '"';

    builder << ", ";
    builder << R"("WebSite": )";
    builder << '"' << writer.getWebSiteAsJsonEscapedString().c_str() << '"';

    builder << '}';

    return builder;
  }

  void skip() {
    skipCustomerId();
    skipFirstName();
    skipLastName();
    skipCompany();
    skipCity();
    skipCountry();
    skipPhone1();
    skipPhone2();
    skipEmail();
    skipSubscriptionDate();
    skipWebSite();
  }

  SBE_NODISCARD static SBE_CONSTEXPR bool isConstLength() SBE_NOEXCEPT {
    return false;
  }

  SBE_NODISCARD static std::size_t computeLength(
      std::size_t customerIdLength = 0,
      std::size_t firstNameLength = 0,
      std::size_t lastNameLength = 0,
      std::size_t companyLength = 0,
      std::size_t cityLength = 0,
      std::size_t countryLength = 0,
      std::size_t phone1Length = 0,
      std::size_t phone2Length = 0,
      std::size_t emailLength = 0,
      std::size_t subscriptionDateLength = 0,
      std::size_t webSiteLength = 0) {
#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif
    std::size_t length = sbeBlockLength();

    length += customerIdHeaderLength();
    if (customerIdLength > 254LL) {
      throw std::runtime_error(
          "customerIdLength too long for length type [E109]");
    }
    length += customerIdLength;

    length += firstNameHeaderLength();
    if (firstNameLength > 65534LL) {
      throw std::runtime_error(
          "firstNameLength too long for length type [E109]");
    }
    length += firstNameLength;

    length += lastNameHeaderLength();
    if (lastNameLength > 65534LL) {
      throw std::runtime_error(
          "lastNameLength too long for length type [E109]");
    }
    length += lastNameLength;

    length += companyHeaderLength();
    if (companyLength > 65534LL) {
      throw std::runtime_error("companyLength too long for length type [E109]");
    }
    length += companyLength;

    length += cityHeaderLength();
    if (cityLength > 65534LL) {
      throw std::runtime_error("cityLength too long for length type [E109]");
    }
    length += cityLength;

    length += countryHeaderLength();
    if (countryLength > 65534LL) {
      throw std::runtime_error("countryLength too long for length type [E109]");
    }
    length += countryLength;

    length += phone1HeaderLength();
    if (phone1Length > 65534LL) {
      throw std::runtime_error("phone1Length too long for length type [E109]");
    }
    length += phone1Length;

    length += phone2HeaderLength();
    if (phone2Length > 65534LL) {
      throw std::runtime_error("phone2Length too long for length type [E109]");
    }
    length += phone2Length;

    length += emailHeaderLength();
    if (emailLength > 65534LL) {
      throw std::runtime_error("emailLength too long for length type [E109]");
    }
    length += emailLength;

    length += subscriptionDateHeaderLength();
    if (subscriptionDateLength > 65534LL) {
      throw std::runtime_error(
          "subscriptionDateLength too long for length type [E109]");
    }
    length += subscriptionDateLength;

    length += webSiteHeaderLength();
    if (webSiteLength > 65534LL) {
      throw std::runtime_error("webSiteLength too long for length type [E109]");
    }
    length += webSiteLength;

    return length;
#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
  }
};
#if SBE_ENABLE_PRECEDENCE_CHECKS == 1
const std::string CustomerResponse::STATE_NAME_LOOKUP[13] = {
    "NOT_WRAPPED",
    "V0_BLOCK",
    "V0_CUSTOMERID_DONE",
    "V0_FIRSTNAME_DONE",
    "V0_LASTNAME_DONE",
    "V0_COMPANY_DONE",
    "V0_CITY_DONE",
    "V0_COUNTRY_DONE",
    "V0_PHONE1_DONE",
    "V0_PHONE2_DONE",
    "V0_EMAIL_DONE",
    "V0_SUBSCRIPTIONDATE_DONE",
    "V0_WEBSITE_DONE",
};

const std::string CustomerResponse::STATE_TRANSITIONS_LOOKUP[13] = {
    "\"wrap(version=0)\"",
    "\"Index(?)\", \"CustomerIdLength()\", \"CustomerId(?)\"",
    "\"FirstNameLength()\", \"FirstName(?)\"",
    "\"LastNameLength()\", \"LastName(?)\"",
    "\"CompanyLength()\", \"Company(?)\"",
    "\"CityLength()\", \"City(?)\"",
    "\"CountryLength()\", \"Country(?)\"",
    "\"Phone1Length()\", \"Phone1(?)\"",
    "\"Phone2Length()\", \"Phone2(?)\"",
    "\"EmailLength()\", \"Email(?)\"",
    "\"SubscriptionDateLength()\", \"SubscriptionDate(?)\"",
    "\"WebSiteLength()\", \"WebSite(?)\"",
    "",
};
#endif

} // namespace test
} // namespace sbe
} // namespace facebook
#endif
