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

#include "../php_mongo.h"
#include "pool.h"

zend_class_entry *mongo_ce_Pool;

MONGO_ARGINFO_STATIC ZEND_BEGIN_ARG_INFO_EX(arginfo_setPoolSize, 0, ZEND_RETURN_VALUE, 1)
	ZEND_ARG_INFO(0, size)
ZEND_END_ARG_INFO()

static zend_function_entry MongoPool_methods[] = {
	PHP_ME(MongoPool, info, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_DEPRECATED)
	PHP_ME(MongoPool, setSize, arginfo_setPoolSize, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_DEPRECATED)
	PHP_ME(MongoPool, getSize, NULL, ZEND_ACC_PUBLIC|ZEND_ACC_STATIC|ZEND_ACC_DEPRECATED)
	{NULL, NULL, NULL}
};

void mongo_init_MongoPool(TSRMLS_D) {
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "MongoPool", MongoPool_methods);
	mongo_ce_Pool = zend_register_internal_class(&ce TSRMLS_CC);
}

PHP_METHOD(MongoPool, setSize) {
	RETURN_LONG(1);
}

PHP_METHOD(MongoPool, getSize) {
	RETURN_LONG(1);
}

PHP_METHOD(MongoPool, info) {
	array_init(return_value);
}

PHP_METHOD(Mongo, setPoolSize) {
	RETURN_LONG(1);
}

PHP_METHOD(Mongo, getPoolSize) {
	RETURN_LONG(1);
}

PHP_METHOD(Mongo, poolDebug) {
	array_init(return_value);
}


/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
