// @generated using thrift/lib/thrift/generate-rpc-metadata-sbe.sh
/* Generated SBE (Simple Binary Encoding) message codec */
#ifndef _APACHE_THRIFT_SBE_REQUESTRPCMETADATA_CXX_H_
#define _APACHE_THRIFT_SBE_REQUESTRPCMETADATA_CXX_H_

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

class RequestRpcMetadata
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
     *       V0_BLOCK -> V0_BLOCK [label="  protocol(?)  "];
     *       V0_BLOCK -> V0_BLOCK [label="  kind(?)  "];
     *       V0_BLOCK -> V0_OTHERMETADATA_DONE [label="  otherMetadataCount(0)  "];
     *       V0_BLOCK -> V0_OTHERMETADATA_N [label="  otherMetadataCount(>0)  "];
     *       V0_OTHERMETADATA_N_BLOCK -> V0_OTHERMETADATA_N_BLOCK [label="  otherMetadata.otherMetadataKeyLength()  "];
     *       V0_OTHERMETADATA_1_BLOCK -> V0_OTHERMETADATA_1_BLOCK [label="  otherMetadata.otherMetadataKeyLength()  "];
     *       V0_OTHERMETADATA_N_BLOCK -> V0_OTHERMETADATA_N_OTHERMETADATAKEY_DONE [label="  otherMetadata.otherMetadataKey(?)  "];
     *       V0_OTHERMETADATA_1_BLOCK -> V0_OTHERMETADATA_1_OTHERMETADATAKEY_DONE [label="  otherMetadata.otherMetadataKey(?)  "];
     *       V0_OTHERMETADATA_N_OTHERMETADATAKEY_DONE -> V0_OTHERMETADATA_N_OTHERMETADATAKEY_DONE [label="  otherMetadata.otherMetadataValueLength()  "];
     *       V0_OTHERMETADATA_1_OTHERMETADATAKEY_DONE -> V0_OTHERMETADATA_1_OTHERMETADATAKEY_DONE [label="  otherMetadata.otherMetadataValueLength()  "];
     *       V0_OTHERMETADATA_N_OTHERMETADATAKEY_DONE -> V0_OTHERMETADATA_N_OTHERMETADATAVALUE_DONE [label="  otherMetadata.otherMetadataValue(?)  "];
     *       V0_OTHERMETADATA_1_OTHERMETADATAKEY_DONE -> V0_OTHERMETADATA_1_OTHERMETADATAVALUE_DONE [label="  otherMetadata.otherMetadataValue(?)  "];
     *       V0_OTHERMETADATA_N_OTHERMETADATAVALUE_DONE -> V0_OTHERMETADATA_N_BLOCK [label="  otherMetadata.next()\n  where count - newIndex > 1  "];
     *       V0_OTHERMETADATA_N -> V0_OTHERMETADATA_N_BLOCK [label="  otherMetadata.next()\n  where count - newIndex > 1  "];
     *       V0_OTHERMETADATA_N_OTHERMETADATAVALUE_DONE -> V0_OTHERMETADATA_1_BLOCK [label="  otherMetadata.next()\n  where count - newIndex == 1  "];
     *       V0_OTHERMETADATA_N -> V0_OTHERMETADATA_1_BLOCK [label="  otherMetadata.next()\n  where count - newIndex == 1  "];
     *       V0_OTHERMETADATA_1_OTHERMETADATAVALUE_DONE -> V0_OTHERMETADATA_DONE [label="  otherMetadata.resetCountToIndex()  "];
     *       V0_OTHERMETADATA_DONE -> V0_OTHERMETADATA_DONE [label="  otherMetadata.resetCountToIndex()  "];
     *       V0_OTHERMETADATA_N_OTHERMETADATAVALUE_DONE -> V0_OTHERMETADATA_DONE [label="  otherMetadata.resetCountToIndex()  "];
     *       V0_OTHERMETADATA_N -> V0_OTHERMETADATA_DONE [label="  otherMetadata.resetCountToIndex()  "];
     *       V0_OTHERMETADATA_1_OTHERMETADATAVALUE_DONE -> V0_OTHERMETADATA_1_OTHERMETADATAVALUE_DONE [label="  nameLength()  "];
     *       V0_OTHERMETADATA_DONE -> V0_OTHERMETADATA_DONE [label="  nameLength()  "];
     *       V0_OTHERMETADATA_1_OTHERMETADATAVALUE_DONE -> V0_NAME_DONE [label="  name(?)  "];
     *       V0_OTHERMETADATA_DONE -> V0_NAME_DONE [label="  name(?)  "];
     *       V0_NAME_DONE -> V0_NAME_DONE [label="  interactionMetadataLength()  "];
     *       V0_NAME_DONE -> V0_INTERACTIONMETADATA_DONE [label="  interactionMetadata(?)  "];
     *       V0_INTERACTIONMETADATA_DONE -> V0_INTERACTIONMETADATA_DONE [label="  optionalMetdataLength()  "];
     *       V0_INTERACTIONMETADATA_DONE -> V0_OPTIONALMETDATA_DONE [label="  optionalMetdata(?)  "];
     *   }
     * }</pre>
     */
    enum class CodecState
    {
        NOT_WRAPPED = 0,
        V0_BLOCK = 1,
        V0_OTHERMETADATA_N = 2,
        V0_OTHERMETADATA_N_BLOCK = 3,
        V0_OTHERMETADATA_1_BLOCK = 4,
        V0_OTHERMETADATA_DONE = 5,
        V0_OTHERMETADATA_N_OTHERMETADATAKEY_DONE = 6,
        V0_OTHERMETADATA_N_OTHERMETADATAVALUE_DONE = 7,
        V0_OTHERMETADATA_1_OTHERMETADATAKEY_DONE = 8,
        V0_OTHERMETADATA_1_OTHERMETADATAVALUE_DONE = 9,
        V0_NAME_DONE = 10,
        V0_INTERACTIONMETADATA_DONE = 11,
        V0_OPTIONALMETDATA_DONE = 12,
    };

    static const std::string STATE_NAME_LOOKUP[13];
    static const std::string STATE_TRANSITIONS_LOOKUP[13];

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
    static constexpr std::uint16_t SBE_BLOCK_LENGTH = static_cast<std::uint16_t>(2);
    static constexpr std::uint16_t SBE_TEMPLATE_ID = static_cast<std::uint16_t>(1);
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

    RequestRpcMetadata() = default;

    RequestRpcMetadata(
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

    RequestRpcMetadata(
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

    RequestRpcMetadata(char *buffer, const std::uint64_t bufferLength) :
        RequestRpcMetadata(buffer, 0, bufferLength, sbeBlockLength(), sbeSchemaVersion())
    {
    }

    RequestRpcMetadata(
        char *buffer,
        const std::uint64_t bufferLength,
        const std::uint64_t actingBlockLength,
        const std::uint64_t actingVersion) :
        RequestRpcMetadata(buffer, 0, bufferLength, actingBlockLength, actingVersion)
    {
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint16_t sbeBlockLength() SBE_NOEXCEPT
    {
        return static_cast<std::uint16_t>(2);
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t sbeBlockAndHeaderLength() SBE_NOEXCEPT
    {
        return messageHeader::encodedLength() + sbeBlockLength();
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint16_t sbeTemplateId() SBE_NOEXCEPT
    {
        return static_cast<std::uint16_t>(1);
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

    RequestRpcMetadata &wrapForEncode(char *buffer, const std::uint64_t offset, const std::uint64_t bufferLength)
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

    RequestRpcMetadata &wrapAndApplyHeader(char *buffer, const std::uint64_t offset, const std::uint64_t bufferLength)
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

    RequestRpcMetadata &wrapForDecode(
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

    RequestRpcMetadata &sbeRewind()
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
        RequestRpcMetadata skipper(m_buffer, m_offset, m_bufferLength, sbeBlockLength(), m_actingVersion, m_codecState);
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
            case CodecState::V0_OPTIONALMETDATA_DONE:
                return;
            default:
                throw AccessOrderError(std::string("Not fully encoded, current state: ") +
                    codecStateName(m_codecState) + ", allowed transitions: " +
                    codecStateTransitions(m_codecState));
        }
#endif
    }


    SBE_NODISCARD static const char *protocolMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t protocolId() SBE_NOEXCEPT
    {
        return 1;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t protocolSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool protocolInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t protocolEncodingOffset() SBE_NOEXCEPT
    {
        return 0;
    }

private:
    void onProtocolAccessed() const
    {
        if (codecState() == CodecState::NOT_WRAPPED)
        {
            throw AccessOrderError(std::string("Illegal field access order. ") +
                "Cannot access field \"protocol\" in state: " + codecStateName(codecState()) +
                ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                "]. Please see the diagram in the docs of the enum RequestRpcMetadata::CodecState.");
        }
    }

public:
    SBE_NODISCARD static SBE_CONSTEXPR std::size_t protocolEncodingLength() SBE_NOEXCEPT
    {
        return 1;
    }

    SBE_NODISCARD std::uint8_t protocolRaw() const
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onProtocolAccessed();
#endif
        std::uint8_t val;
        std::memcpy(&val, m_buffer + m_offset + 0, sizeof(std::uint8_t));
        return (val);
    }

    SBE_NODISCARD ProtocolId::Value protocol() const
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onProtocolAccessed();
#endif
        std::uint8_t val;
        std::memcpy(&val, m_buffer + m_offset + 0, sizeof(std::uint8_t));
        return ProtocolId::get((val));
    }

    RequestRpcMetadata &protocol(const ProtocolId::Value value)
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onProtocolAccessed();
#endif
        std::uint8_t val = (value);
        std::memcpy(m_buffer + m_offset + 0, &val, sizeof(std::uint8_t));
        return *this;
    }

    SBE_NODISCARD static const char *kindMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static SBE_CONSTEXPR std::uint16_t kindId() SBE_NOEXCEPT
    {
        return 2;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t kindSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool kindInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::size_t kindEncodingOffset() SBE_NOEXCEPT
    {
        return 1;
    }

private:
    void onKindAccessed() const
    {
        if (codecState() == CodecState::NOT_WRAPPED)
        {
            throw AccessOrderError(std::string("Illegal field access order. ") +
                "Cannot access field \"kind\" in state: " + codecStateName(codecState()) +
                ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                "]. Please see the diagram in the docs of the enum RequestRpcMetadata::CodecState.");
        }
    }

public:
    SBE_NODISCARD static SBE_CONSTEXPR std::size_t kindEncodingLength() SBE_NOEXCEPT
    {
        return 1;
    }

    SBE_NODISCARD std::uint8_t kindRaw() const
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onKindAccessed();
#endif
        std::uint8_t val;
        std::memcpy(&val, m_buffer + m_offset + 1, sizeof(std::uint8_t));
        return (val);
    }

    SBE_NODISCARD RpcKind::Value kind() const
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onKindAccessed();
#endif
        std::uint8_t val;
        std::memcpy(&val, m_buffer + m_offset + 1, sizeof(std::uint8_t));
        return RpcKind::get((val));
    }

    RequestRpcMetadata &kind(const RpcKind::Value value)
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onKindAccessed();
#endif
        std::uint8_t val = (value);
        std::memcpy(m_buffer + m_offset + 1, &val, sizeof(std::uint8_t));
        return *this;
    }

    class OtherMetadata
    {
    private:
        char *m_buffer = nullptr;
        std::uint64_t m_bufferLength = 0;
        std::uint64_t m_initialPosition = 0;
        std::uint64_t *m_positionPtr = nullptr;
        std::uint64_t m_blockLength = 0;
        std::uint64_t m_count = 0;
        std::uint64_t m_index = 0;
        std::uint64_t m_offset = 0;
        std::uint64_t m_actingVersion = 0;

        SBE_NODISCARD std::uint64_t *sbePositionPtr() SBE_NOEXCEPT
        {
            return m_positionPtr;
        }

        CodecState *m_codecStatePtr = nullptr;

        CodecState codecState() const SBE_NOEXCEPT
        {
            return *m_codecStatePtr;
        }

        CodecState *codecStatePtr()
        {
            return m_codecStatePtr;
        }

        void codecState(CodecState codecState)
        {
            *m_codecStatePtr = codecState;
        }

    public:
        OtherMetadata() = default;

        inline void wrapForDecode(
            char *buffer,
            std::uint64_t *pos,
            const std::uint64_t actingVersion,
            const std::uint64_t bufferLength,
             CodecState *codecState)
        {
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
            m_codecStatePtr = codecState;
        }

        inline void wrapForEncode(
            char *buffer,
            const std::uint16_t count,
            std::uint64_t *pos,
            const std::uint64_t actingVersion,
            const std::uint64_t bufferLength,
             CodecState *codecState)
        {
    #if defined(__GNUG__) && !defined(__clang__)
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wtype-limits"
    #endif
            if (count > 65534)
            {
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
            m_codecStatePtr = codecState;
        }

    private:
        void onNextElementAccessed()
        {
            std::uint64_t remaining = m_count - m_index;
            if (remaining > 1)
            {
                switch (codecState())
                {
                    case CodecState::V0_OTHERMETADATA_N_OTHERMETADATAVALUE_DONE:
                    case CodecState::V0_OTHERMETADATA_N:
                        codecState(CodecState::V0_OTHERMETADATA_N_BLOCK);
                        break;
                    default:
                        throw AccessOrderError(std::string("Illegal field access order. ") +
                            "Cannot access next element in repeating group \"otherMetadata\" in state: " + codecStateName(codecState()) +
                            ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                            "]. Please see the diagram in the docs of the enum RequestRpcMetadata::CodecState.");
                }
            }
            else if (1 == remaining)
            {
                switch (codecState())
                {
                    case CodecState::V0_OTHERMETADATA_N_OTHERMETADATAVALUE_DONE:
                    case CodecState::V0_OTHERMETADATA_N:
                        codecState(CodecState::V0_OTHERMETADATA_1_BLOCK);
                        break;
                    default:
                        throw AccessOrderError(std::string("Illegal field access order. ") +
                            "Cannot access next element in repeating group \"otherMetadata\" in state: " + codecStateName(codecState()) +
                            ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                            "]. Please see the diagram in the docs of the enum RequestRpcMetadata::CodecState.");
                }
            }
        }

        void onResetCountToIndex()
        {
            switch (codecState())
            {
                case CodecState::V0_OTHERMETADATA_1_OTHERMETADATAVALUE_DONE:
                case CodecState::V0_OTHERMETADATA_DONE:
                case CodecState::V0_OTHERMETADATA_N_OTHERMETADATAVALUE_DONE:
                case CodecState::V0_OTHERMETADATA_N:
                    codecState(CodecState::V0_OTHERMETADATA_DONE);
                    break;
                default:
                    throw AccessOrderError(std::string("Illegal field access order. ") +
                        "Cannot reset count of repeating group \"otherMetadata\" in state: " + codecStateName(codecState()) +
                        ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                        "]. Please see the diagram in the docs of the enum RequestRpcMetadata::CodecState.");
            }
        }

    public:
        static SBE_CONSTEXPR std::uint64_t sbeHeaderSize() SBE_NOEXCEPT
        {
            return 4;
        }

        static SBE_CONSTEXPR std::uint64_t sbeBlockLength() SBE_NOEXCEPT
        {
            return 0;
        }

        SBE_NODISCARD std::uint64_t sbeActingBlockLength() SBE_NOEXCEPT
        {
            return m_blockLength;
        }

        SBE_NODISCARD std::uint64_t sbePosition() const SBE_NOEXCEPT
        {
            return *m_positionPtr;
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
            *m_positionPtr = sbeCheckPosition(position);
        }

        SBE_NODISCARD inline std::uint64_t count() const SBE_NOEXCEPT
        {
            return m_count;
        }

        SBE_NODISCARD inline bool hasNext() const SBE_NOEXCEPT
        {
            return m_index < m_count;
        }

        inline OtherMetadata &next()
        {
            if (m_index >= m_count)
            {
                throw std::runtime_error("index >= count [E108]");
            }
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
            onNextElementAccessed();
#endif
            m_offset = *m_positionPtr;
            if (SBE_BOUNDS_CHECK_EXPECT(((m_offset + m_blockLength) > m_bufferLength), false))
            {
                throw std::runtime_error("buffer too short for next group index [E108]");
            }
            *m_positionPtr = m_offset + m_blockLength;
            ++m_index;

            return *this;
        }

        inline std::uint64_t resetCountToIndex()
        {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
            onResetCountToIndex();
#endif
            m_count = m_index;
            GroupSizeEncoding dimensions(m_buffer, m_initialPosition, m_bufferLength, m_actingVersion);
            dimensions.numInGroup(static_cast<std::uint16_t>(m_count));
            return m_count;
        }

        template<class Func> inline void forEach(Func &&func)
        {
            while (hasNext())
            {
                next();
                func(*this);
            }
        }


        SBE_NODISCARD static const char *otherMetadataKeyMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
        {
            switch (metaAttribute)
            {
                case MetaAttribute::PRESENCE: return "required";
                default: return "";
            }
        }

        static const char *otherMetadataKeyCharacterEncoding() SBE_NOEXCEPT
        {
            return "UTF-8";
        }

        static SBE_CONSTEXPR std::uint64_t otherMetadataKeySinceVersion() SBE_NOEXCEPT
        {
            return 0;
        }

        bool otherMetadataKeyInActingVersion() SBE_NOEXCEPT
        {
            return true;
        }

        static SBE_CONSTEXPR std::uint16_t otherMetadataKeyId() SBE_NOEXCEPT
        {
            return 1;
        }

        static SBE_CONSTEXPR std::uint64_t otherMetadataKeyHeaderLength() SBE_NOEXCEPT
        {
            return 4;
        }

    private:
        void onOtherMetadataKeyLengthAccessed() const
        {
            switch (codecState())
            {
                case CodecState::V0_OTHERMETADATA_N_BLOCK:
                    break;
                case CodecState::V0_OTHERMETADATA_1_BLOCK:
                    break;
                default:
                    throw AccessOrderError(std::string("Illegal field access order. ") +
                        "Cannot decode length of var data \"otherMetadata.otherMetadataKey\" in state: " + codecStateName(codecState()) +
                        ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                        "]. Please see the diagram in the docs of the enum RequestRpcMetadata::CodecState.");
            }
        }

    public:
        SBE_NODISCARD std::uint32_t otherMetadataKeyLength() const
        {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
            onOtherMetadataKeyLengthAccessed();
#endif
            std::uint32_t length;
            std::memcpy(&length, m_buffer + sbePosition(), sizeof(std::uint32_t));
            return SBE_LITTLE_ENDIAN_ENCODE_32(length);
        }

    private:
        void onOtherMetadataKeyAccessed()
        {
            switch (codecState())
            {
                case CodecState::V0_OTHERMETADATA_N_BLOCK:
                    codecState(CodecState::V0_OTHERMETADATA_N_OTHERMETADATAKEY_DONE);
                    break;
                case CodecState::V0_OTHERMETADATA_1_BLOCK:
                    codecState(CodecState::V0_OTHERMETADATA_1_OTHERMETADATAKEY_DONE);
                    break;
                default:
                    throw AccessOrderError(std::string("Illegal field access order. ") +
                        "Cannot access field \"otherMetadata.otherMetadataKey\" in state: " + codecStateName(codecState()) +
                        ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                        "]. Please see the diagram in the docs of the enum RequestRpcMetadata::CodecState.");
            }
        }

    public:
        std::uint64_t skipOtherMetadataKey()
        {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
            onOtherMetadataKeyAccessed();
#endif
            std::uint64_t lengthOfLengthField = 4;
            std::uint64_t lengthPosition = sbePosition();
            std::uint32_t lengthFieldValue;
            std::memcpy(&lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint32_t));
            std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue);
            sbePosition(lengthPosition + lengthOfLengthField + dataLength);
            return dataLength;
        }

        SBE_NODISCARD const char *otherMetadataKey()
        {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
            onOtherMetadataKeyAccessed();
#endif
            std::uint32_t lengthFieldValue;
            std::memcpy(&lengthFieldValue, m_buffer + sbePosition(), sizeof(std::uint32_t));
            const char *fieldPtr = m_buffer + sbePosition() + 4;
            sbePosition(sbePosition() + 4 + SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue));
            return fieldPtr;
        }

        std::uint64_t getOtherMetadataKey(char *dst, const std::uint64_t length)
        {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
            onOtherMetadataKeyAccessed();
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

        OtherMetadata &putOtherMetadataKey(const char *src, const std::uint32_t length)
        {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
            onOtherMetadataKeyAccessed();
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

        std::string getOtherMetadataKeyAsString()
        {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
            onOtherMetadataKeyAccessed();
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

        std::string getOtherMetadataKeyAsJsonEscapedString()
        {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
            onOtherMetadataKeyAccessed();
#endif
            std::ostringstream oss;
            std::string s = getOtherMetadataKeyAsString();

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
        std::string_view getOtherMetadataKeyAsStringView()
        {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
            onOtherMetadataKeyAccessed();
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

        OtherMetadata &putOtherMetadataKey(const std::string &str)
        {
            if (str.length() > 1073741824)
            {
                throw std::runtime_error("std::string too long for length type [E109]");
            }
            return putOtherMetadataKey(str.data(), static_cast<std::uint32_t>(str.length()));
        }

        #if __cplusplus >= 201703L
        OtherMetadata &putOtherMetadataKey(const std::string_view str)
        {
            if (str.length() > 1073741824)
            {
                throw std::runtime_error("std::string too long for length type [E109]");
            }
            return putOtherMetadataKey(str.data(), static_cast<std::uint32_t>(str.length()));
        }
        #endif

        SBE_NODISCARD static const char *otherMetadataValueMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
        {
            switch (metaAttribute)
            {
                case MetaAttribute::PRESENCE: return "required";
                default: return "";
            }
        }

        static const char *otherMetadataValueCharacterEncoding() SBE_NOEXCEPT
        {
            return "null";
        }

        static SBE_CONSTEXPR std::uint64_t otherMetadataValueSinceVersion() SBE_NOEXCEPT
        {
            return 0;
        }

        bool otherMetadataValueInActingVersion() SBE_NOEXCEPT
        {
            return true;
        }

        static SBE_CONSTEXPR std::uint16_t otherMetadataValueId() SBE_NOEXCEPT
        {
            return 2;
        }

        static SBE_CONSTEXPR std::uint64_t otherMetadataValueHeaderLength() SBE_NOEXCEPT
        {
            return 4;
        }

    private:
        void onOtherMetadataValueLengthAccessed() const
        {
            switch (codecState())
            {
                case CodecState::V0_OTHERMETADATA_N_OTHERMETADATAKEY_DONE:
                    break;
                case CodecState::V0_OTHERMETADATA_1_OTHERMETADATAKEY_DONE:
                    break;
                default:
                    throw AccessOrderError(std::string("Illegal field access order. ") +
                        "Cannot decode length of var data \"otherMetadata.otherMetadataValue\" in state: " + codecStateName(codecState()) +
                        ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                        "]. Please see the diagram in the docs of the enum RequestRpcMetadata::CodecState.");
            }
        }

    public:
        SBE_NODISCARD std::uint32_t otherMetadataValueLength() const
        {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
            onOtherMetadataValueLengthAccessed();
#endif
            std::uint32_t length;
            std::memcpy(&length, m_buffer + sbePosition(), sizeof(std::uint32_t));
            return SBE_LITTLE_ENDIAN_ENCODE_32(length);
        }

    private:
        void onOtherMetadataValueAccessed()
        {
            switch (codecState())
            {
                case CodecState::V0_OTHERMETADATA_N_OTHERMETADATAKEY_DONE:
                    codecState(CodecState::V0_OTHERMETADATA_N_OTHERMETADATAVALUE_DONE);
                    break;
                case CodecState::V0_OTHERMETADATA_1_OTHERMETADATAKEY_DONE:
                    codecState(CodecState::V0_OTHERMETADATA_1_OTHERMETADATAVALUE_DONE);
                    break;
                default:
                    throw AccessOrderError(std::string("Illegal field access order. ") +
                        "Cannot access field \"otherMetadata.otherMetadataValue\" in state: " + codecStateName(codecState()) +
                        ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                        "]. Please see the diagram in the docs of the enum RequestRpcMetadata::CodecState.");
            }
        }

    public:
        std::uint64_t skipOtherMetadataValue()
        {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
            onOtherMetadataValueAccessed();
#endif
            std::uint64_t lengthOfLengthField = 4;
            std::uint64_t lengthPosition = sbePosition();
            std::uint32_t lengthFieldValue;
            std::memcpy(&lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint32_t));
            std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue);
            sbePosition(lengthPosition + lengthOfLengthField + dataLength);
            return dataLength;
        }

        SBE_NODISCARD const char *otherMetadataValue()
        {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
            onOtherMetadataValueAccessed();
#endif
            std::uint32_t lengthFieldValue;
            std::memcpy(&lengthFieldValue, m_buffer + sbePosition(), sizeof(std::uint32_t));
            const char *fieldPtr = m_buffer + sbePosition() + 4;
            sbePosition(sbePosition() + 4 + SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue));
            return fieldPtr;
        }

        std::uint64_t getOtherMetadataValue(char *dst, const std::uint64_t length)
        {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
            onOtherMetadataValueAccessed();
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

        OtherMetadata &putOtherMetadataValue(const char *src, const std::uint32_t length)
        {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
            onOtherMetadataValueAccessed();
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

        std::string getOtherMetadataValueAsString()
        {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
            onOtherMetadataValueAccessed();
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

        std::string getOtherMetadataValueAsJsonEscapedString()
        {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
            onOtherMetadataValueAccessed();
#endif
            std::ostringstream oss;
            std::string s = getOtherMetadataValueAsString();

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
        std::string_view getOtherMetadataValueAsStringView()
        {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
            onOtherMetadataValueAccessed();
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

        OtherMetadata &putOtherMetadataValue(const std::string &str)
        {
            if (str.length() > 1073741824)
            {
                throw std::runtime_error("std::string too long for length type [E109]");
            }
            return putOtherMetadataValue(str.data(), static_cast<std::uint32_t>(str.length()));
        }

        #if __cplusplus >= 201703L
        OtherMetadata &putOtherMetadataValue(const std::string_view str)
        {
            if (str.length() > 1073741824)
            {
                throw std::runtime_error("std::string too long for length type [E109]");
            }
            return putOtherMetadataValue(str.data(), static_cast<std::uint32_t>(str.length()));
        }
        #endif

        template<typename CharT, typename Traits>
        friend std::basic_ostream<CharT, Traits> & operator << (
            std::basic_ostream<CharT, Traits> &builder, OtherMetadata &writer)
        {
            builder << '{';
            builder << R"("otherMetadataKey": )";
            builder << '"' <<
                writer.getOtherMetadataKeyAsJsonEscapedString().c_str() << '"';

            builder << ", ";
            builder << R"("otherMetadataValue": )";
            builder << '"' <<
                writer.skipOtherMetadataValue() << " bytes of raw data\"";
            builder << '}';

            return builder;
        }

        void skip()
        {
            skipOtherMetadataKey();
            skipOtherMetadataValue();
        }

        SBE_NODISCARD static SBE_CONSTEXPR bool isConstLength() SBE_NOEXCEPT
        {
            return false;
        }

        SBE_NODISCARD static std::size_t computeLength(
            std::size_t otherMetadataKeyLength = 0,
            std::size_t otherMetadataValueLength = 0)
        {
#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif
            std::size_t length = sbeBlockLength();

            length += otherMetadataKeyHeaderLength();
            if (otherMetadataKeyLength > 1073741824LL)
            {
                throw std::runtime_error("otherMetadataKeyLength too long for length type [E109]");
            }
            length += otherMetadataKeyLength;

            length += otherMetadataValueHeaderLength();
            if (otherMetadataValueLength > 1073741824LL)
            {
                throw std::runtime_error("otherMetadataValueLength too long for length type [E109]");
            }
            length += otherMetadataValueLength;

            return length;
#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
        }
    };

private:
    OtherMetadata m_otherMetadata;

public:
    SBE_NODISCARD static SBE_CONSTEXPR std::uint16_t otherMetadataId() SBE_NOEXCEPT
    {
        return 3;
    }

private:
    void onOtherMetadataAccessed(std::uint64_t remaining, std::string action)
    {
        if (0 == remaining)
        {
            switch (codecState())
            {
                case CodecState::V0_BLOCK:
                    codecState(CodecState::V0_OTHERMETADATA_DONE);
                    break;
                default:
                    throw AccessOrderError(std::string("Illegal field access order. ") +
                        "Cannot " + action + " count of repeating group \"otherMetadata\" in state: " + codecStateName(codecState()) +
                        ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                        "]. Please see the diagram in the docs of the enum RequestRpcMetadata::CodecState.");
            }
        }
        else
        {
            switch (codecState())
            {
                case CodecState::V0_BLOCK:
                    codecState(CodecState::V0_OTHERMETADATA_N);
                    break;
                default:
                    throw AccessOrderError(std::string("Illegal field access order. ") +
                        "Cannot " + action + " count of repeating group \"otherMetadata\" in state: " + codecStateName(codecState()) +
                        ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                        "]. Please see the diagram in the docs of the enum RequestRpcMetadata::CodecState.");
            }
        }
    }

public:
    SBE_NODISCARD inline OtherMetadata &otherMetadata()
    {
        m_otherMetadata.wrapForDecode(m_buffer, sbePositionPtr(), m_actingVersion, m_bufferLength, codecStatePtr());
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onOtherMetadataAccessed(m_otherMetadata.count(), "decode");
#endif
        return m_otherMetadata;
    }

    OtherMetadata &otherMetadataCount(const std::uint16_t count)
    {
        m_otherMetadata.wrapForEncode(m_buffer, count, sbePositionPtr(), m_actingVersion, m_bufferLength, codecStatePtr());
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onOtherMetadataAccessed(count, "encode");
#endif
        return m_otherMetadata;
    }

    SBE_NODISCARD static SBE_CONSTEXPR std::uint64_t otherMetadataSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    SBE_NODISCARD bool otherMetadataInActingVersion() const SBE_NOEXCEPT
    {
        return true;
    }

    SBE_NODISCARD static const char *nameMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static const char *nameCharacterEncoding() SBE_NOEXCEPT
    {
        return "UTF-8";
    }

    static SBE_CONSTEXPR std::uint64_t nameSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    bool nameInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    static SBE_CONSTEXPR std::uint16_t nameId() SBE_NOEXCEPT
    {
        return 4;
    }

    static SBE_CONSTEXPR std::uint64_t nameHeaderLength() SBE_NOEXCEPT
    {
        return 4;
    }

private:
    void onNameLengthAccessed() const
    {
        switch (codecState())
        {
            case CodecState::V0_OTHERMETADATA_1_OTHERMETADATAVALUE_DONE:
                break;
            case CodecState::V0_OTHERMETADATA_DONE:
                break;
            default:
                throw AccessOrderError(std::string("Illegal field access order. ") +
                    "Cannot decode length of var data \"name\" in state: " + codecStateName(codecState()) +
                    ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                    "]. Please see the diagram in the docs of the enum RequestRpcMetadata::CodecState.");
        }
    }

public:
    SBE_NODISCARD std::uint32_t nameLength() const
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onNameLengthAccessed();
#endif
        std::uint32_t length;
        std::memcpy(&length, m_buffer + sbePosition(), sizeof(std::uint32_t));
        return SBE_LITTLE_ENDIAN_ENCODE_32(length);
    }

private:
    void onNameAccessed()
    {
        switch (codecState())
        {
            case CodecState::V0_OTHERMETADATA_1_OTHERMETADATAVALUE_DONE:
            case CodecState::V0_OTHERMETADATA_DONE:
                codecState(CodecState::V0_NAME_DONE);
                break;
            default:
                throw AccessOrderError(std::string("Illegal field access order. ") +
                    "Cannot access field \"name\" in state: " + codecStateName(codecState()) +
                    ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                    "]. Please see the diagram in the docs of the enum RequestRpcMetadata::CodecState.");
        }
    }

public:
    std::uint64_t skipName()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onNameAccessed();
#endif
        std::uint64_t lengthOfLengthField = 4;
        std::uint64_t lengthPosition = sbePosition();
        std::uint32_t lengthFieldValue;
        std::memcpy(&lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint32_t));
        std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue);
        sbePosition(lengthPosition + lengthOfLengthField + dataLength);
        return dataLength;
    }

    SBE_NODISCARD const char *name()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onNameAccessed();
#endif
        std::uint32_t lengthFieldValue;
        std::memcpy(&lengthFieldValue, m_buffer + sbePosition(), sizeof(std::uint32_t));
        const char *fieldPtr = m_buffer + sbePosition() + 4;
        sbePosition(sbePosition() + 4 + SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue));
        return fieldPtr;
    }

    std::uint64_t getName(char *dst, const std::uint64_t length)
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onNameAccessed();
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

    RequestRpcMetadata &putName(const char *src, const std::uint32_t length)
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onNameAccessed();
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

    std::string getNameAsString()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onNameAccessed();
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

    std::string getNameAsJsonEscapedString()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onNameAccessed();
