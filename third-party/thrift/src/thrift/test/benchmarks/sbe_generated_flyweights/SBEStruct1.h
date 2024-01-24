/* Generated SBE (Simple Binary Encoding) message codec */
#ifndef _APACHE_THRIFT_BENCHMARKS_SBESTRUCT1_CXX_H_
#define _APACHE_THRIFT_BENCHMARKS_SBESTRUCT1_CXX_H_

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

#include "GroupSizeEncoding.h"
#include "MessageHeader.h"
#include "VarStringEncoding.h"

namespace apache {
namespace thrift {
namespace benchmarks {

class SBEStruct1 {
 private:
  char* m_buffer = nullptr;
  std::uint64_t m_bufferLength = 0;
  std::uint64_t m_offset = 0;
  std::uint64_t m_position = 0;
  std::uint64_t m_actingBlockLength = 0;
  std::uint64_t m_actingVersion = 0;

  inline std::uint64_t* sbePositionPtr() SBE_NOEXCEPT { return &m_position; }

 public:
  static const std::uint16_t SBE_BLOCK_LENGTH = static_cast<std::uint16_t>(4);
  static const std::uint16_t SBE_TEMPLATE_ID = static_cast<std::uint16_t>(1);
  static const std::uint16_t SBE_SCHEMA_ID = static_cast<std::uint16_t>(1);
  static const std::uint16_t SBE_SCHEMA_VERSION = static_cast<std::uint16_t>(0);
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

  SBEStruct1() = default;

