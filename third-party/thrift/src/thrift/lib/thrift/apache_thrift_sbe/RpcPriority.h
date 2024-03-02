// @generated using thrift/lib/thrift/generate-rpc-metadata-sbe.sh
/* Generated SBE (Simple Binary Encoding) message codec */
#ifndef _APACHE_THRIFT_SBE_RPCPRIORITY_CXX_H_
#define _APACHE_THRIFT_SBE_RPCPRIORITY_CXX_H_

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

class RpcPriority
{
public:
    enum Value
    {
        HIGH_IMPORTANT = static_cast<std::uint8_t>(0),
        HIGH = static_cast<std::uint8_t>(1),
        IMPORTANT = static_cast<std::uint8_t>(2),
        NORMAL = static_cast<std::uint8_t>(3),
        BEST_EFFORT = static_cast<std::uint8_t>(4),
        N_PRIORITIES = static_cast<std::uint8_t>(5),
        NULL_VALUE = static_cast<std::uint8_t>(255)
    };

    static RpcPriority::Value get(const std::uint8_t value)
    {
        switch (value)
        {
            case static_cast<std::uint8_t>(0): return HIGH_IMPORTANT;
            case static_cast<std::uint8_t>(1): return HIGH;
            case static_cast<std::uint8_t>(2): return IMPORTANT;
            case static_cast<std::uint8_t>(3): return NORMAL;
            case static_cast<std::uint8_t>(4): return BEST_EFFORT;
            case static_cast<std::uint8_t>(5): return N_PRIORITIES;
            case static_cast<std::uint8_t>(255): return NULL_VALUE;
        }

        throw std::runtime_error("unknown value for enum RpcPriority [E103]");
    }

    static const char *c_str(const RpcPriority::Value value)
    {
        switch (value)
        {
            case HIGH_IMPORTANT: return "HIGH_IMPORTANT";
            case HIGH: return "HIGH";
            case IMPORTANT: return "IMPORTANT";
            case NORMAL: return "NORMAL";
            case BEST_EFFORT: return "BEST_EFFORT";
            case N_PRIORITIES: return "N_PRIORITIES";
            case NULL_VALUE: return "NULL_VALUE";
        }

        throw std::runtime_error("unknown value for enum RpcPriority [E103]:");
    }

    template<typename CharT, typename Traits>
    friend std::basic_ostream<CharT, Traits> & operator << (
        std::basic_ostream<CharT, Traits> &os, RpcPriority::Value m)
    {
        return os << RpcPriority::c_str(m);
    }
};

}
}
}

#endif
