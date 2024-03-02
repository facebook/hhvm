// @generated using thrift/lib/thrift/generate-rpc-metadata-sbe.sh
/* Generated SBE (Simple Binary Encoding) message codec */
#ifndef _APACHE_THRIFT_SBE_REQUESTRPCMETADATAOPTIONAL_CXX_H_
#define _APACHE_THRIFT_SBE_REQUESTRPCMETADATAOPTIONAL_CXX_H_

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

class RequestRpcMetadataOptional
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
     *       V0_BLOCK -> V0_BLOCK [label="  clientTimeoutMs(?)  "];
     *       V0_BLOCK -> V0_BLOCK [label="  queueTimeoutMs(?)  "];
     *       V0_BLOCK -> V0_BLOCK [label="  priority(?)  "];
     *       V0_BLOCK -> V0_BLOCK [label="  compression(?)  "];
     *       V0_BLOCK -> V0_BLOCK [label="  compressionConfig(?)  "];
     *       V0_BLOCK -> V0_BLOCK [label="  fdMetadata(?)  "];
     *       V0_BLOCK -> V0_BLOCK [label="  loadMetricLength()  "];
     *       V0_BLOCK -> V0_LOADMETRIC_DONE [label="  loadMetric(?)  "];
     *       V0_LOADMETRIC_DONE -> V0_LOADMETRIC_DONE [label="  tenantIdLength()  "];
     *       V0_LOADMETRIC_DONE -> V0_TENANTID_DONE [label="  tenantId(?)  "];
     *       V0_TENANTID_DONE -> V0_TENANTID_DONE [label="  serviceTraceMetaLength()  "];
     *       V0_TENANTID_DONE -> V0_SERVICETRACEMETA_DONE [label="  serviceTraceMeta(?)  "];
     *       V0_SERVICETRACEMETA_DONE -> V0_SERVICETRACEMETA_DONE [label="  loggingContextLength()  "];
     *       V0_SERVICETRACEMETA_DONE -> V0_LOGGINGCONTEXT_DONE [label="  loggingContext(?)  "];
     *   }
     * }</pre>
     */
    enum class CodecState
    {
        NOT_WRAPPED = 0,
        V0_BLOCK = 1,
        V0_LOADMETRIC_DONE = 2,
        V0_TENANTID_DONE = 3,
        V0_SERVICETRACEMETA_DONE = 4,
        V0_LOGGINGCONTEXT_DONE = 5,
    };

    static const std::string STATE_NAME_LOOKUP[6];
    static const std::string STATE_TRANSITIONS_LOOKUP[6];

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
    static constexpr std::uint16_t SBE_BLOCK_LENGTH = static_cast<std::uint16_t>(31);
    static constexpr std::uint16_t SBE_TEMPLATE_ID = static_cast<std::uint16_t>(5);
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

    RequestRpcMetadataOptional() = default;

    RequestRpcMetadataOptional(
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

    RequestRpcMetadataOptional(
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

    RequestRpcMetadataOptional(char *buffer, const std::uint64_t bufferLength) :
        RequestRpcMetadataOptional(buffer, 0, bufferLength, sbeBlockLength(), sbeSchemaVersion())
    {
    }

    RequestRpcMetadataOptional(
        char *buffer,
        const std::uint64_t bufferLength,
        const std::uint64_t actingBlockLength,
        const std::uint64_t actingVersion) :
        RequestRpcMetadataOptional(buffer, 0, bufferLength, actingBlockLength, actingVersion)
    {
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint16_t sbeBlockLength() SBE_NOEXCEPT
    {
        return static_cast<std::uint16_t>(31);
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t sbeBlockAndHeaderLength() SBE_NOEXCEPT
    {
        return messageHeader::encodedLength() + sbeBlockLength();
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint16_t sbeTemplateId() SBE_NOEXCEPT
    {
        return static_cast<std::uint16_t>(5);
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

    RequestRpcMetadataOptional &wrapForEncode(char *buffer, const std::uint64_t offset, const std::uint64_t bufferLength)
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

    RequestRpcMetadataOptional &wrapAndApplyHeader(char *buffer, const std::uint64_t offset, const std::uint64_t bufferLength)
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

    RequestRpcMetadataOptional &wrapForDecode(
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

    RequestRpcMetadataOptional &sbeRewind()
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
        RequestRpcMetadataOptional skipper(m_buffer, m_offset, m_bufferLength, sbeBlockLength(), m_actingVersion, m_codecState);
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
            case CodecState::V0_LOGGINGCONTEXT_DONE:
                return;
            default:
                throw AccessOrderError(std::string("Not fully encoded, current state: ") +
                    codecStateName(m_codecState) + ", allowed transitions: " +
                    codecStateTransitions(m_codecState));
        }
#endif
    }


    SBE_NODISCARD static const char *clientTimeoutMsMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t clientTimeoutMsId() SBE_NOEXCEPT
    {
        return 1;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t clientTimeoutMsSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool clientTimeoutMsInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t clientTimeoutMsEncodingOffset() SBE_NOEXCEPT
    {
        return 0;
    }

private:
    void onClientTimeoutMsAccessed() const
    {
        if (codecState() == CodecState::NOT_WRAPPED)
        {
            throw AccessOrderError(std::string("Illegal field access order. ") +
                "Cannot access field \"clientTimeoutMs\" in state: " + codecStateName(codecState()) +
                ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                "]. Please see the diagram in the docs of the enum RequestRpcMetadataOptional::CodecState.");
        }
    }

public:
    static SBE_CONSTEXPR std::int32_t clientTimeoutMsNullValue() SBE_NOEXCEPT
    {
        return SBE_NULLVALUE_INT32;
    }

    static SBE_CONSTEXPR std::int32_t clientTimeoutMsMinValue() SBE_NOEXCEPT
    {
        return INT32_C(-2147483647);
    }

    static SBE_CONSTEXPR std::int32_t clientTimeoutMsMaxValue() SBE_NOEXCEPT
    {
        return INT32_C(2147483647);
    }

    static SBE_CONSTEXPR std::size_t clientTimeoutMsEncodingLength() SBE_NOEXCEPT
    {
        return 4;
    }

    SBE_NODISCARD std::int32_t clientTimeoutMs() const
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onClientTimeoutMsAccessed();
#endif
        std::int32_t val;
        std::memcpy(&val, m_buffer + m_offset + 0, sizeof(std::int32_t));
        return SBE_LITTLE_ENDIAN_ENCODE_32(val);
    }

    RequestRpcMetadataOptional &clientTimeoutMs(const std::int32_t value)
    {
        std::int32_t val = SBE_LITTLE_ENDIAN_ENCODE_32(value);
        std::memcpy(m_buffer + m_offset + 0, &val, sizeof(std::int32_t));
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onClientTimeoutMsAccessed();
#endif
        return *this;
    }

    SBE_NODISCARD static const char *queueTimeoutMsMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t queueTimeoutMsId() SBE_NOEXCEPT
    {
        return 2;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t queueTimeoutMsSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool queueTimeoutMsInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t queueTimeoutMsEncodingOffset() SBE_NOEXCEPT
    {
        return 4;
    }

private:
    void onQueueTimeoutMsAccessed() const
    {
        if (codecState() == CodecState::NOT_WRAPPED)
        {
            throw AccessOrderError(std::string("Illegal field access order. ") +
                "Cannot access field \"queueTimeoutMs\" in state: " + codecStateName(codecState()) +
                ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                "]. Please see the diagram in the docs of the enum RequestRpcMetadataOptional::CodecState.");
        }
    }

public:
    static SBE_CONSTEXPR std::int32_t queueTimeoutMsNullValue() SBE_NOEXCEPT
    {
        return SBE_NULLVALUE_INT32;
    }

    static SBE_CONSTEXPR std::int32_t queueTimeoutMsMinValue() SBE_NOEXCEPT
    {
        return INT32_C(-2147483647);
    }

    static SBE_CONSTEXPR std::int32_t queueTimeoutMsMaxValue() SBE_NOEXCEPT
    {
        return INT32_C(2147483647);
    }

    static SBE_CONSTEXPR std::size_t queueTimeoutMsEncodingLength() SBE_NOEXCEPT
    {
        return 4;
    }

    SBE_NODISCARD std::int32_t queueTimeoutMs() const
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onQueueTimeoutMsAccessed();
#endif
        std::int32_t val;
        std::memcpy(&val, m_buffer + m_offset + 4, sizeof(std::int32_t));
        return SBE_LITTLE_ENDIAN_ENCODE_32(val);
    }

    RequestRpcMetadataOptional &queueTimeoutMs(const std::int32_t value)
    {
        std::int32_t val = SBE_LITTLE_ENDIAN_ENCODE_32(value);
        std::memcpy(m_buffer + m_offset + 4, &val, sizeof(std::int32_t));
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onQueueTimeoutMsAccessed();
#endif
        return *this;
    }

    SBE_NODISCARD static const char *priorityMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t priorityId() SBE_NOEXCEPT
    {
        return 3;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t prioritySinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool priorityInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t priorityEncodingOffset() SBE_NOEXCEPT
    {
        return 8;
    }

private:
    void onPriorityAccessed() const
    {
        if (codecState() == CodecState::NOT_WRAPPED)
        {
            throw AccessOrderError(std::string("Illegal field access order. ") +
                "Cannot access field \"priority\" in state: " + codecStateName(codecState()) +
                ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                "]. Please see the diagram in the docs of the enum RequestRpcMetadataOptional::CodecState.");
        }
    }

public:
    SBE_NODISCARD static SBE_CONSTEXPR std::size_t priorityEncodingLength() SBE_NOEXCEPT
    {
        return 1;
    }

    SBE_NODISCARD std::uint8_t priorityRaw() const
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onPriorityAccessed();
#endif
        std::uint8_t val;
        std::memcpy(&val, m_buffer + m_offset + 8, sizeof(std::uint8_t));
        return (val);
    }

    SBE_NODISCARD RpcPriority::Value priority() const
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onPriorityAccessed();
#endif
        std::uint8_t val;
        std::memcpy(&val, m_buffer + m_offset + 8, sizeof(std::uint8_t));
        return RpcPriority::get((val));
    }

    RequestRpcMetadataOptional &priority(const RpcPriority::Value value)
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onPriorityAccessed();
#endif
        std::uint8_t val = (value);
        std::memcpy(m_buffer + m_offset + 8, &val, sizeof(std::uint8_t));
        return *this;
    }

    SBE_NODISCARD static const char *compressionMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::PRESENCE: return "optional";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t compressionId() SBE_NOEXCEPT
    {
        return 4;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t compressionSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool compressionInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t compressionEncodingOffset() SBE_NOEXCEPT
    {
        return 9;
    }

private:
    void onCompressionAccessed() const
    {
        if (codecState() == CodecState::NOT_WRAPPED)
        {
            throw AccessOrderError(std::string("Illegal field access order. ") +
                "Cannot access field \"compression\" in state: " + codecStateName(codecState()) +
                ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                "]. Please see the diagram in the docs of the enum RequestRpcMetadataOptional::CodecState.");
        }
    }

public:
    SBE_NODISCARD static SBE_CONSTEXPR std::size_t compressionEncodingLength() SBE_NOEXCEPT
    {
        return 1;
    }

    SBE_NODISCARD std::uint8_t compressionRaw() const
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onCompressionAccessed();
#endif
        std::uint8_t val;
        std::memcpy(&val, m_buffer + m_offset + 9, sizeof(std::uint8_t));
        return (val);
    }

    SBE_NODISCARD CompressionAlgorithm::Value compression() const
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onCompressionAccessed();
#endif
        std::uint8_t val;
        std::memcpy(&val, m_buffer + m_offset + 9, sizeof(std::uint8_t));
        return CompressionAlgorithm::get((val));
    }

    RequestRpcMetadataOptional &compression(const CompressionAlgorithm::Value value)
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onCompressionAccessed();
#endif
        std::uint8_t val = (value);
        std::memcpy(m_buffer + m_offset + 9, &val, sizeof(std::uint8_t));
        return *this;
    }

    SBE_NODISCARD static const char *compressionConfigMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::PRESENCE: return "optional";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t compressionConfigId() SBE_NOEXCEPT
    {
        return 5;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t compressionConfigSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool compressionConfigInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t compressionConfigEncodingOffset() SBE_NOEXCEPT
    {
        return 10;
    }

private:
    void onCompressionConfigAccessed() const
    {
        if (codecState() == CodecState::NOT_WRAPPED)
        {
            throw AccessOrderError(std::string("Illegal field access order. ") +
                "Cannot access field \"compressionConfig\" in state: " + codecStateName(codecState()) +
                ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                "]. Please see the diagram in the docs of the enum RequestRpcMetadataOptional::CodecState.");
        }
    }

public:
private:
    CompressionConfig m_compressionConfig;

public:
    SBE_NODISCARD CompressionConfig &compressionConfig()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onCompressionConfigAccessed();
#endif
        m_compressionConfig.wrap(m_buffer, m_offset + 10, m_actingVersion, m_bufferLength);
        return m_compressionConfig;
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
        return 6;
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
        return 19;
    }

private:
    void onFdMetadataAccessed() const
    {
        if (codecState() == CodecState::NOT_WRAPPED)
        {
            throw AccessOrderError(std::string("Illegal field access order. ") +
                "Cannot access field \"fdMetadata\" in state: " + codecStateName(codecState()) +
                ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                "]. Please see the diagram in the docs of the enum RequestRpcMetadataOptional::CodecState.");
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
        m_fdMetadata.wrap(m_buffer, m_offset + 19, m_actingVersion, m_bufferLength);
        return m_fdMetadata;
    }

    SBE_NODISCARD static const char *loadMetricMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static const char *loadMetricCharacterEncoding() SBE_NOEXCEPT
    {
        return "null";
    }

    static SBE_CONSTEXPR std::uint64_t loadMetricSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    bool loadMetricInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    static SBE_CONSTEXPR std::uint16_t loadMetricId() SBE_NOEXCEPT
    {
        return 7;
    }

    static SBE_CONSTEXPR std::uint64_t loadMetricHeaderLength() SBE_NOEXCEPT
    {
        return 4;
    }

private:
    void onLoadMetricLengthAccessed() const
    {
        switch (codecState())
        {
            case CodecState::V0_BLOCK:
                break;
            default:
                throw AccessOrderError(std::string("Illegal field access order. ") +
                    "Cannot decode length of var data \"loadMetric\" in state: " + codecStateName(codecState()) +
                    ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                    "]. Please see the diagram in the docs of the enum RequestRpcMetadataOptional::CodecState.");
        }
    }

public:
    SBE_NODISCARD std::uint32_t loadMetricLength() const
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onLoadMetricLengthAccessed();
#endif
        std::uint32_t length;
        std::memcpy(&length, m_buffer + sbePosition(), sizeof(std::uint32_t));
        return SBE_LITTLE_ENDIAN_ENCODE_32(length);
    }

private:
    void onLoadMetricAccessed()
    {
        switch (codecState())
        {
            case CodecState::V0_BLOCK:
                codecState(CodecState::V0_LOADMETRIC_DONE);
                break;
            default:
                throw AccessOrderError(std::string("Illegal field access order. ") +
                    "Cannot access field \"loadMetric\" in state: " + codecStateName(codecState()) +
                    ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                    "]. Please see the diagram in the docs of the enum RequestRpcMetadataOptional::CodecState.");
        }
    }

public:
    std::uint64_t skipLoadMetric()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onLoadMetricAccessed();
#endif
        std::uint64_t lengthOfLengthField = 4;
        std::uint64_t lengthPosition = sbePosition();
        std::uint32_t lengthFieldValue;
        std::memcpy(&lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint32_t));
        std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue);
        sbePosition(lengthPosition + lengthOfLengthField + dataLength);
        return dataLength;
    }

    SBE_NODISCARD const char *loadMetric()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onLoadMetricAccessed();