  SBEStruct1(
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

  SBEStruct1(char* buffer, const std::uint64_t bufferLength)
      : SBEStruct1(
            buffer, 0, bufferLength, sbeBlockLength(), sbeSchemaVersion()) {}

  SBEStruct1(
      char* buffer,
      const std::uint64_t bufferLength,
      const std::uint64_t actingBlockLength,
      const std::uint64_t actingVersion)
      : SBEStruct1(buffer, 0, bufferLength, actingBlockLength, actingVersion) {}

  SBE_NODISCARD static SBE_CONSTEXPR std::uint16_t sbeBlockLength()
      SBE_NOEXCEPT {
    return static_cast<std::uint16_t>(4);
  }

  SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t sbeBlockAndHeaderLength()
      SBE_NOEXCEPT {
    return messageHeader::encodedLength() + sbeBlockLength();
  }

  SBE_NODISCARD static SBE_CONSTEXPR std::uint16_t sbeTemplateId()
      SBE_NOEXCEPT {
    return static_cast<std::uint16_t>(1);
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

  SBEStruct1& wrapForEncode(
      char* buffer,
      const std::uint64_t offset,
      const std::uint64_t bufferLength) {
    m_buffer = buffer;
    m_bufferLength = bufferLength;
    m_offset = offset;
    m_actingBlockLength = sbeBlockLength();
    m_actingVersion = sbeSchemaVersion();
    m_position = sbeCheckPosition(m_offset + m_actingBlockLength);
    return *this;
  }

  SBEStruct1& wrapAndApplyHeader(
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
    return *this;
  }

  SBEStruct1& wrapForDecode(
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
    return *this;
  }

  SBEStruct1& sbeRewind() {
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
    SBEStruct1 skipper(
        m_buffer, m_offset, m_bufferLength, sbeBlockLength(), m_actingVersion);
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

  SBE_NODISCARD static const char* field_1MetaAttribute(
      const MetaAttribute metaAttribute) SBE_NOEXCEPT {
    switch (metaAttribute) {
      case MetaAttribute::PRESENCE:
        return "required";
      default:
        return "";
    }
  }

  static SBE_CONSTEXPR std::uint16_t field_1Id() SBE_NOEXCEPT { return 1; }

  SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t field_1SinceVersion()
      SBE_NOEXCEPT {
    return 0;
  }

  SBE_NODISCARD bool field_1InActingVersion() SBE_NOEXCEPT { return true; }

  SBE_NODISCARD static SBE_CONSTEXPR std::size_t field_1EncodingOffset()
      SBE_NOEXCEPT {
    return 0;
  }

  static SBE_CONSTEXPR std::int32_t field_1NullValue() SBE_NOEXCEPT {
    return SBE_NULLVALUE_INT32;
  }

  static SBE_CONSTEXPR std::int32_t field_1MinValue() SBE_NOEXCEPT {
    return INT32_C(-2147483647);
  }

  static SBE_CONSTEXPR std::int32_t field_1MaxValue() SBE_NOEXCEPT {
    return INT32_C(2147483647);
  }

  static SBE_CONSTEXPR std::size_t field_1EncodingLength() SBE_NOEXCEPT {
    return 4;
  }

  SBE_NODISCARD std::int32_t field_1() const SBE_NOEXCEPT {
    std::int32_t val;
    std::memcpy(&val, m_buffer + m_offset + 0, sizeof(std::int32_t));
    return SBE_LITTLE_ENDIAN_ENCODE_32(val);
  }

  SBEStruct1& field_1(const std::int32_t value) SBE_NOEXCEPT {
    std::int32_t val = SBE_LITTLE_ENDIAN_ENCODE_32(value);
    std::memcpy(m_buffer + m_offset + 0, &val, sizeof(std::int32_t));
    return *this;
  }

  class Field_3 {
   private:
    char* m_buffer = nullptr;
    std::uint64_t m_bufferLength = 0;
    std::uint64_t m_initialPosition = 0;
    std::uint64_t* m_positionPtr = nullptr;
    std::uint64_t m_blockLength = 0;
    std::uint64_t m_count = 0;
    std::uint64_t m_index = 0;
    std::uint64_t m_offset = 0;
    std::uint64_t m_actingVersion = 0;

    SBE_NODISCARD std::uint64_t* sbePositionPtr() SBE_NOEXCEPT {
      return m_positionPtr;
    }

   public:
    Field_3() = default;

    inline void wrapForDecode(
        char* buffer,
        std::uint64_t* pos,
        const std::uint64_t actingVersion,
        const std::uint64_t bufferLength) {
      GroupSizeEncoding dimensions(buffer, *pos, bufferLength, actingVersion);
      m_buffer = buffer;
      m_bufferLength = bufferLength;
      m_blockLength = dimensions.blockLength();
      m_count = dimensions.numInGroup();
      m_index = 0;
      m_actingVersion = actingVersion;
      m_initialPosition = *pos;
      m_positionPtr = pos;
      *m_positionPtr = *m_positionPtr + 4;
    }

    inline void wrapForEncode(
        char* buffer,
        const std::uint16_t count,
        std::uint64_t* pos,
        const std::uint64_t actingVersion,
        const std::uint64_t bufferLength) {
#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif
      if (count > 65534) {
        throw std::runtime_error("count outside of allowed range [E110]");
      }
#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
      m_buffer = buffer;
      m_bufferLength = bufferLength;
      GroupSizeEncoding dimensions(buffer, *pos, bufferLength, actingVersion);
      dimensions.blockLength(static_cast<std::uint16_t>(0));
      dimensions.numInGroup(static_cast<std::uint16_t>(count));
      m_index = 0;
      m_count = count;
      m_blockLength = 0;
      m_actingVersion = actingVersion;
      m_initialPosition = *pos;
      m_positionPtr = pos;
      *m_positionPtr = *m_positionPtr + 4;
    }

    static SBE_CONSTEXPR std::uint64_t sbeHeaderSize() SBE_NOEXCEPT {
      return 4;
    }

    static SBE_CONSTEXPR std::uint64_t sbeBlockLength() SBE_NOEXCEPT {
      return 0;
    }

    SBE_NODISCARD std::uint64_t sbeActingBlockLength() SBE_NOEXCEPT {
      return m_blockLength;
    }

    SBE_NODISCARD std::uint64_t sbePosition() const SBE_NOEXCEPT {
      return *m_positionPtr;
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    std::uint64_t sbeCheckPosition(const std::uint64_t position) {
      if (SBE_BOUNDS_CHECK_EXPECT((position > m_bufferLength), false)) {
        throw std::runtime_error("buffer too short [E100]");
      }
      return position;
    }

    void sbePosition(const std::uint64_t position) {
      *m_positionPtr = sbeCheckPosition(position);
    }

    SBE_NODISCARD inline std::uint64_t count() const SBE_NOEXCEPT {
      return m_count;
    }

    SBE_NODISCARD inline bool hasNext() const SBE_NOEXCEPT {
      return m_index < m_count;
    }

    inline Field_3& next() {
      if (m_index >= m_count) {
        throw std::runtime_error("index >= count [E108]");
      }
      m_offset = *m_positionPtr;
      if (SBE_BOUNDS_CHECK_EXPECT(
              ((m_offset + m_blockLength) > m_bufferLength), false)) {
        throw std::runtime_error(
            "buffer too short for next group index [E108]");
      }
      *m_positionPtr = m_offset + m_blockLength;
      ++m_index;

      return *this;
    }

    inline std::uint64_t resetCountToIndex() {
      m_count = m_index;
      GroupSizeEncoding dimensions(
          m_buffer, m_initialPosition, m_bufferLength, m_actingVersion);
      dimensions.numInGroup(static_cast<std::uint16_t>(m_count));
      return m_count;
    }

    template <class Func>
    inline void forEach(Func&& func) {
      while (hasNext()) {
        next();
        func(*this);
      }
    }

    SBE_NODISCARD static const char* list_entryMetaAttribute(
        const MetaAttribute metaAttribute) SBE_NOEXCEPT {
      switch (metaAttribute) {
        case MetaAttribute::PRESENCE:
          return "required";
        default:
          return "";
      }
    }

    static const char* list_entryCharacterEncoding() SBE_NOEXCEPT {
      return "UTF-8";
    }

    static SBE_CONSTEXPR std::uint64_t list_entrySinceVersion() SBE_NOEXCEPT {
      return 0;
    }

    bool list_entryInActingVersion() SBE_NOEXCEPT { return true; }

    static SBE_CONSTEXPR std::uint16_t list_entryId() SBE_NOEXCEPT {
      return 100;
    }

    static SBE_CONSTEXPR std::uint64_t list_entryHeaderLength() SBE_NOEXCEPT {
      return 4;
    }

    SBE_NODISCARD std::uint32_t list_entryLength() const {
      std::uint32_t length;
      std::memcpy(&length, m_buffer + sbePosition(), sizeof(std::uint32_t));
      return SBE_LITTLE_ENDIAN_ENCODE_32(length);
    }

    std::uint64_t skipList_entry() {
      std::uint64_t lengthOfLengthField = 4;
      std::uint64_t lengthPosition = sbePosition();
      std::uint32_t lengthFieldValue;
      std::memcpy(
          &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint32_t));
      std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue);
      sbePosition(lengthPosition + lengthOfLengthField + dataLength);
      return dataLength;
    }

    SBE_NODISCARD const char* list_entry() {
      std::uint32_t lengthFieldValue;
      std::memcpy(
          &lengthFieldValue, m_buffer + sbePosition(), sizeof(std::uint32_t));
      const char* fieldPtr = m_buffer + sbePosition() + 4;
      sbePosition(
          sbePosition() + 4 + SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue));
      return fieldPtr;
    }

    std::uint64_t getList_entry(char* dst, const std::uint64_t length) {
      std::uint64_t lengthOfLengthField = 4;
      std::uint64_t lengthPosition = sbePosition();
      sbePosition(lengthPosition + lengthOfLengthField);
      std::uint32_t lengthFieldValue;
      std::memcpy(
          &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint32_t));
      std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue);
      std::uint64_t bytesToCopy = length < dataLength ? length : dataLength;
      std::uint64_t pos = sbePosition();
      sbePosition(pos + dataLength);
      std::memcpy(dst, m_buffer + pos, static_cast<std::size_t>(bytesToCopy));
      return bytesToCopy;
    }

