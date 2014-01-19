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
#include <php.h>
#include <zend_exceptions.h>

#ifdef WIN32
#  include <memory.h>
#  ifndef int64_t
     typedef __int64 int64_t;
#  endif
#endif

#include "php_mongo.h"
#include "bson.h"
#include "types/date.h"
#include "types/id.h"

extern zend_class_entry *mongo_ce_BinData,
	*mongo_ce_Code,
	*mongo_ce_Date,
	*mongo_ce_Id,
	*mongo_ce_Regex,
	*mongo_ce_Timestamp,
	*mongo_ce_MinKey,
	*mongo_ce_MaxKey,
	*mongo_ce_Exception,
	*mongo_ce_CursorException,
	*mongo_ce_Int32,
	*mongo_ce_Int64;

ZEND_EXTERN_MODULE_GLOBALS(mongo)

static int prep_obj_for_db(buffer *buf, HashTable *array TSRMLS_DC);
#if PHP_VERSION_ID >= 50300
static int apply_func_args_wrapper(void **data TSRMLS_DC, int num_args, va_list args, zend_hash_key *key);
#else
static int apply_func_args_wrapper(void **data, int num_args, va_list args, zend_hash_key *key);
#endif
static int is_utf8(const char *s, int len);
static int insert_helper(buffer *buf, zval *doc, int max TSRMLS_DC);

/* The position is not increased, we are just filling in the first 4 bytes with
 * the size.  */
static int php_mongo_serialize_size(char *start, buffer *buf, int max_size TSRMLS_DC)
{
	int total = MONGO_32((buf->pos - start));

	if (buf->pos - start > max_size) {
		zend_throw_exception_ex(mongo_ce_Exception, 3 TSRMLS_CC, "document fragment is too large: %d, max: %d", buf->pos - start, max_size);
		return FAILURE;
	}
	memcpy(start, &total, INT_32);
	return SUCCESS;
}


static int prep_obj_for_db(buffer *buf, HashTable *array TSRMLS_DC)
{
	zval temp, **data, *newid;

	/* if _id field doesn't exist, add it */
	if (zend_hash_find(array, "_id", 4, (void**)&data) == FAILURE) {
		/* create new MongoId */
		MAKE_STD_ZVAL(newid);
		object_init_ex(newid, mongo_ce_Id);
		MONGO_METHOD(MongoId, __construct, &temp, newid);

		/* add to obj */
		zend_hash_add(array, "_id", 4, &newid, sizeof(zval*), NULL);

		/* set to data */
		data = &newid;
	}

	php_mongo_serialize_element("_id", 3, data, buf, 0 TSRMLS_CC);
	if (EG(exception)) {
		return FAILURE;
	}

	return SUCCESS;
}


/* serialize a zval */
int zval_to_bson(buffer *buf, HashTable *hash, int prep, int max_document_size TSRMLS_DC)
{
	uint start;
	int num = 0;

	/* check buf size */
	if (BUF_REMAINING <= 5) {
		resize_buf(buf, 5);
	}

	/* keep a record of the starting position as an offset, in case the memory
	 * is resized */
	start = buf->pos-buf->start;

	/* skip first 4 bytes to leave room for size */
	buf->pos += INT_32;

	if (zend_hash_num_elements(hash) > 0) {
		if (prep) {
			prep_obj_for_db(buf, hash TSRMLS_CC);
			num++;
		}

#if PHP_VERSION_ID >= 50300
		zend_hash_apply_with_arguments(hash TSRMLS_CC, (apply_func_args_t)apply_func_args_wrapper, 3, buf, prep, &num);
#else
		zend_hash_apply_with_arguments(hash, (apply_func_args_t)apply_func_args_wrapper, 4, buf, prep, &num TSRMLS_CC);
#endif
	}

	php_mongo_serialize_null(buf);
	php_mongo_serialize_size(buf->start + start, buf, max_document_size TSRMLS_CC);
	return EG(exception) ? FAILURE : num;
}

#if PHP_VERSION_ID >= 50300
static int apply_func_args_wrapper(void **data TSRMLS_DC, int num_args, va_list args, zend_hash_key *key)
#else
static int apply_func_args_wrapper(void **data, int num_args, va_list args, zend_hash_key *key)
#endif
{
	buffer *buf = va_arg(args, buffer*);
	int prep = va_arg(args, int);
	int *num = va_arg(args, int*);
#if ZTS && PHP_VERSION_ID < 50300
	void ***tsrm_ls = va_arg(args, void***);
#endif

	if (key->nKeyLength) {
		return php_mongo_serialize_element(key->arKey, key->nKeyLength - 1, (zval**)data, buf, prep TSRMLS_CC);
	} else {
		long current = key->h;
		int pos = 29, negative = 0;
		char name[30];

		/* If the key is a number in ascending order, we're still dealing with
		 * an array, not an object, so increase the count */
		if (key->h == (unsigned int)*num) {
			(*num)++;
		}

		name[pos--] = '\0';

		/* take care of negative indexes */
		if (current < 0) {
			current *= -1;
			negative = 1;
		}

		do {
			int digit = current % 10;

			digit += (int)'0';
			name[pos--] = (char)digit;
			current = current / 10;
		} while (current > 0);

		if (negative) {
			name[pos--] = '-';
		}

		return php_mongo_serialize_element(name + pos + 1, strlen(name + pos + 1), (zval**)data, buf, prep TSRMLS_CC);
	}
}

