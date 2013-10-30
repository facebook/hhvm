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

#include "../php_mongo.h"
#include "../collection.h"
#include "../cursor.h"
#include "gridfs.h"
#include "gridfs_stream.h"

extern zend_class_entry *mongo_ce_BinData;
extern zend_class_entry *mongo_ce_GridFS;
extern zend_class_entry *mongo_ce_GridFSException;
extern zend_class_entry *mongo_ce_GridFSFile;
extern zend_class_entry *mongo_ce_Int32;
extern zend_class_entry *mongo_ce_Int64;

zend_class_entry *mongo_ce_GridFSFile = NULL;

typedef int (*apply_copy_func_t)(void *to, char *from, int len);
static int apply_to_cursor(zval *cursor, apply_copy_func_t apply_copy_func, void *to, int max TSRMLS_DC);
static int copy_bytes(void *to, char *from, int len);
static int copy_file(void *to, char *from, int len);

/* {{{ proto MongoGridFSFile::__construct(MongoGridFS gridfs, array file)
   Creates a new MongoGridFSFile object */
PHP_METHOD(MongoGridFSFile, __construct)
{
	zval *gridfs = 0, *file = 0;
	long flags = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Oa|l", &gridfs, mongo_ce_GridFS, &file, &flags) == FAILURE) {
		zval *object = getThis();

		ZVAL_NULL(object);
		return;
	}

	zend_update_property(mongo_ce_GridFSFile, getThis(), "gridfs", strlen("gridfs"), gridfs TSRMLS_CC);
	zend_update_property(mongo_ce_GridFSFile, getThis(), "file", strlen("file"), file TSRMLS_CC);
	zend_update_property_long(mongo_ce_GridFSFile, getThis(), "flags", strlen("flags"), flags TSRMLS_CC);
}
/* }}} */

/* {{{ proto string MongoGridFSFile::getFilename()
   Returns this file's filename */
PHP_METHOD(MongoGridFSFile, getFilename)
{
	zval *file = zend_read_property(mongo_ce_GridFSFile, getThis(), "file", strlen("file"), NOISY TSRMLS_CC);

	if (zend_hash_find(HASH_P(file), "filename", strlen("filename") + 1, (void**)&return_value_ptr) == SUCCESS) {
		RETURN_ZVAL(*return_value_ptr, 1, 0);
	}
	RETURN_NULL();
}
/* }}} */

/* {{{ proto int MongoGridFSFile::getSize()
   Returns this file's size */
PHP_METHOD(MongoGridFSFile, getSize)
{
	zval *file = zend_read_property(mongo_ce_GridFSFile, getThis(), "file", strlen("file"), NOISY TSRMLS_CC);

	if (zend_hash_find(HASH_P(file), "length", strlen("length") + 1, (void**)&return_value_ptr) == SUCCESS) {
		RETURN_ZVAL(*return_value_ptr, 1, 0);
	}
	RETURN_NULL();
}
/* }}} */

/* {{{ proto int MongoGridFSFile::write([string filename = null])
   Writes this file to the filesystem */