#endif
        std::ostringstream oss;
        std::string s = getNameAsString();

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
    std::string_view getNameAsStringView()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onNameAccessed();
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

    RequestRpcMetadata &putName(const std::string &str)
    {
        if (str.length() > 1073741824)
        {
            throw std::runtime_error("std::string too long for length type [E109]");
        }
        return putName(str.data(), static_cast<std::uint32_t>(str.length()));
    }

    #if __cplusplus >= 201703L
    RequestRpcMetadata &putName(const std::string_view str)
    {
        if (str.length() > 1073741824)
        {
            throw std::runtime_error("std::string too long for length type [E109]");
        }
        return putName(str.data(), static_cast<std::uint32_t>(str.length()));
    }
    #endif

    SBE_NODISCARD static const char *interactionMetadataMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static const char *interactionMetadataCharacterEncoding() SBE_NOEXCEPT
    {
        return "null";
    }

    static SBE_CONSTEXPR std::uint64_t interactionMetadataSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    bool interactionMetadataInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    static SBE_CONSTEXPR std::uint16_t interactionMetadataId() SBE_NOEXCEPT
    {
        return 5;
    }

    static SBE_CONSTEXPR std::uint64_t interactionMetadataHeaderLength() SBE_NOEXCEPT
    {
        return 4;
    }