int php_mongo_serialize_element(const char *name, int name_len, zval **data, buffer *buf, int prep TSRMLS_DC)
{
	if (prep && strcmp(name, "_id") == 0) {
		return ZEND_HASH_APPLY_KEEP;
	}

	switch (Z_TYPE_PP(data)) {
		case IS_NULL:
			PHP_MONGO_SERIALIZE_KEY(BSON_NULL);
			break;

		case IS_LONG:
			if (MonGlo(native_long)) {
#if SIZEOF_LONG == 4
			PHP_MONGO_SERIALIZE_KEY(BSON_INT);
			php_mongo_serialize_int(buf, Z_LVAL_PP(data));
#else
# if SIZEOF_LONG == 8
			PHP_MONGO_SERIALIZE_KEY(BSON_LONG);
			php_mongo_serialize_long(buf, Z_LVAL_PP(data));
# else
#  error The PHP number size is neither 4 or 8 bytes; no clue what to do with that!
# endif
#endif
			} else {
				PHP_MONGO_SERIALIZE_KEY(BSON_INT);
				php_mongo_serialize_int(buf, Z_LVAL_PP(data));
			}
			break;

		case IS_DOUBLE:
			PHP_MONGO_SERIALIZE_KEY(BSON_DOUBLE);
			php_mongo_serialize_double(buf, Z_DVAL_PP(data));
			break;

		case IS_BOOL:
			PHP_MONGO_SERIALIZE_KEY(BSON_BOOL);
			php_mongo_serialize_bool(buf, Z_BVAL_PP(data));
			break;

		case IS_STRING: {
			PHP_MONGO_SERIALIZE_KEY(BSON_STRING);

			/* if this is not a valid string, stop */
			if (!is_utf8(Z_STRVAL_PP(data), Z_STRLEN_PP(data))) {
				zend_throw_exception_ex(mongo_ce_Exception, 12 TSRMLS_CC, "non-utf8 string: %s", Z_STRVAL_PP(data));
				return ZEND_HASH_APPLY_STOP;
			}

			php_mongo_serialize_int(buf, Z_STRLEN_PP(data) + 1);
			php_mongo_serialize_string(buf, Z_STRVAL_PP(data), Z_STRLEN_PP(data));
			break;
		}

		case IS_ARRAY: {
			int num;
			/* if we realloc, we need an offset, not an abs pos (phew) */
			int type_offset = buf->pos-buf->start;

			/* serialize */
			PHP_MONGO_SERIALIZE_KEY(BSON_ARRAY);
			num = zval_to_bson(buf, Z_ARRVAL_PP(data), NO_PREP, MONGO_DEFAULT_MAX_DOCUMENT_SIZE TSRMLS_CC);
			if (EG(exception)) {
				return ZEND_HASH_APPLY_STOP;
			}

			/* now go back and set the type bit */
			if (num == zend_hash_num_elements(Z_ARRVAL_PP(data))) {
				buf->start[type_offset] = BSON_ARRAY;
			} else {
				buf->start[type_offset] = BSON_OBJECT;
			}

			break;
		}

		case IS_OBJECT: {
			zend_class_entry *clazz = Z_OBJCE_PP(data);

			/* check for defined classes */
			/* MongoId */
			if (clazz == mongo_ce_Id) {
				mongo_id *id;

				PHP_MONGO_SERIALIZE_KEY(BSON_OID);
				id = (mongo_id*)zend_object_store_get_object(*data TSRMLS_CC);
				if (!id->id) {
					return ZEND_HASH_APPLY_KEEP;
				}

				php_mongo_serialize_bytes(buf, id->id, OID_SIZE);
			}
			/* MongoDate */
			else if (clazz == mongo_ce_Date) {
				PHP_MONGO_SERIALIZE_KEY(BSON_DATE);
				php_mongo_serialize_date(buf, *data TSRMLS_CC);
			}
			/* MongoRegex */
			else if (clazz == mongo_ce_Regex) {
				PHP_MONGO_SERIALIZE_KEY(BSON_REGEX);
				php_mongo_serialize_regex(buf, *data TSRMLS_CC);
			}
			/* MongoCode */
			else if (clazz == mongo_ce_Code) {
				PHP_MONGO_SERIALIZE_KEY(BSON_CODE);
				php_mongo_serialize_code(buf, *data TSRMLS_CC);
				if (EG(exception)) {
					return ZEND_HASH_APPLY_STOP;
				}
			}
			/* MongoBin */
			else if (clazz == mongo_ce_BinData) {
				PHP_MONGO_SERIALIZE_KEY(BSON_BINARY);
				php_mongo_serialize_bin_data(buf, *data TSRMLS_CC);
			}
			/* MongoTimestamp */
			else if (clazz == mongo_ce_Timestamp) {
				PHP_MONGO_SERIALIZE_KEY(BSON_TIMESTAMP);
				php_mongo_serialize_ts(buf, *data TSRMLS_CC);
			}
			/* MongoMinKey */
			else if (clazz == mongo_ce_MinKey) {
				PHP_MONGO_SERIALIZE_KEY(BSON_MINKEY);
			}
			/* MongoMaxKey */
			else if (clazz == mongo_ce_MaxKey) {
				PHP_MONGO_SERIALIZE_KEY(BSON_MAXKEY);
			}
			/* Integer types */
			else if (clazz == mongo_ce_Int32) {
				PHP_MONGO_SERIALIZE_KEY(BSON_INT);
				php_mongo_serialize_int32(buf, *data TSRMLS_CC);
			}
			else if (clazz == mongo_ce_Int64) {
				PHP_MONGO_SERIALIZE_KEY(BSON_LONG);
				php_mongo_serialize_int64(buf, *data TSRMLS_CC);
			}
			/* serialize a normal object */
			else {
				HashTable *hash = Z_OBJPROP_PP(data);

				/* go through the k/v pairs and serialize them */
				PHP_MONGO_SERIALIZE_KEY(BSON_OBJECT);

				zval_to_bson(buf, hash, NO_PREP, MONGO_DEFAULT_MAX_MESSAGE_SIZE TSRMLS_CC);
				if (EG(exception)) {
					return ZEND_HASH_APPLY_STOP;
				}
			}
			break;
		}

		default:
			assert(false);
			break;
	}

	return ZEND_HASH_APPLY_KEEP;
}

