// @generated using thrift/lib/thrift/generate-rpc-metadata-sbe.sh
/* Generated SBE (Simple Binary Encoding) message codec */
#ifndef _APACHE_THRIFT_SBE_RESPONSERPCMETADATAOPTIONAL_CXX_H_
#define _APACHE_THRIFT_SBE_RESPONSERPCMETADATAOPTIONAL_CXX_H_

#if __cplusplus >= 201103L
#  define SBE_CONSTEXPR constexpr
#  define SBE_NOEXCEPT noexcept
#else
#  define SBE_CONSTEXPR
#  define SBE_NOEXCEPT
#endif

#if __cplusplus >= 201703L
#  include <string_view>
#  define SBE_NODISCARD [[nodiscard]]
#else
#  define SBE_NODISCARD
#endif

#if !defined(__STDC_LIMIT_MACROS)
#  define __STDC_LIMIT_MACROS 1
#endif

#include <cstdint>
#include <limits>
#include <cstring>
#include <iomanip>
#include <ostream>
#include <stdexcept>
#include <sstream>
#include <string>
#include <vector>
#include <tuple>

#if defined(WIN32) || defined(_WIN32)
#  define SBE_BIG_ENDIAN_ENCODE_16(v) _byteswap_ushort(v)
#  define SBE_BIG_ENDIAN_ENCODE_32(v) _byteswap_ulong(v)
#  define SBE_BIG_ENDIAN_ENCODE_64(v) _byteswap_uint64(v)
#  define SBE_LITTLE_ENDIAN_ENCODE_16(v) (v)
#  define SBE_LITTLE_ENDIAN_ENCODE_32(v) (v)
#  define SBE_LITTLE_ENDIAN_ENCODE_64(v) (v)
#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__
#  define SBE_BIG_ENDIAN_ENCODE_16(v) __builtin_bswap16(v)
#  define SBE_BIG_ENDIAN_ENCODE_32(v) __builtin_bswap32(v)
#  define SBE_BIG_ENDIAN_ENCODE_64(v) __builtin_bswap64(v)
#  define SBE_LITTLE_ENDIAN_ENCODE_16(v) (v)
#  define SBE_LITTLE_ENDIAN_ENCODE_32(v) (v)
#  define SBE_LITTLE_ENDIAN_ENCODE_64(v) (v)
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__
#  define SBE_LITTLE_ENDIAN_ENCODE_16(v) __builtin_bswap16(v)
#  define SBE_LITTLE_ENDIAN_ENCODE_32(v) __builtin_bswap32(v)
#  define SBE_LITTLE_ENDIAN_ENCODE_64(v) __builtin_bswap64(v)
#  define SBE_BIG_ENDIAN_ENCODE_16(v) (v)
#  define SBE_BIG_ENDIAN_ENCODE_32(v) (v)
#  define SBE_BIG_ENDIAN_ENCODE_64(v) (v)
#else
#  error "Byte Ordering of platform not determined. Set __BYTE_ORDER__ manually before including this file."
#endif

#if !defined(SBE_BOUNDS_CHECK_EXPECT)
#  if defined(SBE_NO_BOUNDS_CHECK)
#    define SBE_BOUNDS_CHECK_EXPECT(exp, c) (false)
#  elif defined(_MSC_VER)
#    define SBE_BOUNDS_CHECK_EXPECT(exp, c) (exp)
#  else 
#    define SBE_BOUNDS_CHECK_EXPECT(exp, c) (__builtin_expect(exp, c))
#  endif

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


#include "ProtocolId.h"
#include "ErrorClassification.h"
#include "MessageHeader.h"
#include "BooleanType.h"
#include "CodecConfig.h"
#include "GroupSizeEncoding.h"
#include "VarStringEncoding.h"
#include "RpcKind.h"
#include "CompressionAlgorithm.h"
#include "ErrorKind.h"
#include "VarDataEncoding.h"
#include "RpcPriority.h"
#include "FdMetadata.h"
#include "PayloadExceptionMetadata.h"
#include "QueueMetadata.h"
#include "ErrorSafety.h"
#include "ErrorBlame.h"
#include "CompressionConfig.h"
#include "MetadataType.h"