#endif
        std::uint32_t lengthFieldValue;
        std::memcpy(&lengthFieldValue, m_buffer + sbePosition(), sizeof(std::uint32_t));
        const char *fieldPtr = m_buffer + sbePosition() + 4;
        sbePosition(sbePosition() + 4 + SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue));
        return fieldPtr;
    }

    std::uint64_t getLoadMetric(char *dst, const std::uint64_t length)
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onLoadMetricAccessed();
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

    RequestRpcMetadataOptional &putLoadMetric(const char *src, const std::uint32_t length)
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onLoadMetricAccessed();
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

    std::string getLoadMetricAsString()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onLoadMetricAccessed();
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

    std::string getLoadMetricAsJsonEscapedString()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onLoadMetricAccessed();
#endif
        std::ostringstream oss;
        std::string s = getLoadMetricAsString();

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
    std::string_view getLoadMetricAsStringView()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onLoadMetricAccessed();
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

    RequestRpcMetadataOptional &putLoadMetric(const std::string &str)
    {
        if (str.length() > 1073741824)
        {
            throw std::runtime_error("std::string too long for length type [E109]");
        }
        return putLoadMetric(str.data(), static_cast<std::uint32_t>(str.length()));
    }

    #if __cplusplus >= 201703L
    RequestRpcMetadataOptional &putLoadMetric(const std::string_view str)
    {
        if (str.length() > 1073741824)
        {
            throw std::runtime_error("std::string too long for length type [E109]");
        }
        return putLoadMetric(str.data(), static_cast<std::uint32_t>(str.length()));
    }
    #endif

    SBE_NODISCARD static const char *tenantIdMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static const char *tenantIdCharacterEncoding() SBE_NOEXCEPT
    {
        return "UTF-8";
    }

    static SBE_CONSTEXPR std::uint64_t tenantIdSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    bool tenantIdInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    static SBE_CONSTEXPR std::uint16_t tenantIdId() SBE_NOEXCEPT
    {
        return 8;
    }

    static SBE_CONSTEXPR std::uint64_t tenantIdHeaderLength() SBE_NOEXCEPT
    {
        return 4;
    }

