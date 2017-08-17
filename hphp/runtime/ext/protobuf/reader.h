#ifndef PROTOBUF_PHP_READER_H
#define PROTOBUF_PHP_READER_H

#include <stdint.h>
#include <sys/types.h>

namespace HPHP {

typedef struct {
    const uint8_t *string;
    size_t len;
    size_t pos;
} reader_t;

void reader_init(reader_t *reader, char *string, size_t len);
int reader_has_more(reader_t *reader);
int reader_read_double(reader_t *reader, double *value);
int reader_read_fixed32(reader_t *reader, long *value);
int reader_read_fixed64(reader_t *reader, long *value);
int reader_read_float(reader_t *reader, double *value);
int reader_read_int(reader_t *reader, long *value);
int reader_read_signed_int(reader_t *reader, long *value);
int reader_read_string(reader_t *reader, char **string, int *len);
int reader_read_tag(reader_t *reader, uint32_t *field_number, uint8_t *wire_type);
int reader_skip_32bit(reader_t *reader);
int reader_skip_64bit(reader_t *reader);
int reader_skip_length_delimited(reader_t *reader);
int reader_skip_varint(reader_t *reader);
int reader_read_uint64(reader_t *reader, uint64_t *value);
} /*hphp*/
#endif /* PROTOBUF_PHP_READER_H */
