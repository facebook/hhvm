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
#ifndef MONGO_UTIL_POOL_H
#define MONGO_UTIL_POOL_H

void mongo_init_MongoPool(TSRMLS_D);

PHP_METHOD(MongoPool, setSize);
PHP_METHOD(MongoPool, getSize);
PHP_METHOD(MongoPool, info);

PHP_METHOD(Mongo, setPoolSize);
PHP_METHOD(Mongo, getPoolSize);
PHP_METHOD(Mongo, poolDebug);

#endif

/*
 * Local variables:
 * tab-width: 4
 * c-basic-offset: 4
 * End:
 * vim600: fdm=marker
 * vim: noet sw=4 ts=4
 */
