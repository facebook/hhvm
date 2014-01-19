/**
 *  Copyright 2009-2013 10gen, Inc.
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *  http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 */
#ifndef PHP_BSON_H
#define PHP_BSON_H 1


#define OID_SIZE 12

/* BSON type constants from http://bsonspec.org/#/specification */
#define BSON_DOUBLE    0x01
#define BSON_STRING    0x02
#define BSON_OBJECT    0x03
#define BSON_ARRAY     0x04
#define BSON_BINARY    0x05
#define BSON_UNDEF     0x06
#define BSON_OID       0x07
#define BSON_BOOL      0x08
#define BSON_DATE      0x09
#define BSON_NULL      0x0A
#define BSON_REGEX     0x0B
#define BSON_DBREF     0x0C
#define BSON_CODE__D   0x0D
#define BSON_SYMBOL    0x0E
#define BSON_CODE      0x0F
#define BSON_INT       0x10
#define BSON_TIMESTAMP 0x11
#define BSON_LONG      0x12
#define BSON_MINKEY    0xFF
#define BSON_MAXKEY    0x7F

#define GROW_SLOWLY 1048576
#define INVALID_STRING_LEN(len) (len < 0 || len > (64*1024*1024))

#define CREATE_BUF_STATIC(n) char b[n];         \
	buf.start = buf.pos = b;                    \
	buf.end = b+n;

/* driver */
int php_mongo_serialize_element(const char* name, int name_len, zval**, buffer*, int TSRMLS_DC);

/* objects */
void php_mongo_serialize_date(buffer*, zval* TSRMLS_DC);
void php_mongo_serialize_regex(buffer*, zval* TSRMLS_DC);
void php_mongo_serialize_code(buffer*, zval* TSRMLS_DC);
void php_mongo_serialize_ts(buffer*, zval* TSRMLS_DC);
void php_mongo_serialize_bin_data(buffer*, zval* TSRMLS_DC);
void php_mongo_serialize_int32(buffer*, zval* TSRMLS_DC);
void php_mongo_serialize_int64(buffer*, zval* TSRMLS_DC);

/* simple types */
void php_mongo_serialize_double(buffer*, double);
void php_mongo_serialize_string(buffer*, char*, int);
void php_mongo_serialize_long(buffer*, int64_t);
void php_mongo_serialize_int(buffer*, int);
void php_mongo_serialize_byte(buffer*, char);
void php_mongo_serialize_bytes(buffer*, char*, int);
void php_mongo_serialize_key(buffer*, const char*, int, int TSRMLS_DC);
void php_mongo_serialize_ns(buffer*, char* TSRMLS_DC);

int php_mongo_write_insert(buffer*, char*, zval*, int max_document_size, int max_message_size TSRMLS_DC);
int php_mongo_write_batch_insert(buffer*, char*, int flags, zval*, int max_document_size, int max_message_size TSRMLS_DC);
int php_mongo_write_query(buffer*, mongo_cursor*, int max_document_size, int max_message_size TSRMLS_DC);
int php_mongo_write_get_more(buffer*, mongo_cursor* TSRMLS_DC);
int php_mongo_write_delete(buffer*, char*, int, zval*, int max_document_size, int max_message_size TSRMLS_DC);
int php_mongo_write_update(buffer*, char*, int, zval*, zval*, int max_document_size, int max_message_size TSRMLS_DC);
int php_mongo_write_kill_cursors(buffer*, int64_t, int max_message_size TSRMLS_DC);

#define php_mongo_set_type(buf, type) php_mongo_serialize_byte(buf, (char)type)
#define php_mongo_serialize_null(buf) php_mongo_serialize_byte(buf, (char)0)
#define php_mongo_serialize_bool(buf, b) php_mongo_serialize_byte(buf, (char)b)

int resize_buf(buffer*, int);

int zval_to_bson(buffer*, HashTable*, int, int max_document_size TSRMLS_DC);
char* bson_to_zval(char*, HashTable* TSRMLS_DC);

/* Initialize buffer to contain "\0", so mongo_buf_append will start appending
 * at the beginning. */
void mongo_buf_init(char *dest);

/* Takes a buffer and a string to add to the buffer.  The buffer must be large
 * enough to append the string and the string must be null-terminated. This
 * will not work for strings containing null characters (e.g., BSON). */
void mongo_buf_append(char *dest, char *piece);

/* Returns the actual limit to send over the wire, based on batch size, current
 * position, and user limit */
int mongo_get_limit(mongo_cursor *cursor);


#if PHP_C_BIGENDIAN
/* Reverse the bytes in an int, wheeee stupid byte tricks */
# define BYTE1_32(b) ((b & 0xff000000) >> 24)
# define BYTE2_32(b) ((b & 0x00ff0000) >> 8)
# define BYTE3_32(b) ((b & 0x0000ff00) << 8)
# define BYTE4_32(b) ((b & 0x000000ff) << 24)
# define MONGO_32(b) (BYTE4_32(b) | BYTE3_32(b) | BYTE2_32(b) | BYTE1_32(b))

# define BYTE1_64(b) ((b & 0xff00000000000000ll) >> 56)
# define BYTE2_64(b) ((b & 0x00ff000000000000ll) >> 40)
# define BYTE3_64(b) ((b & 0x0000ff0000000000ll) >> 24)
# define BYTE4_64(b) ((b & 0x000000ff00000000ll) >> 8)
# define BYTE5_64(b) ((b & 0x00000000ff000000ll) << 8)
# define BYTE6_64(b) ((b & 0x0000000000ff0000ll) << 24)
# define BYTE7_64(b) ((b & 0x000000000000ff00ll) << 40)
# define BYTE8_64(b) ((b & 0x00000000000000ffll) << 56)
# define MONGO_64(b) (BYTE8_64(b) | BYTE7_64(b) | BYTE6_64(b) | BYTE5_64(b) | BYTE4_64(b) | BYTE3_64(b) | BYTE2_64(b) | BYTE1_64(b))
#else
# define MONGO_32(b) (b)
# define MONGO_64(b) (b)
#endif

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
