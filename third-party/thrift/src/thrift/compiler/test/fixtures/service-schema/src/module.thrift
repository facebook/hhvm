/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

include "extend.thrift"
include "include.thrift"

package "facebook.com/thrift/test/fixtures/service_schema"

namespace java.swift test.fixtures.service_schema

enum Result {
  OK = 0,
  SO_SO = 1,
  GOOD = 2,
}

exception CustomException {
  1: string name;
  2: Result result = Result.SO_SO;
}

service PrimitivesService {
  i64 init(1: i64 param0, 2: i64 param1);
  Result method_that_throws() throws (1: CustomException e);
  void return_void_method(1: i64 id, 2: include.I i);
}

service ExtendedService extends extend.BaseService {
  i64 init(1: i64 param0, 2: i64 param1);
}
