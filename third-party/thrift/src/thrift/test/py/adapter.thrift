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

namespace py thrift.test.py.adapter

include "thrift/test/py/adapter_bar.thrift"

struct Foo {
  1: adapter_bar.Bar (
    py.adapter = 'thrift.test.py.adapter_for_tests.AdapterTestStructToDict',
  ) structField;
  2: optional adapter_bar.Bar (
    py.adapter = 'thrift.test.py.adapter_for_tests.AdapterTestStructToDict',
  ) oStructField;
  3: map<
    i32,
    adapter_bar.Bar (
      py.adapter = 'thrift.test.py.adapter_for_tests.AdapterTestStructToDict',
    )
  > mapField;
}

struct FooWithoutAdapters {
  1: adapter_bar.Bar structField;
  2: optional adapter_bar.Bar oStructField;
  3: map<i32, adapter_bar.Bar> mapField;
}
