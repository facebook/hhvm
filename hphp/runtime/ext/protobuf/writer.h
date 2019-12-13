#ifndef PROTOBUF_PHP_WRITER_H
#define PROTOBUF_PHP_WRITER_H

#include <stdint.h>
#include <stdlib.h>

#define smart_free free
#define smart_malloc malloc
#define smart_realloc realloc

namespace HPHP {

typedef struct {
    uint8_t *data;
    size_t pos;
    size_t size;
} writer_t;

void writer_free_pack(writer_t *writer);
void writer_init(writer_t *writer);
char *writer_get_pack(writer_t *writer, int *size);
int writer_write_double(writer_t *writer, uint32_t field_number, double value);
int writer_write_fixed32(writer_t *writer, uint32_t field_number, int32_t value);
int writer_write_fixed64(writer_t *writer, uint32_t field_number, int64_t value);
int writer_write_float(writer_t *writer, uint32_t field_number, double value);
int writer_write_int(writer_t *writer, uint32_t field_number, int64_t value);
int writer_write_signed_int(writer_t *writer, uint32_t field_number, int64_t value);
int writer_write_string(writer_t *writer, uint32_t field_number, const char *str, size_t len);
int writer_write_uint64(writer_t *writer, uint32_t field_number, uint64_t value);

#define writer_write_message(writer, field_number, str, len) writer_write_string((writer), (field_number), (str), (len))
} /*hphp*/
#endif /* PROTOBUF_PHP_WRITER_H */