private:
    void onTenantIdLengthAccessed() const
    {
        switch (codecState())
        {
            case CodecState::V0_LOADMETRIC_DONE:
                break;
            default:
                throw AccessOrderError(std::string("Illegal field access order. ") +
                    "Cannot decode length of var data \"tenantId\" in state: " + codecStateName(codecState()) +
                    ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                    "]. Please see the diagram in the docs of the enum RequestRpcMetadataOptional::CodecState.");
        }
    }

public:
    SBE_NODISCARD std::uint32_t tenantIdLength() const
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onTenantIdLengthAccessed();
#endif
        std::uint32_t length;
        std::memcpy(&length, m_buffer + sbePosition(), sizeof(std::uint32_t));
        return SBE_LITTLE_ENDIAN_ENCODE_32(length);
    }

private:
    void onTenantIdAccessed()
    {
        switch (codecState())
        {
            case CodecState::V0_LOADMETRIC_DONE:
                codecState(CodecState::V0_TENANTID_DONE);
                break;
            default:
                throw AccessOrderError(std::string("Illegal field access order. ") +
                    "Cannot access field \"tenantId\" in state: " + codecStateName(codecState()) +
                    ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                    "]. Please see the diagram in the docs of the enum RequestRpcMetadataOptional::CodecState.");
        }
    }

