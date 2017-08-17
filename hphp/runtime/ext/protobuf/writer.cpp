#include <string.h>
#include "hphp/runtime/base/memory-manager.h"
#include "ext_protobuf.h"
#include "writer.h"

namespace HPHP {

#define WRITER_DATA_SIZE_INCREMENT 1024
#define WRITER_VARINT_SPACE 10
#define WRITER_32BIT_SPACE 4
#define WRITER_64BIT_SPACE 8

static inline int writer_ensure_space(writer_t *writer, size_t len);
static inline void writer_write_varint(writer_t *writer, int64_t value);

static inline void write_fixed32(fixed32_t value, uint8_t *out);
static inline void write_fixed64(fixed64_t value, uint8_t *out);

static inline void writer_write_varint_uint64(writer_t *writer, uint64_t value);

void writer_free_pack(writer_t *writer)
{
    if (writer->data != NULL) {
        smart_free(writer->data);
        writer->data = NULL;

        writer->size = 0;
        writer->pos = 0;
    }
}

void writer_init(writer_t *writer)
{
    if ((writer->data = (uint8_t *) smart_malloc(WRITER_DATA_SIZE_INCREMENT)) != NULL)
        writer->size = WRITER_DATA_SIZE_INCREMENT;
    else
        writer->size = 0;

    writer->pos = 0;
}

char *writer_get_pack(writer_t *writer, int *size)
{
    *size = writer->pos;
    writer->data[writer->pos] = '\0';
    return (char *) writer->data;
}

int writer_write_double(writer_t *writer, uint32_t field_number, double value)
{
    int64_t key;
    fixed64_t val;

    if (writer_ensure_space(writer, WRITER_VARINT_SPACE + WRITER_64BIT_SPACE) != 0)
        return -1;

    key = ((uint64_t) field_number << 3 | WIRE_TYPE_64BIT);

    writer_write_varint(writer, key);

    val.d_val = value;
    write_fixed64(val, writer->data + writer->pos);
    writer->pos += WRITER_64BIT_SPACE;

    return 0;
}

int writer_write_fixed32(writer_t *writer, uint32_t field_number, int32_t value)
{
    int64_t key;
    fixed32_t val;

    if (writer_ensure_space(writer, WRITER_VARINT_SPACE + WRITER_32BIT_SPACE) != 0)
        return -1;

    key = ((uint64_t) field_number << 3 | WIRE_TYPE_32BIT);

    writer_write_varint(writer, key);

    val.i_val = value;
    write_fixed32(val, writer->data + writer->pos);
    writer->pos += WRITER_32BIT_SPACE;

    return 0;
}

int writer_write_fixed64(writer_t *writer, uint32_t field_number, int64_t value)
{
    int64_t key;
    fixed64_t val;

    if (writer_ensure_space(writer, WRITER_VARINT_SPACE + WRITER_64BIT_SPACE) != 0)
        return -1;

    key = ((uint64_t) field_number << 3 | WIRE_TYPE_64BIT);

    writer_write_varint(writer, key);

    val.i_val = value;
    write_fixed64(val, writer->data + writer->pos);
    writer->pos += WRITER_64BIT_SPACE;

    return 0;
}

int writer_write_float(writer_t *writer, uint32_t field_number, double value)
{
    int64_t key;
    fixed32_t val;

    if (writer_ensure_space(writer, WRITER_VARINT_SPACE + WRITER_32BIT_SPACE) != 0)
        return -1;

    key = ((uint64_t) field_number << 3 | WIRE_TYPE_32BIT);

    writer_write_varint(writer, key);

    val.f_val = value;
    write_fixed32(val, writer->data + writer->pos);
    writer->pos += WRITER_32BIT_SPACE;

    return 0;
}

int writer_write_int(writer_t *writer, uint32_t field_number, int64_t value)
{
    int64_t key;

    if (writer_ensure_space(writer, 2 * WRITER_VARINT_SPACE) != 0)
        return -1;

    key = ((uint64_t) field_number << 3 | WIRE_TYPE_VARINT);

    writer_write_varint(writer, key);
    writer_write_varint(writer, value);

    return 0;
}

int writer_write_uint64(writer_t *writer, uint32_t field_number, uint64_t value)
{
    int64_t key;

    if (writer_ensure_space(writer, 2 * WRITER_VARINT_SPACE) != 0) {
        return -1;
    }
    key = ((uint64_t) field_number << 3 | WIRE_TYPE_VARINT);

    writer_write_varint(writer, key);
    writer_write_varint_uint64(writer, value);

    return 0;
}

int writer_write_signed_int(writer_t *writer, uint32_t field_number, int64_t value)
{
    int64_t key;

    if (writer_ensure_space(writer, 2 * WRITER_VARINT_SPACE) != 0)
        return -1;

    key = ((uint64_t) field_number << 3 | WIRE_TYPE_VARINT);

    writer_write_varint(writer, key);

    if (value < 0)
        *(uint64_t *) &value = ((uint64_t) (-value) * 2) - 1;
    else
        *(uint64_t *) &value = 2 * value;

    writer_write_varint(writer, value);

    return 0;
}

int writer_write_string(writer_t *writer, uint32_t field_number, const char *str, size_t len)
{
    int64_t key;

    if (writer_ensure_space(writer, 2 * WRITER_VARINT_SPACE + len) != 0)
        return -1;

    key = ((uint64_t) field_number << 3 | WIRE_TYPE_LENGTH_DELIMITED);

    writer_write_varint(writer, key);
    writer_write_varint(writer, len);
    memcpy(writer->data + writer->pos, str, len);
    writer->pos += len;

    return 0;
}

static inline int writer_ensure_space(writer_t *writer, size_t space)
{
    uint8_t *data;

    if ((writer->size - writer->pos) + 1 < space) {
        writer->size += space;
        data = (uint8_t *) smart_realloc(writer->data, writer->size);

        if (data == NULL)
            return -1;

        writer->data = data;
    }

    return 0;
}

static inline void writer_write_varint(writer_t *writer, int64_t value)
{
    int i;
    uint8_t byte;

    if (value == 0) {
        writer->data[writer->pos++] = 0;
    } else if (value < 0) {
        for (i = 0; i < WRITER_VARINT_SPACE - 1; i++) {
            writer->data[writer->pos++] = value | 0x80;
            *(uint64_t *) &value >>= 7;
        }

        writer->data[writer->pos++] = value;
    } else {
        do {
            byte = value;
            value >>= 7;

            if (value != 0)
                byte |= 0x80;
            else
                byte &= 0x7F;

            writer->data[writer->pos++] = byte;
        } while (value != 0);
    }
}

static inline void writer_write_varint_uint64(writer_t *writer, uint64_t value)
{
    uint8_t byte = 0;

    if (value == 0) {
        writer->data[writer->pos++] = 0;
    } else {
        do {
            byte = value;
            value >>= 7;
        
            if (value != 0) {
                byte |= 0x80;
            } else {
                byte &= 0x7F;                
            }
        
            writer->data[writer->pos++] = byte;
        } while (value != 0);
    }
}

static inline void write_fixed32(fixed32_t value, uint8_t *out)
{
#ifndef WORDS_BIGENDIAN
    memcpy(out, &value.u_val, 4);
#else
    out[0] = value.u_val;
    out[1] = value.u_val >> 8;
    out[2] = value.u_val >> 16;
    out[3] = value.u_val >> 24;
#endif
}

static inline void write_fixed64(fixed64_t value, uint8_t *out)
{
#ifndef WORDS_BIGENDIAN
    memcpy(out, &value.u_val, 8);
#else
    fixed32_t lo;
    lo.u_val = value.u_val;

    fixed32_t hi;
    hi.u_val = value.u_val >> 32;

    write_fixed32(lo, out);
    write_fixed32(hi, out + 4);
#endif
}

} /*hphp*/