int resize_buf(buffer *buf, int size)
{
	int total = buf->end - buf->start;
	int used = buf->pos - buf->start;

	total = total < GROW_SLOWLY ? total*2 : total + INITIAL_BUF_SIZE;
	while (total-used < size) {
		total += size;
	}

	buf->start = (char*)erealloc(buf->start, total);
	buf->pos = buf->start + used;
	buf->end = buf->start + total;
	return total;
}

/*
 * create a bson date
 *
 * type: 9
 * 8 bytes of ms since the epoch
 */
void php_mongo_serialize_date(buffer *buf, zval *date TSRMLS_DC)
{
	int64_t ms;
	zval *sec = zend_read_property(mongo_ce_Date, date, "sec", 3, 0 TSRMLS_CC);
	zval *usec = zend_read_property(mongo_ce_Date, date, "usec", 4, 0 TSRMLS_CC);

	ms = ((int64_t)Z_LVAL_P(sec) * 1000) + ((int64_t)Z_LVAL_P(usec) / 1000);
	php_mongo_serialize_long(buf, ms);
}

#if defined(_MSC_VER)
# define strtoll(s, f, b) _atoi64(s)
#elif !defined(HAVE_STRTOLL)
# if defined(HAVE_ATOLL)
#  define strtoll(s, f, b) atoll(s)
# else
#  define strtoll(s, f, b) strtol(s, f, b)
# endif
#endif


/*
 * create a bson int from an Int32 object
 */
void php_mongo_serialize_int32(buffer *buf, zval *data TSRMLS_DC)
{
	int value;
	zval *zvalue = zend_read_property(mongo_ce_Int32, data, "value", 5, 0 TSRMLS_CC);
	value = strtol(Z_STRVAL_P(zvalue), NULL, 10);

	php_mongo_serialize_int(buf, value);
}

/*
 * create a bson long from an Int64 object
 */
void php_mongo_serialize_int64(buffer *buf, zval *data TSRMLS_DC)
{
	int64_t value;
	zval *zvalue = zend_read_property(mongo_ce_Int64, data, "value", 5, 0 TSRMLS_CC);
	value = strtoll(Z_STRVAL_P(zvalue), NULL, 10);

	php_mongo_serialize_long(buf, value);
}

/*
 * create a bson regex
 *
 * type: 11
 * cstring cstring
 */
void php_mongo_serialize_regex(buffer *buf, zval *regex TSRMLS_DC)
{
	zval *z;

	z = zend_read_property(mongo_ce_Regex, regex, "regex", 5, 0 TSRMLS_CC);
	php_mongo_serialize_string(buf, Z_STRVAL_P(z), Z_STRLEN_P(z));
	z = zend_read_property(mongo_ce_Regex, regex, "flags", 5, 0 TSRMLS_CC);
	php_mongo_serialize_string(buf, Z_STRVAL_P(z), Z_STRLEN_P(z));
}

/*
 * create a bson code with scope
 *
 * type: 15
 * 4 bytes total size
 * 4 bytes cstring size + NULL
 * cstring
 * bson object scope
 */
void php_mongo_serialize_code(buffer *buf, zval *code TSRMLS_DC)
{
	uint start;
	zval *zid;

	/* save spot for size */
	start = buf->pos-buf->start;
	buf->pos += INT_32;
	zid = zend_read_property(mongo_ce_Code, code, "code", 4, NOISY TSRMLS_CC);
	/* string size */
	php_mongo_serialize_int(buf, Z_STRLEN_P(zid) + 1);
	/* string */
	php_mongo_serialize_string(buf, Z_STRVAL_P(zid), Z_STRLEN_P(zid));
	/* scope */
	zid = zend_read_property(mongo_ce_Code, code, "scope", 5, NOISY TSRMLS_CC);
	zval_to_bson(buf, HASH_P(zid), NO_PREP, MONGO_DEFAULT_MAX_MESSAGE_SIZE TSRMLS_CC);
	if (EG(exception)) {
		return;
	}

	/* get total size */
	php_mongo_serialize_size(buf->start + start, buf, MONGO_DEFAULT_MAX_MESSAGE_SIZE TSRMLS_CC);
}

/*
 * create bson binary data
 *
 * type: 5
 * 4 bytes: length of bindata
 * 1 byte: bindata type
 * bindata
 */
void php_mongo_serialize_bin_data(buffer *buf, zval *bin TSRMLS_DC)
{
	zval *zbin, *ztype;

	zbin = zend_read_property(mongo_ce_BinData, bin, "bin", 3, 0 TSRMLS_CC);
	ztype = zend_read_property(mongo_ce_BinData, bin, "type", 4, 0 TSRMLS_CC);

	/*
	 * type 2 has the redundant structure:
	 *
	 * |------|--|-------==========|
	 *  length 02 length   bindata
	 *
	 *   - 4 bytes: length of bindata (+4 for length below)
	 *   - 1 byte type (0x02)
	 *   - N bytes: 4 bytes of length of the following bindata + bindata
	 *
	 * All other types have:
	 *
	 * |------|--|==========|
	 *  length     bindata
	 *        type
	 */
	if (Z_LVAL_P(ztype) == 2) {
		/* length */
		php_mongo_serialize_int(buf, Z_STRLEN_P(zbin) + 4);
		/* 02 */
		php_mongo_serialize_byte(buf, 2);
		/* length */
		php_mongo_serialize_int(buf, Z_STRLEN_P(zbin));
	} else {
		/* length */
		php_mongo_serialize_int(buf, Z_STRLEN_P(zbin));
		/* type */
		php_mongo_serialize_byte(buf, (unsigned char)Z_LVAL_P(ztype));
	}

	/* bindata */
	php_mongo_serialize_bytes(buf, Z_STRVAL_P(zbin), Z_STRLEN_P(zbin));
}

/*
 * create bson timestamp
 *
 * type: 17
 * 4 bytes seconds since epoch
 * 4 bytes increment
 */