PHP_METHOD(MongoGridFSFile, write)
{
	char *filename = 0;
	int filename_len, total = 0;
	zval *gridfs, *file, *chunks, *query, *cursor, *sort, tmp;
	zval **id, **size;
	int len;
	FILE *fp;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|s", &filename, &filename_len) == FAILURE) {
		return;
	}

	gridfs = zend_read_property(mongo_ce_GridFSFile, getThis(), "gridfs", strlen("gridfs"), NOISY TSRMLS_CC);
	file = zend_read_property(mongo_ce_GridFSFile, getThis(), "file", strlen("file"), NOISY TSRMLS_CC);

	if (zend_hash_find(HASH_P(file), "length", strlen("length") + 1, (void**)&size) == FAILURE) {
		zend_throw_exception(mongo_ce_GridFSException, "couldn't find file size", 14 TSRMLS_CC);
		return;
	}

	if (Z_TYPE_PP(size) == IS_DOUBLE) {
		len = (int)Z_DVAL_PP(size);
	} else if (Z_TYPE_PP(size) == IS_LONG) {
		len = Z_LVAL_PP(size);
	} else if (Z_TYPE_PP(size) == IS_OBJECT && (Z_OBJCE_PP(size) == mongo_ce_Int32 || Z_OBJCE_PP(size) == mongo_ce_Int64)) {
		zval *sizet = zend_read_property(mongo_ce_Int64, *size, "value", strlen("value"), NOISY TSRMLS_CC);
		if (Z_TYPE_P(sizet) != IS_STRING) {
			zval_ptr_dtor(&cursor);
			zend_throw_exception(mongo_ce_GridFSException, "couldn't find file size, value object broken", 0 TSRMLS_CC);
			return;
		}
		len = atoi(Z_STRVAL_P(sizet));
	} else {
		zval_ptr_dtor(&cursor);
		zend_throw_exception(mongo_ce_GridFSException, "couldn't find file size, property invalid", 0 TSRMLS_CC);
		return;
	}

	/* Make sure that there's an index on chunks so we can sort by chunk num */
	chunks = zend_read_property(mongo_ce_GridFS, gridfs, "chunks", strlen("chunks"), NOISY TSRMLS_CC);

	php_mongo_ensure_gridfs_index(&tmp, chunks TSRMLS_CC);
	zval_dtor(&tmp);

	if (!filename) {
		zval **temp;
		if (zend_hash_find(HASH_P(file), "filename", strlen("filename") + 1, (void**) &temp) == SUCCESS) {
			convert_to_string_ex(temp);
			filename = Z_STRVAL_PP(temp);
		} else {
			zend_throw_exception(mongo_ce_GridFSException, "Cannot find filename", 15 TSRMLS_CC);
			return;
		}
	}

	fp = fopen(filename, "wb");
	if (!fp) {
		zend_throw_exception_ex(mongo_ce_GridFSException, 16 TSRMLS_CC, "could not open destination file %s", filename);
		return;
	}

	zend_hash_find(HASH_P(file), "_id", strlen("_id") + 1, (void**)&id);

	MAKE_STD_ZVAL(query);
	array_init(query);
	zval_add_ref(id);
	add_assoc_zval(query, "files_id", *id);

	MAKE_STD_ZVAL(cursor);
	MONGO_METHOD1(MongoCollection, find, cursor, chunks, query);

	MAKE_STD_ZVAL(sort);
	array_init(sort);
	add_assoc_long(sort, "n", 1);

	MONGO_METHOD1(MongoCursor, sort, cursor, cursor, sort);

	if ((total = apply_to_cursor(cursor, copy_file, fp, len TSRMLS_CC)) == FAILURE) {
		zend_throw_exception(mongo_ce_GridFSException, "error reading chunk of file", 17 TSRMLS_CC);
	}

	fclose(fp);

	zval_ptr_dtor(&cursor);
	zval_ptr_dtor(&sort);
	zval_ptr_dtor(&query);

	RETURN_LONG(total);
}
/* }}} */

/* {{{ proto stream MongoGridFSFile::getResource()
   Returns a resource that can be used to read the stored file */
PHP_METHOD(MongoGridFSFile, getResource)
{
	php_stream * stream;

	stream = gridfs_stream_init(getThis() TSRMLS_CC);
	if (!stream) {
		zend_throw_exception(mongo_ce_GridFSException, "couldn't create a php_stream", 18 TSRMLS_CC);
		return;
	}

	php_stream_to_zval(stream, return_value);
}
/* }}} */

/* {{{ proto string MongoGridFSFile::getBytes()
   Returns this file's contents as a string of bytes */