    Field_3& putList_entry(const char* src, const std::uint32_t length) {
      std::uint64_t lengthOfLengthField = 4;
      std::uint64_t lengthPosition = sbePosition();
      std::uint32_t lengthFieldValue = SBE_LITTLE_ENDIAN_ENCODE_32(length);
      sbePosition(lengthPosition + lengthOfLengthField);
      std::memcpy(
          m_buffer + lengthPosition, &lengthFieldValue, sizeof(std::uint32_t));
      if (length != std::uint32_t(0)) {
        std::uint64_t pos = sbePosition();
        sbePosition(pos + length);
        std::memcpy(m_buffer + pos, src, length);
      }
      return *this;
    }

    std::string getList_entryAsString() {
      std::uint64_t lengthOfLengthField = 4;
      std::uint64_t lengthPosition = sbePosition();
      sbePosition(lengthPosition + lengthOfLengthField);
      std::uint32_t lengthFieldValue;
      std::memcpy(
          &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint32_t));
      std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue);
      std::uint64_t pos = sbePosition();
      const std::string result(m_buffer + pos, dataLength);
      sbePosition(pos + dataLength);
      return result;
    }

    std::string getList_entryAsJsonEscapedString() {
      std::ostringstream oss;
      std::string s = getList_entryAsString();

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
    std::string_view getList_entryAsStringView() {
      std::uint64_t lengthOfLengthField = 4;
      std::uint64_t lengthPosition = sbePosition();
      sbePosition(lengthPosition + lengthOfLengthField);
      std::uint32_t lengthFieldValue;
      std::memcpy(
          &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint32_t));
      std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue);
      std::uint64_t pos = sbePosition();
      const std::string_view result(m_buffer + pos, dataLength);
      sbePosition(pos + dataLength);
      return result;
    }
#endif

