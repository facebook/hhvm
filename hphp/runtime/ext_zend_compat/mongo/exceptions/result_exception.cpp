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
#include "result_exception.h"

extern zend_class_entry *mongo_ce_Exception;

zend_class_entry *mongo_ce_ResultException;

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_getdocument, 0, 0, 0)
ZEND_END_ARG_INFO()

static zend_function_entry MongoResultException_methods[] = {
	PHP_ME(MongoResultException, getDocument, arginfo_getdocument, ZEND_ACC_PUBLIC)
	{NULL, NULL, NULL}
};

/* {{{ proto array MongoResultException::getDocument(void)
 * Returns the full result document from mongodb */
PHP_METHOD(MongoResultException, getDocument)
{
	zval *h;

	h = zend_read_property(mongo_ce_ResultException, getThis(), "document", strlen("document"), NOISY TSRMLS_CC);

	RETURN_ZVAL(h, 1, 0);
}
/* }}} */

void mongo_init_MongoResultException(TSRMLS_D)
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "MongoResultException", MongoResultException_methods);
	mongo_ce_ResultException = zend_register_internal_class_ex(&ce, mongo_ce_Exception, NULL TSRMLS_CC);

	zend_declare_property_null(mongo_ce_ResultException, "document", strlen("document"), ZEND_ACC_PUBLIC TSRMLS_CC);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