void php_mongo_serialize_ts(buffer *buf, zval *time TSRMLS_DC)
{
	zval *ts, *inc;

	ts = zend_read_property(mongo_ce_Timestamp, time, "sec", strlen("sec"), NOISY TSRMLS_CC);
	inc = zend_read_property(mongo_ce_Timestamp, time, "inc", strlen("inc"), NOISY TSRMLS_CC);

	php_mongo_serialize_int(buf, Z_LVAL_P(inc));
	php_mongo_serialize_int(buf, Z_LVAL_P(ts));
}

void php_mongo_serialize_byte(buffer *buf, char b)
{
	if (BUF_REMAINING <= 1) {
		resize_buf(buf, 1);
	}
	*(buf->pos) = b;
	buf->pos += 1;
}

void php_mongo_serialize_bytes(buffer *buf, char *str, int str_len)
{
	if (BUF_REMAINING <= str_len) {
		resize_buf(buf, str_len);
	}
	memcpy(buf->pos, str, str_len);
	buf->pos += str_len;
}

void php_mongo_serialize_string(buffer *buf, char *str, int str_len)
{
	if (BUF_REMAINING <= str_len + 1) {
		resize_buf(buf, str_len + 1);
	}

	memcpy(buf->pos, str, str_len);
	/* add \0 at the end of the string */
	buf->pos[str_len] = 0;
	buf->pos += str_len + 1;
}

void php_mongo_serialize_int(buffer *buf, int num)
{
	int i = MONGO_32(num);

	if (BUF_REMAINING <= INT_32) {
		resize_buf(buf, INT_32);
	}

	memcpy(buf->pos, &i, INT_32);
	buf->pos += INT_32;
}

void php_mongo_serialize_long(buffer *buf, int64_t num)
{
	int64_t i = MONGO_64(num);

	if (BUF_REMAINING <= INT_64) {
		resize_buf(buf, INT_64);
	}

	memcpy(buf->pos, &i, INT_64);
	buf->pos += INT_64;
}

void php_mongo_serialize_double(buffer *buf, double num)
{
	int64_t dest, *dest_p;

	dest_p = &dest;
	memcpy(dest_p, &num, 8);
	dest = MONGO_64(dest);

	if (BUF_REMAINING <= INT_64) {
		resize_buf(buf, INT_64);
	}

	memcpy(buf->pos, dest_p, DOUBLE_64);
	buf->pos += DOUBLE_64;
}

/*
 * prep == true
 * we are inserting, so keys can't have .'s in them
 */
void php_mongo_serialize_key(buffer *buf, const char *str, int str_len, int prep TSRMLS_DC)
{
	if (str[0] == '\0' && !MonGlo(allow_empty_keys)) {
		zend_throw_exception_ex(mongo_ce_Exception, 1 TSRMLS_CC, "zero-length keys are not allowed, did you use $ with double quotes?");
		return;
	}

	if (BUF_REMAINING <= str_len + 1) {
		resize_buf(buf, str_len + 1);
	}

	if (memchr(str, '\0', str_len) != NULL) {
		zend_throw_exception_ex(mongo_ce_Exception, 2 TSRMLS_CC, "'\\0' not allowed in key: %s\\0...", str);
		return;
	}

	if (prep && (strchr(str, '.') != 0)) {
		zend_throw_exception_ex(mongo_ce_Exception, 2 TSRMLS_CC, "'.' not allowed in key: %s", str);
		return;
	}

	if (MonGlo(cmd_char) && strchr(str, MonGlo(cmd_char)[0]) == str) {
		*(buf->pos) = '$';
		memcpy(buf->pos + 1, str + 1, str_len-1);
	} else {
		memcpy(buf->pos, str, str_len);
	}

	/* add \0 at the end of the string */
	buf->pos[str_len] = 0;
	buf->pos += str_len + 1;
}

/*
 * replaces collection names starting with MonGlo(cmd_char)
 * with the '$' character.
 *
 * TODO: this doesn't handle main.$oplog-type situations (if
 * MonGlo(cmd_char) is set)
 */
void php_mongo_serialize_ns(buffer *buf, char *str TSRMLS_DC)
{
	char *collection = strchr(str, '.') + 1;

	if (BUF_REMAINING <= (int)strlen(str) + 1) {
		resize_buf(buf, strlen(str) + 1);
	}

	if (MonGlo(cmd_char) && strchr(collection, MonGlo(cmd_char)[0]) == collection) {
		memcpy(buf->pos, str, collection-str);
		buf->pos += collection-str;
		*(buf->pos) = '$';
		memcpy(buf->pos + 1, collection + 1, strlen(collection)-1);
		buf->pos[strlen(collection)] = 0;
		buf->pos += strlen(collection) + 1;
	} else {
		memcpy(buf->pos, str, strlen(str));
		buf->pos[strlen(str)] = 0;
		buf->pos += strlen(str) + 1;
	}
}

/* Returns:
 *  0 on success,
 * -1 when an exception in zval_to_bson was thrown
 * -2 when there were no elements
 * -3 when a fragment or document was too large
 * An exception is also thrown when the return value is not 0 */
static int insert_helper(buffer *buf, zval *doc, int max_document_size TSRMLS_DC)
{
	int start = buf->pos - buf->start;
	int result = zval_to_bson(buf, HASH_P(doc), PREP, max_document_size TSRMLS_CC);

	/* throw exception if serialization crapped out */
	if (EG(exception) || FAILURE == result) {
		return -1;
	} else if (0 == result) {
		/* return if there were 0 elements */
		zend_throw_exception_ex(mongo_ce_Exception, 4 TSRMLS_CC, "no elements in doc");
		return -2;
	}

	/* throw an exception if the doc was too big */
	if (buf->pos - (buf->start + start) > max_document_size) {
		zend_throw_exception_ex(mongo_ce_Exception, 5 TSRMLS_CC, "size of BSON doc is %d bytes, max is %d", buf->pos - (buf->start + start), max_document_size);
		return -3;
	}

	return (php_mongo_serialize_size(buf->start + start, buf, max_document_size TSRMLS_CC) == SUCCESS) ? 0 : -3;
}

