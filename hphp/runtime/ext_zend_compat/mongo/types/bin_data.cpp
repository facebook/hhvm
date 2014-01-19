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

zend_class_entry *mongo_ce_BinData = NULL;

/* {{{ MongoBinData::__construct()
 */
PHP_METHOD(MongoBinData, __construct)
{
	char *bin;
	long bin_len, type = 2;

	if (zend_parse_parameters(ZEND_NUM_ARGS() TSRMLS_CC, "s|l", &bin, &bin_len, &type) == FAILURE) {
		return;
	}

	if (ZEND_NUM_ARGS() == 1) {
		php_error_docref(NULL TSRMLS_CC, MONGO_E_DEPRECATED, "The default value for type will change to 0 in the future. Please pass in '2' explicitly.");
	}

	zend_update_property_stringl(mongo_ce_BinData, getThis(), "bin", strlen("bin"), bin, bin_len TSRMLS_CC);
	zend_update_property_long(mongo_ce_BinData, getThis(), "type", strlen("type"), type TSRMLS_CC);
}
/* }}} */


/* {{{ MongoBinData::__toString()
 */
PHP_METHOD(MongoBinData, __toString)
{
	RETURN_STRING( "<Mongo Binary Data>", 1 );
}
/* }}} */


static zend_function_entry MongoBinData_methods[] = {
	PHP_ME(MongoBinData, __construct, NULL, ZEND_ACC_PUBLIC)
	PHP_ME(MongoBinData, __toString, NULL, ZEND_ACC_PUBLIC)
	{ NULL, NULL, NULL }
};

void mongo_init_MongoBinData(TSRMLS_D)
{
	zend_class_entry ce;

	INIT_CLASS_ENTRY(ce, "MongoBinData", MongoBinData_methods);
	mongo_ce_BinData = zend_register_internal_class(&ce TSRMLS_CC);

	/* fields */
	zend_declare_property_string(mongo_ce_BinData, "bin", strlen("bin"), "", ZEND_ACC_PUBLIC TSRMLS_CC);
	zend_declare_property_long(mongo_ce_BinData, "type", strlen("type"), 0, ZEND_ACC_PUBLIC TSRMLS_CC);

	/* constants */
	/* can't use FUNCTION because it's a reserved word */
	zend_declare_class_constant_long(mongo_ce_BinData, "FUNC", strlen("FUNC"), 0x01 TSRMLS_CC);
	/* can't use ARRAY because it's a reserved word */
	zend_declare_class_constant_long(mongo_ce_BinData, "BYTE_ARRAY", strlen("BYTE_ARRAY"), 0x02 TSRMLS_CC);
	zend_declare_class_constant_long(mongo_ce_BinData, "UUID", strlen("UUID"), 0x03 TSRMLS_CC);
	zend_declare_class_constant_long(mongo_ce_BinData, "MD5", strlen("MD5"), 0x05 TSRMLS_CC);
	zend_declare_class_constant_long(mongo_ce_BinData, "CUSTOM", strlen("CUSTOM"), 0x80 TSRMLS_CC);
}

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
