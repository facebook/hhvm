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

zend_class_entry *mongo_ce_Code = NULL;

/* {{{ MongoCode::__construct(string)
 */
PHP_METHOD(MongoCode, __construct)
{
	char *code;
	int code_len;
	zval *zcope = 0;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|a", &code, &code_len, &zcope) == FAILURE) {
		return;
	}

	zend_update_property_stringl(mongo_ce_Code, getThis(), "code", strlen("code"), code, code_len TSRMLS_CC);

	if (!zcope) {
		MAKE_STD_ZVAL(zcope);
		array_init(zcope);
	} else {
		zval_add_ref(&zcope);
	}

	zend_update_property(mongo_ce_Code, getThis(), "scope", strlen("scope"), zcope TSRMLS_CC);

	/* get rid of extra ref */
	zval_ptr_dtor(&zcope);
}
/* }}} */


/* {{{ MongoCode::__toString()
 */
PHP_METHOD(MongoCode, __toString)
{
	zval *zode = zend_read_property(mongo_ce_Code, getThis(), "code", strlen("code"), NOISY TSRMLS_CC);
	convert_to_string_ex(&zode);

	RETURN_STRING(Z_STRVAL_P(zode), 1);
}
/* }}} */


static zend_function_entry MongoCode_methods[] = {
	PHP_ME(MongoCode, __construct, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(MongoCode, __toString, NULL, ZEND_ACC_PUBLIC)
	{ NULL, NULL, NULL }
};

void mongo_init_MongoCode(TSRMLS_D)
{
	zend_class_entry ce;
	INIT_CLASS_ENTRY(ce, "MongoCode", MongoCode_methods);
	mongo_ce_Code = zend_register_internal_class(&ce TSRMLS_CC);

	zend_declare_property_string(mongo_ce_Code, "code", strlen("code"), "", ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_null(mongo_ce_Code, "scope", strlen("scope"), ZEND_ACC_PUBLIC TSRMLS_CC);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