public:
    std::uint64_t skipTenantId()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onTenantIdAccessed();
#endif
        std::uint64_t lengthOfLengthField = 4;
        std::uint64_t lengthPosition = sbePosition();
        std::uint32_t lengthFieldValue;
        std::memcpy(&lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint32_t));
        std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue);
        sbePosition(lengthPosition + lengthOfLengthField + dataLength);
        return dataLength;
    }

    SBE_NODISCARD const char *tenantId()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onTenantIdAccessed();
#endif
        std::uint32_t lengthFieldValue;
        std::memcpy(&lengthFieldValue, m_buffer + sbePosition(), sizeof(std::uint32_t));
        const char *fieldPtr = m_buffer + sbePosition() + 4;
        sbePosition(sbePosition() + 4 + SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue));
        return fieldPtr;
    }

    std::uint64_t getTenantId(char *dst, const std::uint64_t length)
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onTenantIdAccessed();
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

    RequestRpcMetadataOptional &putTenantId(const char *src, const std::uint32_t length)
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onTenantIdAccessed();
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

    std::string getTenantIdAsString()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onTenantIdAccessed();
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

    std::string getTenantIdAsJsonEscapedString()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onTenantIdAccessed();
#endif
        std::ostringstream oss;
        std::string s = getTenantIdAsString();

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
    std::string_view getTenantIdAsStringView()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onTenantIdAccessed();
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

    RequestRpcMetadataOptional &putTenantId(const std::string &str)
    {
        if (str.length() > 1073741824)
        {
            throw std::runtime_error("std::string too long for length type [E109]");
        }
        return putTenantId(str.data(), static_cast<std::uint32_t>(str.length()));
    }

    #if __cplusplus >= 201703L
    RequestRpcMetadataOptional &putTenantId(const std::string_view str)
    {
        if (str.length() > 1073741824)
        {
            throw std::runtime_error("std::string too long for length type [E109]");
        }
        return putTenantId(str.data(), static_cast<std::uint32_t>(str.length()));
    }
    #endif

    SBE_NODISCARD static const char *serviceTraceMetaMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static const char *serviceTraceMetaCharacterEncoding() SBE_NOEXCEPT
    {
        return "UTF-8";
    }

    static SBE_CONSTEXPR std::uint64_t serviceTraceMetaSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    bool serviceTraceMetaInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    static SBE_CONSTEXPR std::uint16_t serviceTraceMetaId() SBE_NOEXCEPT
    {
        return 9;
    }

    static SBE_CONSTEXPR std::uint64_t serviceTraceMetaHeaderLength() SBE_NOEXCEPT
    {
        return 4;
    }