    Field_3& putList_entry(const std::string& str) {
      if (str.length() > 1073741824) {
        throw std::runtime_error("std::string too long for length type [E109]");
      }
      return putList_entry(
          str.data(), static_cast<std::uint32_t>(str.length()));
    }

#if __cplusplus >= 201703L
    Field_3& putList_entry(const std::string_view str) {
      if (str.length() > 1073741824) {
        throw std::runtime_error("std::string too long for length type [E109]");
      }
      return putList_entry(
          str.data(), static_cast<std::uint32_t>(str.length()));
    }
#endif

    template <typename CharT, typename Traits>
    friend std::basic_ostream<CharT, Traits>& operator<<(
        std::basic_ostream<CharT, Traits>& builder, Field_3& writer) {
      builder << '{';
      builder << R"("list_entry": )";
      builder << '"' << writer.getList_entryAsJsonEscapedString().c_str()
              << '"';

      builder << '}';

      return builder;
    }

    void skip() { skipList_entry(); }

    SBE_NODISCARD static SBE_CONSTEXPR bool isConstLength() SBE_NOEXCEPT {
      return false;
    }

    SBE_NODISCARD static std::size_t computeLength(
        std::size_t list_entryLength = 0) {
#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif
      std::size_t length = sbeBlockLength();

      length += list_entryHeaderLength();
      if (list_entryLength > 1073741824LL) {
        throw std::runtime_error(
            "list_entryLength too long for length type [E109]");
      }
      length += list_entryLength;

      return length;
#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
    }
  };

 private:
  Field_3 m_field_3;

 public:
  SBE_NODISCARD static SBE_CONSTEXPR std::uint16_t field_3Id() SBE_NOEXCEPT {
    return 3;
  }

  SBE_NODISCARD inline Field_3& field_3() {
    m_field_3.wrapForDecode(
        m_buffer, sbePositionPtr(), m_actingVersion, m_bufferLength);
    return m_field_3;
  }

  Field_3& field_3Count(const std::uint16_t count) {
    m_field_3.wrapForEncode(
        m_buffer, count, sbePositionPtr(), m_actingVersion, m_bufferLength);
    return m_field_3;
  }

  SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t field_3SinceVersion()
      SBE_NOEXCEPT {
    return 0;
  }

  SBE_NODISCARD bool field_3InActingVersion() const SBE_NOEXCEPT {
    return true;
  }

  class Field_4 {
   private:
    char* m_buffer = nullptr;
    std::uint64_t m_bufferLength = 0;
    std::uint64_t m_initialPosition = 0;
    std::uint64_t* m_positionPtr = nullptr;
    std::uint64_t m_blockLength = 0;
    std::uint64_t m_count = 0;
    std::uint64_t m_index = 0;
    std::uint64_t m_offset = 0;
    std::uint64_t m_actingVersion = 0;

    SBE_NODISCARD std::uint64_t* sbePositionPtr() SBE_NOEXCEPT {
      return m_positionPtr;
    }

   public:
    Field_4() = default;

    inline void wrapForDecode(
        char* buffer,
        std::uint64_t* pos,
        const std::uint64_t actingVersion,
        const std::uint64_t bufferLength) {
      GroupSizeEncoding dimensions(buffer, *pos, bufferLength, actingVersion);
      m_buffer = buffer;
      m_bufferLength = bufferLength;
      m_blockLength = dimensions.blockLength();
      m_count = dimensions.numInGroup();
      m_index = 0;
      m_actingVersion = actingVersion;
      m_initialPosition = *pos;
      m_positionPtr = pos;
      *m_positionPtr = *m_positionPtr + 4;
    }

    inline void wrapForEncode(
        char* buffer,
        const std::uint16_t count,
        std::uint64_t* pos,
        const std::uint64_t actingVersion,
        const std::uint64_t bufferLength) {
#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif
      if (count > 65534) {
        throw std::runtime_error("count outside of allowed range [E110]");
      }
#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
      m_buffer = buffer;
      m_bufferLength = bufferLength;
      GroupSizeEncoding dimensions(buffer, *pos, bufferLength, actingVersion);
      dimensions.blockLength(static_cast<std::uint16_t>(1));
      dimensions.numInGroup(static_cast<std::uint16_t>(count));
      m_index = 0;
      m_count = count;
      m_blockLength = 1;
      m_actingVersion = actingVersion;
      m_initialPosition = *pos;
      m_positionPtr = pos;
      *m_positionPtr = *m_positionPtr + 4;
    }

    static SBE_CONSTEXPR std::uint64_t sbeHeaderSize() SBE_NOEXCEPT {
      return 4;
    }

    static SBE_CONSTEXPR std::uint64_t sbeBlockLength() SBE_NOEXCEPT {
      return 1;
    }

    SBE_NODISCARD std::uint64_t sbeActingBlockLength() SBE_NOEXCEPT {
      return m_blockLength;
    }

    SBE_NODISCARD std::uint64_t sbePosition() const SBE_NOEXCEPT {
      return *m_positionPtr;
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    std::uint64_t sbeCheckPosition(const std::uint64_t position) {
      if (SBE_BOUNDS_CHECK_EXPECT((position > m_bufferLength), false)) {
        throw std::runtime_error("buffer too short [E100]");
      }
      return position;
    }

    void sbePosition(const std::uint64_t position) {
      *m_positionPtr = sbeCheckPosition(position);
    }

    SBE_NODISCARD inline std::uint64_t count() const SBE_NOEXCEPT {
      return m_count;
    }

    SBE_NODISCARD inline bool hasNext() const SBE_NOEXCEPT {
      return m_index < m_count;
    }

    inline Field_4& next() {
      if (m_index >= m_count) {
        throw std::runtime_error("index >= count [E108]");
      }
      m_offset = *m_positionPtr;
      if (SBE_BOUNDS_CHECK_EXPECT(
              ((m_offset + m_blockLength) > m_bufferLength), false)) {
        throw std::runtime_error(
            "buffer too short for next group index [E108]");
      }
      *m_positionPtr = m_offset + m_blockLength;
      ++m_index;

      return *this;
    }

    inline std::uint64_t resetCountToIndex() {
      m_count = m_index;
      GroupSizeEncoding dimensions(
          m_buffer, m_initialPosition, m_bufferLength, m_actingVersion);
      dimensions.numInGroup(static_cast<std::uint16_t>(m_count));
      return m_count;
    }

    template <class Func>
    inline void forEach(Func&& func) {
      while (hasNext()) {
        next();
        func(*this);
      }
    }

    SBE_NODISCARD static const char* map_valueMetaAttribute(
        const MetaAttribute metaAttribute) SBE_NOEXCEPT {
      switch (metaAttribute) {
        case MetaAttribute::PRESENCE:
          return "required";
        default:
          return "";
      }
    }

    static SBE_CONSTEXPR std::uint16_t map_valueId() SBE_NOEXCEPT {
      return 101;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t map_valueSinceVersion()
        SBE_NOEXCEPT {
      return 0;
    }

    SBE_NODISCARD bool map_valueInActingVersion() SBE_NOEXCEPT { return true; }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t map_valueEncodingOffset()
        SBE_NOEXCEPT {
      return 0;
    }

    static SBE_CONSTEXPR std::int8_t map_valueNullValue() SBE_NOEXCEPT {
      return SBE_NULLVALUE_INT8;
    }

    static SBE_CONSTEXPR std::int8_t map_valueMinValue() SBE_NOEXCEPT {
      return static_cast<std::int8_t>(-127);
    }

    static SBE_CONSTEXPR std::int8_t map_valueMaxValue() SBE_NOEXCEPT {
      return static_cast<std::int8_t>(127);
    }

    static SBE_CONSTEXPR std::size_t map_valueEncodingLength() SBE_NOEXCEPT {
      return 1;
    }

    SBE_NODISCARD std::int8_t map_value() const SBE_NOEXCEPT {
      std::int8_t val;
      std::memcpy(&val, m_buffer + m_offset + 0, sizeof(std::int8_t));
      return (val);
    }

    Field_4& map_value(const std::int8_t value) SBE_NOEXCEPT {
      std::int8_t val = (value);
      std::memcpy(m_buffer + m_offset + 0, &val, sizeof(std::int8_t));
      return *this;
    }

    SBE_NODISCARD static const char* map_keyMetaAttribute(
        const MetaAttribute metaAttribute) SBE_NOEXCEPT {
      switch (metaAttribute) {
        case MetaAttribute::PRESENCE:
          return "required";
        default:
          return "";
      }
    }

    static const char* map_keyCharacterEncoding() SBE_NOEXCEPT {
      return "UTF-8";
    }

    static SBE_CONSTEXPR std::uint64_t map_keySinceVersion() SBE_NOEXCEPT {
      return 0;
    }

    bool map_keyInActingVersion() SBE_NOEXCEPT { return true; }

    static SBE_CONSTEXPR std::uint16_t map_keyId() SBE_NOEXCEPT { return 102; }

    static SBE_CONSTEXPR std::uint64_t map_keyHeaderLength() SBE_NOEXCEPT {
      return 4;
    }

    SBE_NODISCARD std::uint32_t map_keyLength() const {
      std::uint32_t length;
      std::memcpy(&length, m_buffer + sbePosition(), sizeof(std::uint32_t));
      return SBE_LITTLE_ENDIAN_ENCODE_32(length);
    }

    std::uint64_t skipMap_key() {
      std::uint64_t lengthOfLengthField = 4;
      std::uint64_t lengthPosition = sbePosition();
      std::uint32_t lengthFieldValue;
      std::memcpy(
          &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint32_t));
      std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue);
      sbePosition(lengthPosition + lengthOfLengthField + dataLength);
      return dataLength;
    }

    SBE_NODISCARD const char* map_key() {
      std::uint32_t lengthFieldValue;
      std::memcpy(
          &lengthFieldValue, m_buffer + sbePosition(), sizeof(std::uint32_t));
      const char* fieldPtr = m_buffer + sbePosition() + 4;
      sbePosition(
          sbePosition() + 4 + SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue));
      return fieldPtr;
    }

    std::uint64_t getMap_key(char* dst, const std::uint64_t length) {
      std::uint64_t lengthOfLengthField = 4;
      std::uint64_t lengthPosition = sbePosition();
      sbePosition(lengthPosition + lengthOfLengthField);
      std::uint32_t lengthFieldValue;
      std::memcpy(
          &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint32_t));
      std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue);
      std::uint64_t bytesToCopy = length < dataLength ? length : dataLength;
      std::uint64_t pos = sbePosition();
      sbePosition(pos + dataLength);
      std::memcpy(dst, m_buffer + pos, static_cast<std::size_t>(bytesToCopy));
      return bytesToCopy;
    }

    Field_4& putMap_key(const char* src, const std::uint32_t length) {
      std::uint64_t lengthOfLengthField = 4;
      std::uint64_t lengthPosition = sbePosition();
      std::uint32_t lengthFieldValue = SBE_LITTLE_ENDIAN_ENCODE_32(length);
      sbePosition(lengthPosition + lengthOfLengthField);
      std::memcpy(
          m_buffer + lengthPosition, &lengthFieldValue, sizeof(std::uint32_t));
      if (length != std::uint32_t(0)) {
        std::uint64_t pos = sbePosition();
        sbePosition(pos + length);
        std::memcpy(m_buffer + pos, src, length);
      }
      return *this;
    }

    std::string getMap_keyAsString() {
      std::uint64_t lengthOfLengthField = 4;
      std::uint64_t lengthPosition = sbePosition();
      sbePosition(lengthPosition + lengthOfLengthField);
      std::uint32_t lengthFieldValue;
      std::memcpy(
          &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint32_t));
      std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue);
      std::uint64_t pos = sbePosition();
      const std::string result(m_buffer + pos, dataLength);
      sbePosition(pos + dataLength);
      return result;
    }

    std::string getMap_keyAsJsonEscapedString() {
      std::ostringstream oss;
      std::string s = getMap_keyAsString();

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
    std::string_view getMap_keyAsStringView() {
      std::uint64_t lengthOfLengthField = 4;
      std::uint64_t lengthPosition = sbePosition();
      sbePosition(lengthPosition + lengthOfLengthField);
      std::uint32_t lengthFieldValue;
      std::memcpy(
          &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint32_t));
      std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue);
      std::uint64_t pos = sbePosition();
      const std::string_view result(m_buffer + pos, dataLength);
      sbePosition(pos + dataLength);
      return result;
    }
