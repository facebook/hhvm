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
#ifndef __TYPES_ID_H__
#define __TYPES_ID_H__

int php_mongo_id_serialize(zval*, unsigned char**, zend_uint*, zend_serialize_data* TSRMLS_DC);
int php_mongo_id_unserialize(zval**, zend_class_entry*, const unsigned char*, zend_uint, zend_unserialize_data* TSRMLS_DC);
int php_mongo_compare_ids(zval*, zval* TSRMLS_DC);

PHP_METHOD(MongoId, __construct);
PHP_METHOD(MongoId, __toString);
PHP_METHOD(MongoId, __set_state);
PHP_METHOD(MongoId, getTimestamp);
PHP_METHOD(MongoId, getPID);
PHP_METHOD(MongoId, getInc);
PHP_METHOD(MongoId, getHostname);

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
