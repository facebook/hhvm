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

include "thrift/annotation/thrift.thrift"

@thrift.AllowLegacyMissingUris
package;

namespace py3 pickle_thrift

struct easy_pickle {
  3: string name;
  1: i32 val;
}

struct nested_pickle {
  1: easy_pickle c;
}

union nested_pickle_union {
  1: easy_pickle c;
  2: i32 i32_val;
}

exception nested_pickle_exception {
  1: easy_pickle c;
  2: i32 i32_val;
}

struct struct_container {
  1: list<easy_pickle> list_easy;
  # @lint-ignore THRIFTCHECKS
  2: set<easy_pickle> set_easy;
  # @lint-ignore THRIFTCHECKS
  3: map<easy_pickle, easy_pickle> map_easy_easy;
  4: list<list<easy_pickle>> list_list_easy;
  # @lint-ignore THRIFTCHECKS
  5: list<set<easy_pickle>> list_set_easy;
  # @lint-ignore THRIFTCHECKS
  6: list<map<easy_pickle, easy_pickle>> list_map_easy_easy;
  7: list<map<string, easy_pickle>> list_map_str_easy;
  # @lint-ignore THRIFTCHECKS
  8: list<map<easy_pickle, string>> list_map_easy_str;
  # @lint-ignore THRIFTCHECKS
  9: list<map<easy_pickle, list<string>>> list_map_easy_list_str;
}

union union_container {
  1: list<easy_pickle> list_easy;
  # @lint-ignore THRIFTCHECKS
  2: set<easy_pickle> set_easy;
  # @lint-ignore THRIFTCHECKS
  3: map<easy_pickle, easy_pickle> map_easy_easy;
}

exception exception_container {
  1: list<easy_pickle> list_easy;
  # @lint-ignore THRIFTCHECKS
  2: set<easy_pickle> set_easy;
  # @lint-ignore THRIFTCHECKS
  3: map<easy_pickle, easy_pickle> map_easy_easy;
}