namespace apache {
namespace thrift {
namespace sbe {

class ResponseRpcMetadataOptional
{
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
     *       V0_BLOCK -> V0_BLOCK [label="  proxiedPayloadMetadata(?)  "];
     *       V0_BLOCK -> V0_BLOCK [label="  queueMetadata(?)  "];
     *       V0_BLOCK -> V0_BLOCK [label="  fdMetadata(?)  "];
     *       V0_BLOCK -> V0_BLOCK [label="  frameworkMetadataLength()  "];
     *       V0_BLOCK -> V0_FRAMEWORKMETADATA_DONE [label="  frameworkMetadata(?)  "];
     *   }
     * }</pre>
     */
    enum class CodecState
    {
        NOT_WRAPPED = 0,
        V0_BLOCK = 1,
        V0_FRAMEWORKMETADATA_DONE = 2,
    };

    static const std::string STATE_NAME_LOOKUP[3];
    static const std::string STATE_TRANSITIONS_LOOKUP[3];

    static std::string codecStateName(CodecState state)
    {
        return STATE_NAME_LOOKUP[static_cast<int>(state)];
    }

    static std::string codecStateTransitions(CodecState state)
    {
        return STATE_TRANSITIONS_LOOKUP[static_cast<int>(state)];
    }

    char *m_buffer = nullptr;
    std::uint64_t m_bufferLength = 0;
    std::uint64_t m_offset = 0;
    std::uint64_t m_position = 0;
    std::uint64_t m_actingBlockLength = 0;
    std::uint64_t m_actingVersion = 0;
    CodecState m_codecState = CodecState::NOT_WRAPPED;

    CodecState codecState() const
    {
        return m_codecState;
    }

    CodecState *codecStatePtr()
    {
        return &m_codecState;
    }

    void codecState(CodecState newState)
    {
        m_codecState = newState;
    }

    inline std::uint64_t *sbePositionPtr() SBE_NOEXCEPT
    {
        return &m_position;
    }

public:
    static constexpr std::uint16_t SBE_BLOCK_LENGTH = static_cast<std::uint16_t>(21);
    static constexpr std::uint16_t SBE_TEMPLATE_ID = static_cast<std::uint16_t>(8);
    static constexpr std::uint16_t SBE_SCHEMA_ID = static_cast<std::uint16_t>(1);
    static constexpr std::uint16_t SBE_SCHEMA_VERSION = static_cast<std::uint16_t>(0);
    static constexpr const char* SBE_SEMANTIC_VERSION = "5.2";

    enum MetaAttribute
    {
        EPOCH, TIME_UNIT, SEMANTIC_TYPE, PRESENCE
    };

    union sbe_float_as_uint_u
    {
        float fp_value;
        std::uint32_t uint_value;
    };

    union sbe_double_as_uint_u
    {
        double fp_value;
        std::uint64_t uint_value;
    };

    using messageHeader = MessageHeader;

    class AccessOrderError : public std::logic_error
    {
    public:
        explicit AccessOrderError(const std::string &msg) : std::logic_error(msg) {}
    };

    ResponseRpcMetadataOptional() = default;

    ResponseRpcMetadataOptional(
        char *buffer,
        const std::uint64_t offset,
        const std::uint64_t bufferLength,
        const std::uint64_t actingBlockLength,
        const std::uint64_t actingVersion,
        CodecState codecState) :
        m_buffer(buffer),
        m_bufferLength(bufferLength),
        m_offset(offset),
        m_position(sbeCheckPosition(offset + actingBlockLength)),
        m_actingBlockLength(actingBlockLength),
        m_actingVersion(actingVersion),
        m_codecState(codecState)
    {
    }

    ResponseRpcMetadataOptional(
        char *buffer,
        const std::uint64_t offset,
        const std::uint64_t bufferLength,
        const std::uint64_t actingBlockLength,
        const std::uint64_t actingVersion) :
        m_buffer(buffer),
        m_bufferLength(bufferLength),
        m_offset(offset),
        m_position(sbeCheckPosition(offset + actingBlockLength)),
        m_actingBlockLength(actingBlockLength),
        m_actingVersion(actingVersion)
    {
    }

    ResponseRpcMetadataOptional(char *buffer, const std::uint64_t bufferLength) :
        ResponseRpcMetadataOptional(buffer, 0, bufferLength, sbeBlockLength(), sbeSchemaVersion())
    {
    }