int php_mongo_write_insert(buffer *buf, char *ns, zval *doc, int max_document_size, int max_message_size TSRMLS_DC)
{
	mongo_msg_header header;
	int start = buf->pos - buf->start;

	CREATE_HEADER(buf, ns, OP_INSERT);

	if (insert_helper(buf, doc, max_document_size TSRMLS_CC) != 0) {
		return FAILURE;
	}

	return php_mongo_serialize_size(buf->start + start, buf, max_message_size TSRMLS_CC);
}

int php_mongo_write_batch_insert(buffer *buf, char *ns, int flags, zval *docs, int max_document_size, int max_message_size TSRMLS_DC)
{
	int start = buf->pos - buf->start, count = 0;
	HashPosition pointer;
	zval **doc;
	mongo_msg_header header;

	CREATE_HEADER_WITH_OPTS(buf, ns, OP_INSERT, flags);

	for (
		zend_hash_internal_pointer_reset_ex(HASH_P(docs), &pointer);
		zend_hash_get_current_data_ex(HASH_P(docs), (void**)&doc, &pointer) == SUCCESS;
		zend_hash_move_forward_ex(HASH_P(docs), &pointer)
	) {
		if (IS_SCALAR_PP(doc)) {
			continue;
		}

		if (insert_helper(buf, *doc, max_document_size TSRMLS_CC) != 0) {
			/* An exception has already been thrown */
			return FAILURE;
		}

		if (buf->pos - buf->start >= max_message_size) {
			zend_throw_exception_ex(mongo_ce_Exception, 5 TSRMLS_CC, "current batch size is too large: %d, max: %d", buf->pos - buf->start, max_message_size);
			return FAILURE;
		}

		count++;
	}

	/* if there are no elements, don't bother saving */
	if (count == 0) {
		zend_throw_exception_ex(mongo_ce_Exception, 6 TSRMLS_CC, "no documents given");
		return FAILURE;
	}

	/* this is a hard limit in the db server (util/messages.cpp) */
	if (buf->pos - (buf->start + start) > max_message_size) {
		zend_throw_exception_ex(mongo_ce_Exception, 3 TSRMLS_CC, "insert too large: %d, max: %d", buf->pos - (buf->start + start), max_message_size);
		return FAILURE;
	}

	return php_mongo_serialize_size(buf->start + start, buf, max_message_size TSRMLS_CC);
}

int php_mongo_write_update(buffer *buf, char *ns, int flags, zval *criteria, zval *newobj, int max_document_size, int max_message_size TSRMLS_DC)
{
	mongo_msg_header header;
	int start = buf->pos - buf->start;

	CREATE_HEADER(buf, ns, OP_UPDATE);

	php_mongo_serialize_int(buf, flags);

	if (
		zval_to_bson(buf, HASH_P(criteria), NO_PREP, max_document_size TSRMLS_CC) == FAILURE ||
		EG(exception) ||
		zval_to_bson(buf, HASH_P(newobj), NO_PREP, max_document_size TSRMLS_CC) == FAILURE ||
		EG(exception) /* Having this twice does make sense, as zval_to_bson can thrown an exception */
	) {
		return FAILURE;
	}

	return php_mongo_serialize_size(buf->start + start, buf, max_message_size TSRMLS_CC);
}

int php_mongo_write_delete(buffer *buf, char *ns, int flags, zval *criteria, int max_document_size, int max_message_size TSRMLS_DC)
{
	mongo_msg_header header;
	int start = buf->pos - buf->start;

	CREATE_HEADER(buf, ns, OP_DELETE);

	php_mongo_serialize_int(buf, flags);

	if (zval_to_bson(buf, HASH_P(criteria), NO_PREP, max_document_size TSRMLS_CC) == FAILURE || EG(exception)) {
		return FAILURE;
	}

	return php_mongo_serialize_size(buf->start + start, buf, max_message_size TSRMLS_CC);
}

/*
 * Creates a query string in buf.
 *
 * The following fields of cursor are used:
 *  - ns
 *  - opts
 *  - skip
 *  - limit
 *  - query
 *  - fields
 *
 */
int php_mongo_write_query(buffer *buf, mongo_cursor *cursor, int max_document_size, int max_message_size TSRMLS_DC)
{
	mongo_msg_header header;
	int start = buf->pos - buf->start;

	CREATE_HEADER_WITH_OPTS(buf, cursor->ns, OP_QUERY, cursor->opts);
	cursor->send.request_id = header.request_id;

	php_mongo_serialize_int(buf, cursor->skip);
	php_mongo_serialize_int(buf, mongo_get_limit(cursor));

	if (zval_to_bson(buf, HASH_P(cursor->query), NO_PREP, max_document_size TSRMLS_CC) == FAILURE || EG(exception)) {
		return FAILURE;
	}
	if (cursor->fields && zend_hash_num_elements(HASH_P(cursor->fields)) > 0) {
		if (zval_to_bson(buf, HASH_P(cursor->fields), NO_PREP, max_document_size TSRMLS_CC) == FAILURE || EG(exception)) {
			return FAILURE;
		}
	}

	return php_mongo_serialize_size(buf->start + start, buf, max_message_size TSRMLS_CC);
}

int php_mongo_write_kill_cursors(buffer *buf, int64_t cursor_id, int max_message_size TSRMLS_DC)
{
	mongo_msg_header header;

	CREATE_MSG_HEADER(MonGlo(request_id)++, 0, OP_KILL_CURSORS);
	APPEND_HEADER(buf, 0);

	/* # of cursors */
	php_mongo_serialize_int(buf, 1);
	/* cursor ids */
	php_mongo_serialize_long(buf, cursor_id);
	return php_mongo_serialize_size(buf->start, buf, max_message_size TSRMLS_CC);
}

/*
 * Creates a GET_MORE request
 *
 * The following fields of cursor are used:
 *  - ns
 *  - recv.request_id
 *  - limit
 *  - cursor_id
 */
