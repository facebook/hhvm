// @generated using thrift/lib/thrift/generate-rpc-metadata-sbe.sh
/* Generated SBE (Simple Binary Encoding) message codec */
#ifndef _APACHE_THRIFT_SBE_ERRORBLAME_CXX_H_
#define _APACHE_THRIFT_SBE_ERRORBLAME_CXX_H_

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

class ErrorBlame
{
public:
    enum Value
    {
        UNSPECIFIED = static_cast<std::uint8_t>(0),
        SERVER = static_cast<std::uint8_t>(1),
        CLIENT = static_cast<std::uint8_t>(2),
        NULL_VALUE = static_cast<std::uint8_t>(255)
    };

    static ErrorBlame::Value get(const std::uint8_t value)
    {
        switch (value)
        {
            case static_cast<std::uint8_t>(0): return UNSPECIFIED;
            case static_cast<std::uint8_t>(1): return SERVER;
            case static_cast<std::uint8_t>(2): return CLIENT;
            case static_cast<std::uint8_t>(255): return NULL_VALUE;
        }

        throw std::runtime_error("unknown value for enum ErrorBlame [E103]");
    }

    static const char *c_str(const ErrorBlame::Value value)
    {
        switch (value)
        {
            case UNSPECIFIED: return "UNSPECIFIED";
            case SERVER: return "SERVER";
            case CLIENT: return "CLIENT";
            case NULL_VALUE: return "NULL_VALUE";
        }

        throw std::runtime_error("unknown value for enum ErrorBlame [E103]:");
    }

    template<typename CharT, typename Traits>
    friend std::basic_ostream<CharT, Traits> & operator << (
        std::basic_ostream<CharT, Traits> &os, ErrorBlame::Value m)
    {
        return os << ErrorBlame::c_str(m);
    }
};

}
}
}

#endif