PHP_METHOD(MongoGridFSFile, getBytes)
{
	zval *file, *gridfs, *chunks, *query, *cursor, *sort, *temp;
	zval **id, **size;
	char *str, *str_ptr;
	int len;
	mongo_cursor *cursorobj;
	zval *flags;

	file = zend_read_property(mongo_ce_GridFSFile, getThis(), "file", strlen("file"), NOISY TSRMLS_CC);
	zend_hash_find(HASH_P(file), "_id", strlen("_id") + 1, (void**)&id);

	if (zend_hash_find(HASH_P(file), "length", strlen("length") + 1, (void**)&size) == FAILURE) {
		zend_throw_exception(mongo_ce_GridFSException, "couldn't find file size", 14 TSRMLS_CC);
		return;
	}

	/* make sure that there's an index on chunks so we can sort by chunk num */
	gridfs = zend_read_property(mongo_ce_GridFSFile, getThis(), "gridfs", strlen("gridfs"), NOISY TSRMLS_CC);
	chunks = zend_read_property(mongo_ce_GridFS, gridfs, "chunks", strlen("chunks"), NOISY TSRMLS_CC);

	MAKE_STD_ZVAL(temp);
	php_mongo_ensure_gridfs_index(temp, chunks TSRMLS_CC);
	zval_dtor(temp);

	/* query for chunks */
	MAKE_STD_ZVAL(query);
	array_init(query);
	zval_add_ref(id);
	add_assoc_zval(query, "files_id", *id);

	MAKE_STD_ZVAL(cursor);
	MONGO_METHOD1(MongoCollection, find, cursor, chunks, query);

	/* Copy the flags from the original cursor and apply it to this one */
	flags = zend_read_property(mongo_ce_GridFSFile, getThis(), "flags", strlen("flags"), NOISY TSRMLS_CC);
	cursorobj = (mongo_cursor*)zend_object_store_get_object(cursor TSRMLS_CC);
	convert_to_long(flags);
	cursorobj->opts = Z_LVAL_P(flags);

	MAKE_STD_ZVAL(sort);
	array_init(sort);
	add_assoc_long(sort, "n", 1);

	MONGO_METHOD1(MongoCursor, sort, temp, cursor, sort);
	zval_ptr_dtor(&temp);

	zval_ptr_dtor(&query);
	zval_ptr_dtor(&sort);

	if (Z_TYPE_PP(size) == IS_DOUBLE) {
		len = (int)Z_DVAL_PP(size);
	} else if (Z_TYPE_PP(size) == IS_LONG) {
		len = Z_LVAL_PP(size);
	} else if (Z_TYPE_PP(size) == IS_OBJECT && (Z_OBJCE_PP(size) == mongo_ce_Int32 || Z_OBJCE_PP(size) == mongo_ce_Int64)) {
		zval *sizet = zend_read_property(mongo_ce_Int64, *size, "value", strlen("value"), NOISY TSRMLS_CC);
		if (Z_TYPE_P(sizet) != IS_STRING) {
			zval_ptr_dtor(&cursor);
			zend_throw_exception(mongo_ce_GridFSException, "couldn't find file size, value object broken", 0 TSRMLS_CC);
			return;
		}
		len = atoi(Z_STRVAL_P(sizet));
	} else {
		zval_ptr_dtor(&cursor);
		zend_throw_exception(mongo_ce_GridFSException, "couldn't find file size, property invalid", 0 TSRMLS_CC);
		return;
	}

	str = (char *)ecalloc(len + 1, 1);
	str_ptr = str;

	if (apply_to_cursor(cursor, copy_bytes, &str, len + 1 TSRMLS_CC) == FAILURE) {
		zval_ptr_dtor(&cursor);
		efree(str_ptr);

		if (EG(exception)) {
			return;
		}
		zend_throw_exception(mongo_ce_GridFSException, "error reading chunk of file", 17 TSRMLS_CC);
		return;
	}

	zval_ptr_dtor(&cursor);

	str_ptr[len] = '\0';

	RETURN_STRINGL(str_ptr, len, 0);
}
/* }}} */

static int copy_bytes(void *to, char *from, int len)
{
	char *winIsDumb = *(char**)to;
	memcpy(winIsDumb, from, len);
	winIsDumb += len;
	*((char**)to) = (char*)winIsDumb;

	return len;
}

static int copy_file(void *to, char *from, int len)
{
	int written = fwrite(from, 1, len, (FILE*)to);

	if (written != len) {
		zend_error(E_WARNING, "Incorrect byte count. Expected: %d, got %d", len, written);
	}

	return written;
}