private:
    void onServiceTraceMetaLengthAccessed() const
    {
        switch (codecState())
        {
            case CodecState::V0_TENANTID_DONE:
                break;
            default:
                throw AccessOrderError(std::string("Illegal field access order. ") +
                    "Cannot decode length of var data \"serviceTraceMeta\" in state: " + codecStateName(codecState()) +
                    ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                    "]. Please see the diagram in the docs of the enum RequestRpcMetadataOptional::CodecState.");
        }
    }

public:
    SBE_NODISCARD std::uint32_t serviceTraceMetaLength() const
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onServiceTraceMetaLengthAccessed();
#endif
        std::uint32_t length;
        std::memcpy(&length, m_buffer + sbePosition(), sizeof(std::uint32_t));
        return SBE_LITTLE_ENDIAN_ENCODE_32(length);
    }

private:
    void onServiceTraceMetaAccessed()
    {
        switch (codecState())
        {
            case CodecState::V0_TENANTID_DONE:
                codecState(CodecState::V0_SERVICETRACEMETA_DONE);
                break;
            default:
                throw AccessOrderError(std::string("Illegal field access order. ") +
                    "Cannot access field \"serviceTraceMeta\" in state: " + codecStateName(codecState()) +
                    ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                    "]. Please see the diagram in the docs of the enum RequestRpcMetadataOptional::CodecState.");
        }
    }

