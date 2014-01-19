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

extern zend_class_entry *mongo_ce_Exception;

zend_class_entry *mongo_ce_Regex = NULL;

/* {{{ MongoRegex::__construct()
 */
PHP_METHOD(MongoRegex, __construct)
{
	zval *regex;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "z", &regex) == FAILURE) {
		return;
	}

	if (Z_TYPE_P(regex) == IS_OBJECT && Z_OBJCE_P(regex) == mongo_ce_Regex) {
		zval *oregex, *oflags;

		oregex = zend_read_property(mongo_ce_Regex, regex, "regex", strlen("regex"), NOISY TSRMLS_CC);
		zend_update_property(mongo_ce_Regex, getThis(), "regex", strlen("regex"), oregex TSRMLS_CC);

		oflags = zend_read_property(mongo_ce_Regex, regex, "flags", strlen("flags"), NOISY TSRMLS_CC);
		zend_update_property(mongo_ce_Regex, getThis(), "flags", strlen("flags"), oflags TSRMLS_CC);

	} else if (Z_TYPE_P(regex) == IS_STRING) {
		int pattern_len, flags_len;
		char *re = Z_STRVAL_P(regex);
		char *eopattern = strrchr(re, '/');

		if (!eopattern) {
			zend_throw_exception(mongo_ce_Exception, "invalid regex", 9 TSRMLS_CC);
			return;
		}

		pattern_len = eopattern - re - 1;

		if (pattern_len < 0) {
			zend_throw_exception(mongo_ce_Exception, "invalid regex", 9 TSRMLS_CC);
			return;
		}

		/* move beyond the second '/' in /foo/bar */
		eopattern++;
		flags_len = Z_STRLEN_P(regex) - (eopattern - re);

		zend_update_property_stringl(mongo_ce_Regex, getThis(), "regex", strlen("regex"), re + 1, pattern_len TSRMLS_CC);
		zend_update_property_stringl(mongo_ce_Regex, getThis(), "flags", strlen("flags"), eopattern, flags_len TSRMLS_CC);
	}
}
/* }}} */


/* {{{ MongoRegex::__toString()
 */
PHP_METHOD(MongoRegex, __toString)
{
	char *field_name;
	zval *zre = zend_read_property(mongo_ce_Regex, getThis(), "regex", strlen("regex"), NOISY TSRMLS_CC);
	zval *zopts = zend_read_property(mongo_ce_Regex, getThis(), "flags", strlen("flags"), NOISY TSRMLS_CC);
	char *re = Z_STRVAL_P(zre);
	char *opts = Z_STRVAL_P(zopts);

	spprintf(&field_name, 0, "/%s/%s", re, opts);
	RETVAL_STRING(field_name, 0);
}
/* }}} */


static zend_function_entry MongoRegex_methods[] = {
	PHP_ME(MongoRegex, __construct, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(MongoRegex, __toString, NULL, ZEND_ACC_PUBLIC)
	{ NULL, NULL, NULL }
};

void mongo_init_MongoRegex(TSRMLS_D)
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "MongoRegex", MongoRegex_methods);
	mongo_ce_Regex = zend_register_internal_class(&ce TSRMLS_CC);

	zend_declare_property_string(mongo_ce_Regex, "regex", strlen("regex"), "", ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_string(mongo_ce_Regex, "flags", strlen("flags"), "", ZEND_ACC_PUBLIC TSRMLS_CC);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
