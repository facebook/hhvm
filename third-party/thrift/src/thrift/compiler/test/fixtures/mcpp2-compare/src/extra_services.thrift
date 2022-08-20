/*
 * Copyright (c) Facebook, Inc. and its affiliates.
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

include "module.thrift"

namespace cpp2 extra.svc

struct containerStruct2 {
  1: bool fieldA;
  101: required bool req_fieldA;
  201: optional bool opt_fieldA;
  2: map<string, bool> fieldB;
  102: required map<string, bool> req_fieldB;
  202: optional map<string, bool> opt_fieldB;
  3: set<i32> fieldC = [1, 2, 3, 4];
  103: required set<i32> req_fieldC = [1, 2, 3, 4];
  203: optional set<i32> opt_fieldC = [1, 2, 3, 4];
  4: string fieldD;
  5: string fieldE = "somestring";
  105: required string req_fieldE = "somestring";
  205: optional string opt_fieldE = "somestring";
}

service ExtraService extends module.ParamService {
  bool simple_function();
  void throws_function() throws (
    1: module.AnException ex,
    2: module.AnotherException aex,
  ) (thread = "eb");
  bool throws_function2(1: bool param1) throws (
    1: module.AnException ex,
    2: module.AnotherException aex,
  ) (priority = "HIGH");
  map<i32, string> throws_function3(1: bool param1, 3: string param2) throws (
    2: module.AnException ex,
    5: module.AnotherException aex,
  );
  oneway void oneway_void_ret();
  oneway void oneway_void_ret_i32_i32_i32_i32_i32_param(
    1: i32 param1,
    2: i32 param2,
    3: i32 param3,
    4: i32 param4,
    5: i32 param5,
  );
  oneway void oneway_void_ret_map_setlist_param(
    1: map<string, i64> param1,
    3: set<list<string>> param2,
  ) (thread = "eb");
  oneway void oneway_void_ret_struct_param(1: module.MyStruct param1);
  oneway void oneway_void_ret_listunion_param(
    1: list<module.ComplexUnion> param1,
  );
}