private:
    void onInteractionMetadataLengthAccessed() const
    {
        switch (codecState())
        {
            case CodecState::V0_NAME_DONE:
                break;
            default:
                throw AccessOrderError(std::string("Illegal field access order. ") +
                    "Cannot decode length of var data \"interactionMetadata\" in state: " + codecStateName(codecState()) +
                    ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                    "]. Please see the diagram in the docs of the enum RequestRpcMetadata::CodecState.");
        }
    }

public:
    SBE_NODISCARD std::uint32_t interactionMetadataLength() const
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onInteractionMetadataLengthAccessed();
#endif
        std::uint32_t length;
        std::memcpy(&length, m_buffer + sbePosition(), sizeof(std::uint32_t));
        return SBE_LITTLE_ENDIAN_ENCODE_32(length);
    }

private:
    void onInteractionMetadataAccessed()
    {
        switch (codecState())
        {
            case CodecState::V0_NAME_DONE:
                codecState(CodecState::V0_INTERACTIONMETADATA_DONE);
                break;
            default:
                throw AccessOrderError(std::string("Illegal field access order. ") +
                    "Cannot access field \"interactionMetadata\" in state: " + codecStateName(codecState()) +
                    ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                    "]. Please see the diagram in the docs of the enum RequestRpcMetadata::CodecState.");
        }
    }