public:
    std::uint64_t skipServiceTraceMeta()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onServiceTraceMetaAccessed();
#endif
        std::uint64_t lengthOfLengthField = 4;
        std::uint64_t lengthPosition = sbePosition();
        std::uint32_t lengthFieldValue;
        std::memcpy(&lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint32_t));
        std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue);
        sbePosition(lengthPosition + lengthOfLengthField + dataLength);
        return dataLength;
    }

    SBE_NODISCARD const char *serviceTraceMeta()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onServiceTraceMetaAccessed();
#endif
        std::uint32_t lengthFieldValue;
        std::memcpy(&lengthFieldValue, m_buffer + sbePosition(), sizeof(std::uint32_t));
        const char *fieldPtr = m_buffer + sbePosition() + 4;
        sbePosition(sbePosition() + 4 + SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue));
        return fieldPtr;
    }

    std::uint64_t getServiceTraceMeta(char *dst, const std::uint64_t length)
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onServiceTraceMetaAccessed();
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

    RequestRpcMetadataOptional &putServiceTraceMeta(const char *src, const std::uint32_t length)
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onServiceTraceMetaAccessed();
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

    std::string getServiceTraceMetaAsString()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onServiceTraceMetaAccessed();
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

    std::string getServiceTraceMetaAsJsonEscapedString()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onServiceTraceMetaAccessed();
#endif
        std::ostringstream oss;
        std::string s = getServiceTraceMetaAsString();

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
    std::string_view getServiceTraceMetaAsStringView()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onServiceTraceMetaAccessed();
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

    RequestRpcMetadataOptional &putServiceTraceMeta(const std::string &str)
    {
        if (str.length() > 1073741824)
        {
            throw std::runtime_error("std::string too long for length type [E109]");
        }
        return putServiceTraceMeta(str.data(), static_cast<std::uint32_t>(str.length()));
    }

    #if __cplusplus >= 201703L
    RequestRpcMetadataOptional &putServiceTraceMeta(const std::string_view str)
    {
        if (str.length() > 1073741824)
        {
            throw std::runtime_error("std::string too long for length type [E109]");
        }
        return putServiceTraceMeta(str.data(), static_cast<std::uint32_t>(str.length()));
    }
    #endif

    SBE_NODISCARD static const char *loggingContextMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static const char *loggingContextCharacterEncoding() SBE_NOEXCEPT
    {
        return "null";
    }

    static SBE_CONSTEXPR std::uint64_t loggingContextSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    bool loggingContextInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    static SBE_CONSTEXPR std::uint16_t loggingContextId() SBE_NOEXCEPT
    {
        return 10;
    }

    static SBE_CONSTEXPR std::uint64_t loggingContextHeaderLength() SBE_NOEXCEPT
    {
        return 4;
    }

