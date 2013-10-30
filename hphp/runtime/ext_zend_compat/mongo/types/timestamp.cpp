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

ZEND_EXTERN_MODULE_GLOBALS(mongo)

zend_class_entry *mongo_ce_Timestamp = NULL;

/* Timestamp is 4 bytes of seconds since epoch and 4 bytes of increment. */
PHP_METHOD(MongoTimestamp, __construct)
{
	long sec = 0, inc = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "|ll", &sec, &inc) == FAILURE) {
		return;
	}

	if (ZEND_NUM_ARGS() == 0) {
		sec = time(0);
	}
	if (ZEND_NUM_ARGS() <= 1 && !inc) {
		inc = MonGlo(ts_inc)++;
	}

	zend_update_property_long(mongo_ce_Timestamp, getThis(), "sec", strlen("sec"), sec TSRMLS_CC);
	zend_update_property_long(mongo_ce_Timestamp, getThis(), "inc", strlen("inc"), inc TSRMLS_CC);
}

/* Just convert the seconds field to a string. */
PHP_METHOD(MongoTimestamp, __toString)
{
	char *str;
	zval *sec = zend_read_property(mongo_ce_Timestamp, getThis(), "sec", strlen("sec"), NOISY TSRMLS_CC);

	spprintf(&str, 0, "%ld", Z_LVAL_P(sec));
	RETURN_STRING(str, 0);
}

static zend_function_entry MongoTimestamp_methods[] = {
	PHP_ME(MongoTimestamp, __construct, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(MongoTimestamp, __toString, NULL, ZEND_ACC_PUBLIC)
	{ NULL, NULL, NULL }
};

void mongo_init_MongoTimestamp(TSRMLS_D)
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "MongoTimestamp", MongoTimestamp_methods);
	mongo_ce_Timestamp = zend_register_internal_class(&ce TSRMLS_CC);

	zend_declare_property_long(mongo_ce_Timestamp, "sec", strlen("sec"), 0, ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_long(mongo_ce_Timestamp, "inc", strlen("inc"), 0, ZEND_ACC_PUBLIC TSRMLS_CC);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