#endif

    Field_4& putMap_key(const std::string& str) {
      if (str.length() > 1073741824) {
        throw std::runtime_error("std::string too long for length type [E109]");
      }
      return putMap_key(str.data(), static_cast<std::uint32_t>(str.length()));
    }

#if __cplusplus >= 201703L
    Field_4& putMap_key(const std::string_view str) {
      if (str.length() > 1073741824) {
        throw std::runtime_error("std::string too long for length type [E109]");
      }
      return putMap_key(str.data(), static_cast<std::uint32_t>(str.length()));
    }
#endif

    template <typename CharT, typename Traits>
    friend std::basic_ostream<CharT, Traits>& operator<<(
        std::basic_ostream<CharT, Traits>& builder, Field_4& writer) {
      builder << '{';
      builder << R"("map_value": )";
      builder << +writer.map_value();

      builder << ", ";
      builder << R"("map_key": )";
      builder << '"' << writer.getMap_keyAsJsonEscapedString().c_str() << '"';

      builder << '}';

      return builder;
    }

    void skip() { skipMap_key(); }

    SBE_NODISCARD static SBE_CONSTEXPR bool isConstLength() SBE_NOEXCEPT {
      return false;
    }

    SBE_NODISCARD static std::size_t computeLength(
        std::size_t map_keyLength = 0) {
#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif
      std::size_t length = sbeBlockLength();

      length += map_keyHeaderLength();
      if (map_keyLength > 1073741824LL) {
        throw std::runtime_error(
            "map_keyLength too long for length type [E109]");
      }
      length += map_keyLength;

      return length;
#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
    }
  };

 private:
  Field_4 m_field_4;

 public:
  SBE_NODISCARD static SBE_CONSTEXPR std::uint16_t field_4Id() SBE_NOEXCEPT {
    return 4;
  }

  SBE_NODISCARD inline Field_4& field_4() {
    m_field_4.wrapForDecode(
        m_buffer, sbePositionPtr(), m_actingVersion, m_bufferLength);
    return m_field_4;
  }

  Field_4& field_4Count(const std::uint16_t count) {
    m_field_4.wrapForEncode(
        m_buffer, count, sbePositionPtr(), m_actingVersion, m_bufferLength);
    return m_field_4;
  }

  SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t field_4SinceVersion()
      SBE_NOEXCEPT {
    return 0;
  }

  SBE_NODISCARD bool field_4InActingVersion() const SBE_NOEXCEPT {
    return true;
  }

  SBE_NODISCARD static const char* field_2MetaAttribute(
      const MetaAttribute metaAttribute) SBE_NOEXCEPT {
    switch (metaAttribute) {
      case MetaAttribute::PRESENCE:
        return "required";
      default:
        return "";
    }
  }

  static const char* field_2CharacterEncoding() SBE_NOEXCEPT { return "UTF-8"; }

  static SBE_CONSTEXPR std::uint64_t field_2SinceVersion() SBE_NOEXCEPT {
    return 0;
  }

  bool field_2InActingVersion() SBE_NOEXCEPT { return true; }

  static SBE_CONSTEXPR std::uint16_t field_2Id() SBE_NOEXCEPT { return 2; }

  static SBE_CONSTEXPR std::uint64_t field_2HeaderLength() SBE_NOEXCEPT {
    return 4;
  }

  SBE_NODISCARD std::uint32_t field_2Length() const {
    std::uint32_t length;
    std::memcpy(&length, m_buffer + sbePosition(), sizeof(std::uint32_t));
    return SBE_LITTLE_ENDIAN_ENCODE_32(length);
  }

  std::uint64_t skipField_2() {
    std::uint64_t lengthOfLengthField = 4;
    std::uint64_t lengthPosition = sbePosition();
    std::uint32_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint32_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue);
    sbePosition(lengthPosition + lengthOfLengthField + dataLength);
    return dataLength;
  }

  SBE_NODISCARD const char* field_2() {
    std::uint32_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + sbePosition(), sizeof(std::uint32_t));
    const char* fieldPtr = m_buffer + sbePosition() + 4;
    sbePosition(
        sbePosition() + 4 + SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue));
    return fieldPtr;
  }

  std::uint64_t getField_2(char* dst, const std::uint64_t length) {
    std::uint64_t lengthOfLengthField = 4;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint32_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint32_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue);
    std::uint64_t bytesToCopy = length < dataLength ? length : dataLength;
    std::uint64_t pos = sbePosition();
    sbePosition(pos + dataLength);
    std::memcpy(dst, m_buffer + pos, static_cast<std::size_t>(bytesToCopy));
    return bytesToCopy;
  }

  SBEStruct1& putField_2(const char* src, const std::uint32_t length) {
    std::uint64_t lengthOfLengthField = 4;
    std::uint64_t lengthPosition = sbePosition();
    std::uint32_t lengthFieldValue = SBE_LITTLE_ENDIAN_ENCODE_32(length);
    sbePosition(lengthPosition + lengthOfLengthField);
    std::memcpy(
        m_buffer + lengthPosition, &lengthFieldValue, sizeof(std::uint32_t));
    if (length != std::uint32_t(0)) {
      std::uint64_t pos = sbePosition();
      sbePosition(pos + length);
      std::memcpy(m_buffer + pos, src, length);
    }
    return *this;
  }

  std::string getField_2AsString() {
    std::uint64_t lengthOfLengthField = 4;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint32_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint32_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue);
    std::uint64_t pos = sbePosition();
    const std::string result(m_buffer + pos, dataLength);
    sbePosition(pos + dataLength);
    return result;
  }

  std::string getField_2AsJsonEscapedString() {
    std::ostringstream oss;
    std::string s = getField_2AsString();

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
  std::string_view getField_2AsStringView() {
    std::uint64_t lengthOfLengthField = 4;
    std::uint64_t lengthPosition = sbePosition();
    sbePosition(lengthPosition + lengthOfLengthField);
    std::uint32_t lengthFieldValue;
    std::memcpy(
        &lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint32_t));
    std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue);
    std::uint64_t pos = sbePosition();
    const std::string_view result(m_buffer + pos, dataLength);
    sbePosition(pos + dataLength);
    return result;
  }