int php_mongo_write_get_more(buffer *buf, mongo_cursor *cursor TSRMLS_DC)
{
	mongo_msg_header header;
	int start = buf->pos - buf->start;

	CREATE_RESPONSE_HEADER(buf, cursor->ns, cursor->recv.request_id, OP_GET_MORE);
	cursor->send.request_id = header.request_id;

	php_mongo_serialize_int(buf, mongo_get_limit(cursor));
	php_mongo_serialize_long(buf, cursor->cursor_id);

	return php_mongo_serialize_size(buf->start + start, buf, cursor->connection->max_message_size TSRMLS_CC);
}


int mongo_get_limit(mongo_cursor *cursor)
{
	int lim_at;

	if (cursor->limit < 0) {
		return cursor->limit;
	} else if (cursor->batch_size < 0) {
		return cursor->batch_size;
	}

	lim_at = cursor->limit > cursor->batch_size ? cursor->limit - cursor->at : cursor->limit;

	if (cursor->batch_size && (!lim_at || cursor->batch_size <= lim_at)) {
		return cursor->batch_size;
	} else if (lim_at && (!cursor->batch_size || lim_at < cursor->batch_size)) {
		return lim_at;
	}

	return 0;
}


char* bson_to_zval(char *buf, HashTable *result TSRMLS_DC)
{
	/* buf_start is used for debugging
	 *
	 * If the deserializer runs into bson it can't parse, it will dump the
	 * bytes to that point.
	 *
	 * We lose buf's position as we iterate, so we need buf_start to save it. */
	char *buf_start = buf;
	unsigned char type;

	if (buf == 0) {
		return 0;
	}

	/* for size */
	buf += INT_32;

	while ((type = *buf++) != 0) {
		char *name;
		zval *value;

		name = buf;
		/* get past field name */
		buf += strlen(buf) + 1;

		MAKE_STD_ZVAL(value);
		ZVAL_NULL(value);

		/* get value */
		switch(type) {
			case BSON_OID: {
				mongo_id *this_id;
				zval *str = 0;

				object_init_ex(value, mongo_ce_Id);

				this_id = (mongo_id*)zend_object_store_get_object(value TSRMLS_CC);
				this_id->id = estrndup(buf, OID_SIZE);

				MAKE_STD_ZVAL(str);
				ZVAL_NULL(str);

				MONGO_METHOD(MongoId, __toString, str, value);
				zend_update_property(mongo_ce_Id, value, "$id", strlen("$id"), str TSRMLS_CC);
				zval_ptr_dtor(&str);

				buf += OID_SIZE;
				break;
			}

			case BSON_DOUBLE: {
				double d = *(double*)buf;
				int64_t i, *i_p;
				i_p = &i;

				memcpy(i_p, &d, DOUBLE_64);
				i = MONGO_64(i);
				memcpy(&d, i_p, DOUBLE_64);

				ZVAL_DOUBLE(value, d);
				buf += DOUBLE_64;
				break;
			}

			case BSON_SYMBOL:
			case BSON_STRING: {
				/* len includes \0 */
				int len = MONGO_32(*((int*)buf));

				if (INVALID_STRING_LEN(len)) {
					zval_ptr_dtor(&value);
					zend_throw_exception_ex(mongo_ce_CursorException, 21 TSRMLS_CC, "invalid string length for key \"%s\": %d", name, len);
					return 0;
				}
				buf += INT_32;

				ZVAL_STRINGL(value, buf, len-1, 1);
				buf += len;
				break;
			}

			case BSON_OBJECT:
			case BSON_ARRAY: {
				array_init(value);
				buf = bson_to_zval(buf, Z_ARRVAL_P(value) TSRMLS_CC);
				if (EG(exception)) {
					zval_ptr_dtor(&value);
					return 0;
				}
				break;
			}

			case BSON_BINARY: {
				unsigned char type;
				int len = MONGO_32(*(int*)buf);

				if (INVALID_STRING_LEN(len)) {
					zval_ptr_dtor(&value);
					zend_throw_exception_ex(mongo_ce_CursorException, 22 TSRMLS_CC, "invalid binary length for key \"%s\": %d", name, len);
					return 0;
				}
				buf += INT_32;

				type = *buf++;

				/* If the type is 2, check if the binary data is prefixed by its
				 * length.
				 *
				 * There is an infinitesimally small chance that the first four
				 * bytes will happen to be the length of the rest of the
				 * string.  In this case, the data will be corrupted. */
				if ((int)type == 2) {
					int len2 = MONGO_32(*(int*)buf);

					/* If the lengths match, the data is to spec, so we use
					 * len2 as the true length. */
					if (len2 == len - 4) {
						len = len2;
						buf += INT_32;
					}
				}

				object_init_ex(value, mongo_ce_BinData);

				zend_update_property_stringl(mongo_ce_BinData, value, "bin", strlen("bin"), buf, len TSRMLS_CC);
				zend_update_property_long(mongo_ce_BinData, value, "type", strlen("type"), type TSRMLS_CC);

				buf += len;
				break;
			}

			case BSON_BOOL: {
				char d = *buf++;

				ZVAL_BOOL(value, d);
				break;
			}

			case BSON_UNDEF:
			case BSON_NULL: {
				ZVAL_NULL(value);
				break;
			}

			case BSON_INT: {
				ZVAL_LONG(value, MONGO_32(*((int*)buf)));
				buf += INT_32;
				break;
			}

			case BSON_LONG: {
				if (MonGlo(long_as_object)) {
					char *buffer;

#ifdef WIN32
					spprintf(&buffer, 0, "%I64d", (int64_t)MONGO_64(*((int64_t*)buf)));
#else
					spprintf(&buffer, 0, "%lld", (long long int)MONGO_64(*((int64_t*)buf)));
#endif
					object_init_ex(value, mongo_ce_Int64);

					zend_update_property_string(mongo_ce_Int64, value, "value", strlen("value"), buffer TSRMLS_CC);

					efree(buffer);
				} else {
					if (MonGlo(native_long)) {
#if SIZEOF_LONG == 4
						zend_throw_exception_ex(mongo_ce_CursorException, 23 TSRMLS_CC, "Can not natively represent the long %llu on this platform", (int64_t)MONGO_64(*((int64_t*)buf)));
						zval_ptr_dtor(&value);
						return 0;
#else
# if SIZEOF_LONG == 8
						ZVAL_LONG(value, (long)MONGO_64(*((int64_t*)buf)));
# else
#  error The PHP number size is neither 4 or 8 bytes; no clue what to do with that!
# endif
#endif
					} else {
						ZVAL_DOUBLE(value, (double)MONGO_64(*((int64_t*)buf)));
					}
				}
				buf += INT_64;
				break;
			}

			case BSON_DATE: {
				int64_t d = MONGO_64(*((int64_t*)buf));

				buf += INT_64;

				object_init_ex(value, mongo_ce_Date);
				php_mongo_date_init(value, d TSRMLS_CC);

				break;
			}

			case BSON_REGEX: {
				char *regex, *flags;
				int regex_len, flags_len;

				regex = buf;
				regex_len = strlen(buf);
				buf += regex_len + 1;

				flags = buf;
				flags_len = strlen(buf);
				buf += flags_len + 1;

				object_init_ex(value, mongo_ce_Regex);

				zend_update_property_stringl(mongo_ce_Regex, value, "regex", strlen("regex"), regex, regex_len TSRMLS_CC);
				zend_update_property_stringl(mongo_ce_Regex, value, "flags", strlen("flags"), flags, flags_len TSRMLS_CC);

				break;
			}

			case BSON_CODE:
			case BSON_CODE__D: {
				zval *zcope;
				int code_len;
				char *code;

				/* CODE has a useless total size field */
				if (type == BSON_CODE) {
					buf += INT_32;
				}

				/* length of code (includes \0) */
				code_len = MONGO_32(*(int*)buf);
				if (INVALID_STRING_LEN(code_len)) {
					zval_ptr_dtor(&value);
					zend_throw_exception_ex(mongo_ce_CursorException, 24 TSRMLS_CC, "invalid code length for key \"%s\": %d", name, code_len);
					return 0;
				}
				buf += INT_32;

				code = buf;
				buf += code_len;

				/* initialize scope array */
				MAKE_STD_ZVAL(zcope);
				array_init(zcope);

				if (type == BSON_CODE) {
					buf = bson_to_zval(buf, HASH_P(zcope) TSRMLS_CC);
					if (EG(exception)) {
						zval_ptr_dtor(&zcope);
						return 0;
					}
				}

				object_init_ex(value, mongo_ce_Code);
				/* exclude \0 */
				zend_update_property_stringl(mongo_ce_Code, value, "code", strlen("code"), code, code_len-1 TSRMLS_CC);
				zend_update_property(mongo_ce_Code, value, "scope", strlen("scope"), zcope TSRMLS_CC);
				zval_ptr_dtor(&zcope);

				break;
			}

			/* DEPRECATED
			 * database reference (12)
			 *   - 4 bytes ns length (includes trailing \0)
			 *   - ns + \0
			 *   - 12 bytes MongoId
			 * This converts the deprecated, old-style db ref type
			 * into the new type (array('$ref' => ..., $id => ...)). */
			case BSON_DBREF: {
				int ns_len;
				char *ns;
				zval *zoid;
				mongo_id *this_id;

				/* ns */
				ns_len = *(int*)buf;
				if (INVALID_STRING_LEN(ns_len)) {
					zval_ptr_dtor(&value);
					zend_throw_exception_ex(mongo_ce_CursorException, 3 TSRMLS_CC, "invalid dbref length for key \"%s\": %d", name, ns_len);
					return 0;
				}
				buf += INT_32;
				ns = buf;
				buf += ns_len;

				/* id */
				MAKE_STD_ZVAL(zoid);
				object_init_ex(zoid, mongo_ce_Id);

				this_id = (mongo_id*)zend_object_store_get_object(zoid TSRMLS_CC);
				this_id->id = estrndup(buf, OID_SIZE);

				buf += OID_SIZE;

				/* put it all together */
				array_init(value);
				add_assoc_stringl(value, "$ref", ns, ns_len-1, 1);
				add_assoc_zval(value, "$id", zoid);
				break;
			}

			/* MongoTimestamp (17)
			 * 8 bytes total:
			 *  - sec: 4 bytes
			 *  - inc: 4 bytes */
			case BSON_TIMESTAMP: {
				object_init_ex(value, mongo_ce_Timestamp);
				zend_update_property_long(mongo_ce_Timestamp, value, "inc", strlen("inc"), MONGO_32(*(int*)buf) TSRMLS_CC);
				buf += INT_32;
				zend_update_property_long(mongo_ce_Timestamp, value, "sec", strlen("sec"), MONGO_32(*(int*)buf) TSRMLS_CC);
				buf += INT_32;
				break;
			}

			/* max and min keys are used only for sharding, and
			 * cannot be resaved to the database at the moment */
			/* MongoMinKey (0) */
			case BSON_MINKEY: {
				object_init_ex(value, mongo_ce_MinKey);
				break;
			}

			/* MongoMinKey (127) */
			case BSON_MAXKEY: {
				object_init_ex(value, mongo_ce_MaxKey);
				break;
			}

			default: {
				/* If we run into a type we don't recognize, there's either been
				 * some corruption or we've messed up on the parsing.  Either way,
				 * it's helpful to know the situation that led us here, so this
				 * dumps the buffer up to this point to stdout and returns.
				 *
				 * We can't dump any more of the buffer, unfortunately, because we
				 * don't keep track of the size.  Besides, if it is corrupt, the
				 * size might be messed up, too. */
				char *msg, *pos, *tmpl;
				int i, width, len;
				unsigned char t = type;

				tmpl = "type 0x00 not supported:";

				/* each byte is " xx" (3 chars) */
				width = 3;
				len = (buf - buf_start) * width;

				msg = (char*)emalloc(strlen(tmpl) + len + 1);
				memcpy(msg, tmpl, strlen(tmpl));
				pos = msg + 7;

				sprintf(pos++, "%x", t / 16);
				t = t % 16;
				sprintf(pos++, "%x", t);
				/* remove '\0' added by sprintf */
				*(pos) = ' ';

				/* jump to end of template */
				pos = msg + strlen(tmpl);
				for (i=0; i<buf-buf_start; i++) {
					sprintf(pos, " %02x", (unsigned char)buf_start[i]);
					pos += width;
				}
				/* sprintf 0-terminates the string */

				zend_throw_exception(mongo_ce_Exception, msg, 17 TSRMLS_CC);
				efree(msg);
				return 0;
			}
		}

		zend_symtable_update(result, name, strlen(name) + 1, &value, sizeof(zval*), NULL);
	}

	return buf;
}

