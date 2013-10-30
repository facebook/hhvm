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
#include "../cursor.h"
#include "gridfs_cursor.h"
#include "gridfs_file.h"

extern zend_class_entry *mongo_ce_Cursor;
extern zend_class_entry *mongo_ce_Exception;
extern zend_class_entry *mongo_ce_GridFS;
extern zend_class_entry *mongo_ce_GridFSCursor;
extern zend_class_entry *mongo_ce_GridFSFile;

zend_class_entry *mongo_ce_GridFSCursor = NULL;

/* {{{ proto MongoGridFSCursor::__construct(MongoGridFS gridfs, resource connection, string ns, array query, array fields)
   Creates a new MongoGridFSCursor object */
PHP_METHOD(MongoGridFSCursor, __construct)
{
	zval temp;
	zval *gridfs = 0, *connection = 0, *ns = 0, *query = 0, *fields = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "Ozzzz", &gridfs, mongo_ce_GridFS, &connection, &ns, &query, &fields) == FAILURE) {
		zval *object = getThis();
		ZVAL_NULL(object);
		return;
	}

	zend_update_property(mongo_ce_GridFSCursor, getThis(), "gridfs", strlen("gridfs"), gridfs TSRMLS_CC);

	MONGO_METHOD4(MongoCursor, __construct, &temp, getThis(), connection, ns, query, fields);
}
/* }}} */

/* {{{ proto MongoGridFSFile MongoGridFSCursor::getNext()
   Return the next file to which this cursor points, and advance the cursor */
PHP_METHOD(MongoGridFSCursor, getNext)
{
	MONGO_METHOD(MongoCursor, next, return_value, getThis());
	MONGO_METHOD(MongoGridFSCursor, current, return_value, getThis());
}
/* }}} */

/* {{{ proto MongoGridFSFile MongoGridFSCursor::current()
   Returns the current file */
PHP_METHOD(MongoGridFSCursor, current)
{
	zval temp;
	zval *gridfs;
	zval *flags;
	mongo_cursor *cursor = (mongo_cursor*)zend_object_store_get_object(getThis() TSRMLS_CC);
	MONGO_CHECK_INITIALIZED(cursor->resource, MongoGridFSCursor);

	if (!cursor->current) {
		RETURN_NULL();
	}

	MAKE_STD_ZVAL(flags);
	ZVAL_LONG(flags, cursor->opts);

	object_init_ex(return_value, mongo_ce_GridFSFile);

	gridfs = zend_read_property(mongo_ce_GridFSCursor, getThis(), "gridfs", strlen("gridfs"), NOISY TSRMLS_CC);

	MONGO_METHOD3(MongoGridFSFile, __construct, &temp, return_value, gridfs, cursor->current, flags);
	zval_ptr_dtor(&flags);
}
/* }}} */

static zend_function_entry MongoGridFSCursor_methods[] = {
	PHP_ME(MongoGridFSCursor, __construct, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(MongoGridFSCursor, getNext, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(MongoGridFSCursor, current, NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

void mongo_init_MongoGridFSCursor(TSRMLS_D)
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "MongoGridFSCursor", MongoGridFSCursor_methods);
	mongo_ce_GridFSCursor = zend_register_internal_class_ex(&ce, mongo_ce_Cursor, "MongoCursor" TSRMLS_CC);

	zend_declare_property_null(mongo_ce_GridFSCursor, "gridfs", strlen("gridfs"), ZEND_ACC_PROTECTED TSRMLS_CC);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