    ResponseRpcMetadataOptional(
        char *buffer,
        const std::uint64_t bufferLength,
        const std::uint64_t actingBlockLength,
        const std::uint64_t actingVersion) :
        ResponseRpcMetadataOptional(buffer, 0, bufferLength, actingBlockLength, actingVersion)
    {
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint16_t sbeBlockLength() SBE_NOEXCEPT
    {
        return static_cast<std::uint16_t>(21);
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t sbeBlockAndHeaderLength() SBE_NOEXCEPT
    {
        return messageHeader::encodedLength() + sbeBlockLength();
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint16_t sbeTemplateId() SBE_NOEXCEPT
    {
        return static_cast<std::uint16_t>(8);
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint16_t sbeSchemaId() SBE_NOEXCEPT
    {
        return static_cast<std::uint16_t>(1);
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint16_t sbeSchemaVersion() SBE_NOEXCEPT
    {
        return static_cast<std::uint16_t>(0);
    }

    SBE_NODISCARD static const char *sbeSemanticVersion() SBE_NOEXCEPT
    {
        return "5.2";
    }

    SBE_NODISCARD static SBE_CONSTEXPR const char *sbeSemanticType() SBE_NOEXCEPT
    {
        return "";
    }

    SBE_NODISCARD std::uint64_t offset() const SBE_NOEXCEPT
    {
        return m_offset;
    }

    ResponseRpcMetadataOptional &wrapForEncode(char *buffer, const std::uint64_t offset, const std::uint64_t bufferLength)
    {
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

    ResponseRpcMetadataOptional &wrapAndApplyHeader(char *buffer, const std::uint64_t offset, const std::uint64_t bufferLength)
    {
        messageHeader hdr(buffer, offset, bufferLength, sbeSchemaVersion());

        hdr
            .blockLength(sbeBlockLength())
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

    ResponseRpcMetadataOptional &wrapForDecode(
        char *buffer,
        const std::uint64_t offset,
        const std::uint64_t actingBlockLength,
        const std::uint64_t actingVersion,
        const std::uint64_t bufferLength)
    {
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

    ResponseRpcMetadataOptional &sbeRewind()
    {
        return wrapForDecode(m_buffer, m_offset, m_actingBlockLength, m_actingVersion, m_bufferLength);
    }

    SBE_NODISCARD std::uint64_t sbePosition() const SBE_NOEXCEPT
    {
        return m_position;
    }

    // NOLINTNEXTLINE(readability-convert-member-functions-to-static)
    std::uint64_t sbeCheckPosition(const std::uint64_t position)
    {
        if (SBE_BOUNDS_CHECK_EXPECT((position > m_bufferLength), false))
        {
            throw std::runtime_error("buffer too short [E100]");
        }
        return position;
    }

    void sbePosition(const std::uint64_t position)
    {
        m_position = sbeCheckPosition(position);
    }

    SBE_NODISCARD std::uint64_t encodedLength() const SBE_NOEXCEPT
    {
        return sbePosition() - m_offset;
    }

    SBE_NODISCARD std::uint64_t decodeLength() const
    {
        ResponseRpcMetadataOptional skipper(m_buffer, m_offset, m_bufferLength, sbeBlockLength(), m_actingVersion, m_codecState);
        skipper.skip();
        return skipper.encodedLength();
    }

    SBE_NODISCARD const char *buffer() const SBE_NOEXCEPT
    {
        return m_buffer;
    }

    SBE_NODISCARD char *buffer() SBE_NOEXCEPT
    {
        return m_buffer;
    }

    SBE_NODISCARD std::uint64_t bufferLength() const SBE_NOEXCEPT
    {
        return m_bufferLength;
    }

    SBE_NODISCARD std::uint64_t actingVersion() const SBE_NOEXCEPT
    {
        return m_actingVersion;
    }

    void checkEncodingIsComplete()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        switch (m_codecState)
        {
            case CodecState::V0_FRAMEWORKMETADATA_DONE:
                return;
            default:
                throw AccessOrderError(std::string("Not fully encoded, current state: ") +
                    codecStateName(m_codecState) + ", allowed transitions: " +
                    codecStateTransitions(m_codecState));
        }
#endif
    }


    SBE_NODISCARD static const char *proxiedPayloadMetadataMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t proxiedPayloadMetadataId() SBE_NOEXCEPT
    {
        return 1;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t proxiedPayloadMetadataSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool proxiedPayloadMetadataInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t proxiedPayloadMetadataEncodingOffset() SBE_NOEXCEPT
    {
        return 0;
    }

private:
    void onProxiedPayloadMetadataAccessed() const
    {
        if (codecState() == CodecState::NOT_WRAPPED)
        {
            throw AccessOrderError(std::string("Illegal field access order. ") +
                "Cannot access field \"proxiedPayloadMetadata\" in state: " + codecStateName(codecState()) +
                ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                "]. Please see the diagram in the docs of the enum ResponseRpcMetadataOptional::CodecState.");
        }
    }

public:
    SBE_NODISCARD static SBE_CONSTEXPR std::size_t proxiedPayloadMetadataEncodingLength() SBE_NOEXCEPT
    {
        return 1;
    }

    SBE_NODISCARD std::uint8_t proxiedPayloadMetadataRaw() const
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onProxiedPayloadMetadataAccessed();
#endif
        std::uint8_t val;
        std::memcpy(&val, m_buffer + m_offset + 0, sizeof(std::uint8_t));
        return (val);
    }

    SBE_NODISCARD BooleanType::Value proxiedPayloadMetadata() const
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onProxiedPayloadMetadataAccessed();
#endif
        std::uint8_t val;
        std::memcpy(&val, m_buffer + m_offset + 0, sizeof(std::uint8_t));
        return BooleanType::get((val));
    }

    ResponseRpcMetadataOptional &proxiedPayloadMetadata(const BooleanType::Value value)
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onProxiedPayloadMetadataAccessed();
#endif
        std::uint8_t val = (value);
        std::memcpy(m_buffer + m_offset + 0, &val, sizeof(std::uint8_t));
        return *this;
    }

    SBE_NODISCARD static const char *queueMetadataMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::PRESENCE: return "optional";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t queueMetadataId() SBE_NOEXCEPT
    {
        return 2;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t queueMetadataSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool queueMetadataInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t queueMetadataEncodingOffset() SBE_NOEXCEPT
    {
        return 1;
    }

private:
    void onQueueMetadataAccessed() const
    {
        if (codecState() == CodecState::NOT_WRAPPED)
        {
            throw AccessOrderError(std::string("Illegal field access order. ") +
                "Cannot access field \"queueMetadata\" in state: " + codecStateName(codecState()) +
                ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                "]. Please see the diagram in the docs of the enum ResponseRpcMetadataOptional::CodecState.");
        }
    }

public:
private:
    QueueMetadata m_queueMetadata;

public:
    SBE_NODISCARD QueueMetadata &queueMetadata()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onQueueMetadataAccessed();
#endif
        m_queueMetadata.wrap(m_buffer, m_offset + 1, m_actingVersion, m_bufferLength);
        return m_queueMetadata;
    }

    SBE_NODISCARD static const char *fdMetadataMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::PRESENCE: return "optional";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t fdMetadataId() SBE_NOEXCEPT
    {
        return 3;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t fdMetadataSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool fdMetadataInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t fdMetadataEncodingOffset() SBE_NOEXCEPT
    {
        return 9;
    }

private:
    void onFdMetadataAccessed() const
    {
        if (codecState() == CodecState::NOT_WRAPPED)
        {
            throw AccessOrderError(std::string("Illegal field access order. ") +
                "Cannot access field \"fdMetadata\" in state: " + codecStateName(codecState()) +
                ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                "]. Please see the diagram in the docs of the enum ResponseRpcMetadataOptional::CodecState.");
        }
    }

public:
private:
    FdMetadata m_fdMetadata;

public:
    SBE_NODISCARD FdMetadata &fdMetadata()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onFdMetadataAccessed();
#endif
        m_fdMetadata.wrap(m_buffer, m_offset + 9, m_actingVersion, m_bufferLength);
        return m_fdMetadata;
    }

    SBE_NODISCARD static const char *frameworkMetadataMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static const char *frameworkMetadataCharacterEncoding() SBE_NOEXCEPT
    {
        return "null";
    }

    static SBE_CONSTEXPR std::uint64_t frameworkMetadataSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    bool frameworkMetadataInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    static SBE_CONSTEXPR std::uint16_t frameworkMetadataId() SBE_NOEXCEPT
    {
        return 4;
    }

    static SBE_CONSTEXPR std::uint64_t frameworkMetadataHeaderLength() SBE_NOEXCEPT
    {
        return 4;
    }

private:
    void onFrameworkMetadataLengthAccessed() const
    {
        switch (codecState())
        {
            case CodecState::V0_BLOCK:
                break;
            default:
                throw AccessOrderError(std::string("Illegal field access order. ") +
                    "Cannot decode length of var data \"frameworkMetadata\" in state: " + codecStateName(codecState()) +
                    ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                    "]. Please see the diagram in the docs of the enum ResponseRpcMetadataOptional::CodecState.");
        }
    }

public:
    SBE_NODISCARD std::uint32_t frameworkMetadataLength() const
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onFrameworkMetadataLengthAccessed();
#endif
        std::uint32_t length;
        std::memcpy(&length, m_buffer + sbePosition(), sizeof(std::uint32_t));
        return SBE_LITTLE_ENDIAN_ENCODE_32(length);
    }

private:
    void onFrameworkMetadataAccessed()
    {
        switch (codecState())
        {
            case CodecState::V0_BLOCK:
                codecState(CodecState::V0_FRAMEWORKMETADATA_DONE);
                break;
            default:
                throw AccessOrderError(std::string("Illegal field access order. ") +
                    "Cannot access field \"frameworkMetadata\" in state: " + codecStateName(codecState()) +
                    ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                    "]. Please see the diagram in the docs of the enum ResponseRpcMetadataOptional::CodecState.");
        }
    }

public:
    std::uint64_t skipFrameworkMetadata()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onFrameworkMetadataAccessed();
#endif
        std::uint64_t lengthOfLengthField = 4;
        std::uint64_t lengthPosition = sbePosition();
        std::uint32_t lengthFieldValue;
        std::memcpy(&lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint32_t));
        std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue);
        sbePosition(lengthPosition + lengthOfLengthField + dataLength);
        return dataLength;
    }

    SBE_NODISCARD const char *frameworkMetadata()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onFrameworkMetadataAccessed();
#endif
        std::uint32_t lengthFieldValue;
        std::memcpy(&lengthFieldValue, m_buffer + sbePosition(), sizeof(std::uint32_t));
        const char *fieldPtr = m_buffer + sbePosition() + 4;
        sbePosition(sbePosition() + 4 + SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue));
        return fieldPtr;
    }