static int apply_to_cursor(zval *cursor, apply_copy_func_t apply_copy_func, void *to, int max TSRMLS_DC)
{
	int total = 0;
	zval *next;

	MAKE_STD_ZVAL(next);
	MONGO_METHOD(MongoCursor, getNext, next, cursor);

	if (EG(exception)) {
		return FAILURE;
	}

	if (Z_TYPE_P(next) != IS_ARRAY) {
		zval_ptr_dtor(&next);
		return FAILURE;
	}

	while (Z_TYPE_P(next) == IS_ARRAY) {
		zval **zdata;

		/* Check if data field exists. If it doesn't, we've probably got an
		 * error message from the db, so return that */
		if (zend_hash_find(HASH_P(next), "data", 5, (void**)&zdata) == FAILURE) {
			if (zend_hash_exists(HASH_P(next), "$err", 5)) {
				zval_ptr_dtor(&next);
				return FAILURE;
			}
			continue;
		}

		/* This copies the next chunk -> *to
		 * Due to a talent I have for not reading directions, older versions of
		 * the driver store files as raw bytes, not MongoBinData. So, we'll
		 * check for and handle both cases. */
		if (Z_TYPE_PP(zdata) == IS_STRING) {
			/* raw bytes */
			if (total + Z_STRLEN_PP(zdata) > max) {
				zend_throw_exception_ex(mongo_ce_GridFSException, 1 TSRMLS_CC, "There is more data associated with this file than the metadata specifies");
				return FAILURE;
			}
			total += apply_copy_func(to, Z_STRVAL_PP(zdata), Z_STRLEN_PP(zdata));

		} else if (Z_TYPE_PP(zdata) == IS_OBJECT && Z_OBJCE_PP(zdata) == mongo_ce_BinData) {
			/* MongoBinData */
			zval *bin = zend_read_property(mongo_ce_BinData, *zdata, "bin", strlen("bin"), NOISY TSRMLS_CC);

			if (total + Z_STRLEN_P(bin) > max) {
				zval **n;

				if (zend_hash_find(HASH_P(next), "n", strlen("n") + 1, (void**)&n) == SUCCESS) {
					convert_to_long_ex(n);
					zend_throw_exception_ex(mongo_ce_GridFSException, 1 TSRMLS_CC, "There is more data associated with this file than the metadata specifies (reading chunk %d)", Z_LVAL_PP(n));
				} else {
					zend_throw_exception_ex(mongo_ce_GridFSException, 1 TSRMLS_CC, "There is more data associated with this file than the metadata specifies");
				}
				zval_ptr_dtor(&next);

				return FAILURE;
			}
			total += apply_copy_func(to, Z_STRVAL_P(bin), Z_STRLEN_P(bin));

		} else {
			/* If it's not a string or a MongoBinData, give up */
			zval_ptr_dtor(&next);
			return FAILURE;
		}

		/* get ready for the next iteration */
		zval_ptr_dtor(&next);
		MAKE_STD_ZVAL(next);
		MONGO_METHOD(MongoCursor, getNext, next, cursor);
	}
	zval_ptr_dtor(&next);

	/* return the number of bytes copied */
	return total;
}

static zend_function_entry MongoGridFSFile_methods[] = {
	PHP_ME(MongoGridFSFile, __construct, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(MongoGridFSFile, getFilename, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(MongoGridFSFile, getSize, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(MongoGridFSFile, write, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(MongoGridFSFile, getBytes, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(MongoGridFSFile, getResource, NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

void mongo_init_MongoGridFSFile(TSRMLS_D)
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "MongoGridFSFile", MongoGridFSFile_methods);
	mongo_ce_GridFSFile = zend_register_internal_class(&ce TSRMLS_CC);

	zend_declare_property_null(mongo_ce_GridFSFile, "file", strlen("file"), ZEND_ACC_PUBLIC TSRMLS_CC);

	zend_declare_property_null(mongo_ce_GridFSFile, "gridfs", strlen("gridfs"), ZEND_ACC_PROTECTED TSRMLS_CC);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