public:
    std::uint64_t skipInteractionMetadata()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onInteractionMetadataAccessed();
#endif
        std::uint64_t lengthOfLengthField = 4;
        std::uint64_t lengthPosition = sbePosition();
        std::uint32_t lengthFieldValue;
        std::memcpy(&lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint32_t));
        std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue);
        sbePosition(lengthPosition + lengthOfLengthField + dataLength);
        return dataLength;
    }

    SBE_NODISCARD const char *interactionMetadata()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onInteractionMetadataAccessed();
#endif
        std::uint32_t lengthFieldValue;
        std::memcpy(&lengthFieldValue, m_buffer + sbePosition(), sizeof(std::uint32_t));
        const char *fieldPtr = m_buffer + sbePosition() + 4;
        sbePosition(sbePosition() + 4 + SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue));
        return fieldPtr;
    }

    std::uint64_t getInteractionMetadata(char *dst, const std::uint64_t length)
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onInteractionMetadataAccessed();
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

    RequestRpcMetadata &putInteractionMetadata(const char *src, const std::uint32_t length)
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onInteractionMetadataAccessed();
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

    std::string getInteractionMetadataAsString()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onInteractionMetadataAccessed();
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

    std::string getInteractionMetadataAsJsonEscapedString()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onInteractionMetadataAccessed();