    std::uint64_t getFrameworkMetadata(char *dst, const std::uint64_t length)
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onFrameworkMetadataAccessed();
#endif
        std::uint64_t lengthOfLengthField = 4;
        std::uint64_t lengthPosition = sbePosition();
        sbePosition(lengthPosition + lengthOfLengthField);
        std::uint32_t lengthFieldValue;
        std::memcpy(&lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint32_t));
        std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue);
        std::uint64_t bytesToCopy = length < dataLength ? length : dataLength;
        std::uint64_t pos = sbePosition();
        sbePosition(pos + dataLength);
        std::memcpy(dst, m_buffer + pos, static_cast<std::size_t>(bytesToCopy));
        return bytesToCopy;
    }

    ResponseRpcMetadataOptional &putFrameworkMetadata(const char *src, const std::uint32_t length)
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onFrameworkMetadataAccessed();
#endif
        std::uint64_t lengthOfLengthField = 4;
        std::uint64_t lengthPosition = sbePosition();
        std::uint32_t lengthFieldValue = SBE_LITTLE_ENDIAN_ENCODE_32(length);
        sbePosition(lengthPosition + lengthOfLengthField);
        std::memcpy(m_buffer + lengthPosition, &lengthFieldValue, sizeof(std::uint32_t));
        if (length != std::uint32_t(0))
        {
            std::uint64_t pos = sbePosition();
            sbePosition(pos + length);
            std::memcpy(m_buffer + pos, src, length);
        }
        return *this;
    }

    std::string getFrameworkMetadataAsString()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onFrameworkMetadataAccessed();