static int is_utf8(const char *s, int len)
{
	int i;

	for (i = 0; i < len; i++) {
		if (i + 3 < len && (s[i] & 248) == 240 && (s[i + 1] & 192) == 128 && (s[i + 2] & 192) == 128 && (s[i + 3] & 192) == 128) {
			i += 3;
		} else if (i + 2 < len && (s[i] & 240) == 224 && (s[i + 1] & 192) == 128 && (s[i + 2] & 192) == 128) {
			i += 2;
		} else if (i + 1 < len && (s[i] & 224) == 192 && (s[i + 1] & 192) == 128) {
			i += 1;
		} else if ((s[i] & 128) != 0) {
			return 0;
		}
	}
	return 1;
}

/* {{{ proto string bson_encode(mixed document)
   Takes any type of PHP var and turns it into BSON */
PHP_FUNCTION(bson_encode)
{
	zval *z;
	buffer buf;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &z) == FAILURE) {
		return;
	}

	switch (Z_TYPE_P(z)) {
		case IS_NULL: {
			RETURN_STRING("", 1);
			break;
		}

		case IS_LONG: {
			CREATE_BUF_STATIC(9);
#if SIZEOF_LONG == 4
			php_mongo_serialize_int(&buf, Z_LVAL_P(z));
			RETURN_STRINGL(buf.start, 4, 1);
#else
			php_mongo_serialize_long(&buf, Z_LVAL_P(z));
			RETURN_STRINGL(buf.start, 8, 1);
#endif
			break;
		}

		case IS_DOUBLE: {
			CREATE_BUF_STATIC(9);
			php_mongo_serialize_double(&buf, Z_DVAL_P(z));
			RETURN_STRINGL(b, 8, 1);
			break;
		}

		case IS_BOOL: {
			if (Z_BVAL_P(z)) {
				RETURN_STRINGL("\x01", 1, 1);
			} else {
				RETURN_STRINGL("\x00", 1, 1);
			}
			break;
		}

		case IS_STRING: {
			RETURN_STRINGL(Z_STRVAL_P(z), Z_STRLEN_P(z), 1);
			break;
		}

		case IS_OBJECT: {
			zend_class_entry *clazz = Z_OBJCE_P(z);

			if (clazz == mongo_ce_Id) {
				mongo_id *id = (mongo_id*)zend_object_store_get_object(z TSRMLS_CC);
				RETURN_STRINGL(id->id, 12, 1);
				break;
			} else if (clazz == mongo_ce_Date) {
				CREATE_BUF_STATIC(9);
				php_mongo_serialize_date(&buf, z TSRMLS_CC);
				RETURN_STRINGL(buf.start, 8, 0);
				break;
			} else if (clazz == mongo_ce_Regex) {
				CREATE_BUF(buf, 128);

				php_mongo_serialize_regex(&buf, z TSRMLS_CC);
				RETVAL_STRINGL(buf.start, buf.pos-buf.start, 1);
				efree(buf.start);
				break;
			} else if (clazz == mongo_ce_Code) {
				CREATE_BUF(buf, INITIAL_BUF_SIZE);

				php_mongo_serialize_code(&buf, z TSRMLS_CC);
				RETVAL_STRINGL(buf.start, buf.pos-buf.start, 1);
				efree(buf.start);
				break;
			} else if (clazz == mongo_ce_BinData) {
				CREATE_BUF(buf, INITIAL_BUF_SIZE);

				php_mongo_serialize_bin_data(&buf, z TSRMLS_CC);
				RETVAL_STRINGL(buf.start, buf.pos-buf.start, 1);
				efree(buf.start);
				break;
			} else if (clazz == mongo_ce_Timestamp) {
				CREATE_BUF(buf, 9);
				buf.pos[8] = (char)0;

				php_mongo_serialize_bin_data(&buf, z TSRMLS_CC);
				RETURN_STRINGL(buf.start, 8, 0);
				break;
			}
		}

		/* fallthrough for a normal obj */
		case IS_ARRAY: {
			CREATE_BUF(buf, INITIAL_BUF_SIZE);
			zval_to_bson(&buf, HASH_P(z), 0, MONGO_DEFAULT_MAX_MESSAGE_SIZE TSRMLS_CC);

			RETVAL_STRINGL(buf.start, buf.pos-buf.start, 1);
			efree(buf.start);
			break;
		}

		default:
			zend_throw_exception(zend_exception_get_default(TSRMLS_C), "couldn't serialize element", 0 TSRMLS_CC);
			return;
	}
}
/* }}} */

/* {{{ proto array bson_decode(string bson)
   Takes a serialized BSON object and turns it into a PHP array. This only deserializes entire documents! */
PHP_FUNCTION(bson_decode)
{
	char *str;
	int str_len;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s", &str, &str_len) == FAILURE) {
		return;
	}

	array_init(return_value);
	bson_to_zval(str, HASH_P(return_value) TSRMLS_CC);
}
/* }}} */

void mongo_buf_init(char *dest)
{
	dest[0] = '\0';
}

void mongo_buf_append(char *dest, char *piece)
{
	int pos = strlen(dest);
	memcpy(dest + pos, piece, strlen(piece) + 1);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
