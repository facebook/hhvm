/**
 *  Copyright 2009-2010 10gen, Inc.
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

#include <stdio.h>
#include <php.h>

#include "php_mongo.h"
#include "test_pool.h"
#include "../util/pool.h"

int test_mongo_util_pool(TSRMLS_D) {
  printf("running pool tests: ");
  test_mongo_util_pool_get_pool(TSRMLS_C);
  printf("\n");
}

int test_mongo_util_pool_get_pool(TSRMLS_D) {
  HashTable *pools = 0;
  zend_rsrc_list_entry *le;

  // check that no connection pools exist
  if (zend_hash_find(&EG(persistent_list), "mongoConnectionPool",
                     sizeof("mongoConnectionPool"), (void**)&le) == SUCCESS) {
    zend_hash_del(&EG(persistent_list), CONNECTION_POOLS, sizeof(CONNECTION_POOLS));
  }
  assert(!zend_hash_exists(&EG(persistent_list), CONNECTION_POOLS, sizeof(CONNECTION_POOLS)));

  // get a connection pool
  pools = mongo_util_pool__get_connection_pools(TSRMLS_C);
  assert(pools != 0);
  assert(zend_hash_exists(&EG(persistent_list), CONNECTION_POOLS, sizeof(CONNECTION_POOLS)));

  // save le->ptr == 0

  // get a connection pool
  // remove any created connection pool
  zend_hash_del(&EG(persistent_list), CONNECTION_POOLS, sizeof(CONNECTION_POOLS));
  assert(!zend_hash_exists(&EG(persistent_list), CONNECTION_POOLS, sizeof(CONNECTION_POOLS)));

  printf(".");
}

int test_mongo_util_pool_init() {
  printf(".");
}

int test_mongo_util_pool_get() {
  printf(".");
}

int test_mongo_util_pool_done() {
  printf(".");
}

int test_mongo_util_pool_failed() {
  printf(".");
}

int test_mongo_util_pool_shutdown() {
  printf(".");
}

int test_mongo_util_pool_stack_pop() {
  printf(".");
}

int test_mongo_util_pool_stack_push() {
  printf(".");
}

int test_mongo_util_pool_close() {
  printf(".");
}

int test_mongo_util_pool_rm() {
  printf(".");
}

int test_mongo_util_pool_get_id() {
  printf(".");
}

int test_mongo_util_pool_connect() {
  printf(".");
}

int test_mongo_util_pool_get_monitor() {
  printf(".");
}