#endif
        std::uint64_t lengthOfLengthField = 4;
        std::uint64_t lengthPosition = sbePosition();
        sbePosition(lengthPosition + lengthOfLengthField);
        std::uint32_t lengthFieldValue;
        std::memcpy(&lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint32_t));
        std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue);
        std::uint64_t pos = sbePosition();
        const std::string result(m_buffer + pos, dataLength);
        sbePosition(pos + dataLength);
        return result;
    }

    std::string getFrameworkMetadataAsJsonEscapedString()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onFrameworkMetadataAccessed();
#endif
        std::ostringstream oss;
        std::string s = getFrameworkMetadataAsString();

        for (const auto c : s)
        {
            switch (c)
            {
                case '"': oss << "\\\""; break;
                case '\\': oss << "\\\\"; break;
                case '\b': oss << "\\b"; break;
                case '\f': oss << "\\f"; break;
                case '\n': oss << "\\n"; break;
                case '\r': oss << "\\r"; break;
                case '\t': oss << "\\t"; break;

                default:
                    if ('\x00' <= c && c <= '\x1f')
                    {
                        oss << "\\u" << std::hex << std::setw(4)
                            << std::setfill('0') << (int)(c);
                    }
                    else
                    {
                        oss << c;
                    }
            }
        }

        return oss.str();
    }

    #if __cplusplus >= 201703L
    std::string_view getFrameworkMetadataAsStringView()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onFrameworkMetadataAccessed();
