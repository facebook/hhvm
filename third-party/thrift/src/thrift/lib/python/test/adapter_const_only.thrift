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

// Test case: adapter on const from included file (no fields).
package "thrift.com/python/test/adapter_const_only"
namespace py3 python_test

include "thrift/lib/python/test/dependency.thrift"

@dependency.IncludedAdapter{signature = "ConstAdapter"}
const string CONST_WITH_INCLUDED_ADAPTER = "test_value";