private:
    void onLoggingContextLengthAccessed() const
    {
        switch (codecState())
        {
            case CodecState::V0_SERVICETRACEMETA_DONE:
                break;
            default:
                throw AccessOrderError(std::string("Illegal field access order. ") +
                    "Cannot decode length of var data \"loggingContext\" in state: " + codecStateName(codecState()) +
                    ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                    "]. Please see the diagram in the docs of the enum RequestRpcMetadataOptional::CodecState.");
        }
    }

public:
    SBE_NODISCARD std::uint32_t loggingContextLength() const
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onLoggingContextLengthAccessed();
#endif
        std::uint32_t length;
        std::memcpy(&length, m_buffer + sbePosition(), sizeof(std::uint32_t));
        return SBE_LITTLE_ENDIAN_ENCODE_32(length);
    }

private:
    void onLoggingContextAccessed()
    {
        switch (codecState())
        {
            case CodecState::V0_SERVICETRACEMETA_DONE:
                codecState(CodecState::V0_LOGGINGCONTEXT_DONE);
                break;
            default:
                throw AccessOrderError(std::string("Illegal field access order. ") +
                    "Cannot access field \"loggingContext\" in state: " + codecStateName(codecState()) +
                    ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                    "]. Please see the diagram in the docs of the enum RequestRpcMetadataOptional::CodecState.");
        }
    }

public:
    std::uint64_t skipLoggingContext()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onLoggingContextAccessed();
#endif
        std::uint64_t lengthOfLengthField = 4;
        std::uint64_t lengthPosition = sbePosition();
        std::uint32_t lengthFieldValue;
        std::memcpy(&lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint32_t));
        std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue);
        sbePosition(lengthPosition + lengthOfLengthField + dataLength);
        return dataLength;
    }

    SBE_NODISCARD const char *loggingContext()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onLoggingContextAccessed();
#endif
        std::uint32_t lengthFieldValue;
        std::memcpy(&lengthFieldValue, m_buffer + sbePosition(), sizeof(std::uint32_t));
        const char *fieldPtr = m_buffer + sbePosition() + 4;
        sbePosition(sbePosition() + 4 + SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue));
        return fieldPtr;
    }

    std::uint64_t getLoggingContext(char *dst, const std::uint64_t length)
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onLoggingContextAccessed();
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

    RequestRpcMetadataOptional &putLoggingContext(const char *src, const std::uint32_t length)
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onLoggingContextAccessed();
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

    std::string getLoggingContextAsString()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onLoggingContextAccessed();
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

    std::string getLoggingContextAsJsonEscapedString()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onLoggingContextAccessed();
#endif
        std::ostringstream oss;
        std::string s = getLoggingContextAsString();

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
    std::string_view getLoggingContextAsStringView()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onLoggingContextAccessed();
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

    RequestRpcMetadataOptional &putLoggingContext(const std::string &str)
    {
        if (str.length() > 1073741824)
        {
            throw std::runtime_error("std::string too long for length type [E109]");
        }
        return putLoggingContext(str.data(), static_cast<std::uint32_t>(str.length()));
    }

    #if __cplusplus >= 201703L
    RequestRpcMetadataOptional &putLoggingContext(const std::string_view str)
    {
        if (str.length() > 1073741824)
        {
            throw std::runtime_error("std::string too long for length type [E109]");
        }
        return putLoggingContext(str.data(), static_cast<std::uint32_t>(str.length()));
    }
    #endif