#endif
        std::uint64_t lengthOfLengthField = 4;
        std::uint64_t lengthPosition = sbePosition();
        sbePosition(lengthPosition + lengthOfLengthField);
        std::uint32_t lengthFieldValue;
        std::memcpy(&lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint32_t));
        std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue);
        std::uint64_t pos = sbePosition();
        const std::string_view result(m_buffer + pos, dataLength);
        sbePosition(pos + dataLength);
        return result;
    }
    #endif

    ResponseRpcMetadataOptional &putFrameworkMetadata(const std::string &str)
    {
        if (str.length() > 1073741824)
        {
            throw std::runtime_error("std::string too long for length type [E109]");
        }
        return putFrameworkMetadata(str.data(), static_cast<std::uint32_t>(str.length()));
    }

    #if __cplusplus >= 201703L
    ResponseRpcMetadataOptional &putFrameworkMetadata(const std::string_view str)
    {
        if (str.length() > 1073741824)
        {
            throw std::runtime_error("std::string too long for length type [E109]");
        }
        return putFrameworkMetadata(str.data(), static_cast<std::uint32_t>(str.length()));
    }
    #endif

template<typename CharT, typename Traits>
friend std::basic_ostream<CharT, Traits> & operator << (
    std::basic_ostream<CharT, Traits> &builder, const ResponseRpcMetadataOptional &_writer)
{
    ResponseRpcMetadataOptional writer(
        _writer.m_buffer,
        _writer.m_offset,
        _writer.m_bufferLength,
        _writer.m_actingBlockLength,
        _writer.m_actingVersion);

    builder << '{';
    builder << R"("Name": "ResponseRpcMetadataOptional", )";
    builder << R"("sbeTemplateId": )";
    builder << writer.sbeTemplateId();
    builder << ", ";

    builder << R"("proxiedPayloadMetadata": )";
    builder << '"' << writer.proxiedPayloadMetadata() << '"';

    builder << ", ";
    builder << R"("queueMetadata": )";
    builder << writer.queueMetadata();

    builder << ", ";
    builder << R"("fdMetadata": )";
    builder << writer.fdMetadata();

    builder << ", ";
    builder << R"("frameworkMetadata": )";
    builder << '"' <<
        writer.skipFrameworkMetadata() << " bytes of raw data\"";
    builder << '}';

    return builder;
}

void skip()
{
    skipFrameworkMetadata();
}

SBE_NODISCARD static SBE_CONSTEXPR bool isConstLength() SBE_NOEXCEPT
{
    return false;
}

SBE_NODISCARD static std::size_t computeLength(std::size_t frameworkMetadataLength = 0)
{
#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif
    std::size_t length = sbeBlockLength();

    length += frameworkMetadataHeaderLength();
    if (frameworkMetadataLength > 1073741824LL)
    {
        throw std::runtime_error("frameworkMetadataLength too long for length type [E109]");
    }
    length += frameworkMetadataLength;

    return length;
#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
}
};

const std::string ResponseRpcMetadataOptional::STATE_NAME_LOOKUP[3] =
{
    "NOT_WRAPPED",
    "V0_BLOCK",
    "V0_FRAMEWORKMETADATA_DONE",
};

const std::string ResponseRpcMetadataOptional::STATE_TRANSITIONS_LOOKUP[3] =
{
    "\"wrap(version=0)\"",
    "\"proxiedPayloadMetadata(?)\", \"queueMetadata(?)\", \"fdMetadata(?)\", \"frameworkMetadataLength()\", \"frameworkMetadata(?)\"",
    "",
};

}
}
}
#endif
