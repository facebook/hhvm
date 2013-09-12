/**
 *  Copyright 2009-2013210gen, Inc.
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
#ifndef __MONGOCLIENT_H__
#define __MONGOCLIENT_H__

int php_mongo_create_le(mongo_cursor *cursor, char *name TSRMLS_DC);
zend_object_value php_mongoclient_new(zend_class_entry *class_type TSRMLS_DC);
void mongo_init_MongoClient(TSRMLS_D);
void php_mongo_ctor(INTERNAL_FUNCTION_PARAMETERS, int bc);

/* Helper for connecting the servers */
mongo_connection *php_mongo_connect(mongoclient *link, int flags TSRMLS_DC);
HashTable *mongo_get_debug_info(zval *object, int *is_temp TSRMLS_DC);

#if PHP_VERSION_ID >= 50400
zval *mongo_read_property(zval *object, zval *member, int type, const zend_literal *key TSRMLS_DC);
#else
zval *mongo_read_property(zval *object, zval *member, int type TSRMLS_DC);
#endif

/* MongoClient class */
PHP_METHOD(MongoClient, __construct);
PHP_METHOD(MongoClient, getConnections);
PHP_METHOD(MongoClient, connect);
PHP_METHOD(MongoClient, pairConnect);
PHP_METHOD(MongoClient, persistConnect);
PHP_METHOD(MongoClient, pairPersistConnect);
PHP_METHOD(MongoClient, __toString);
PHP_METHOD(MongoClient, __get);
PHP_METHOD(MongoClient, selectDB);
PHP_METHOD(MongoClient, selectCollection);
PHP_METHOD(MongoClient, getReadPreference);
PHP_METHOD(MongoClient, setReadPreference);
PHP_METHOD(MongoClient, dropDB);
PHP_METHOD(MongoClient, lastError);
PHP_METHOD(MongoClient, prevError);
PHP_METHOD(MongoClient, resetError);
PHP_METHOD(MongoClient, forceError);
PHP_METHOD(MongoClient, close);
PHP_METHOD(MongoClient, listDBs);
PHP_METHOD(MongoClient, getHosts);

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
