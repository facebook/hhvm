#ifndef PROTOBUF_PHP_PROTOBUF_H
#define PROTOBUF_PHP_PROTOBUF_H

#include <stdint.h>
#include "hphp/runtime/ext/extension.h"


namespace HPHP {

typedef const Variant & CVarRef;

enum {
    WIRE_TYPE_VARINT = 0,
    WIRE_TYPE_64BIT  = 1,
    WIRE_TYPE_LENGTH_DELIMITED = 2,
    WIRE_TYPE_32BIT = 5
};

typedef union {
    float f_val;
    int32_t i_val;
    uint32_t u_val;
} fixed32_t;

typedef union {
    double d_val;
    int64_t i_val;
    uint64_t u_val;
} fixed64_t;

} /*hphp*/
#endif /* PROTOBUF_PHP_PROTOBUF_H */