#endif
        std::ostringstream oss;
        std::string s = getInteractionMetadataAsString();

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
    std::string_view getInteractionMetadataAsStringView()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onInteractionMetadataAccessed();
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

    RequestRpcMetadata &putInteractionMetadata(const std::string &str)
    {
        if (str.length() > 1073741824)
        {
            throw std::runtime_error("std::string too long for length type [E109]");
        }
        return putInteractionMetadata(str.data(), static_cast<std::uint32_t>(str.length()));
    }

    #if __cplusplus >= 201703L
    RequestRpcMetadata &putInteractionMetadata(const std::string_view str)
    {
        if (str.length() > 1073741824)
        {
            throw std::runtime_error("std::string too long for length type [E109]");
        }
        return putInteractionMetadata(str.data(), static_cast<std::uint32_t>(str.length()));
    }
    #endif

    SBE_NODISCARD static const char *optionalMetdataMetaAttribute(const MetaAttribute metaAttribute) SBE_NOEXCEPT
    {
        switch (metaAttribute)
        {
            case MetaAttribute::PRESENCE: return "required";
            default: return "";
        }
    }

    static const char *optionalMetdataCharacterEncoding() SBE_NOEXCEPT
    {
        return "null";
    }

    static SBE_CONSTEXPR std::uint64_t optionalMetdataSinceVersion() SBE_NOEXCEPT
    {
        return 0;
    }

    bool optionalMetdataInActingVersion() SBE_NOEXCEPT
    {
        return true;
    }

    static SBE_CONSTEXPR std::uint16_t optionalMetdataId() SBE_NOEXCEPT
    {
        return 6;
    }

    static SBE_CONSTEXPR std::uint64_t optionalMetdataHeaderLength() SBE_NOEXCEPT
    {
        return 4;
    }

