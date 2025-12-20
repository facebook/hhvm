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

include "thrift/lib/py3/test/stream/if/included.thrift"
include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

namespace cpp2 thrift.py3.test
namespace py3 thrift.py3.test

exception FuncEx {}

exception StreamEx {}

service StreamTestService {
  stream<i32> returnstream(1: i32 i32_from, 2: i32 i32_to);
  stream<string> methodNameStream();
  stream<i32 throws (1: StreamEx e)> streamthrows(1: bool t) throws (
    1: FuncEx e,
  );
  stream<string> stringstream();
  included.Included, stream<included.Included> returnresponseandstream(
    1: included.Included foo,
  );
}