template<typename CharT, typename Traits>
friend std::basic_ostream<CharT, Traits> & operator << (
    std::basic_ostream<CharT, Traits> &builder, const RequestRpcMetadataOptional &_writer)
{
    RequestRpcMetadataOptional writer(
        _writer.m_buffer,
        _writer.m_offset,
        _writer.m_bufferLength,
        _writer.m_actingBlockLength,
        _writer.m_actingVersion);

    builder << '{';
    builder << R"("Name": "RequestRpcMetadataOptional", )";
    builder << R"("sbeTemplateId": )";
    builder << writer.sbeTemplateId();
    builder << ", ";

    builder << R"("clientTimeoutMs": )";
    builder << +writer.clientTimeoutMs();

    builder << ", ";
    builder << R"("queueTimeoutMs": )";
    builder << +writer.queueTimeoutMs();

    builder << ", ";
    builder << R"("priority": )";
    builder << '"' << writer.priority() << '"';

    builder << ", ";
    builder << R"("compression": )";
    builder << '"' << writer.compression() << '"';

    builder << ", ";
    builder << R"("compressionConfig": )";
    builder << writer.compressionConfig();

    builder << ", ";
    builder << R"("fdMetadata": )";
    builder << writer.fdMetadata();

    builder << ", ";
    builder << R"("loadMetric": )";
    builder << '"' <<
        writer.skipLoadMetric() << " bytes of raw data\"";
    builder << ", ";
    builder << R"("tenantId": )";
    builder << '"' <<
        writer.getTenantIdAsJsonEscapedString().c_str() << '"';

    builder << ", ";
    builder << R"("serviceTraceMeta": )";
    builder << '"' <<
        writer.getServiceTraceMetaAsJsonEscapedString().c_str() << '"';

    builder << ", ";
    builder << R"("loggingContext": )";
    builder << '"' <<
        writer.skipLoggingContext() << " bytes of raw data\"";
    builder << '}';

    return builder;
}

void skip()
{
    skipLoadMetric();
    skipTenantId();
    skipServiceTraceMeta();
    skipLoggingContext();
}

SBE_NODISCARD static SBE_CONSTEXPR bool isConstLength() SBE_NOEXCEPT
{
    return false;
}

SBE_NODISCARD static std::size_t computeLength(
    std::size_t loadMetricLength = 0,
    std::size_t tenantIdLength = 0,
    std::size_t serviceTraceMetaLength = 0,
    std::size_t loggingContextLength = 0)
{
#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif
    std::size_t length = sbeBlockLength();

    length += loadMetricHeaderLength();
    if (loadMetricLength > 1073741824LL)
    {
        throw std::runtime_error("loadMetricLength too long for length type [E109]");
    }
    length += loadMetricLength;

    length += tenantIdHeaderLength();
    if (tenantIdLength > 1073741824LL)
    {
        throw std::runtime_error("tenantIdLength too long for length type [E109]");
    }
    length += tenantIdLength;

    length += serviceTraceMetaHeaderLength();
    if (serviceTraceMetaLength > 1073741824LL)
    {
        throw std::runtime_error("serviceTraceMetaLength too long for length type [E109]");
    }
    length += serviceTraceMetaLength;

    length += loggingContextHeaderLength();
    if (loggingContextLength > 1073741824LL)
    {
        throw std::runtime_error("loggingContextLength too long for length type [E109]");
    }
    length += loggingContextLength;

    return length;
#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
}
};

const std::string RequestRpcMetadataOptional::STATE_NAME_LOOKUP[6] =
{
    "NOT_WRAPPED",
    "V0_BLOCK",
    "V0_LOADMETRIC_DONE",
    "V0_TENANTID_DONE",
    "V0_SERVICETRACEMETA_DONE",
    "V0_LOGGINGCONTEXT_DONE",
};

const std::string RequestRpcMetadataOptional::STATE_TRANSITIONS_LOOKUP[6] =
{
    "\"wrap(version=0)\"",
    "\"clientTimeoutMs(?)\", \"queueTimeoutMs(?)\", \"priority(?)\", \"compression(?)\", \"compressionConfig(?)\", \"fdMetadata(?)\", \"loadMetricLength()\", \"loadMetric(?)\"",
    "\"tenantIdLength()\", \"tenantId(?)\"",
    "\"serviceTraceMetaLength()\", \"serviceTraceMeta(?)\"",
    "\"loggingContextLength()\", \"loggingContext(?)\"",
    "",
};

}
}
}
#endif