private:
    void onOptionalMetdataLengthAccessed() const
    {
        switch (codecState())
        {
            case CodecState::V0_INTERACTIONMETADATA_DONE:
                break;
            default:
                throw AccessOrderError(std::string("Illegal field access order. ") +
                    "Cannot decode length of var data \"optionalMetdata\" in state: " + codecStateName(codecState()) +
                    ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                    "]. Please see the diagram in the docs of the enum RequestRpcMetadata::CodecState.");
        }
    }

public:
    SBE_NODISCARD std::uint32_t optionalMetdataLength() const
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onOptionalMetdataLengthAccessed();
#endif
        std::uint32_t length;
        std::memcpy(&length, m_buffer + sbePosition(), sizeof(std::uint32_t));
        return SBE_LITTLE_ENDIAN_ENCODE_32(length);
    }

private:
    void onOptionalMetdataAccessed()
    {
        switch (codecState())
        {
            case CodecState::V0_INTERACTIONMETADATA_DONE:
                codecState(CodecState::V0_OPTIONALMETDATA_DONE);
                break;
            default:
                throw AccessOrderError(std::string("Illegal field access order. ") +
                    "Cannot access field \"optionalMetdata\" in state: " + codecStateName(codecState()) +
                    ". Expected one of these transitions: [" + codecStateTransitions(codecState()) +
                    "]. Please see the diagram in the docs of the enum RequestRpcMetadata::CodecState.");
        }
    }

