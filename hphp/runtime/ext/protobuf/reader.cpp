#include <string.h>

#include "ext_protobuf.h"
#include "reader.h"

namespace HPHP {

#define READER_LEFT(reader) (reader)->len - (reader)->pos

static inline fixed32_t reader_read_fixed_uint32(const uint8_t *data);
static inline fixed64_t reader_read_fixed_uint64(const uint8_t *data);

static inline int reader_read_varint(reader_t *reader, uint64_t *value);

void reader_init(reader_t *reader, char *string, size_t len)
{
    reader->string = (const uint8_t *) string;
    reader->len = len;
    reader->pos = 0;
}

int reader_has_more(reader_t *reader)
{
    return reader->len != reader->pos;
}

int reader_read_double(reader_t *reader, double *value)
{
    fixed64_t val;

    if (READER_LEFT(reader) >= 8)
    {
        val = reader_read_fixed_uint64(reader->string + reader->pos);
        *value = val.d_val;
        reader->pos += 8;
        return 0;
    }

    return -1;
}

int reader_read_fixed32(reader_t *reader, long *value)
{
    fixed32_t val;

    if (READER_LEFT(reader) >= 4)
    {
        val = reader_read_fixed_uint32(reader->string + reader->pos);
        *value = val.i_val;
        reader->pos += 4;
        return 0;
    }

    return -1;
}

int reader_read_fixed64(reader_t *reader, long *value)
{
    fixed64_t val;

    if (READER_LEFT(reader) >= 8)
    {
        val = reader_read_fixed_uint64(reader->string + reader->pos);
        *value = val.i_val;
        reader->pos += 8;
        return 0;
    }

    return -1;
}

int reader_read_float(reader_t *reader, double *value)
{
    fixed32_t val;

    if (READER_LEFT(reader) >= 4)
    {
        val = reader_read_fixed_uint32(reader->string + reader->pos);
        *value = val.f_val;
        reader->pos += 4;
        return 0;
    }

    return -1;
}

int reader_read_int(reader_t *reader, long *value)
{
    uint64_t val;

    if (reader_read_varint(reader, &val) != 0)
        return -1;

    *value = *(int64_t *) &val;

    return 0;
}

int reader_read_uint64(reader_t *reader, uint64_t *value)
{
    if (reader_read_varint(reader, value) != 0) {
        return -1;
    } else {
        return 0;
    }
}

int reader_read_signed_int(reader_t *reader, long *value)
{
    uint64_t val;

    if (reader_read_varint(reader, &val) != 0)
        return -1;

    if (val & 1)
        val = -((*(uint64_t *) &val) >> 1) - 1;
    else
        val = ((*(uint64_t *) &val) >> 1);

    *value = val;

    return 0;
}

int reader_read_string(reader_t *reader, char **string, int *len)
{
    uint64_t l;

    if (reader_read_varint(reader, &l) != 0)
        return -1;

    if (READER_LEFT(reader) < l)
        return -1;

    if (string != NULL)
    {
        *string = (char *) (reader->string + reader->pos);
        *len = l;
    }

    reader->pos += l;

    return 0;
}

int reader_read_tag(reader_t *reader, uint32_t *field_number, uint8_t *wire_type)
{
    uint64_t key;

    if (reader_read_varint(reader, &key) != 0)
        return -1;

    *wire_type = key & 0x07;
    *field_number = (key >> 3);

    return 0;
}

int reader_skip_32bit(reader_t *reader)
{
    if (READER_LEFT(reader) >= 4)
    {
        reader->pos += 4;
        return 0;
    }

    return -1;
}

int reader_skip_64bit(reader_t *reader)
{
    if (READER_LEFT(reader) >= 8)
    {
        reader->pos += 8;
        return 0;
    }

    return -1;
}

int reader_skip_length_delimited(reader_t *reader)
{
    return reader_read_string(reader, NULL, NULL);
}

int reader_skip_varint(reader_t *reader)
{
    return reader_read_varint(reader, NULL);
}

static inline int reader_read_varint(reader_t *reader, uint64_t *value)
{
    int i = 0;
    uint64_t byte;
    uint64_t val = 0;

    while (reader_has_more(reader) && i <= 10)
    {
        byte = reader->string[reader->pos];
        byte &= 0x7F;
        byte <<= (7 * i++);
        val |= byte;

        if (reader->string[reader->pos++] <= 0x7F)
        {
            if (value != NULL)
                *value = val;

            return 0;
        }
    }

    return -1;
}

static inline fixed32_t reader_read_fixed_uint32(const uint8_t *data)
{
    fixed32_t val;
#ifndef WORDS_BIGENDIAN
    memcpy (&val.u_val, data, 4);
    return val;
#else
    val.u_val = data[0] | (data[1] << 8) | (data[2] << 16) | (data[3] << 24);
    return val;
#endif
}

static inline fixed64_t reader_read_fixed_uint64(const uint8_t *data)
{
    fixed64_t val;
#ifndef WORDS_BIGENDIAN
    memcpy (&val.u_val, data, 8);
    return val;
#else
    fixed32_t lo = reader_read_fixed_uint32(data);
    fixed32_t hi = reader_read_fixed_uint32(data + 4);

    val.u_val = (uint64_t) lo.u_val | ((uint64_t) hi.u_val) << 32;

    return val;
#endif
}

} /*hphp*/
