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
#include "cursor_exception.h"

extern zend_class_entry *mongo_ce_Exception;

zend_class_entry *mongo_ce_CursorException;

static zend_function_entry MongoCursorException_methods[] = {
	PHP_ME(MongoCursorException, getHost, NULL, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

PHP_METHOD(MongoCursorException, getHost)
{
	zval *h;

	h = zend_read_property(mongo_ce_CursorException, getThis(), "host", strlen("host"), NOISY TSRMLS_CC);

	RETURN_ZVAL(h, 1, 0);
}
/* }}} */

void mongo_init_MongoCursorException(TSRMLS_D)
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "MongoCursorException", MongoCursorException_methods);
	mongo_ce_CursorException = zend_register_internal_class_ex(&ce, mongo_ce_Exception, NULL TSRMLS_CC);

	zend_declare_property_null(mongo_ce_CursorException, "host", strlen("host"), ZEND_ACC_PRIVATE TSRMLS_CC);
	zend_declare_property_long(mongo_ce_CursorException, "fd", strlen("fd"), 0, ZEND_ACC_PRIVATE TSRMLS_CC);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