public:
    std::uint64_t skipOptionalMetdata()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onOptionalMetdataAccessed();
#endif
        std::uint64_t lengthOfLengthField = 4;
        std::uint64_t lengthPosition = sbePosition();
        std::uint32_t lengthFieldValue;
        std::memcpy(&lengthFieldValue, m_buffer + lengthPosition, sizeof(std::uint32_t));
        std::uint64_t dataLength = SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue);
        sbePosition(lengthPosition + lengthOfLengthField + dataLength);
        return dataLength;
    }

    SBE_NODISCARD const char *optionalMetdata()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onOptionalMetdataAccessed();
#endif
        std::uint32_t lengthFieldValue;
        std::memcpy(&lengthFieldValue, m_buffer + sbePosition(), sizeof(std::uint32_t));
        const char *fieldPtr = m_buffer + sbePosition() + 4;
        sbePosition(sbePosition() + 4 + SBE_LITTLE_ENDIAN_ENCODE_32(lengthFieldValue));
        return fieldPtr;
    }

    std::uint64_t getOptionalMetdata(char *dst, const std::uint64_t length)
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onOptionalMetdataAccessed();
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

    RequestRpcMetadata &putOptionalMetdata(const char *src, const std::uint32_t length)
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onOptionalMetdataAccessed();
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

    std::string getOptionalMetdataAsString()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onOptionalMetdataAccessed();
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

    std::string getOptionalMetdataAsJsonEscapedString()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onOptionalMetdataAccessed();
