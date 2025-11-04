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

namespace cpp2 metadata.test.services

include "thrift/lib/cpp2/test/metadata/typedef_test.thrift"
include "thrift/lib/cpp2/test/metadata/simple_structs_test.thrift"

exception CutoffException {
  1: string reason;
}

@simple_structs_test.Nat{data = "service"}
service ParentService {
  @simple_structs_test.Nat{data = "function"}
  i32 parentFun();
}

interaction FileAccess {
  void seek(1: i32 delta);
}

interaction LegacyFileAccessConstructor {
  void legacyOpen(1: string path);
  void legacySeek(1: i32 delta);
}

service MyTestService extends ParentService {
  list<typedef_test.Types> getAllTypes();
  typedef_test.Types getType(
    @simple_structs_test.Nat{data = "argument"}
    1: typedef_test.StringMap stringMap,
  ) throws (1: CutoffException ex);
  oneway void noReturn();

  FileAccess open(1: string path);
  performs LegacyFileAccessConstructor;
}
