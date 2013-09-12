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

#ifndef TEST_POOL
#define TEST_POOL

int test_mongo_util_pool();

int test_mongo_util_pool_get_pool();
int test_mongo_util_pool_init();
int test_mongo_util_pool_get();
int test_mongo_util_pool_done();
int test_mongo_util_pool_failed();
int test_mongo_util_pool_shutdown();
int test_mongo_util_pool_stack_pop();
int test_mongo_util_pool_stack_push();
int test_mongo_util_pool_close();
int test_mongo_util_pool_rm();
int test_mongo_util_pool_get_id();
int test_mongo_util_pool_connect();
int test_mongo_util_pool_get_monitor();

#endif