#endif
        std::ostringstream oss;
        std::string s = getOptionalMetdataAsString();

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
    std::string_view getOptionalMetdataAsStringView()
    {
#if defined(SBE_ENABLE_PRECEDENCE_CHECKS)
        onOptionalMetdataAccessed();
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

    RequestRpcMetadata &putOptionalMetdata(const std::string &str)
    {
        if (str.length() > 1073741824)
        {
            throw std::runtime_error("std::string too long for length type [E109]");
        }
        return putOptionalMetdata(str.data(), static_cast<std::uint32_t>(str.length()));
    }

    #if __cplusplus >= 201703L
    RequestRpcMetadata &putOptionalMetdata(const std::string_view str)
    {
        if (str.length() > 1073741824)
        {
            throw std::runtime_error("std::string too long for length type [E109]");
        }
        return putOptionalMetdata(str.data(), static_cast<std::uint32_t>(str.length()));
    }
    #endif

template<typename CharT, typename Traits>
friend std::basic_ostream<CharT, Traits> & operator << (
    std::basic_ostream<CharT, Traits> &builder, const RequestRpcMetadata &_writer)
{
    RequestRpcMetadata writer(
        _writer.m_buffer,
        _writer.m_offset,
        _writer.m_bufferLength,
        _writer.m_actingBlockLength,
        _writer.m_actingVersion);

    builder << '{';
    builder << R"("Name": "RequestRpcMetadata", )";
    builder << R"("sbeTemplateId": )";
    builder << writer.sbeTemplateId();
    builder << ", ";

    builder << R"("protocol": )";
    builder << '"' << writer.protocol() << '"';

    builder << ", ";
    builder << R"("kind": )";
    builder << '"' << writer.kind() << '"';

    builder << ", ";
    {
        bool atLeastOne = false;
        builder << R"("otherMetadata": [)";
        writer.otherMetadata().forEach(
            [&](OtherMetadata &otherMetadata)
            {
                if (atLeastOne)
                {
                    builder << ", ";
                }
                atLeastOne = true;
                builder << otherMetadata;
            });
        builder << ']';
    }

    builder << ", ";
    builder << R"("name": )";
    builder << '"' <<
        writer.getNameAsJsonEscapedString().c_str() << '"';

    builder << ", ";
    builder << R"("interactionMetadata": )";
    builder << '"' <<
        writer.skipInteractionMetadata() << " bytes of raw data\"";
    builder << ", ";
    builder << R"("optionalMetdata": )";
    builder << '"' <<
        writer.skipOptionalMetdata() << " bytes of raw data\"";
    builder << '}';

    return builder;
}

void skip()
{
    auto &otherMetadataGroup { otherMetadata() };
    while (otherMetadataGroup.hasNext())
    {
        otherMetadataGroup.next().skip();
    }
    skipName();
    skipInteractionMetadata();
    skipOptionalMetdata();
}

SBE_NODISCARD static SBE_CONSTEXPR bool isConstLength() SBE_NOEXCEPT
{
    return false;
}

SBE_NODISCARD static std::size_t computeLength(
    const std::vector<std::tuple<
        std::size_t,
        std::size_t>> &otherMetadataItemLengths = {},
    std::size_t nameLength = 0,
    std::size_t interactionMetadataLength = 0,
    std::size_t optionalMetdataLength = 0)
{
#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtype-limits"
#endif
    std::size_t length = sbeBlockLength();

    length += OtherMetadata::sbeHeaderSize();
    if (otherMetadataItemLengths.size() > 65534LL)
    {
        throw std::runtime_error("otherMetadataItemLengths.size() outside of allowed range [E110]");
    }

    for (const auto &e: otherMetadataItemLengths)
    {
        #if __cplusplus >= 201703L
        length += std::apply(OtherMetadata::computeLength, e);
        #else
        length += OtherMetadata::computeLength(std::get<0>(e), std::get<1>(e));
        #endif
    }

    length += nameHeaderLength();
    if (nameLength > 1073741824LL)
    {
        throw std::runtime_error("nameLength too long for length type [E109]");
    }
    length += nameLength;

    length += interactionMetadataHeaderLength();
    if (interactionMetadataLength > 1073741824LL)
    {
        throw std::runtime_error("interactionMetadataLength too long for length type [E109]");
    }
    length += interactionMetadataLength;

    length += optionalMetdataHeaderLength();
    if (optionalMetdataLength > 1073741824LL)
    {
        throw std::runtime_error("optionalMetdataLength too long for length type [E109]");
    }
    length += optionalMetdataLength;

    return length;
#if defined(__GNUG__) && !defined(__clang__)
#pragma GCC diagnostic pop
#endif
}
};

const std::string RequestRpcMetadata::STATE_NAME_LOOKUP[13] =
{
    "NOT_WRAPPED",
    "V0_BLOCK",
    "V0_OTHERMETADATA_N",
    "V0_OTHERMETADATA_N_BLOCK",
    "V0_OTHERMETADATA_1_BLOCK",
    "V0_OTHERMETADATA_DONE",
    "V0_OTHERMETADATA_N_OTHERMETADATAKEY_DONE",
    "V0_OTHERMETADATA_N_OTHERMETADATAVALUE_DONE",
    "V0_OTHERMETADATA_1_OTHERMETADATAKEY_DONE",
    "V0_OTHERMETADATA_1_OTHERMETADATAVALUE_DONE",
    "V0_NAME_DONE",
    "V0_INTERACTIONMETADATA_DONE",
    "V0_OPTIONALMETDATA_DONE",
};

const std::string RequestRpcMetadata::STATE_TRANSITIONS_LOOKUP[13] =
{
    "\"wrap(version=0)\"",
    "\"protocol(?)\", \"kind(?)\", \"otherMetadataCount(0)\", \"otherMetadataCount(>0)\"",
    "\"otherMetadata.next()\", \"otherMetadata.resetCountToIndex()\"",
    "\"otherMetadata.otherMetadataKeyLength()\", \"otherMetadata.otherMetadataKey(?)\"",
    "\"otherMetadata.otherMetadataKeyLength()\", \"otherMetadata.otherMetadataKey(?)\"",
    "\"otherMetadata.resetCountToIndex()\", \"nameLength()\", \"name(?)\"",
    "\"otherMetadata.otherMetadataValueLength()\", \"otherMetadata.otherMetadataValue(?)\"",
    "\"otherMetadata.next()\", \"otherMetadata.resetCountToIndex()\"",
    "\"otherMetadata.otherMetadataValueLength()\", \"otherMetadata.otherMetadataValue(?)\"",
    "\"otherMetadata.resetCountToIndex()\", \"nameLength()\", \"name(?)\"",
    "\"interactionMetadataLength()\", \"interactionMetadata(?)\"",
    "\"optionalMetdataLength()\", \"optionalMetdata(?)\"",
    "",
};

}
}
}
#endif
