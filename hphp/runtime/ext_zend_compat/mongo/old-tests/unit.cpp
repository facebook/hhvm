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
#include <sapi/embed/php_embed.h>

#include "unit.h"
#include "lib/test_mongo.h"
#include "lib/test_pool.h"

int main() {
  printf("Running tests...\n");

  PHP_EMBED_START_BLOCK(0, 0);

  test_mongo();
  test_mongo_util_pool(TSRMLS_C);

  PHP_EMBED_END_BLOCK();

  printf("Done.\n");
  return 0;
}

int run_test(int (*t)(void)) {
  t();
  printf(".");
}