#endif

  SBEStruct1& putField_2(const std::string& str) {
    if (str.length() > 1073741824) {
      throw std::runtime_error("std::string too long for length type [E109]");
    }
    return putField_2(str.data(), static_cast<std::uint32_t>(str.length()));
  }

#if __cplusplus >= 201703L
  SBEStruct1& putField_2(const std::string_view str) {
    if (str.length() > 1073741824) {
      throw std::runtime_error("std::string too long for length type [E109]");
    }
    return putField_2(str.data(), static_cast<std::uint32_t>(str.length()));
  }
#endif

  template <typename CharT, typename Traits>
  friend std::basic_ostream<CharT, Traits>& operator<<(
      std::basic_ostream<CharT, Traits>& builder, const SBEStruct1& _writer) {
    SBEStruct1 writer(
        _writer.m_buffer,
        _writer.m_offset,
        _writer.m_bufferLength,
        _writer.m_actingBlockLength,
        _writer.m_actingVersion);

    builder << '{';
    builder << R"("Name": "SBEStruct1", )";
    builder << R"("sbeTemplateId": )";
    builder << writer.sbeTemplateId();
    builder << ", ";

    builder << R"("field_1": )";
    builder << +writer.field_1();

    builder << ", ";
    {
      bool atLeastOne = false;
      builder << R"("field_3": [)";
      writer.field_3().forEach([&](Field_3& field_3) {
        if (atLeastOne) {
          builder << ", ";
        }
        atLeastOne = true;
        builder << field_3;
      });
      builder << ']';
    }

    builder << ", ";
    {
      bool atLeastOne = false;
      builder << R"("field_4": [)";
      writer.field_4().forEach([&](Field_4& field_4) {
        if (atLeastOne) {
          builder << ", ";
        }
        atLeastOne = true;
        builder << field_4;
      });
      builder << ']';
    }

    builder << ", ";
    builder << R"("field_2": )";
    builder << '"' << writer.getField_2AsJsonEscapedString().c_str() << '"';

    builder << '}';

    return builder;
  }

  void skip() {
    auto& field_3Group{field_3()};
    while (field_3Group.hasNext()) {
      field_3Group.next().skip();
    }
    auto& field_4Group{field_4()};
    while (field_4Group.hasNext()) {
      field_4Group.next().skip();
    }
    skipField_2();
  }

  SBE_NODISCARD static SBE_CONSTEXPR bool isConstLength() SBE_NOEXCEPT {
    return false;
  }

  SBE_NODISCARD static std::size_t computeLength(
      const std::vector<std::tuple<std::size_t>>& field_3ItemLengths = {},
      const std::vector<std::tuple<std::size_t>>& field_4ItemLengths = {},
      std::size_t field_2Length = 0) {
#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif
    std::size_t length = sbeBlockLength();

    length += Field_3::sbeHeaderSize();
    if (field_3ItemLengths.size() > 65534LL) {
      throw std::runtime_error(
          "field_3ItemLengths.size() outside of allowed range [E110]");
    }

    for (const auto& e : field_3ItemLengths) {
#if __cplusplus >= 201703L
      length += std::apply(Field_3::computeLength, e);
#else
      length += Field_3::computeLength(std::get<0>(e));
#endif
    }

    length += Field_4::sbeHeaderSize();
    if (field_4ItemLengths.size() > 65534LL) {
      throw std::runtime_error(
          "field_4ItemLengths.size() outside of allowed range [E110]");
    }

    for (const auto& e : field_4ItemLengths) {
#if __cplusplus >= 201703L
      length += std::apply(Field_4::computeLength, e);
#else
      length += Field_4::computeLength(std::get<0>(e));
#endif
    }

    length += field_2HeaderLength();
    if (field_2Length > 1073741824LL) {
      throw std::runtime_error("field_2Length too long for length type [E109]");
    }
    length += field_2Length;

    return length;
#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
  }
};
} // namespace benchmarks
} // namespace thrift
} // namespace apache
#endif
