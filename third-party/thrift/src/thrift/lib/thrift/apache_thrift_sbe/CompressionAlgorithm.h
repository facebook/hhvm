// @generated using thrift/lib/thrift/generate-rpc-metadata-sbe.sh
/* Generated SBE (Simple Binary Encoding) message codec */
#ifndef _APACHE_THRIFT_SBE_COMPRESSIONALGORITHM_CXX_H_
#define _APACHE_THRIFT_SBE_COMPRESSIONALGORITHM_CXX_H_

#if !defined(__STDC_LIMIT_MACROS)
#  define __STDC_LIMIT_MACROS 1
#endif

#include <cstdint>
#include <iomanip>
#include <limits>
#include <ostream>
#include <stdexcept>
#include <sstream>
#include <string>

#define SBE_NULLVALUE_INT8 (std::numeric_limits<std::int8_t>::min)()
#define SBE_NULLVALUE_INT16 (std::numeric_limits<std::int16_t>::min)()
#define SBE_NULLVALUE_INT32 (std::numeric_limits<std::int32_t>::min)()
#define SBE_NULLVALUE_INT64 (std::numeric_limits<std::int64_t>::min)()
#define SBE_NULLVALUE_UINT8 (std::numeric_limits<std::uint8_t>::max)()
#define SBE_NULLVALUE_UINT16 (std::numeric_limits<std::uint16_t>::max)()
#define SBE_NULLVALUE_UINT32 (std::numeric_limits<std::uint32_t>::max)()
#define SBE_NULLVALUE_UINT64 (std::numeric_limits<std::uint64_t>::max)()

namespace apache {
namespace thrift {
namespace sbe {

class CompressionAlgorithm
{
public:
    enum Value
    {
        NONE = static_cast<std::uint8_t>(0),
        ZLIB = static_cast<std::uint8_t>(1),
        ZSTD = static_cast<std::uint8_t>(2),
        NULL_VALUE = static_cast<std::uint8_t>(255)
    };

    static CompressionAlgorithm::Value get(const std::uint8_t value)
    {
        switch (value)
        {
            case static_cast<std::uint8_t>(0): return NONE;
            case static_cast<std::uint8_t>(1): return ZLIB;
            case static_cast<std::uint8_t>(2): return ZSTD;
            case static_cast<std::uint8_t>(255): return NULL_VALUE;
        }

        throw std::runtime_error("unknown value for enum CompressionAlgorithm [E103]");
    }

    static const char *c_str(const CompressionAlgorithm::Value value)
    {
        switch (value)
        {
            case NONE: return "NONE";
            case ZLIB: return "ZLIB";
            case ZSTD: return "ZSTD";
            case NULL_VALUE: return "NULL_VALUE";
        }

        throw std::runtime_error("unknown value for enum CompressionAlgorithm [E103]:");
    }

    template<typename CharT, typename Traits>
    friend std::basic_ostream<CharT, Traits> & operator << (
        std::basic_ostream<CharT, Traits> &os, CompressionAlgorithm::Value m)
    {
        return os << CompressionAlgorithm::c_str(m);
    }
};

}
}
}

#endif
